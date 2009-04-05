#include <portability.h>

/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  soar_db.cpp
 *
 * =======================================================================
 * Description  :  Useful database functions for Soar modules
 * =======================================================================
 */

#include "soar_db.h"

#include <assert.h>
#include <list>

#include "soar_module.h"
#include "agent.h"
#include "sqlite3.h"

namespace soar_module
{
	/////////////////////////////////////////////////////////////
	// Status Object
	/////////////////////////////////////////////////////////////

	template <typename T>
	void status_object<T>::set_status( T new_status ) { my_status = new_status; }

	template <typename T>
	void status_object<T>::set_errno( int new_errno ) { my_errno = new_errno; }

	template <typename T>
	void status_object<T>::set_errmsg( const char *new_msg )
	{
		if ( my_errmsg )
			delete my_errmsg;

		if ( new_msg )
		{
			size_t my_len = strlen( new_msg );
			my_errmsg = new char[ my_len + 1 ];
			strcpy( my_errmsg, new_msg );
			my_errmsg[ my_len ] = '\0';
		}
	}

	//

	template <typename T>
	status_object<T>::status_object(): my_errno( 0 ), my_errmsg( NULL ) {}

	template <typename T>
	status_object<T>::~status_object()
	{
		set_errmsg( NULL );
	}

	//

	template <typename T>
	T status_object<T>::get_status() { return my_status; }

	template <typename T>
	int status_object<T>::get_errno() { return my_errno; }

	template <typename T>
	const char *status_object<T>::get_errmsg() { return my_errmsg; }


	/////////////////////////////////////////////////////////////
	// Database
	/////////////////////////////////////////////////////////////

	database::database()
	{
		set_status( disconnected );
	}


	/////////////////////////////////////////////////////////////
	// Statement
	/////////////////////////////////////////////////////////////

	statement::statement( const char *new_sql, timer *new_query_timer ): sql( new_sql ), query_timer( new_query_timer )
	{
		set_status( unprepared );
	}

	statement::~statement() {}

	//

	exec_result statement::execute( statement_action post_action )
	{
		exec_result return_val = err;

		if ( get_status() == ready )
		{
			if ( query_timer )
				query_timer->start();

			return_val = _exec();
			assert( return_val != err );

			if ( query_timer )
				query_timer->stop();

			// post-action
			switch ( post_action )
			{
				case op_reinit:
					reinitialize();
					break;

				case op_clean:
					clean();
					break;
			}
		}

		return return_val;
	}

	void statement::prepare()
	{
		if ( get_status() == unprepared )
		{
			if ( _prep() )
				set_status( ready );
		}
	}

	void statement::reinitialize()
	{
		_reinit();
	}

	void statement::clean()
	{
		if ( get_status() != unprepared )
		{
			if ( _destroy() )
				set_status( unprepared );
		}
	}


	/////////////////////////////////////////////////////////////
	// Statement Container
	/////////////////////////////////////////////////////////////

	statement_container::statement_container(): statements( new std::list<statement *>() ) {}

	statement_container::~statement_container()
	{
		for ( std::list<statement *>::iterator p=statements->begin(); p!=statements->end(); p++ )
			delete (*p);

		delete statements;
	}

	//

	void statement_container::add( statement *new_statement )
	{
		statements->push_back( new_statement );
	}

	//

	void statement_container::prepare()
	{
		for ( std::list<statement *>::iterator p=statements->begin(); p!=statements->end(); p++ )
		{
			(*p)->prepare();
			assert( (*p)->get_status() == ready );
		}
	}

	void statement_container::clean()
	{
		for ( std::list<statement *>::iterator p=statements->begin(); p!=statements->end(); p++ )
		{
			(*p)->clean();
			assert( (*p)->get_status() == unprepared );
		}
	}


	/////////////////////////////////////////////////////////////
	// SQLite Database
	/////////////////////////////////////////////////////////////

	sqlite_database::sqlite_database(): database(), my_db( NULL )
	{
		set_errno( SQLITE_OK );
	}

	sqlite_database::~sqlite_database() {}

	//

	sqlite3 *sqlite_database::get_db() { return my_db; }

	//

	void sqlite_database::connect( const char *file_name, int flags )
	{
		int sqlite_err = sqlite3_open_v2( file_name, &( my_db ), flags, NULL );

		if ( sqlite_err == SQLITE_OK )
		{
			set_status( connected );

			set_errno( sqlite_err );
			set_errmsg( NULL );
		}
		else
		{
			set_status( problem );

			set_errno( sqlite_err );
			set_errmsg( sqlite3_errmsg( my_db ) );
		}
	}

	void sqlite_database::disconnect()
	{
		if ( get_status() == connected )
		{
			sqlite3_close( my_db );
			set_status( disconnected );
		}
	}

	//

	long sqlite_database::last_insert_rowid() { return (long) sqlite3_last_insert_rowid( my_db ); }

	long sqlite_database::memory_usage() { return (long) sqlite3_memory_used(); }

	long sqlite_database::memory_highwater() { return (long) sqlite3_memory_highwater( false ); }


	/////////////////////////////////////////////////////////////
	// SQLite Statement
	/////////////////////////////////////////////////////////////

	void sqlite_statement::sqlite_err()
	{
		set_errno( sqlite3_errcode( my_db->get_db() ) );
		set_errmsg( sqlite3_errmsg( my_db->get_db() ) );
	}

	bool sqlite_statement::_prep()
	{
		const char *tail;
		bool return_val = false;

		if ( sqlite3_prepare_v2( my_db->get_db(), sql, SQLITE_PREP_STR_MAX, &( my_stmt ), &tail ) == SQLITE_OK )
		{
			return_val = true;
		}
		else
		{
			sqlite_err();
		}

		return return_val;
	}

	void sqlite_statement::_reinit()
	{
		sqlite3_reset( my_stmt );
	}

	bool sqlite_statement::_destroy()
	{
		sqlite3_finalize( my_stmt );
		my_stmt = NULL;

		return true;
	}

	exec_result sqlite_statement::_exec()
	{
		int sqlite_res = sqlite3_step( my_stmt );
		exec_result return_val = err;

		if ( ( sqlite_res != SQLITE_OK ) &&
			 ( sqlite_res != SQLITE_DONE ) &&
			 ( sqlite_res != SQLITE_ROW ) )
		{
			sqlite_err();
		}
		else
		{
			return_val = ( ( sqlite_res == SQLITE_ROW )?( row ):( ok ) );
		}

		return return_val;
	}

	//

	sqlite_statement::sqlite_statement( sqlite_database *new_db, const char *new_sql, timer *new_query_timer ): statement( new_sql, new_query_timer ), my_db( new_db ), my_stmt( NULL ) {}

	sqlite_statement::~sqlite_statement()
	{
		if ( my_stmt )
			_destroy();
	}

	//

	void sqlite_statement::bind_int( int param, long val )
	{
		SQLITE_BIND_INT( my_stmt, param, val );
	}

	void sqlite_statement::bind_double( int param, double val )
	{
		sqlite3_bind_double( my_stmt, param, val );
	}

	void sqlite_statement::bind_null( int param )
	{
		sqlite3_bind_null( my_stmt, param );
	}

	void sqlite_statement::bind_text( int param, const char *val )
	{
		sqlite3_bind_text( my_stmt, param, val, SQLITE_PREP_STR_MAX, SQLITE_STATIC );
	}

	//

	long sqlite_statement::column_int( int col )
	{
		return SQLITE_COLUMN_INT( my_stmt, col );
	}

	double sqlite_statement::column_double( int col )
	{
		return sqlite3_column_int( my_stmt, col );
	}

	const char *sqlite_statement::column_text( int col )
	{
		return (const char *) sqlite3_column_text( my_stmt, col );
	}

	value_type sqlite_statement::column_type( int col )
	{
		int col_type = sqlite3_column_type( my_stmt, col );
		value_type return_val = null_t;

		switch ( col_type )
		{
			case SQLITE_INTEGER:
				return_val = int_t;
				break;

			case SQLITE_FLOAT:
				return_val = double_t;
				break;

			case SQLITE_TEXT:
				return_val = text_t;
				break;
		}

		return return_val;
	}


	/////////////////////////////////////////////////////////////
	// SQLite Statement Container
	/////////////////////////////////////////////////////////////

	sqlite_statement_container::sqlite_statement_container( agent *new_agent ): statement_container(), structures( new std::list<const char *>() ), my_agent( new_agent ) {}

	sqlite_statement_container::~sqlite_statement_container()
	{
		delete structures;
	}

	void sqlite_statement_container::add_structure( const char *new_structure )
	{
		structures->push_back( new_structure );
	}

	void sqlite_statement_container::structure()
	{
		sqlite_statement *temp_stmt;

		for ( std::list<const char *>::iterator p=structures->begin(); p!=structures->end(); p++ )
		{
			temp_stmt = new sqlite_statement( this->my_agent->epmem_db, (*p) );

			temp_stmt->prepare();
			assert( temp_stmt->get_status() == ready );
			temp_stmt->execute();

			delete temp_stmt;
		}
	}
}
