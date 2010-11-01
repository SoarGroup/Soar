/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  soar_db.h
 *
 * =======================================================================
 */

#ifndef SOAR_DB_H
#define SOAR_DB_H

#include <portability.h>

#include <list>
#include <assert.h>

#include "soar_module.h"
#include "sqlite3.h"

// separates this functionality
// just for Soar modules
namespace soar_module
{
	///////////////////////////////////////////////////////////////////////////
	// Constants/Enums
	///////////////////////////////////////////////////////////////////////////

	// when preparing statements with strings, read till the first zero terminator
	#define SQLITE_PREP_STR_MAX -1

	// database connection status
	enum db_status { disconnected, connected, problem };

	// statement status
	enum statement_status { unprepared, ready };

	// statement action
	enum statement_action { op_none, op_reinit, op_clean };

	// value type
	enum value_type { null_t, int_t, double_t, text_t };

	// execution result
	enum exec_result { row, ok, err };


	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

	template <typename T>
	class status_object
	{
		protected:
			T my_status;
			int my_errno;
			char *my_errmsg;

			inline void set_status( T new_status ) { my_status = new_status; }
			inline void set_errno( int new_errno ) { my_errno = new_errno; }

			inline void set_errmsg( const char *new_msg )
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

		public:
			status_object(): my_errno( 0 ), my_errmsg( NULL ) {}
			virtual ~status_object()
			{
				set_errmsg( NULL );
			}

			//

			inline T get_status() { return my_status; }
			inline int get_errno() { return my_errno; }
			inline const char *get_errmsg() { return my_errmsg; }
	};


	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

	class database: public status_object<db_status>
	{
		public:
			database()
			{
				set_status( disconnected );
			}
	};


	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

	class statement: public status_object<statement_status>
	{
		protected:
			const char *sql;
			timer *query_timer;

			virtual exec_result _exec() = 0;
			virtual bool _prep() = 0;
			virtual void _reinit() = 0;
			virtual bool _destroy() = 0;

		public:
			statement( const char *new_sql, timer *new_query_timer ): sql( new_sql ), query_timer( new_query_timer )
			{
				set_status( unprepared );
			}

			virtual ~statement() {}

			//

			inline exec_result execute( statement_action post_action = op_none )
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
							
						case op_none:
							break;
					}
				}

				return return_val;
			}

			inline void prepare()
			{
				if ( get_status() == unprepared )
				{
					if ( _prep() )
						set_status( ready );
				}
			}

			inline void reinitialize()
			{
				_reinit();
			}

			inline void clean()
			{
				if ( get_status() != unprepared )
				{
					if ( _destroy() )
						set_status( unprepared );
				}
			}
	};

	
	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

	class statement_container
	{
		protected:
			std::list<statement *> *statements;

		public:
			statement_container(): statements( new std::list<statement *>() ) {}
			
			virtual ~statement_container()
			{
				for ( std::list<statement *>::iterator p=statements->begin(); p!=statements->end(); p++ )
					delete (*p);

				delete statements;
			}

			//

			void add( statement *new_statement )
			{
				statements->push_back( new_statement );
			}

			//

			void prepare()
			{
				for ( std::list<statement *>::iterator p=statements->begin(); p!=statements->end(); p++ )
				{
					(*p)->prepare();
					assert( (*p)->get_status() == ready );
				}
			}

			void clean()
			{
				for ( std::list<statement *>::iterator p=statements->begin(); p!=statements->end(); p++ )
				{
					(*p)->clean();
					assert( (*p)->get_status() == unprepared );
				}
			}
	};


	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

	class sqlite_database: public database
	{
		protected:
			sqlite3 *my_db;

		public:
			sqlite_database(): database(), my_db( NULL )
			{
				set_errno( SQLITE_OK );
			}

			virtual ~sqlite_database() {}

			//

			inline sqlite3 *get_db() { return my_db; }

			//

			inline void connect( const char *file_name, int flags = ( SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE ) )
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

			inline void disconnect()
			{
				if ( get_status() == connected )
				{
					sqlite3_close( my_db );
					set_status( disconnected );
				}
			}

			//

			inline int64_t last_insert_rowid() { return static_cast<int64_t>( sqlite3_last_insert_rowid( my_db ) ); }
			inline int64_t memory_usage() { return static_cast<int64_t>( sqlite3_memory_used() ); }
			inline int64_t memory_highwater() { return static_cast<int64_t>( sqlite3_memory_highwater( false ) ); }
	};


	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

	class sqlite_statement: public statement
	{
		protected:
			sqlite_database *my_db;
			sqlite3_stmt *my_stmt;

			inline void sqlite_err()
			{
				set_errno( sqlite3_errcode( my_db->get_db() ) );
				set_errmsg( sqlite3_errmsg( my_db->get_db() ) );
			}

			virtual bool _prep()
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

			virtual void _reinit()
			{
				sqlite3_reset( my_stmt );
			}

			virtual bool _destroy()
			{
				sqlite3_finalize( my_stmt );
				my_stmt = NULL;

				return true;
			}

			virtual exec_result _exec()
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


		public:
			sqlite_statement( sqlite_database *new_db, const char *new_sql, timer *new_query_timer = NULL ): statement( new_sql, new_query_timer ), my_db( new_db ), my_stmt( NULL ) {}

			virtual ~sqlite_statement()
			{
				if ( my_stmt )
					_destroy();
			}

			//

			inline void bind_int( int param, int64_t val )
			{
				sqlite3_bind_int64( my_stmt, param, static_cast<sqlite3_int64>( val ) );
			}

			inline void bind_double( int param, double val )
			{
				sqlite3_bind_double( my_stmt, param, val );
			}

			inline void bind_null( int param )
			{
				sqlite3_bind_null( my_stmt, param );
			}

			inline void bind_text( int param, const char *val )
			{
				sqlite3_bind_text( my_stmt, param, val, SQLITE_PREP_STR_MAX, SQLITE_STATIC );
			}

			//

			inline int64_t column_int( int col )
			{
				return sqlite3_column_int64( my_stmt, col );
			}

			inline double column_double( int col )
			{
				return sqlite3_column_double( my_stmt, col );
			}

			inline const char *column_text( int col )
			{
				return reinterpret_cast<const char *>( sqlite3_column_text( my_stmt, col ) );
			}

			inline value_type column_type( int col )
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
	};


	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

	class sqlite_statement_container: public statement_container
	{
		protected:
			std::list<const char *> *structures;

			sqlite_database *my_db;

		public:
			sqlite_statement_container( sqlite_database *new_db ): statement_container(), structures( new std::list<const char *>() ), my_db( new_db ) {}

			virtual ~sqlite_statement_container()
			{
				delete structures;
			}

			//

			inline void add_structure( const char *new_structure )
			{
				structures->push_back( new_structure );
			}

			void structure()
			{
				sqlite_statement *temp_stmt;

				for ( std::list<const char *>::iterator p=structures->begin(); p!=structures->end(); p++ )
				{
					temp_stmt = new sqlite_statement( my_db, (*p) );

					temp_stmt->prepare();
					assert( temp_stmt->get_status() == ready );
					temp_stmt->execute();

					delete temp_stmt;
				}
			}
	};
}

#endif
