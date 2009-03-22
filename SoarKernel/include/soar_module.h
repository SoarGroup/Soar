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

#include <map>
#include <string>

#include "misc.h"

// separates this functionality
// just for Soar modules
namespace soar_module
{
	///////////////////////////////////////////////////////////////////////////
	// Predicates
	///////////////////////////////////////////////////////////////////////////
	
	// a functor for validating parameter values
	template <typename T>
	class predicate
	{
		public:
			virtual ~predicate() {};
			
			virtual bool operator() ( T /*val*/ ) { return true; };
	};

	// a false predicate
	template <typename T>
	class f_predicate: public predicate<T>
	{
		public:			
			virtual bool operator() ( T /*val*/ ) { return false; };
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
			btw_predicate( T new_min, T new_max, bool new_inclusive ): my_min( new_min ), my_max( new_max ), inclusive( new_inclusive ) {};
		
			bool operator() ( T val ) { return ( ( inclusive )?( ( val >= my_min ) && ( val <= my_max ) ):( ( val > my_min ) && ( val < my_max ) ) ); };
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
			gt_predicate( T new_min, bool new_inclusive ): my_min( new_min ), inclusive( new_inclusive ) {};
		
			bool operator() ( T val ) { return ( ( inclusive )?( ( val >= my_min ) ):( ( val > my_min ) ) ); };
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
			lt_predicate( T new_max, bool new_inclusive ): my_max( new_max ), inclusive( new_inclusive ) {};
		
			bool operator() ( T val ) { return ( ( inclusive )?( ( val <= my_max ) ):( ( val < my_max ) ) ); };
	};

	// superclass for predicates needing
	// agent state
	template <typename T>
	class agent_predicate: public predicate<T>
	{
		protected:
			agent *my_agent;

		public:
			agent_predicate( agent *new_agent ): my_agent( new_agent ) {};
	};

	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	
	
	///////////////////////////////////////////////////////////////////////////
	// Parameters
	///////////////////////////////////////////////////////////////////////////
	
	// all parameters have a name and
	// can be manipulated generically
	// via strings
	class param
	{
		private:
			const char *name;
			
		public:		
			param( const char *new_name ): name( new_name ) {};
			virtual ~param() {};
			
			// 
					
			const char *get_name() { return name; };
			
			//
			
			virtual char *get_string() = 0;
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
			primitive_param( const char *new_name, T new_value, predicate<T> *new_val_pred, predicate<T> *new_prot_pred ): param( new_name ), value( new_value ), val_pred( new_val_pred ), prot_pred( new_prot_pred ) {};
			
			virtual ~primitive_param() 
			{ 
				delete val_pred; 
				delete prot_pred;
			};
			
			//
			
			virtual char *get_string()
			{				
				std::string *temp_str = to_string( value );			
				char *return_val = new char[ temp_str->length() + 1 ];
				strcpy( return_val, temp_str->c_str() );
				return_val[ temp_str->length() ] = '\0';
				delete temp_str;
				
				return return_val;				
			};
			
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
			};
			
			virtual bool validate_string( const char *new_string )
			{
				T new_val;				
				from_string( new_val, new_string );
				
				return (*val_pred)( new_val );
			};			
			
			//
			
			virtual T get_value() { return value; };
			virtual void set_value( T new_value ) { value = new_value; };
	};
	
	// these are easy definitions for int and double parameters
	typedef primitive_param<int> integer_param;
	typedef primitive_param<double> decimal_param;
	

	// a string param deals with character strings
	class string_param: public param
	{
		protected:
			std::string *value;
			predicate<const char *> *val_pred;
			predicate<const char *> *prot_pred;
		
		public:
			string_param( const char *new_name, const char *new_value, predicate<const char *> *new_val_pred, predicate<const char *> *new_prot_pred ): param( new_name ), val_pred( new_val_pred ), prot_pred( new_prot_pred )
			{
				value = new std::string( new_value );
			};
			
			virtual ~string_param()
			{
				delete value;
				delete val_pred;
				delete prot_pred;
			};
			
			//
			
			virtual char *get_string() 
			{				
				char *return_val = new char[ value->length() + 1 ];
				strcpy( return_val, value->c_str() );
				return_val[ value->length() ] = '\0';
				
				return return_val;
			};

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
			};

			virtual bool validate_string( const char *new_value ) 
			{ 
				return (*val_pred)( new_value );
			};
			
			//
			
			virtual const char *get_value() { return value->c_str(); };
			virtual void set_value( const char *new_value ) { value->assign( new_value ); };
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
			constant_param( const char *new_name, T new_value, predicate<T> *new_prot_pred ): param( new_name ), value( new_value ), prot_pred( new_prot_pred ) 
			{
				value_to_string = new std::map<T, const char *>();
				string_to_value = new std::map<std::string, T>();
			};
			
			virtual ~constant_param()
			{				
				delete value_to_string;
				delete string_to_value;
				delete prot_pred;
			};		
			
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
			};
			
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
					value = p->second;
					return true;
				}
			};
			
			virtual bool validate_string( const char *new_string )
			{
				typename std::map<std::string, T>::iterator p;
				std::string temp_str( new_string );
				
				p = string_to_value->find( temp_str );
				
				return ( p != string_to_value->end() );
			};

			//
			
			virtual T get_value() { return value; };
			virtual void set_value( T new_value ) { value = new_value; };
			
			//
			
			virtual void add_mapping( T val, const char *str )
			{
				std::string my_string( str );
				
				// string to value
				(*string_to_value)[ my_string ] = val;
				
				// value to string
				(*value_to_string)[ val ] = str;
			};
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
			};
	};
	
	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////
	

	///////////////////////////////////////////////////////////////////////////
	// Parameter Containers
	///////////////////////////////////////////////////////////////////////////

	// shortcut definition
	typedef std::map<std::string, param *> param_map;
	
	// this class provides for efficient 
	// string->parameter access
	class param_container
	{
		private:
			param_map *params;
			
		protected:			
			agent *my_agent;
			
			void add_param( param *new_param )
			{
				std::string temp_str( new_param->get_name() );
				(*params)[ temp_str ] = new_param;
			};
			
		public:
			param_container( agent *new_agent ): my_agent( new_agent )
			{
				params = new param_map();
			};
			
			virtual ~param_container()
			{
				for ( param_map::iterator p=params->begin(); p!=params->end(); p++ )
					delete p->second;
					
				delete params;
			};
			
			param *get_param( const char *name )
			{
				std::string temp_str( name );
				param_map::iterator p = params->find( temp_str );
				
				if ( p == params->end() )
					return NULL;
				else
					return p->second;
			};
	};

	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////
	// Statistics
	///////////////////////////////////////////////////////////////////////////
	
	// all statistics have a name and
	// can be retrieved generically
	// via strings
	class stat
	{
		private:
			const char *name;
			
		public:		
			stat( const char *new_name ): name( new_name ) {};
			virtual ~stat() {};
			
			// 
					
			const char *get_name() { return name; };
			
			//
			
			virtual char *get_string() = 0;
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
			primitive_stat( const char *new_name, T new_value, predicate<T> *new_prot_pred ): stat( new_name ), value( new_value ), reset_val( new_value ), prot_pred( new_prot_pred ) {};
			
			virtual ~primitive_stat() 
			{
				delete prot_pred;
			};
			
			//
			
			virtual char *get_string()
			{				
				std::string *temp_str = to_string( value );			
				char *return_val = new char[ temp_str->length() + 1 ];
				strcpy( return_val, temp_str->c_str() );
				return_val[ temp_str->length() ] = '\0';
				delete temp_str;
				
				return return_val;				
			};

			void reset()
			{
				if ( !(*prot_pred)( value ) )
					value = reset_val;
			};
			
			//
			
			virtual T get_value() { return value; };
			virtual void set_value( T new_value ) { value = new_value; };			
	};
	
	// these are easy definitions for int and double parameters
	typedef primitive_stat<long> integer_stat;
	typedef primitive_stat<double> decimal_stat;

	///////////////////////////////////////////////////////////////////////////
	// Statistic Containers
	///////////////////////////////////////////////////////////////////////////

	// shortcut definition
	typedef std::map<std::string, stat *> stat_map;
	
	// this class provides for efficient 
	// string->stat access
	class stat_container
	{
		private:
			stat_map *stats;
			
		protected:			
			agent *my_agent;
			
			void add_stat( stat *new_stat )
			{
				std::string temp_str( new_stat->get_name() );
				(*stats)[ temp_str ] = new_stat;
			};
			
		public:
			stat_container( agent *new_agent ): my_agent( new_agent )
			{
				stats = new stat_map();
			};
			
			virtual ~stat_container()
			{
				for ( stat_map::iterator p=stats->begin(); p!=stats->end(); p++ )
					delete p->second;
					
				delete stats;
			};
			
			stat *get_stat( const char *name )
			{
				std::string temp_str( name );
				stat_map::iterator p = stats->find( temp_str );
				
				if ( p == stats->end() )
					return NULL;
				else
					return p->second;
			};

			void reset_stats()
			{
				for ( stat_map::iterator p=stats->begin(); p!=stats->end(); p++ )
					p->second->reset();
			};
	};
}

#endif