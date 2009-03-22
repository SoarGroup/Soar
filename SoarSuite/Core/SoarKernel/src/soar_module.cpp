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

#include "soar_module.h"

using namespace soar_module;

///////////////////////////////////////////////////////////////////////////
// Predicates
///////////////////////////////////////////////////////////////////////////

namespace soar_module
{
	
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
	// param
	/////////////////////////////////////////////////////////////	
	param::param( const char *new_name ): name( new_name ) {};

	param::~param() {};

	const char *param::get_name() { return name; };
			

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

	void param_container::add_param( param *new_param )
	{
		std::string temp_str( new_param->get_name() );
		(*params)[ temp_str ] = new_param;
	};

	param_container::param_container( agent *new_agent ): my_agent( new_agent )
	{
		params = new param_map();
	};

	param_container::~param_container()
	{
		for ( param_map::iterator p=params->begin(); p!=params->end(); p++ )
			delete p->second;
			
		delete params;
	};

	param *param_container::get_param( const char *name )
	{
		std::string temp_str( name );
		param_map::iterator p = params->find( temp_str );
		
		if ( p == params->end() )
			return NULL;
		else
			return p->second;
	};


	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////
	// stat
	/////////////////////////////////////////////////////////////
	stat::stat( const char *new_name ): name( new_name ) {};
			
	stat::~stat() {};
			
	const char *stat::get_name() { return name; };

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
	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////////


	/////////////////////////////////////////////////////////////
	// Statistic Container
	/////////////////////////////////////////////////////////////
	void stat_container::add_stat( stat *new_stat )
	{
		std::string temp_str( new_stat->get_name() );
		(*stats)[ temp_str ] = new_stat;
	};

	stat_container::stat_container( agent *new_agent ): my_agent( new_agent )
	{
		stats = new stat_map();
	};

	stat_container::~stat_container()
	{
		for ( stat_map::iterator p=stats->begin(); p!=stats->end(); p++ )
			delete p->second;
			
		delete stats;
	};

	stat *stat_container::get_stat( const char *name )
	{
		std::string temp_str( name );
		stat_map::iterator p = stats->find( temp_str );
		
		if ( p == stats->end() )
			return NULL;
		else
			return p->second;
	};

	void stat_container::reset_stats()
	{
		for ( stat_map::iterator p=stats->begin(); p!=stats->end(); p++ )
			p->second->reset();
	};
}

