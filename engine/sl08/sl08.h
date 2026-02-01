#ifndef BTANKS_SL08_SLOTSANDSIGNALS_H__
#define BTANKS_SL08_SLOTSANDSIGNALS_H__

/* sl08 - small slot/signals library
 * Copyright (C) 2007-2008 Netive Media Group
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include <cstdlib>
#include <list>
#include <type_traits>


namespace sl08 {

	class default_validator {
	public:
		template <typename result_type>
		bool operator()(result_type r) const {
			return true;
		}
	};

	class exclusive_validator {
	public:
		bool operator()(bool r) const {
			return !r;
		}
	};



	template <typename Signature>
	class base_signal;

	template <typename Signature>
	class base_slot;

	template <typename R, typename... Args>
	class base_slot<R (Args...)> {
		typedef base_signal<R (Args...)> signal_type; 
		typedef std::list<signal_type *> signals_type;
		signals_type signals;
	public: 
		virtual R operator()(Args... arg) const = 0;
		base_slot() : signals() {}

		void connect(signal_type &signal_ref) {
			signal_type *signal = &signal_ref;
			signals.push_back(signal);
			signal->connect(this); 
		}

		void _disconnect(signal_type *signal) {
			for(typename signals_type::iterator i = signals.begin(); i != signals.end(); ) {
				if (*i == signal) {
					i = signals.erase(i);
				} else ++i;
			}
		}
	
		void disconnect() {
			for(typename signals_type::iterator i = signals.begin(); i != signals.end(); ++i) {
				(*i)->_disconnect(this); 
			}
			signals.clear();
		} 

		virtual ~base_slot() { 
			disconnect();
		}
	};

	template <typename Signature, class object_type>
	class slot;

	template <typename R, typename... Args, class object_type>
	class slot<R (Args...), object_type> : public base_slot <R (Args...)> {
	public: 
		typedef base_signal <R (Args...)> signal_type;
		typedef R (object_type::*func_t) (Args...);

		slot() : object(nullptr), func(nullptr) {}
		slot(object_type *object, func_t func) : object(object), func(func) {}

		void assign(object_type *o, func_t f) { object = o; func = f; }
		void assign(object_type *o, func_t f, signal_type &signal_ref) { object = o; func = f; this->connect(signal_ref); }

		R operator()(Args... args) const override {
			return (object->*func)(args...);
		} 

	private: 
		object_type *object;
		func_t func;
	}; 

	template <typename R, typename... Args>
	class base_signal<R (Args...)> {
	protected: 
		typedef base_slot<R (Args...)> slot_type;
		typedef std::list<slot_type *> slots_type;
		slots_type slots;
	
	public: 
		virtual R emit(Args...) const = 0;

		void connect(slot_type *slot) {
			slots.push_back(slot);
		} 

		void _disconnect(slot_type *slot) {
			for(typename slots_type::iterator i = slots.begin(); i != slots.end(); ) { 
				if (slot != *i)
					++i; 
				else 
					i = slots.erase(i);
			} 
		} 

		void disconnect() {
			for(typename slots_type::iterator i = slots.begin(); i != slots.end(); ++i) {
				(*i)->_disconnect(this);
			}
			slots.clear();
		}
		virtual ~base_signal() {
			disconnect();
		}
	};
	
	template <typename Signature, class validator_type = default_validator>
	class signal;

	template <typename R, typename... Args, class validator_type>
	class signal<R (Args...), validator_type> : public base_signal<R (Args...)> {
	public: 
		typedef base_signal<R (Args...)> base_type;
		typedef std::remove_const_t<R> non_const_return_type;
		
		R emit(Args... args) const override {
			validator_type v;
			auto r = non_const_return_type();
			
			for (auto i = base_type::slots.begin(); i != base_type::slots.end(); ++i) {
				r = (**i)(args...);
				if (!v(r))
					return r;
			}
			return r;
		}
	};

	template <typename... Args, class validator_type>
	class signal<void (Args...), validator_type> : public base_signal<void (Args...)> { 
		typedef base_signal<void (Args...)> base_type; 
	public:
		void emit(Args... args) const override {
			for (auto i = base_type::slots.begin(); i != base_type::slots.end(); ++i) {
				(**i)(args...);
			}
		}
	};
}

#endif

