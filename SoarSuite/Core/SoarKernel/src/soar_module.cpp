#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  soar_module.cpp
 *
 * =======================================================================
 * Description  :  Useful functions for Soar modules
 * =======================================================================
 */

#include <algorithm>

#include "soar_module.h"
#include "utilities.h"

namespace soar_module
{
	/////////////////////////////////////////////////////////////
	// Utility functions
	/////////////////////////////////////////////////////////////

	wme *add_module_wme( agent *my_agent, Symbol *id, Symbol *attr, Symbol *value )
	{
		slot *my_slot = make_slot( my_agent, id, attr );
		wme *w = make_wme( my_agent, id, attr, value, false );

		insert_at_head_of_dll( my_slot->wmes, w, next, prev );
		add_wme_to_wm( my_agent, w );

		return w;
	}

	void remove_module_wme( agent *my_agent, wme *w )
	{
		slot *my_slot = find_slot( w->id, w->attr );

		remove_from_dll( my_slot->wmes, w, next, prev );
		remove_wme_from_wm( my_agent, w );
	}
	
	
	/////////////////////////////////////////////////////////////
	// predicate
	/////////////////////////////////////////////////////////////
	template <typename T>
	predicate<T>::~predicate() {};

	template <typename T>
	bool predicate<T>::operator ()( T /*val*/ ) { return true; };


	/////////////////////////////////////////////////////////////
	// f_predicate
	/////////////////////////////////////////////////////////////
	template <typename T>
	bool f_predicate<T>::operator() ( T /*val*/ ) { return false; };


	/////////////////////////////////////////////////////////////
	// btw_predicate
	/////////////////////////////////////////////////////////////
	template <typename T>
	btw_predicate<T>::btw_predicate( T new_min, T new_max, bool new_inclusive ): my_min( new_min ), my_max( new_max ), inclusive( new_inclusive ) {};

	template <typename T>
	bool btw_predicate<T>::operator ()(T val)
	{
		return ( ( inclusive )?( ( val >= my_min ) && ( val <= my_max ) ):( ( val > my_min ) && ( val < my_max ) ) );
	};

	/////////////////////////////////////////////////////////////
	// gt_predicate
	/////////////////////////////////////////////////////////////
	template <typename T>
	gt_predicate<T>::gt_predicate( T new_min, bool new_inclusive ): my_min( new_min ), inclusive( new_inclusive ) {};

	template <typename T>
	bool gt_predicate<T>::operator() ( T val )
	{
		return ( ( inclusive )?( ( val >= my_min ) ):( ( val > my_min ) ) );
	};

	/////////////////////////////////////////////////////////////
	// lt_predicate
	/////////////////////////////////////////////////////////////
	template <typename T>
	lt_predicate<T>::lt_predicate( T new_max, bool new_inclusive ): my_max( new_max ), inclusive( new_inclusive ) {};

	template <typename T>
	bool lt_predicate<T>::operator() ( T val )
	{
		return ( ( inclusive )?( ( val <= my_max ) ):( ( val < my_max ) ) );
	};


	/////////////////////////////////////////////////////////////
	// agent_predicate
	/////////////////////////////////////////////////////////////
	template <typename T>
	agent_predicate<T>::agent_predicate( agent *new_agent ): my_agent( new_agent ) {};


	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////
	// named_object
	/////////////////////////////////////////////////////////////
	named_object::named_object(const char *new_name): name( new_name ) {};

	named_object::~named_object() {};

	const char *named_object::get_name() { return name; };


	/////////////////////////////////////////////////////////////
	// accumulator
	/////////////////////////////////////////////////////////////
	


	/////////////////////////////////////////////////////////////
	// object_container
	/////////////////////////////////////////////////////////////

	template <class T>
	void object_container<T>::add( T *new_object )
	{
		std::string temp_str( new_object->get_name() );
		(*objects)[ temp_str ] = new_object;
	};

	template <class T>
	object_container<T>::object_container( agent *new_agent ): my_agent( new_agent )
	{
		objects = new std::map<std::string, T *>();
	};

	template <class T>
	object_container<T>::~object_container()
	{
		typename std::map<std::string, T *>::iterator p;

		for ( p=objects->begin(); p!=objects->end(); p++ )
			delete p->second;

		delete objects;
	};


	/////////////////////////////////////////////////////////////
	// param
	/////////////////////////////////////////////////////////////
	param::param( const char *new_name ): named_object( new_name ) {};

	param::~param() {};


	/////////////////////////////////////////////////////////////
	// prim_param
	/////////////////////////////////////////////////////////////
	template <typename T>
	primitive_param<T>::primitive_param( const char *new_name, T new_value, predicate<T> *new_val_pred, predicate<T> *new_prot_pred ): param( new_name ), value( new_value ), val_pred( new_val_pred ), prot_pred( new_prot_pred ) {};

	template <typename T>
	primitive_param<T>::~primitive_param()
	{
		delete val_pred;
		delete prot_pred;
	};

	template <typename T>
	char *primitive_param<T>::get_string()
	{
		std::string *temp_str = to_string( value );
		char *return_val = new char[ temp_str->length() + 1 ];
		strcpy( return_val, temp_str->c_str() );
		return_val[ temp_str->length() ] = '\0';
		delete temp_str;

		return return_val;
	};

	template <typename T>
	bool primitive_param<T>::set_string( const char *new_string )
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

	template <typename T>
	bool primitive_param<T>::validate_string( const char *new_string )
	{
		T new_val;
		from_string( new_val, new_string );

		return (*val_pred)( new_val );
	};

	template <typename T>
	T primitive_param<T>::get_value() { return value; };

	template <typename T>
	void primitive_param<T>::set_value( T new_value ) { value = new_value; };


	/////////////////////////////////////////////////////////////
	// string_param
	/////////////////////////////////////////////////////////////
	string_param::string_param( const char *new_name, const char *new_value, predicate<const char *> *new_val_pred, predicate<const char *> *new_prot_pred ): param( new_name ), val_pred( new_val_pred ), prot_pred( new_prot_pred )
	{
		value = new std::string( new_value );
	};

	string_param::~string_param()
	{
		delete value;
		delete val_pred;
		delete prot_pred;
	};

	char *string_param::get_string()
	{
		char *return_val = new char[ value->length() + 1 ];
		strcpy( return_val, value->c_str() );
		return_val[ value->length() ] = '\0';

		return return_val;
	};

	bool string_param::set_string( const char *new_string )
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

	bool string_param::validate_string( const char *new_value )
	{
		return (*val_pred)( new_value );
	};

	const char *string_param::get_value() { return value->c_str(); };

	void string_param::set_value( const char *new_value ) { value->assign( new_value ); };


	/////////////////////////////////////////////////////////////
	// constant_param
	/////////////////////////////////////////////////////////////
	template <typename T>
	constant_param<T>::constant_param( const char *new_name, T new_value, predicate<T> *new_prot_pred ): param( new_name ), value( new_value ), prot_pred( new_prot_pred )
	{
		value_to_string = new std::map<T, const char *>();
		string_to_value = new std::map<std::string, T>();
	};

	template <typename T>
	constant_param<T>::~constant_param()
	{
		delete value_to_string;
		delete string_to_value;
		delete prot_pred;
	};

	template <typename T>
	char *constant_param<T>::get_string()
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

	template <typename T>
	bool constant_param<T>::set_string( const char *new_string )
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
	};

	template <typename T>
	bool constant_param<T>::validate_string( const char *new_string )
	{
		typename std::map<std::string, T>::iterator p;
		std::string temp_str( new_string );

		p = string_to_value->find( temp_str );

		return ( p != string_to_value->end() );
	};

	template <typename T>
	T constant_param<T>::get_value() { return value; };

	template <typename T>
	void constant_param<T>::set_value( T new_value ) { value = new_value; };

	template <typename T>
	void constant_param<T>::add_mapping( T val, const char *str )
	{
		std::string my_string( str );

		// string to value
		(*string_to_value)[ my_string ] = val;

		// value to string
		(*value_to_string)[ val ] = str;
	};


	/////////////////////////////////////////////////////////////
	// boolean_param
	/////////////////////////////////////////////////////////////
	boolean_param::boolean_param( const char *new_name, boolean new_value, predicate<boolean> *new_prot_pred ): constant_param<boolean>( new_name, new_value, new_prot_pred )
	{
		add_mapping( off, "off" );
		add_mapping( on, "on" );
	};


	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////
	// Parameter Container
	/////////////////////////////////////////////////////////////

	// no code necessary


	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////
	// stat
	/////////////////////////////////////////////////////////////
	stat::stat( const char *new_name ): named_object( new_name ) {};

	stat::~stat() {};

	/////////////////////////////////////////////////////////////
	// prim_stat
	/////////////////////////////////////////////////////////////
	template <typename T>
	primitive_stat<T>::primitive_stat( const char *new_name, T new_value, predicate<T> *new_prot_pred ): stat( new_name ), value( new_value ), reset_val( new_value ), prot_pred( new_prot_pred ) {};

	template <typename T>
	primitive_stat<T>::~primitive_stat()
	{
		delete prot_pred;
	};

	template <typename T>
	char *primitive_stat<T>::get_string()
	{
		T my_val = get_value();

		std::string *temp_str = to_string( my_val );
		char *return_val = new char[ temp_str->length() + 1 ];
		strcpy( return_val, temp_str->c_str() );
		return_val[ temp_str->length() ] = '\0';
		delete temp_str;

		return return_val;
	};

	template <typename T>
	void primitive_stat<T>::reset()
	{
		if ( !(*prot_pred)( value ) )
			value = reset_val;
	};

	template <typename T>
	T primitive_stat<T>::get_value() { return value; };

	template <typename T>
	void primitive_stat<T>::set_value( T new_value ) { value = new_value; };


	/////////////////////////////////////////////////////////////
	// Statistic Container
	/////////////////////////////////////////////////////////////

	stat_container::stat_container( agent *new_agent ): object_container<stat>( new_agent ) {};

	void stat_container::reset()
	{
		for ( std::map<std::string, stat *>::iterator p=objects->begin(); p!=objects->end(); p++ )
			p->second->reset();
	};


	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////
	// timer
	/////////////////////////////////////////////////////////////
	timer::timer( const char *new_name, agent *new_agent, timer_level new_level, predicate<timer_level> *new_pred ): named_object( new_name ), my_agent( new_agent ), level( new_level ), pred( new_pred )
	{
		reset();
	};

	timer::~timer()
	{
		delete pred;
	};

	char *timer::get_string()
	{
		double my_value = value();

		std::string *temp_str = to_string( my_value );
		char *return_val = new char[ temp_str->length() + 1 ];
		strcpy( return_val, temp_str->c_str() );
		return_val[ temp_str->length() ] = '\0';
		delete temp_str;

		return return_val;
	};

	void timer::reset()
	{
		reset_timer( &start_t );
		reset_timer( &total_t );
	};

	double timer::value()
	{
		return timer_value( &total_t );
	};

	void timer::start()
	{
		if ( (*pred)( level ) )
			start_timer( my_agent, &start_t );
	};

	void timer::stop()
	{
		if ( (*pred)( level ) )
			stop_timer( my_agent, &start_t, &total_t );
	};


	/////////////////////////////////////////////////////////////
	// Timer Container
	/////////////////////////////////////////////////////////////

	timer_container::timer_container( agent *new_agent ): object_container<timer>( new_agent ) {};

	void timer_container::reset()
	{
		for ( std::map<std::string, timer *>::iterator p=objects->begin(); p!=objects->end(); p++ )
			p->second->reset();
	};
}

