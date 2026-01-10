/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/


#include "ogg_stream.h"
#include "finder.h"
#include <clunk/sample.h>
#include <assert.h>
#include "mrt/logger.h"
#include "mrt/exception.h"
#include "mrt/base_file.h"
#include "mrt/chunk.h"
#include "config.h"
#include "ogg_ex.h"

static size_t stream_read_func  (void *ptr, size_t size, size_t nmemb, void *datasource) {
	//LOG_DEBUG(("read(%p, %u, %u)", ptr, (unsigned)size, (unsigned)nmemb));
	assert(datasource != NULL);
	mrt::BaseFile *file = (mrt::BaseFile *)datasource;
	TRY { 
		int r = file->read(ptr, nmemb * size);
		if (r <= 0)
			return r;
	
		return r / size;
	} CATCH("read_cb", return -1);
}

static int    stream_seek_func  (void *datasource, ogg_int64_t offset, int whence) {
	//LOG_DEBUG(("seek(%u, %d)", (unsigned)offset, whence));
	assert(datasource != NULL);
	mrt::BaseFile *file = (mrt::BaseFile *)datasource;
	TRY { 
		file->seek(offset, whence);
		return 0;
	} CATCH("seek_cb", return -1);
}

static int    stream_close_func (void *datasource) {
	//LOG_DEBUG(("close()"));
	assert(datasource != NULL);
	mrt::BaseFile *file = (mrt::BaseFile *)datasource;
	TRY { 
		file->close();
		delete file;
		return 0;
	} CATCH("close_cb", return -1);
}

static long   stream_tell_func  (void *datasource) {
	//LOG_DEBUG(("tell"));
	assert(datasource != NULL);
	mrt::BaseFile *file = (mrt::BaseFile *)datasource;
	TRY { 
		return file->tell();
	} CATCH("tell_cb", return -1);
}


OggStream::OggStream(const std::string &fname) {
	_file = Finder->get_file(fname, "rb");

	ov_callbacks ov_cb = {};

	ov_cb.read_func = stream_read_func;
	ov_cb.seek_func = stream_seek_func;
	ov_cb.tell_func = stream_tell_func;
	ov_cb.close_func = stream_close_func;

	int r = ov_open_callbacks(_file.release(), &_ogg_stream, NULL, 0, ov_cb);
	if (r < 0)
		throw_ogg(r, ("ov_open('%s')", fname.c_str()));
	
	_vorbis_info = ov_info(&_ogg_stream, -1);

	_spec.sample_rate = _vorbis_info->rate;
	//LOG_DEBUG(("open(%s) : %d", fname.c_str(), sample_rate));
	_spec.format = clunk::AudioSpec::S16;
	_spec.channels = _vorbis_info->channels;

	//_vorbis_comment = ov_comment(&_ogg_stream, -1);
	assert(_vorbis_info != NULL);
}

void OggStream::rewind() {
	LOG_DEBUG(("rewinding stream..."));
	int r = ov_raw_seek(&_ogg_stream, 0);
	if (r != 0)
		throw_ogg(r, ("ov_raw_seek"));
}

bool OggStream::read(clunk::Buffer &data, unsigned hint) {
	if (hint == 0) 
		hint = 44100;
	
	data.set_size(hint);

	int section = 0;
	int r = ov_read(&_ogg_stream, (char *)data.get_ptr(), hint, 0, 2, 1, & section);
	//LOG_DEBUG(("ov_read(%d) = %d (section: %d)", hint, r, section));
	
	if(r >= 0) {
		data.set_size(r);
		
		return r != 0;
	}

	throw_ogg(r, ("ov_read"));
	return false; //:(
}

OggStream::~OggStream() {
	ov_clear(&_ogg_stream);
}

bool OggStream::is_seekable() {
	return ov_seekable(&_ogg_stream) != 0;
}

int64_t OggStream::get_pcm_total() {
	return ov_pcm_total(&_ogg_stream, -1);
}

vorbis_info const *OggStream::get_info() const {
	return _vorbis_info;
}

void OggStream::decode(clunk::Sample &sample, const std::string &fname) {
	OggStream ogg(fname);

	vorbis_info const *info = ogg.get_info();
	assert(info != NULL);

	// At first glance the only reason when file
	// can be non-seekable is when we didn't pass
	// .seek_func callback or when it returned
	// an error.
	//
	// Maybe this if can be replaced with an
	// assert, but  I don't want to rely on
	// implementation detail of libvorbis.
	if (!ogg.is_seekable())
		throw_ex(("ogg file '%s' is not seekable", fname.c_str()));

	// From looking at the source code of libvorbis
	// the only reason this can fail is when the
	// file is non-seekable.
	//
	// Maybe this if can be replaced with an
	// assert, but I don't want to rely on
	// implementation detail of libvorbis.
	int64_t samples_total = ogg.get_pcm_total();
	if (samples_total < 0)
		throw_ogg(samples_total, ("ov_pcm_total('%s')", fname.c_str()));

	clunk::Buffer data;

	data.set_size(samples_total * info->channels * sizeof(int16_t));

	size_t pos = 0;
	for (;;) {
		assert(pos <= data.get_size());
		if (pos == data.get_size())
			break;

		int section = 0;
		long r = ov_read(&ogg._ogg_stream, (char *)data.get_ptr() + pos, data.get_size() - pos, 0, 2, 1, & section);
		if (r < 0)
			throw_ogg(r, ("ov_read('%s')", fname.c_str()));

		if (r == 0)
			break;

		pos += r;
	}

	if (pos != data.get_size())
		throw_ex(("incomplete ov_read('%s'), expected %zu, actual %zu", fname.c_str(), data.get_size(), pos));

	sample.init(data, clunk::AudioSpec(clunk::AudioSpec::S16, info->rate, info->channels));
}
