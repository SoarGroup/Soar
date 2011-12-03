/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/*************************************************************************
 *
 *  file:  soar_module.h
 *
 * =======================================================================
 */

#ifndef SOAR_MODULE_H
#define SOAR_MODULE_H

#include <portability.h>

#include <map>
#include <string>
#include <set>
#include <list>
#include <functional>
#include <assert.h>

#include "misc.h"
#include "symtab.h"
#include "mem.h"

typedef struct wme_struct wme;
typedef struct preference_struct preference;

// separates this functionality
// just for Soar modules
namespace soar_module
{
	/////////////////////////////////////////////////////////////
	// Utility functions
	/////////////////////////////////////////////////////////////

	typedef std::set< wme* > wme_set;
	
	typedef struct symbol_triple_struct
	{
		Symbol* id;
		Symbol* attr;
		Symbol* value;

		symbol_triple_struct( Symbol* new_id, Symbol* new_attr, Symbol* new_value ): id(new_id), attr(new_attr), value(new_value) {}
	} symbol_triple;
	typedef std::list< symbol_triple* > symbol_triple_list;
	
	wme *add_module_wme( agent *my_agent, Symbol *id, Symbol *attr, Symbol *value );
	void remove_module_wme( agent *my_agent, wme *w );
	instantiation* make_fake_instantiation( agent* my_agent, Symbol* state, wme_set* conditions, symbol_triple_list* actions );
	
	///////////////////////////////////////////////////////////////////////////
	// Predicates
	///////////////////////////////////////////////////////////////////////////
	
	// a functor for validating parameter values
	template <typename T>
	class predicate: public std::unary_function<T, bool>
	{
		public:
			virtual ~predicate() {}
			virtual bool operator() ( T /*val*/ ) { return true; }
	};	

	// a false predicate
	template <typename T>
	class f_predicate: public predicate<T>
	{
		public:			
			virtual bool operator() ( T /*val*/ ) { return false; }
	};

	// predefined predicate for validating
	// a value between two values known at
	// predicate initialization
	template <typename T>
	class btw_predicate: public predicate<T>
	{
		private:
			T my_min;
			T my_max;
			bool inclusive;
		
		public:
			btw_predicate( T new_min, T new_max, bool new_inclusive ): my_min( new_min ), my_max( new_max ), inclusive( new_inclusive ) {}

			bool operator() ( T val )
			{
				return ( ( inclusive )?( ( val >= my_min ) && ( val <= my_max ) ):( ( val > my_min ) && ( val < my_max ) ) );
			}
	};
	
	// predefined predicate for validating
	// a value greater than a value known at
	// predicate initialization
	template <typename T>
	class gt_predicate: public predicate<T>
	{
		private:
			T my_min;			
			bool inclusive;
		
		public:
			gt_predicate( T new_min, bool new_inclusive ): my_min( new_min ), inclusive( new_inclusive ) {}
			
			bool operator() ( T val )
			{
				return ( ( inclusive )?( ( val >= my_min ) ):( ( val > my_min ) ) );
			}
	};
		
	// predefined predicate for validating
	// a value less than a value known at
	// predicate initialization
	template <typename T>
	class lt_predicate: public predicate<T>
	{
		private:
			T my_max;			
			bool inclusive;
		
		public:
			lt_predicate( T new_max, bool new_inclusive ): my_max( new_max ), inclusive( new_inclusive ) {}		
			
			bool operator() ( T val )
			{
				return ( ( inclusive )?( ( val <= my_max ) ):( ( val < my_max ) ) );
			}
	};

	// superclass for predicates needing
	// agent state
	template <typename T>
	class agent_predicate: public predicate<T>
	{
		protected:
			agent *my_agent;

		public:
			agent_predicate( agent *new_agent ): my_agent( new_agent ) {}
	};


	///////////////////////////////////////////////////////////////////////////
	// Common for params, stats, timers, etc.
	///////////////////////////////////////////////////////////////////////////

	class named_object
	{
		private:
			const char *name;

		public:
			named_object( const char *new_name ): name( new_name ) {}
			virtual ~named_object() {}

			//

			const char *get_name()
			{
				return name;
			}

			//

			virtual char *get_string() = 0;
	};


	template <typename T>
	class accumulator: public std::unary_function<T, void>
	{
		public:
			virtual ~accumulator() {}

			virtual void operator() ( T /*val*/ ) {}
	};

		
	// this class provides for efficient 
	// string->object access
	template <class T>
	class object_container
	{					
		protected:
			agent *my_agent;
			std::map<std::string, T *> *objects;
			
			void add( T *new_object )
			{
				std::string temp_str( new_object->get_name() );
				(*objects)[ temp_str ] = new_object;
			}

		public:
			object_container( agent *new_agent ): my_agent( new_agent ), objects( new std::map<std::string, T *> ) {}
			
			virtual ~object_container()
			{
				typename std::map<std::string, T *>::iterator p;

				for ( p=objects->begin(); p!=objects->end(); p++ )
					delete p->second;

				delete objects;
			}

			//

			T *get( const char *name )
			{
				std::string temp_str( name );
				typename std::map<std::string, T *>::iterator p = objects->find( temp_str );

				if ( p == objects->end() )
					return NULL;
				else
					return p->second;
			}

			void for_each( accumulator<T *> &f  )
			{
				typename std::map<std::string, T *>::iterator p;

				for ( p=objects->begin(); p!=objects->end(); p++ )
				{
					f( p->second );
				}
			}
	};


	///////////////////////////////////////////////////////////////////////////
	// Parameters
	///////////////////////////////////////////////////////////////////////////
	
	// all parameters have a name and
	// can be manipulated generically
	// via strings
	class param: public named_object
	{			
		public:		
			param( const char *new_name ): named_object( new_name ) {}
			virtual ~param() {}

			//

			virtual bool set_string( const char *new_string ) = 0;
			virtual bool validate_string( const char *new_string ) = 0;			
	};

	
	// a primitive parameter can take any primitive
	// data type as value and is validated via
	// any unary predicate
	template <typename T>
	class primitive_param: public param
	{
		protected:
			T value;
			predicate<T> *val_pred;
			predicate<T> *prot_pred;
		
		public:
			primitive_param( const char *new_name, T new_value, predicate<T> *new_val_pred, predicate<T> *new_prot_pred ): param( new_name ), value( new_value ), val_pred( new_val_pred ), prot_pred( new_prot_pred ) {}
			
			virtual ~primitive_param()
			{
				delete val_pred;
				delete prot_pred;
			}
			
			//
			
			virtual char *get_string()
			{
				std::string temp_str;
				to_string( value, temp_str );
				return strdup( temp_str.c_str() );
			}

			virtual bool set_string( const char *new_string )
			{
				T new_val;
				from_string( new_val, new_string );

				if ( !(*val_pred)( new_val ) || (*prot_pred)( new_val ) )
				{
					return false;
				}
				else
				{
					set_value( new_val );
					return true;
				}
			}

			virtual bool validate_string( const char *new_string )
			{
				T new_val;
				from_string( new_val, new_string );

				return (*val_pred)( new_val );
			}
			
			//
			
			virtual T get_value()
			{
				return value;
			}

			virtual void set_value( T new_value )
			{
				value = new_value;
			}
	};
	
	// these are easy definitions for int and double parameters
	typedef primitive_param<int64_t> integer_param;
	typedef primitive_param<double> decimal_param;
	

	// a string param deals with character strings
	class string_param: public param
	{
		protected:
			std::string *value;
			predicate<const char *> *val_pred;
			predicate<const char *> *prot_pred;
		
		public:
			string_param( const char *new_name, const char *new_value, predicate<const char *> *new_val_pred, predicate<const char *> *new_prot_pred ): param( new_name ), value( new std::string( new_value ) ), val_pred( new_val_pred ), prot_pred( new_prot_pred ) {}

			virtual ~string_param()
			{
				delete value;
				delete val_pred;
				delete prot_pred;
			}
			
			//
			
			virtual char *get_string()
			{
				char *return_val = new char[ value->length() + 1 ];
				strcpy( return_val, value->c_str() );
				return_val[ value->length() ] = '\0';

				return return_val;
			}

			virtual bool set_string( const char *new_string )
			{
				if ( !(*val_pred)( new_string ) || (*prot_pred)( new_string ) )
				{
					return false;
				}
				else
				{
					set_value( new_string );
					return true;
				}
			}

			virtual bool validate_string( const char *new_value )
			{
				return (*val_pred)( new_value );
			}
			
			//
			
			virtual const char *get_value()
			{
				return value->c_str();
			}

			virtual void set_value( const char *new_value )
			{
				value->assign( new_value );
			}
	};

	// a primitive_set param maintains a set of primitives
	template <typename T>
	class primitive_set_param: public param
	{
		protected:
			std::set< T > *my_set;
			std::string *value;
			predicate< T > *prot_pred;

		public:
			primitive_set_param( const char *new_name, predicate< T > *new_prot_pred ): param( new_name ), my_set( new std::set< T >() ), value( new std::string ), prot_pred( new_prot_pred ) {}

			virtual ~primitive_set_param()
			{
				delete my_set;
				delete value;
				delete prot_pred;
			}

			virtual char *get_string()
			{
				char *return_val = new char[ value->length() + 1 ];
				strcpy( return_val, value->c_str() );
				return_val[ value->length() ] = '\0';

				return return_val;
			}

			virtual bool validate_string( const char *new_value )
			{
				T test_val;

				return from_string( test_val, new_value );
			}

			virtual bool set_string( const char *new_string )
			{
				T new_val;
				from_string( new_val, new_string );
				
				if ( (*prot_pred)( new_val ) )
				{
					return false;
				}
				else
				{
					typename std::set< T >::iterator it = my_set->find( new_val );
					std::string temp_str;

					if ( it != my_set->end() )
					{
						my_set->erase( it );

						// regenerate value from scratch
						value->clear();
						for ( it=my_set->begin(); it!=my_set->end(); )
						{
							to_string( *it, temp_str );
							value->append( temp_str );

							it++;

							if ( it != my_set->end() )
								value->append( ", " );
						}
					}
					else
					{
						my_set->insert( new_val );

						if ( !value->empty() )
							value->append( ", " );

						to_string( new_val, temp_str );
						value->append( temp_str );
					}


					return true;
				}
			}

			virtual bool in_set( T test_val )
			{
				return ( my_set->find( test_val ) != my_set->end() );
			}

			virtual typename std::set< T >::iterator set_begin()
			{
				return my_set->begin();
			}

			virtual typename std::set< T >::iterator set_end()
			{
				return my_set->end();
			}
	};

	// these are easy definitions for sets
	typedef primitive_set_param< int64_t > int_set_param;

	// a sym_set param maintains a set of strings
	class sym_set_param: public param
	{
		protected:
			std::set<Symbol *> *my_set;
			std::string *value;
			predicate<const char *> *prot_pred;

			agent *my_agent;

		public:
			sym_set_param( const char *new_name, predicate<const char *> *new_prot_pred, agent *new_agent ): param( new_name ), my_set( new std::set<Symbol *>() ), value( new std::string ), prot_pred( new_prot_pred ), my_agent( new_agent ) {}

			virtual ~sym_set_param()
			{
				for ( std::set<Symbol *>::iterator p=my_set->begin(); p!=my_set->end(); p++ )
					symbol_remove_ref( my_agent, (*p) );
				
				delete my_set;
				delete value;
				delete prot_pred;
			}

			//

			virtual char *get_string()
			{
				char *return_val = new char[ value->length() + 1 ];
				strcpy( return_val, value->c_str() );
				return_val[ value->length() ] = '\0';

				return return_val;
			}

			virtual bool set_string( const char *new_string )
			{
				if ( (*prot_pred)( new_string ) )
				{
					return false;
				}
				else
				{
					set_value( new_string );
					return true;
				}
			}

			virtual bool validate_string( const char * /*new_value*/ )
			{
				return true;
			}

			//

			virtual bool in_set( Symbol *test_sym )
			{
				bool return_val = false;

				if ( ( test_sym->common.symbol_type == SYM_CONSTANT_SYMBOL_TYPE ) ||
					 ( test_sym->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE ) ||
					 ( test_sym->common.symbol_type == FLOAT_CONSTANT_SYMBOL_TYPE ) )
				{
					Symbol *my_sym = test_sym;

					if ( my_sym->common.symbol_type != SYM_CONSTANT_SYMBOL_TYPE )
					{
						std::string temp_str;

						if ( my_sym->common.symbol_type == INT_CONSTANT_SYMBOL_TYPE )
						{
							to_string( my_sym->ic.value, temp_str );
						}
						else
						{
							to_string( my_sym->fc.value, temp_str );
						}

						my_sym = make_sym_constant( my_agent, temp_str.c_str() );
					}

					std::set<Symbol *>::iterator p = my_set->find( my_sym );
					return_val = ( p != my_set->end() );

					if ( test_sym != my_sym )
					{
						symbol_remove_ref( my_agent, my_sym );
					}
				}

				return return_val;
			}

			virtual void set_value( const char *new_value )
			{
				Symbol *my_sym = make_sym_constant( my_agent, new_value );
				std::set<Symbol *>::iterator p = my_set->find( my_sym );

				if ( p != my_set->end() )
				{
					my_set->erase( p );

					// remove for now and when added to the set
					symbol_remove_ref( my_agent, my_sym );
					symbol_remove_ref( my_agent, my_sym );

					// regenerate value from scratch
					value->clear();
					for ( p=my_set->begin(); p!=my_set->end(); )
					{
						value->append( (*p)->sc.name );

						p++;

						if ( p != my_set->end() )
							value->append( ", " );
					}
				}
				else
				{
					my_set->insert( my_sym );

					if ( !value->empty() )
						value->append( ", " );

					value->append( my_sym->sc.name );
				}
			}
	};

	// a constant parameter deals in discrete values
	// for efficiency, internally we use enums, elsewhere
	// strings for user-readability
	template <typename T>
	class constant_param: public param
	{
		protected:		
			T value;
			std::map<T, const char *> *value_to_string;
			std::map<std::string, T> *string_to_value;
			predicate<T> *prot_pred;			
			
		public:						
			constant_param( const char *new_name, T new_value, predicate<T> *new_prot_pred ): param( new_name ), value( new_value ), value_to_string( new std::map<T, const char *>() ), string_to_value( new std::map<std::string, T> ), prot_pred( new_prot_pred ) {}
			
			virtual ~constant_param()
			{
				delete value_to_string;
				delete string_to_value;
				delete prot_pred;
			}
			
			//
			
			virtual char *get_string()
			{
				typename std::map<T, const char *>::iterator p;
				p = value_to_string->find( value );

				if ( p == value_to_string->end() )
					return NULL;
				else
				{
					size_t len = strlen( p->second );
					char *return_val = new char[ len + 1 ];

					strcpy( return_val, p->second );
					return_val[ len ] = '\0';

					return return_val;
				}
			}

			virtual bool set_string( const char *new_string )
			{
				typename std::map<std::string, T>::iterator p;
				std::string temp_str( new_string );

				p = string_to_value->find( temp_str );

				if ( ( p == string_to_value->end() ) || (*prot_pred)( p->second ) )
				{
					return false;
				}
				else
				{
					set_value( p->second );
					return true;
				}
			}

			virtual bool validate_string( const char *new_string )
			{
				typename std::map<std::string, T>::iterator p;
				std::string temp_str( new_string );

				p = string_to_value->find( temp_str );

				return ( p != string_to_value->end() );
			}

			//
			
			virtual T get_value()
			{
				return value;
			}

			virtual void set_value( T new_value )
			{
				value = new_value;
			}
			
			//
			
			virtual void add_mapping( T val, const char *str )
			{
				std::string my_string( str );

				// string to value
				(*string_to_value)[ my_string ] = val;

				// value to string
				(*value_to_string)[ val ] = str;
			}
	};

	// this is an easy implementation of a boolean parameter
	enum boolean { off, on };
	class boolean_param: public constant_param<boolean>
	{
		public:
			boolean_param( const char *new_name, boolean new_value, predicate<boolean> *new_prot_pred ): constant_param<boolean>( new_name, new_value, new_prot_pred )
			{
				add_mapping( off, "off" );
				add_mapping( on, "on" );
			}
	};
	

	///////////////////////////////////////////////////////////////////////////
	// Parameter Container
	///////////////////////////////////////////////////////////////////////////

	typedef object_container<param> param_container;


	///////////////////////////////////////////////////////////////////////////
	// Statistics
	///////////////////////////////////////////////////////////////////////////
	
	// all statistics have a name and
	// can be retrieved generically
	// via strings
	class stat: public named_object
	{			
		public:		
			stat( const char *new_name ): named_object( new_name ) {}
			virtual ~stat() {}
			
			//
			
			virtual void reset() = 0;
	};


	// a primitive statistic can take any primitive
	// data type as value
	template <typename T>
	class primitive_stat: public stat
	{
		private:
			T value;
			T reset_val;
			predicate<T> *prot_pred;
		
		public:
			primitive_stat( const char *new_name, T new_value, predicate<T> *new_prot_pred ): stat( new_name ), value( new_value ), reset_val( new_value ), prot_pred( new_prot_pred ) {}			
			
			virtual ~primitive_stat()
			{
				delete prot_pred;
			}
			
			//
			
			virtual char *get_string()
			{
				T my_val = get_value();

				std::string temp_str;
				to_string( my_val, temp_str );
				return strdup(temp_str.c_str());
			}

			void reset()
			{
				if ( !(*prot_pred)( value ) )
					value = reset_val;
			}
			
			//
			
			virtual T get_value()
			{
				return value;
			}

			virtual void set_value( T new_value )
			{
				value = new_value;
			}
	};

	// these are easy definitions for int and double stats
	typedef primitive_stat<int64_t> integer_stat;
	typedef primitive_stat<double> decimal_stat;	
	
	///////////////////////////////////////////////////////////////////////////
	// Statistic Containers
	///////////////////////////////////////////////////////////////////////////

	class stat_container: public object_container<stat>
	{
		public:
			stat_container( agent *new_agent ): object_container<stat>( new_agent ) {}

			//

			void reset()
			{
				for ( std::map<std::string, stat *>::iterator p=objects->begin(); p!=objects->end(); p++ )
					p->second->reset();
			}
	};


	///////////////////////////////////////////////////////////////////////////
	// timers
	///////////////////////////////////////////////////////////////////////////	

	class timer: public named_object
	{
		public:
			enum timer_level { zero, one, two, three, four, five };
	
		protected:
			agent *my_agent;
			
			soar_process_timer stopwatch;
			soar_timer_accumulator accumulator;

			timer_level level;
			predicate<timer_level> *pred;			

		public:

			timer( const char *new_name, agent *new_agent, timer_level new_level, predicate<timer_level> *new_pred, bool soar_control = true );

			virtual ~timer()
			{
				delete pred;
			}

			//

			virtual char *get_string()
			{
				double my_value = value();

				std::string temp_str;
				to_string( my_value, temp_str );
				return strdup(temp_str.c_str());
			}

			//

			virtual void reset()
			{
				stopwatch.stop();
				accumulator.reset();
			}

			virtual double value()
			{
				return accumulator.get_sec();
			}

			//

			virtual void start()
			{
				if ( (*pred)( level ) )
				{
					stopwatch.start();
				}
			}

			virtual void stop()
			{
				if ( (*pred)( level ) )
				{
					stopwatch.stop();
					accumulator.update(stopwatch);
				}
			}
	};


	///////////////////////////////////////////////////////////////////////////
	// Timer Containers
	///////////////////////////////////////////////////////////////////////////

	class timer_container: public object_container<timer>
	{
		public:
			timer_container( agent *new_agent ): object_container<timer>( new_agent ) {}

			//

			void reset()
			{
				for ( std::map<std::string, timer *>::iterator p=objects->begin(); p!=objects->end(); p++ )
					p->second->reset();
			}
	};


	///////////////////////////////////////////////////////////////////////////
	// Memory Pool Allocators
	///////////////////////////////////////////////////////////////////////////

#define USE_MEM_POOL_ALLOCATORS 1

#ifdef USE_MEM_POOL_ALLOCATORS

	memory_pool* get_memory_pool( agent* my_agent, size_t size );

	template <class T>
	class soar_memory_pool_allocator
	{
	public:
		typedef T			value_type;
		typedef size_t		size_type;
		typedef ptrdiff_t	difference_type;

		typedef T*			pointer;
		typedef const T*	const_pointer;

		typedef T&			reference;
		typedef const T&	const_reference;

	public:
		agent* get_agent() const { return my_agent; }

		soar_memory_pool_allocator( agent* new_agent ): my_agent(new_agent), mem_pool(NULL), size(sizeof(value_type))
		{
			// useful for debugging
			// std::string temp_this( typeid( value_type ).name() );
		}

		soar_memory_pool_allocator( const soar_memory_pool_allocator& obj ): my_agent(obj.get_agent()), mem_pool(NULL), size(sizeof(value_type))
		{
			// useful for debugging
			// std::string temp_this( typeid( value_type ).name() );
		}

		template <class _other>
		soar_memory_pool_allocator( const soar_memory_pool_allocator<_other>& other ): my_agent(other.get_agent()), mem_pool(NULL), size(sizeof(value_type))
		{
			// useful for debugging
			// std::string temp_this( typeid( T ).name() );
			// std::string temp_other( typeid( _other ).name() );
		}

		pointer allocate( size_type n, const void* = 0 )
		{
			size_type test = n;
			test; // prevents release-mode warning, since assert is compiled out
			assert( test == 1 );
			
			if ( !mem_pool )
			{
				mem_pool = get_memory_pool( my_agent, size );
			}
			
			pointer t;
			allocate_with_pool( my_agent, mem_pool, &t );

			return t;
		}

		void deallocate( void* p, size_type n )
		{
			size_type test = n;
			test; // prevents release-mode warning, since assert is compiled out
			assert( test == 1 );

			// not sure if this is correct...
			// it only comes up if an object uses another object's
			// allocator to deallocate memory that it allocated.
			// it's quite possible, then, that the sizes would be off
			if ( !mem_pool )
			{
				mem_pool = get_memory_pool( my_agent, size );
			}
			
			if ( p )
			{
				free_with_pool( mem_pool, p );
			}
		}

		void construct( pointer p, const_reference val )
		{
			new (p) T( val );
		}

		void destroy( pointer p )
		{
			p; // prevents warning
			p->~T();
		}

		size_type max_size() const
		{
			return static_cast< size_type >( -1 );
		}

		const_pointer address( const_reference r ) const
		{
			return &r;
		}

		pointer address( reference r ) const
		{
			return &r;
		}

		template <class U>
		struct rebind
		{
			typedef soar_memory_pool_allocator<U> other;
		};


	private:
		agent* my_agent;
		memory_pool* mem_pool;
		size_type size;

		soar_memory_pool_allocator() {}

	};

#endif

	///////////////////////////////////////////////////////////////////////////
	// Object Store Management
	//
	// Model:
	// 1) Store consists of a set of "objects"
	// 2) Time proceeds forward in discrete "steps"
	// 3) Objects decay according to arbitrary decay function
	// 4) Object references may occur multiple times within each step
	//
	// Implementation:
	// A) Templated object store (internally maintains pointers to type T)
	// B) Bounded history (template parameter N)
	// C) Subclasses implement decay
	///////////////////////////////////////////////////////////////////////////

	template <class T, int N>
	class object_memory
	{
	public:
		typedef uint64_t time_step;
		typedef uint64_t object_reference;

		typedef std::set< const T* > object_set;

	protected:
		typedef struct object_time_reference_struct
		{
			object_reference num_references;
			time_step t_step;
		} object_time_reference;

		typedef struct object_history_struct
		{
			object_time_reference reference_history[ N ];
			unsigned int next_p;
			unsigned int history_ct;

			object_reference history_references;
			object_reference total_references;
			time_step first_reference;

			time_step decay_step;

			object_reference buffered_references;

			//

			const T* this_object;

			//

			object_history_struct( const T* obj )
			{
				this_object = obj;

				buffered_references = 0;

				decay_step = 0;

				history_references = 0;
				total_references = 0;
				first_reference = 0;

				next_p = 0;
				history_ct = 0;
				for ( int i=0; i<N; i++ )
				{
					reference_history[ i ].num_references = 0;
					reference_history[ i ].t_step = 0;
				}
			}

		} object_history;

		unsigned int history_next( unsigned int current )
		{
			return ( ( current == ( N - 1 ) )?( 0 ):( current + 1 ) );
		}

		unsigned int history_prev( unsigned int current )
		{
			return ( ( current == 0 )?( N - 1 ):( current - 1 ) );
		}

		const object_history* get_history( const T* obj )
		{			
			object_history_map::iterator p = object_histories.find( obj );
			if ( p != object_histories.end() )
			{
				return &( p->second );
			}
			else
			{
				return NULL;
			}
		}

		time_step get_current_time()
		{
			return step_count;
		}

		bool is_initialized()
		{
			return initialized;
		}

		virtual time_step estimate_forgetting_time( const object_history* h, time_step t, bool fresh_reference ) = 0;
		virtual bool should_forget( const object_history* h, time_step t ) = 0;

		virtual void _init() = 0;
		virtual void _down() = 0;

	public:
		object_memory(): initialized( false )
		{
		}

		void initialize()
		{
			if ( !initialized )
			{
				step_count = 1;

				_init();
				
				initialized = true;
			}
		}

		void teardown()
		{
			if ( initialized )
			{				
				touched_histories.clear();
				touched_times.clear();
				forgetting_pq.clear();
				forgotten.clear();
				object_histories.clear();

				_down();
				
				initialized = false;
			}
		}

		// return: was this a new object?
		bool reference_object( const T* obj, object_reference num )
		{
			object_history* h = NULL;
			bool return_val = false;
			
			object_history_map::iterator p = object_histories.find( obj );
			if ( p != object_histories.end() )
			{
				h = &( p->second );
			}
			else
			{
				std::pair< object_history_map::iterator, bool > ip = object_histories.insert( std::make_pair< const T*, object_history >( obj, object_history( obj ) ) );
				assert( ip.second );

				h = &( ip.first->second );

				return_val = true;
			}

			h->buffered_references += num;
			touched_histories.insert( h );

			return return_val;
		}

		void remove_object( const T* obj )
		{
			object_history_map::iterator p = object_histories.find( obj );
			if ( p != object_histories.end() )
			{
				touched_histories.erase( &( p->second ) );
				remove_from_pq( &( p->second ) );
				object_histories.erase( p );
			}
		}

		void process_buffered_references()
		{
			history_set::iterator h_p;
			object_history* h;

			// add to history for changed histories
			for ( h_p=touched_histories.begin(); h_p!=touched_histories.end(); h_p++ )
			{
				h = *h_p;

				// update number of references in the current history
				// (has to come before history overwrite)
				h->history_references += ( h->buffered_references - h->reference_history[ h->next_p ].num_references );

				// set history
				h->reference_history[ h->next_p ].t_step = step_count;
				h->reference_history[ h->next_p ].num_references = h->buffered_references;

				// keep track of first reference
				if ( h->total_references == 0 )
				{
					h->first_reference = step_count;
				}

				// update counters
				if ( h->history_ct < N )
				{
					h->history_ct++;
				}
				h->next_p = history_next( h->next_p );
				h->total_references += h->buffered_references;

				// reset buffer counter
				h->buffered_references = 0;

				// update p-queue
				if ( h->decay_step != 0 )
				{
					move_in_pq( h, estimate_forgetting_time( h, step_count, true ) );
				}
				else
				{
					add_to_pq( h, estimate_forgetting_time( h, step_count, true ) );
				}
			}

			touched_histories.clear();
		}

		typename object_set::iterator forgotten_begin()
		{
			return forgotten.begin();
		}

		typename object_set::iterator forgotten_end()
		{
			return forgotten.end();
		}

		void forget()
		{
			forgotten.clear();
			
			if ( !forgetting_pq.empty() )
			{
				forgetting_map::iterator pq_p = forgetting_pq.begin();

				// check if we even have to do anything this time step
				if ( pq_p->first == step_count )
				{
					history_set::iterator d_p=pq_p->second.begin();
					history_set::iterator current_p;

					while ( d_p != pq_p->second.end() )
					{
						current_p = d_p++;

						if ( should_forget( *current_p, step_count ) )
						{
							forgotten.insert( (*current_p)->this_object );
							remove_from_pq( *current_p );
						}
						else
						{
							move_in_pq( *current_p, estimate_forgetting_time( *current_p, step_count, false ) );
						}
					}

					// clean up set
					touched_times.insert( pq_p->first );
					pq_p->second.clear();
				}

				// clean up touched time sets
				for ( time_set::iterator t_p=touched_times.begin(); t_p!=touched_times.end(); t_p++ )
				{
					pq_p = forgetting_pq.find( *t_p );
					if ( ( pq_p != forgetting_pq.end() ) && pq_p->second.empty() )
					{
						forgetting_pq.erase( pq_p );
					}
				}

				touched_times.clear();
			}
		}

		void time_forward()
		{
			step_count++;
		}

		void time_back()
		{
			step_count--;
		}

	private:

		void add_to_pq( object_history* h, time_step t )
		{
			assert( h->decay_step == 0 );
			
			h->decay_step = t;
			forgetting_pq[ t ].insert( h );
		}

		void remove_from_pq( object_history* h )
		{
			if ( h->decay_step )
			{
				forgetting_map::iterator f_p = forgetting_pq.find( h->decay_step );
				if ( f_p != forgetting_pq.end() )
				{
					f_p->second.erase( h );
					touched_times.insert( h->decay_step );
				}

				h->decay_step = 0;
			}
		}

		void move_in_pq( object_history* h, time_step new_t )
		{
			if ( h->decay_step != new_t )
			{
				remove_from_pq( h );
				add_to_pq( h, new_t );
			}
		}

		bool initialized;

		time_step step_count;

		typedef std::map< const T*, object_history > object_history_map;
		object_history_map object_histories;

		typedef std::set< object_history* > history_set;
		history_set touched_histories;

		typedef std::set< time_step > time_set;
		time_set touched_times;

		typedef std::map< time_step, history_set > forgetting_map;
		forgetting_map forgetting_pq;

		object_set forgotten;
	};

	///////////////////////////////////////////////////////////////////////////
	// Base-Level Activation (BLA) Object Store Management
	//
	// Notes:
	// - Approximation of decay time depends upon an estimate of
	//   max reference/time step (template parameter R)
	// - Smallest activation is bounded by activation_low constant
	///////////////////////////////////////////////////////////////////////////

	template <class T, int N, unsigned int R>
	class bla_object_memory : public object_memory<T,N>
	{

	public:
		bla_object_memory(): activation_none( 1.0 ), activation_low( -1000000000 ), time_sum_none( 2.71828182845905 ), use_petrov( true ), decay_rate( -0.5 ), decay_thresh( -2.0 ), pow_cache_bound( 10 )
		{
		}

		// return: was the setting accepted?
		bool set_petrov( bool new_petrov )
		{
			if ( !is_initialized() )
			{
				use_petrov = new_petrov;
				return true;
			}

			return false;
		}

		// takes positive, stores negative
		// return: was the value accepted (0, 1)
		bool set_decay_rate( double new_decay_rate )
		{
			if ( ( new_decay_rate > 0 ) && ( new_decay_rate < 1 ) && !is_initialized() )
			{
				decay_rate = -new_decay_rate;
				return true;
			}

			return false;
		}

		// return: was the setting accepted?
		bool set_decay_thresh( double new_decay_thresh )
		{
			if ( !is_initialized() )
			{
				decay_thresh = new_decay_thresh;
				return true;
			}

			return false;
		}

		// input: cache size in megabytes (0, inf)
		// return: was the setting accepted?
		bool set_pow_cache_bound( uint64_t new_pow_cache_bound )
		{
			if ( ( new_pow_cache_bound > 0 ) && !is_initialized() )
			{
				pow_cache_bound = new_pow_cache_bound;
				return true;
			}

			return false;
		}

		double get_object_activation( T* obj, bool log_result )
		{
			return compute_history_activation( get_history( obj ), get_current_time(), log_result );
		}

	protected:
		void _init()
		{
			// Pre-compute the integer powers of the decay exponent in order to avoid
			// repeated calls to pow() at runtime
			{
				// determine cache size
				{
					// computes how many powers to compute
					// basic idea: solve for the time that would just fall below the decay threshold, given decay rate and assumption of max references/time step
					// t = e^( ( thresh - ln( max_refs ) ) / -decay_rate )
					double cache_full = static_cast<double>( exp( ( decay_thresh - log( static_cast<double>( R ) ) ) / decay_rate ) );

					// we bound this by the max-pow-cache parameter to control the space vs. time tradeoff the cache supports
					// max-pow-cache is in MB, so do the conversion:
					// MB * 1024 bytes/KB * 1024 KB/MB
					double cache_bound_unit = ( static_cast<unsigned int>( pow_cache_bound * 1024 * 1024 ) / static_cast<unsigned int>( sizeof( double ) ) );

					pow_cache_size = static_cast< unsigned int >( ceil( ( cache_full > cache_bound_unit )?( cache_bound_unit ):( cache_full ) ) );
				}

				pow_cache = new double[ pow_cache_size ];

				pow_cache[0] = 0.0;
				for( unsigned int i=1; i<pow_cache_size; i++ )
				{
					pow_cache[ i ] = pow( static_cast<double>( i ), decay_rate );
				}
			}

			// calculate the pre-log'd forgetting threshold, to avoid most
			// calls to log
			decay_thresh_exp = exp( decay_thresh );

			// approximation cache
			{
				approx_cache[0] = 0;
				for ( int i=1; i<R; i++ )
				{
					approx_cache[i] = static_cast< time_step >( ceil( exp( static_cast<double>( decay_thresh - log( static_cast<double>(i) ) ) / static_cast<double>( decay_rate ) ) ) );
				}
			}
		}

		void _down()
		{
			// release power array memory
			delete[] pow_cache;
		}

		time_step estimate_forgetting_time( const object_history* h, time_step t, bool fresh_reference )
		{
			time_step return_val = t;

			// if new reference, we can cheaply under-estimate decay time
			// by treating all reference time steps independently
			// see AAAI FS: (Derbinsky & Laird 2011)
			if ( fresh_reference )
			{
				time_step to_add = 0;

				unsigned int p = h->next_p;
				unsigned int counter = h->history_ct;
				time_step t_diff = 0;
				object_reference approx_ref;

				while ( counter )
				{
					p = history_prev( p );

					t_diff = ( return_val - h->reference_history[ p ].t_step );

					approx_ref = ( ( h->reference_history[ p ].num_references < R )?( h->reference_history[ p ].num_references ):( R-1 ) );
					if ( approx_cache[ approx_ref ] > t_diff )
					{
						to_add += ( approx_cache[ approx_ref ] - t_diff );
					}

					counter--;
				}

				return_val += to_add;
			}

			// if approximation wasn't useful, or we used it previously and are now
			// beyond the underestimate, use binary parameter search to exactly
			// compute the time of forgetting
			if ( return_val == t )
			{
				time_step to_add = 1;

				if ( !should_forget( h, ( return_val + to_add ) ) )
				{					
					// find absolute upper bound
					do
					{
						to_add *= 2;
					} while ( !should_forget( h, ( return_val + to_add ) ) );

					// vanilla binary search within range: (upper/2, upper)
					time_step upper_bound = to_add;
					time_step lower_bound, mid;
					if ( to_add < 4 )
					{
						lower_bound = upper_bound;
					}
					else
					{
						lower_bound = ( to_add / 2 );
					}

					while ( lower_bound != upper_bound )
					{
						mid = ( ( lower_bound + upper_bound ) / 2 );

						if ( should_forget( h, ( return_val + mid ) ) )
						{
							upper_bound = mid;

							if ( upper_bound - lower_bound <= 1 )
							{
								lower_bound = mid;
							}
						}
						else
						{
							lower_bound = mid;

							if ( upper_bound - lower_bound <= 1 )
							{
								lower_bound = upper_bound;
							}
						}
					}

					to_add = upper_bound;
				}

				return_val += to_add;
			}
			
			return return_val;
		}

		bool should_forget( const object_history* h, time_step t )
		{
			return ( compute_history_activation( h, t, false ) < decay_thresh_exp );
		}

	private:	
		double _pow( time_step t_diff )
		{
			if ( t_diff < pow_cache_size )
			{
				return pow_cache[ t_diff ];
			}
			else
			{
				return pow( static_cast<double>( t_diff ), decay_rate );
			}
		}

		double compute_history_activation( const object_history* h, time_step t, bool log_result )
		{
			double return_val = ( ( log_result )?( activation_none ):( time_sum_none ) );

			if ( h && h->history_ct )
			{
				return_val = 0.0;
				
				// sum history
				{
					unsigned int p = h->next_p;
					unsigned int counter = h->history_ct;
					time_step t_diff = 0;

					//

					while ( counter )
					{
						p = history_prev( p );

						t_diff = ( t - h->reference_history[ p ].t_step );
						assert( t_diff > 0 );

						return_val += ( h->reference_history[ p ].num_references * _pow( t_diff ) );
						
						counter--;
					}

					// tail approximation for a bounded history, see (Petrov 2006)
					if ( use_petrov )
					{
						// if ( n > k )
						if ( h->total_references > h->history_references )
						{
							// ( n - k ) * ( tn^(1-d) - tk^(1-d) )
							// -----------------------------------
							// ( 1 - d ) * ( tn - tk )

							// decay_rate is negated (for nice printing)
							double d_inv = ( 1 + decay_rate );
							
							return_val += ( ( ( h->total_references - h->history_references ) * ( pow( static_cast<double>( t - h->first_reference ), d_inv ) - pow( static_cast<double>( t_diff ), d_inv ) ) ) / 
											( d_inv * ( ( t - h->first_reference ) - t_diff ) ) );
						}
					}
				}

				if ( log_result )
				{
					if ( return_val > 0.0 )
					{
						return_val = log( return_val );
						if ( return_val < activation_low )
						{
							return_val = activation_low;
						}
					}
					else
					{
						return_val = activation_low;
					}
				}
			}
			
			return return_val;
		}

		double activation_none;
		double activation_low;
		double time_sum_none;

		bool use_petrov;
		double decay_rate;
		double decay_thresh;
		uint64_t pow_cache_bound;

		double decay_thresh_exp;

		unsigned int pow_cache_size;
		double* pow_cache;

		time_step approx_cache[ R ];
	};

}

#endif
