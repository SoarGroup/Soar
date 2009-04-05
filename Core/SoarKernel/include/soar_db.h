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

#include <list>

#include "soar_module.h"
#include "sqlite3.h"

// separates this functionality
// just for Soar modules
namespace soar_module
{
	///////////////////////////////////////////////////////////////////////////
	// Constants/Enums
	///////////////////////////////////////////////////////////////////////////

	// 32 vs. 64-bit integer functions
	#ifdef SOAR_64

		#define SQLITE_BIND_INT sqlite3_bind_int64
		#define SQLITE_COLUMN_INT sqlite3_column_int64

	#else

		#define SQLITE_BIND_INT sqlite3_bind_int
		#define SQLITE_COLUMN_INT sqlite3_column_int

	#endif

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

			inline void set_status( T new_status );
			inline void set_errno( int new_errno );

			inline void set_errmsg( const char *new_msg );

		public:
			status_object();
			virtual ~status_object();

			//

			inline T get_status();
			inline int get_errno();
			inline const char *get_errmsg();
	};


	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

	class database: public status_object<db_status>
	{
		public:
			database();
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
			statement( const char *new_sql, timer *new_query_timer );
			virtual ~statement();

			//

			inline exec_result execute( statement_action post_action = op_none );
			inline void prepare();
			inline void reinitialize();
			inline void clean();
	};


	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

	class statement_container
	{
		protected:
			std::list<statement *> *statements;

		public:
			statement_container();
			virtual ~statement_container();

			//

			void add( statement *new_statement );

			//

			void prepare();
			void clean();
	};


	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

	class sqlite_database: public database
	{
		protected:
			sqlite3 *my_db;

		public:
			sqlite_database();
			virtual ~sqlite_database();

			//

			inline sqlite3 *get_db();

			//

			inline void connect( const char *file_name, int flags = ( SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE ) );
			inline void disconnect();

			//

			inline long last_insert_rowid();
			inline long memory_usage();
			inline long memory_highwater();
	};


	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

	class sqlite_statement: public statement
	{
		protected:
			sqlite_database *my_db;
			sqlite3_stmt *my_stmt;

			inline void sqlite_err();

			virtual bool _prep();
			virtual void _reinit();
			virtual bool _destroy();
			virtual exec_result _exec();

		public:
			sqlite_statement( sqlite_database *new_db, const char *new_sql, timer *new_query_timer = NULL );
			virtual ~sqlite_statement();

			//

			inline void bind_int( int param, long val );
			inline void bind_double( int param, double val );
			inline void bind_null( int param );
			inline void bind_text( int param, const char *val );

			//

			inline long column_int( int col );
			inline double column_double( int col );
			inline const char *column_text( int col );
			inline value_type column_type( int col );
	};


	///////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////

	class sqlite_statement_container: public statement_container
	{
		protected:
			std::list<const char *> *structures;

			agent *my_agent;

		public:
			sqlite_statement_container( agent *new_agent );
			virtual ~sqlite_statement_container();

			//

			inline void add_structure( const char *new_structure );
			void structure();
	};
}

#endif
