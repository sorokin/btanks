#ifndef BTANKS_SL08_SLOTSANDSIGNALS_H__
#define BTANKS_SL08_SLOTSANDSIGNALS_H__

#include <deque>

#ifndef NULL
#define NULL            ((void*) 0)
#endif

namespace sl08 {

		template <typename T> struct deconst { typedef T type; };
		template <typename T> struct deconst<const T> { typedef T type; };

		template <typename result_type>
		class default_validator {
		public:
			inline bool operator()(result_type r) {
				return true;
			}
		};

		template <typename result_type>
		class exclusive_validator {
		public:
			inline bool operator()(result_type r) {
				return !r;
			}
		};

	

		template <typename return_type> class base_signal0;

		template <typename return_type> 
		class base_slot0 {
			typedef base_signal0 <return_type> signal_type; 
			typedef std::deque<signal_type *> signals_type;
			signals_type signals;
		public: 
			virtual return_type operator() () = 0;
			inline base_slot0 () : signals() {} 

			inline void connect(signal_type &signal_ref) {
				signal_type *signal = &signal_ref;
				signals.push_back(signal);
				signal->connect(this); 
			}

			inline void _disconnect(signal_type *signal) {
				for(typename signals_type::iterator i = signals.begin(); i != signals.end(); ) {
					if (*i == signal) {
						i = signals.erase(i);
					} else ++i;
				}
			}
		
			inline void disconnect() {
				for(typename signals_type::iterator i = signals.begin(); i != signals.end(); ++i) {
					(*i)->_disconnect(this); 
				}
				signals.clear();
			} 
			inline virtual ~base_slot0() { 
				disconnect();
			}
		};

		template <typename return_type, class object_type>
		class slot0 : public base_slot0 <return_type> { 
		public: 
			typedef base_signal0 <return_type> signal_type; 
			typedef return_type (object_type::*func_t) (); 

			inline slot0 () : object(NULL), func(NULL) {}
			inline slot0(object_type *object, func_t func, signal_type * signal = NULL) : object(object), func(func) {}

			inline void assign(object_type *o, func_t f) { object = o; func = f; }
			inline void assign(object_type *o, func_t f, signal_type &signal_ref = NULL) { object = o; func = f; connect(signal_ref); }
	
			inline return_type operator() () { 
				return (object->*func) () ;
			} 
	
		private: 
			object_type *object; 
			func_t func;
		}; 

		
		template <class object_type>
		class slot0 <void, object_type> : public base_slot0 <void>{
		public: 
			typedef base_signal0 <void> signal_type; 
			typedef void (object_type::*func_t) () ;

			inline slot0 () : object(NULL), func(NULL) {}
			inline slot0 (object_type *object, func_t func) : object(object), func(func) {}
			inline void assign(object_type *o, func_t f) { object = o; func = f; }
			inline void assign(object_type *o, func_t f, signal_type &signal_ref) { object = o; func = f; connect(signal_ref); }
	
			inline void operator() () { 
				(object->*func) (); 
			} 
	
		private: 
			object_type *object; 
			func_t func; 
		};
		
		template <typename return_type>
		class base_signal0 {
		protected: 
			typedef base_slot0 <return_type> slot_type; 
			typedef std::deque<slot_type *> slots_type;
			slots_type slots;
		
		public: 
			inline virtual return_type emit () = 0;

			inline void connect(slot_type *slot) {
				slots.push_back(slot);
			} 

			inline void _disconnect(slot_type *slot) {
				for(typename slots_type::iterator i = slots.begin(); i != slots.end(); ) { 
					if (slot != *i) 
						++i; 
					else 
						i = slots.erase(i); 
				} 
			} 

			inline void disconnect() {
				for(typename slots_type::iterator i = slots.begin(); i != slots.end(); ) { 
					(*i)->_disconnect(this);
				} 
			}
			inline virtual ~base_signal0() {
				disconnect();
			}
		};
		
		template <typename return_type, class validator_type = default_validator<return_type> >
		class signal0 : public base_signal0 <return_type> { 
		public: 
			typedef base_signal0 <return_type> parent_type; 
			typedef typename deconst<return_type>::type non_const_return_type;
			
			inline virtual return_type emit () {
				validator_type v;
				non_const_return_type r;
				
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					r = (*i)->operator() ();
					if (!v(r))
						return r;
				}
				return r; 
			} 
		};
			
		template <class validator_type >
		class signal0 <void, validator_type> : public base_signal0 <void> { 
		typedef base_signal0 <void> parent_type; 
		public: 
			inline void emit () {  
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					(*i)->operator() () ; 
				} 
			} 
		};
		template <class validator_type >
		class signal0 <int, validator_type> : public base_signal0 <int> { 
			typedef base_signal0 <int> parent_type; 
		public: 
			inline int emit () {  
				validator_type v;
				int r = (int)0;
				
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					r = (*i)->operator() ();
					if (!v(r))
						return r;
				}
				return r;
			}
		};
		template <class validator_type >
		class signal0 <bool, validator_type> : public base_signal0 <bool> { 
			typedef base_signal0 <bool> parent_type; 
		public: 
			inline bool emit () {  
				validator_type v;
				bool r = (bool)0;
				
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					r = (*i)->operator() ();
					if (!v(r))
						return r;
				}
				return r;
			}
		};
		

		template <typename return_type, typename arg1_type> class base_signal1;

		template <typename return_type, typename arg1_type> 
		class base_slot1 {
			typedef base_signal1 <return_type, arg1_type> signal_type; 
			typedef std::deque<signal_type *> signals_type;
			signals_type signals;
		public: 
			virtual return_type operator() (arg1_type a1) = 0;
			inline base_slot1 () : signals() {} 

			inline void connect(signal_type &signal_ref) {
				signal_type *signal = &signal_ref;
				signals.push_back(signal);
				signal->connect(this); 
			}

			inline void _disconnect(signal_type *signal) {
				for(typename signals_type::iterator i = signals.begin(); i != signals.end(); ) {
					if (*i == signal) {
						i = signals.erase(i);
					} else ++i;
				}
			}
		
			inline void disconnect() {
				for(typename signals_type::iterator i = signals.begin(); i != signals.end(); ++i) {
					(*i)->_disconnect(this); 
				}
				signals.clear();
			} 
			inline virtual ~base_slot1() { 
				disconnect();
			}
		};

		template <typename return_type, typename arg1_type, class object_type>
		class slot1 : public base_slot1 <return_type, arg1_type> { 
		public: 
			typedef base_signal1 <return_type, arg1_type> signal_type; 
			typedef return_type (object_type::*func_t) (arg1_type a1); 

			inline slot1 () : object(NULL), func(NULL) {}
			inline slot1(object_type *object, func_t func, signal_type * signal = NULL) : object(object), func(func) {}

			inline void assign(object_type *o, func_t f) { object = o; func = f; }
			inline void assign(object_type *o, func_t f, signal_type &signal_ref = NULL) { object = o; func = f; connect(signal_ref); }
	
			inline return_type operator() (arg1_type a1) { 
				return (object->*func) (a1) ;
			} 
	
		private: 
			object_type *object; 
			func_t func;
		}; 

		
		template <typename arg1_type, class object_type>
		class slot1 <void, arg1_type, object_type> : public base_slot1 <void, arg1_type>{
		public: 
			typedef base_signal1 <void, arg1_type> signal_type; 
			typedef void (object_type::*func_t) (arg1_type a1) ;

			inline slot1 () : object(NULL), func(NULL) {}
			inline slot1 (object_type *object, func_t func) : object(object), func(func) {}
			inline void assign(object_type *o, func_t f) { object = o; func = f; }
			inline void assign(object_type *o, func_t f, signal_type &signal_ref) { object = o; func = f; connect(signal_ref); }
	
			inline void operator() (arg1_type a1) { 
				(object->*func) (a1); 
			} 
	
		private: 
			object_type *object; 
			func_t func; 
		};
		
		template <typename return_type, typename arg1_type>
		class base_signal1 {
		protected: 
			typedef base_slot1 <return_type, arg1_type> slot_type; 
			typedef std::deque<slot_type *> slots_type;
			slots_type slots;
		
		public: 
			inline virtual return_type emit (arg1_type a1) = 0;

			inline void connect(slot_type *slot) {
				slots.push_back(slot);
			} 

			inline void _disconnect(slot_type *slot) {
				for(typename slots_type::iterator i = slots.begin(); i != slots.end(); ) { 
					if (slot != *i) 
						++i; 
					else 
						i = slots.erase(i); 
				} 
			} 

			inline void disconnect() {
				for(typename slots_type::iterator i = slots.begin(); i != slots.end(); ) { 
					(*i)->_disconnect(this);
				} 
			}
			inline virtual ~base_signal1() {
				disconnect();
			}
		};
		
		template <typename return_type, typename arg1_type, class validator_type = default_validator<return_type> >
		class signal1 : public base_signal1 <return_type, arg1_type> { 
		public: 
			typedef base_signal1 <return_type, arg1_type> parent_type; 
			typedef typename deconst<return_type>::type non_const_return_type;
			
			inline virtual return_type emit (arg1_type a1) {
				validator_type v;
				non_const_return_type r;
				
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					r = (*i)->operator() (a1);
					if (!v(r))
						return r;
				}
				return r; 
			} 
		};
			
		template <typename arg1_type, class validator_type >
		class signal1 <void, arg1_type, validator_type> : public base_signal1 <void, arg1_type> { 
		typedef base_signal1 <void, arg1_type> parent_type; 
		public: 
			inline void emit (arg1_type a1) {  
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					(*i)->operator() (a1) ; 
				} 
			} 
		};
		template <typename arg1_type, class validator_type >
		class signal1 <int, arg1_type, validator_type> : public base_signal1 <int, arg1_type> { 
			typedef base_signal1 <int, arg1_type> parent_type; 
		public: 
			inline int emit (arg1_type a1) {  
				validator_type v;
				int r = (int)0;
				
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					r = (*i)->operator() (a1);
					if (!v(r))
						return r;
				}
				return r;
			}
		};
		template <typename arg1_type, class validator_type >
		class signal1 <bool, arg1_type, validator_type> : public base_signal1 <bool, arg1_type> { 
			typedef base_signal1 <bool, arg1_type> parent_type; 
		public: 
			inline bool emit (arg1_type a1) {  
				validator_type v;
				bool r = (bool)0;
				
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					r = (*i)->operator() (a1);
					if (!v(r))
						return r;
				}
				return r;
			}
		};
		

		template <typename return_type, typename arg1_type, typename arg2_type> class base_signal2;

		template <typename return_type, typename arg1_type, typename arg2_type> 
		class base_slot2 {
			typedef base_signal2 <return_type, arg1_type, arg2_type> signal_type; 
			typedef std::deque<signal_type *> signals_type;
			signals_type signals;
		public: 
			virtual return_type operator() (arg1_type a1, arg2_type a2) = 0;
			inline base_slot2 () : signals() {} 

			inline void connect(signal_type &signal_ref) {
				signal_type *signal = &signal_ref;
				signals.push_back(signal);
				signal->connect(this); 
			}

			inline void _disconnect(signal_type *signal) {
				for(typename signals_type::iterator i = signals.begin(); i != signals.end(); ) {
					if (*i == signal) {
						i = signals.erase(i);
					} else ++i;
				}
			}
		
			inline void disconnect() {
				for(typename signals_type::iterator i = signals.begin(); i != signals.end(); ++i) {
					(*i)->_disconnect(this); 
				}
				signals.clear();
			} 
			inline virtual ~base_slot2() { 
				disconnect();
			}
		};

		template <typename return_type, typename arg1_type, typename arg2_type, class object_type>
		class slot2 : public base_slot2 <return_type, arg1_type, arg2_type> { 
		public: 
			typedef base_signal2 <return_type, arg1_type, arg2_type> signal_type; 
			typedef return_type (object_type::*func_t) (arg1_type a1, arg2_type a2); 

			inline slot2 () : object(NULL), func(NULL) {}
			inline slot2(object_type *object, func_t func, signal_type * signal = NULL) : object(object), func(func) {}

			inline void assign(object_type *o, func_t f) { object = o; func = f; }
			inline void assign(object_type *o, func_t f, signal_type &signal_ref = NULL) { object = o; func = f; connect(signal_ref); }
	
			inline return_type operator() (arg1_type a1, arg2_type a2) { 
				return (object->*func) (a1, a2) ;
			} 
	
		private: 
			object_type *object; 
			func_t func;
		}; 

		
		template <typename arg1_type, typename arg2_type, class object_type>
		class slot2 <void, arg1_type, arg2_type, object_type> : public base_slot2 <void, arg1_type, arg2_type>{
		public: 
			typedef base_signal2 <void, arg1_type, arg2_type> signal_type; 
			typedef void (object_type::*func_t) (arg1_type a1, arg2_type a2) ;

			inline slot2 () : object(NULL), func(NULL) {}
			inline slot2 (object_type *object, func_t func) : object(object), func(func) {}
			inline void assign(object_type *o, func_t f) { object = o; func = f; }
			inline void assign(object_type *o, func_t f, signal_type &signal_ref) { object = o; func = f; connect(signal_ref); }
	
			inline void operator() (arg1_type a1, arg2_type a2) { 
				(object->*func) (a1, a2); 
			} 
	
		private: 
			object_type *object; 
			func_t func; 
		};
		
		template <typename return_type, typename arg1_type, typename arg2_type>
		class base_signal2 {
		protected: 
			typedef base_slot2 <return_type, arg1_type, arg2_type> slot_type; 
			typedef std::deque<slot_type *> slots_type;
			slots_type slots;
		
		public: 
			inline virtual return_type emit (arg1_type a1, arg2_type a2) = 0;

			inline void connect(slot_type *slot) {
				slots.push_back(slot);
			} 

			inline void _disconnect(slot_type *slot) {
				for(typename slots_type::iterator i = slots.begin(); i != slots.end(); ) { 
					if (slot != *i) 
						++i; 
					else 
						i = slots.erase(i); 
				} 
			} 

			inline void disconnect() {
				for(typename slots_type::iterator i = slots.begin(); i != slots.end(); ) { 
					(*i)->_disconnect(this);
				} 
			}
			inline virtual ~base_signal2() {
				disconnect();
			}
		};
		
		template <typename return_type, typename arg1_type, typename arg2_type, class validator_type = default_validator<return_type> >
		class signal2 : public base_signal2 <return_type, arg1_type, arg2_type> { 
		public: 
			typedef base_signal2 <return_type, arg1_type, arg2_type> parent_type; 
			typedef typename deconst<return_type>::type non_const_return_type;
			
			inline virtual return_type emit (arg1_type a1, arg2_type a2) {
				validator_type v;
				non_const_return_type r;
				
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					r = (*i)->operator() (a1, a2);
					if (!v(r))
						return r;
				}
				return r; 
			} 
		};
			
		template <typename arg1_type, typename arg2_type, class validator_type >
		class signal2 <void, arg1_type, arg2_type, validator_type> : public base_signal2 <void, arg1_type, arg2_type> { 
		typedef base_signal2 <void, arg1_type, arg2_type> parent_type; 
		public: 
			inline void emit (arg1_type a1, arg2_type a2) {  
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					(*i)->operator() (a1, a2) ; 
				} 
			} 
		};
		template <typename arg1_type, typename arg2_type, class validator_type >
		class signal2 <int, arg1_type, arg2_type, validator_type> : public base_signal2 <int, arg1_type, arg2_type> { 
			typedef base_signal2 <int, arg1_type, arg2_type> parent_type; 
		public: 
			inline int emit (arg1_type a1, arg2_type a2) {  
				validator_type v;
				int r = (int)0;
				
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					r = (*i)->operator() (a1, a2);
					if (!v(r))
						return r;
				}
				return r;
			}
		};
		template <typename arg1_type, typename arg2_type, class validator_type >
		class signal2 <bool, arg1_type, arg2_type, validator_type> : public base_signal2 <bool, arg1_type, arg2_type> { 
			typedef base_signal2 <bool, arg1_type, arg2_type> parent_type; 
		public: 
			inline bool emit (arg1_type a1, arg2_type a2) {  
				validator_type v;
				bool r = (bool)0;
				
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					r = (*i)->operator() (a1, a2);
					if (!v(r))
						return r;
				}
				return r;
			}
		};
		

		template <typename return_type, typename arg1_type, typename arg2_type, typename arg3_type> class base_signal3;

		template <typename return_type, typename arg1_type, typename arg2_type, typename arg3_type> 
		class base_slot3 {
			typedef base_signal3 <return_type, arg1_type, arg2_type, arg3_type> signal_type; 
			typedef std::deque<signal_type *> signals_type;
			signals_type signals;
		public: 
			virtual return_type operator() (arg1_type a1, arg2_type a2, arg3_type a3) = 0;
			inline base_slot3 () : signals() {} 

			inline void connect(signal_type &signal_ref) {
				signal_type *signal = &signal_ref;
				signals.push_back(signal);
				signal->connect(this); 
			}

			inline void _disconnect(signal_type *signal) {
				for(typename signals_type::iterator i = signals.begin(); i != signals.end(); ) {
					if (*i == signal) {
						i = signals.erase(i);
					} else ++i;
				}
			}
		
			inline void disconnect() {
				for(typename signals_type::iterator i = signals.begin(); i != signals.end(); ++i) {
					(*i)->_disconnect(this); 
				}
				signals.clear();
			} 
			inline virtual ~base_slot3() { 
				disconnect();
			}
		};

		template <typename return_type, typename arg1_type, typename arg2_type, typename arg3_type, class object_type>
		class slot3 : public base_slot3 <return_type, arg1_type, arg2_type, arg3_type> { 
		public: 
			typedef base_signal3 <return_type, arg1_type, arg2_type, arg3_type> signal_type; 
			typedef return_type (object_type::*func_t) (arg1_type a1, arg2_type a2, arg3_type a3); 

			inline slot3 () : object(NULL), func(NULL) {}
			inline slot3(object_type *object, func_t func, signal_type * signal = NULL) : object(object), func(func) {}

			inline void assign(object_type *o, func_t f) { object = o; func = f; }
			inline void assign(object_type *o, func_t f, signal_type &signal_ref = NULL) { object = o; func = f; connect(signal_ref); }
	
			inline return_type operator() (arg1_type a1, arg2_type a2, arg3_type a3) { 
				return (object->*func) (a1, a2, a3) ;
			} 
	
		private: 
			object_type *object; 
			func_t func;
		}; 

		
		template <typename arg1_type, typename arg2_type, typename arg3_type, class object_type>
		class slot3 <void, arg1_type, arg2_type, arg3_type, object_type> : public base_slot3 <void, arg1_type, arg2_type, arg3_type>{
		public: 
			typedef base_signal3 <void, arg1_type, arg2_type, arg3_type> signal_type; 
			typedef void (object_type::*func_t) (arg1_type a1, arg2_type a2, arg3_type a3) ;

			inline slot3 () : object(NULL), func(NULL) {}
			inline slot3 (object_type *object, func_t func) : object(object), func(func) {}
			inline void assign(object_type *o, func_t f) { object = o; func = f; }
			inline void assign(object_type *o, func_t f, signal_type &signal_ref) { object = o; func = f; connect(signal_ref); }
	
			inline void operator() (arg1_type a1, arg2_type a2, arg3_type a3) { 
				(object->*func) (a1, a2, a3); 
			} 
	
		private: 
			object_type *object; 
			func_t func; 
		};
		
		template <typename return_type, typename arg1_type, typename arg2_type, typename arg3_type>
		class base_signal3 {
		protected: 
			typedef base_slot3 <return_type, arg1_type, arg2_type, arg3_type> slot_type; 
			typedef std::deque<slot_type *> slots_type;
			slots_type slots;
		
		public: 
			inline virtual return_type emit (arg1_type a1, arg2_type a2, arg3_type a3) = 0;

			inline void connect(slot_type *slot) {
				slots.push_back(slot);
			} 

			inline void _disconnect(slot_type *slot) {
				for(typename slots_type::iterator i = slots.begin(); i != slots.end(); ) { 
					if (slot != *i) 
						++i; 
					else 
						i = slots.erase(i); 
				} 
			} 

			inline void disconnect() {
				for(typename slots_type::iterator i = slots.begin(); i != slots.end(); ) { 
					(*i)->_disconnect(this);
				} 
			}
			inline virtual ~base_signal3() {
				disconnect();
			}
		};
		
		template <typename return_type, typename arg1_type, typename arg2_type, typename arg3_type, class validator_type = default_validator<return_type> >
		class signal3 : public base_signal3 <return_type, arg1_type, arg2_type, arg3_type> { 
		public: 
			typedef base_signal3 <return_type, arg1_type, arg2_type, arg3_type> parent_type; 
			typedef typename deconst<return_type>::type non_const_return_type;
			
			inline virtual return_type emit (arg1_type a1, arg2_type a2, arg3_type a3) {
				validator_type v;
				non_const_return_type r;
				
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					r = (*i)->operator() (a1, a2, a3);
					if (!v(r))
						return r;
				}
				return r; 
			} 
		};
			
		template <typename arg1_type, typename arg2_type, typename arg3_type, class validator_type >
		class signal3 <void, arg1_type, arg2_type, arg3_type, validator_type> : public base_signal3 <void, arg1_type, arg2_type, arg3_type> { 
		typedef base_signal3 <void, arg1_type, arg2_type, arg3_type> parent_type; 
		public: 
			inline void emit (arg1_type a1, arg2_type a2, arg3_type a3) {  
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					(*i)->operator() (a1, a2, a3) ; 
				} 
			} 
		};
		template <typename arg1_type, typename arg2_type, typename arg3_type, class validator_type >
		class signal3 <int, arg1_type, arg2_type, arg3_type, validator_type> : public base_signal3 <int, arg1_type, arg2_type, arg3_type> { 
			typedef base_signal3 <int, arg1_type, arg2_type, arg3_type> parent_type; 
		public: 
			inline int emit (arg1_type a1, arg2_type a2, arg3_type a3) {  
				validator_type v;
				int r = (int)0;
				
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					r = (*i)->operator() (a1, a2, a3);
					if (!v(r))
						return r;
				}
				return r;
			}
		};
		template <typename arg1_type, typename arg2_type, typename arg3_type, class validator_type >
		class signal3 <bool, arg1_type, arg2_type, arg3_type, validator_type> : public base_signal3 <bool, arg1_type, arg2_type, arg3_type> { 
			typedef base_signal3 <bool, arg1_type, arg2_type, arg3_type> parent_type; 
		public: 
			inline bool emit (arg1_type a1, arg2_type a2, arg3_type a3) {  
				validator_type v;
				bool r = (bool)0;
				
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					r = (*i)->operator() (a1, a2, a3);
					if (!v(r))
						return r;
				}
				return r;
			}
		};
		

		template <typename return_type, typename arg1_type, typename arg2_type, typename arg3_type, typename arg4_type> class base_signal4;

		template <typename return_type, typename arg1_type, typename arg2_type, typename arg3_type, typename arg4_type> 
		class base_slot4 {
			typedef base_signal4 <return_type, arg1_type, arg2_type, arg3_type, arg4_type> signal_type; 
			typedef std::deque<signal_type *> signals_type;
			signals_type signals;
		public: 
			virtual return_type operator() (arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4) = 0;
			inline base_slot4 () : signals() {} 

			inline void connect(signal_type &signal_ref) {
				signal_type *signal = &signal_ref;
				signals.push_back(signal);
				signal->connect(this); 
			}

			inline void _disconnect(signal_type *signal) {
				for(typename signals_type::iterator i = signals.begin(); i != signals.end(); ) {
					if (*i == signal) {
						i = signals.erase(i);
					} else ++i;
				}
			}
		
			inline void disconnect() {
				for(typename signals_type::iterator i = signals.begin(); i != signals.end(); ++i) {
					(*i)->_disconnect(this); 
				}
				signals.clear();
			} 
			inline virtual ~base_slot4() { 
				disconnect();
			}
		};

		template <typename return_type, typename arg1_type, typename arg2_type, typename arg3_type, typename arg4_type, class object_type>
		class slot4 : public base_slot4 <return_type, arg1_type, arg2_type, arg3_type, arg4_type> { 
		public: 
			typedef base_signal4 <return_type, arg1_type, arg2_type, arg3_type, arg4_type> signal_type; 
			typedef return_type (object_type::*func_t) (arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4); 

			inline slot4 () : object(NULL), func(NULL) {}
			inline slot4(object_type *object, func_t func, signal_type * signal = NULL) : object(object), func(func) {}

			inline void assign(object_type *o, func_t f) { object = o; func = f; }
			inline void assign(object_type *o, func_t f, signal_type &signal_ref = NULL) { object = o; func = f; connect(signal_ref); }
	
			inline return_type operator() (arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4) { 
				return (object->*func) (a1, a2, a3, a4) ;
			} 
	
		private: 
			object_type *object; 
			func_t func;
		}; 

		
		template <typename arg1_type, typename arg2_type, typename arg3_type, typename arg4_type, class object_type>
		class slot4 <void, arg1_type, arg2_type, arg3_type, arg4_type, object_type> : public base_slot4 <void, arg1_type, arg2_type, arg3_type, arg4_type>{
		public: 
			typedef base_signal4 <void, arg1_type, arg2_type, arg3_type, arg4_type> signal_type; 
			typedef void (object_type::*func_t) (arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4) ;

			inline slot4 () : object(NULL), func(NULL) {}
			inline slot4 (object_type *object, func_t func) : object(object), func(func) {}
			inline void assign(object_type *o, func_t f) { object = o; func = f; }
			inline void assign(object_type *o, func_t f, signal_type &signal_ref) { object = o; func = f; connect(signal_ref); }
	
			inline void operator() (arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4) { 
				(object->*func) (a1, a2, a3, a4); 
			} 
	
		private: 
			object_type *object; 
			func_t func; 
		};
		
		template <typename return_type, typename arg1_type, typename arg2_type, typename arg3_type, typename arg4_type>
		class base_signal4 {
		protected: 
			typedef base_slot4 <return_type, arg1_type, arg2_type, arg3_type, arg4_type> slot_type; 
			typedef std::deque<slot_type *> slots_type;
			slots_type slots;
		
		public: 
			inline virtual return_type emit (arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4) = 0;

			inline void connect(slot_type *slot) {
				slots.push_back(slot);
			} 

			inline void _disconnect(slot_type *slot) {
				for(typename slots_type::iterator i = slots.begin(); i != slots.end(); ) { 
					if (slot != *i) 
						++i; 
					else 
						i = slots.erase(i); 
				} 
			} 

			inline void disconnect() {
				for(typename slots_type::iterator i = slots.begin(); i != slots.end(); ) { 
					(*i)->_disconnect(this);
				} 
			}
			inline virtual ~base_signal4() {
				disconnect();
			}
		};
		
		template <typename return_type, typename arg1_type, typename arg2_type, typename arg3_type, typename arg4_type, class validator_type = default_validator<return_type> >
		class signal4 : public base_signal4 <return_type, arg1_type, arg2_type, arg3_type, arg4_type> { 
		public: 
			typedef base_signal4 <return_type, arg1_type, arg2_type, arg3_type, arg4_type> parent_type; 
			typedef typename deconst<return_type>::type non_const_return_type;
			
			inline virtual return_type emit (arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4) {
				validator_type v;
				non_const_return_type r;
				
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					r = (*i)->operator() (a1, a2, a3, a4);
					if (!v(r))
						return r;
				}
				return r; 
			} 
		};
			
		template <typename arg1_type, typename arg2_type, typename arg3_type, typename arg4_type, class validator_type >
		class signal4 <void, arg1_type, arg2_type, arg3_type, arg4_type, validator_type> : public base_signal4 <void, arg1_type, arg2_type, arg3_type, arg4_type> { 
		typedef base_signal4 <void, arg1_type, arg2_type, arg3_type, arg4_type> parent_type; 
		public: 
			inline void emit (arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4) {  
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					(*i)->operator() (a1, a2, a3, a4) ; 
				} 
			} 
		};
		template <typename arg1_type, typename arg2_type, typename arg3_type, typename arg4_type, class validator_type >
		class signal4 <int, arg1_type, arg2_type, arg3_type, arg4_type, validator_type> : public base_signal4 <int, arg1_type, arg2_type, arg3_type, arg4_type> { 
			typedef base_signal4 <int, arg1_type, arg2_type, arg3_type, arg4_type> parent_type; 
		public: 
			inline int emit (arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4) {  
				validator_type v;
				int r = (int)0;
				
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					r = (*i)->operator() (a1, a2, a3, a4);
					if (!v(r))
						return r;
				}
				return r;
			}
		};
		template <typename arg1_type, typename arg2_type, typename arg3_type, typename arg4_type, class validator_type >
		class signal4 <bool, arg1_type, arg2_type, arg3_type, arg4_type, validator_type> : public base_signal4 <bool, arg1_type, arg2_type, arg3_type, arg4_type> { 
			typedef base_signal4 <bool, arg1_type, arg2_type, arg3_type, arg4_type> parent_type; 
		public: 
			inline bool emit (arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4) {  
				validator_type v;
				bool r = (bool)0;
				
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					r = (*i)->operator() (a1, a2, a3, a4);
					if (!v(r))
						return r;
				}
				return r;
			}
		};
		

		template <typename return_type, typename arg1_type, typename arg2_type, typename arg3_type, typename arg4_type, typename arg5_type> class base_signal5;

		template <typename return_type, typename arg1_type, typename arg2_type, typename arg3_type, typename arg4_type, typename arg5_type> 
		class base_slot5 {
			typedef base_signal5 <return_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type> signal_type; 
			typedef std::deque<signal_type *> signals_type;
			signals_type signals;
		public: 
			virtual return_type operator() (arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5) = 0;
			inline base_slot5 () : signals() {} 

			inline void connect(signal_type &signal_ref) {
				signal_type *signal = &signal_ref;
				signals.push_back(signal);
				signal->connect(this); 
			}

			inline void _disconnect(signal_type *signal) {
				for(typename signals_type::iterator i = signals.begin(); i != signals.end(); ) {
					if (*i == signal) {
						i = signals.erase(i);
					} else ++i;
				}
			}
		
			inline void disconnect() {
				for(typename signals_type::iterator i = signals.begin(); i != signals.end(); ++i) {
					(*i)->_disconnect(this); 
				}
				signals.clear();
			} 
			inline virtual ~base_slot5() { 
				disconnect();
			}
		};

		template <typename return_type, typename arg1_type, typename arg2_type, typename arg3_type, typename arg4_type, typename arg5_type, class object_type>
		class slot5 : public base_slot5 <return_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type> { 
		public: 
			typedef base_signal5 <return_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type> signal_type; 
			typedef return_type (object_type::*func_t) (arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5); 

			inline slot5 () : object(NULL), func(NULL) {}
			inline slot5(object_type *object, func_t func, signal_type * signal = NULL) : object(object), func(func) {}

			inline void assign(object_type *o, func_t f) { object = o; func = f; }
			inline void assign(object_type *o, func_t f, signal_type &signal_ref = NULL) { object = o; func = f; connect(signal_ref); }
	
			inline return_type operator() (arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5) { 
				return (object->*func) (a1, a2, a3, a4, a5) ;
			} 
	
		private: 
			object_type *object; 
			func_t func;
		}; 

		
		template <typename arg1_type, typename arg2_type, typename arg3_type, typename arg4_type, typename arg5_type, class object_type>
		class slot5 <void, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, object_type> : public base_slot5 <void, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type>{
		public: 
			typedef base_signal5 <void, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type> signal_type; 
			typedef void (object_type::*func_t) (arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5) ;

			inline slot5 () : object(NULL), func(NULL) {}
			inline slot5 (object_type *object, func_t func) : object(object), func(func) {}
			inline void assign(object_type *o, func_t f) { object = o; func = f; }
			inline void assign(object_type *o, func_t f, signal_type &signal_ref) { object = o; func = f; connect(signal_ref); }
	
			inline void operator() (arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5) { 
				(object->*func) (a1, a2, a3, a4, a5); 
			} 
	
		private: 
			object_type *object; 
			func_t func; 
		};
		
		template <typename return_type, typename arg1_type, typename arg2_type, typename arg3_type, typename arg4_type, typename arg5_type>
		class base_signal5 {
		protected: 
			typedef base_slot5 <return_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type> slot_type; 
			typedef std::deque<slot_type *> slots_type;
			slots_type slots;
		
		public: 
			inline virtual return_type emit (arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5) = 0;

			inline void connect(slot_type *slot) {
				slots.push_back(slot);
			} 

			inline void _disconnect(slot_type *slot) {
				for(typename slots_type::iterator i = slots.begin(); i != slots.end(); ) { 
					if (slot != *i) 
						++i; 
					else 
						i = slots.erase(i); 
				} 
			} 

			inline void disconnect() {
				for(typename slots_type::iterator i = slots.begin(); i != slots.end(); ) { 
					(*i)->_disconnect(this);
				} 
			}
			inline virtual ~base_signal5() {
				disconnect();
			}
		};
		
		template <typename return_type, typename arg1_type, typename arg2_type, typename arg3_type, typename arg4_type, typename arg5_type, class validator_type = default_validator<return_type> >
		class signal5 : public base_signal5 <return_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type> { 
		public: 
			typedef base_signal5 <return_type, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type> parent_type; 
			typedef typename deconst<return_type>::type non_const_return_type;
			
			inline virtual return_type emit (arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5) {
				validator_type v;
				non_const_return_type r;
				
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					r = (*i)->operator() (a1, a2, a3, a4, a5);
					if (!v(r))
						return r;
				}
				return r; 
			} 
		};
			
		template <typename arg1_type, typename arg2_type, typename arg3_type, typename arg4_type, typename arg5_type, class validator_type >
		class signal5 <void, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, validator_type> : public base_signal5 <void, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type> { 
		typedef base_signal5 <void, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type> parent_type; 
		public: 
			inline void emit (arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5) {  
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					(*i)->operator() (a1, a2, a3, a4, a5) ; 
				} 
			} 
		};
		template <typename arg1_type, typename arg2_type, typename arg3_type, typename arg4_type, typename arg5_type, class validator_type >
		class signal5 <int, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, validator_type> : public base_signal5 <int, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type> { 
			typedef base_signal5 <int, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type> parent_type; 
		public: 
			inline int emit (arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5) {  
				validator_type v;
				int r = (int)0;
				
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					r = (*i)->operator() (a1, a2, a3, a4, a5);
					if (!v(r))
						return r;
				}
				return r;
			}
		};
		template <typename arg1_type, typename arg2_type, typename arg3_type, typename arg4_type, typename arg5_type, class validator_type >
		class signal5 <bool, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type, validator_type> : public base_signal5 <bool, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type> { 
			typedef base_signal5 <bool, arg1_type, arg2_type, arg3_type, arg4_type, arg5_type> parent_type; 
		public: 
			inline bool emit (arg1_type a1, arg2_type a2, arg3_type a3, arg4_type a4, arg5_type a5) {  
				validator_type v;
				bool r = (bool)0;
				
				for(typename parent_type::slots_type::iterator i = parent_type::slots.begin(); i != parent_type::slots.end(); ++i) { 
					r = (*i)->operator() (a1, a2, a3, a4, a5);
					if (!v(r))
						return r;
				}
				return r;
			}
		};
		
}

#endif

