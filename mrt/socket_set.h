#ifndef __MRT_SOCKET_SET_H__
#define __MRT_SOCKET_SET_H__

/* M-runtime for c++
 * Copyright (C) 2005-2008 Vladimir Menshakov
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

#include "export_mrt.h"

namespace mrt {

class Socket;

class MRTAPI SocketSet {
public: 
	static const int Read;
	static const int Write;
	static const int Exception;

	SocketSet();
	void add(const Socket &sock, const int how = Read | Write | Exception);
	void add(const Socket *sock, const int how = Read | Write | Exception);
	void remove(const Socket &sock);
	
	const int check(const unsigned int timeout);
	const bool check(const Socket &sock, const int how);
	const bool check(const Socket *sock, const int how);
	
	void reset();
	
	~SocketSet();
protected: 
	void * _r_set, *_w_set, *_e_set;
	int _n;
private: 
	SocketSet(const SocketSet &);
	const SocketSet & operator=(const SocketSet &);

};
}

#endif

