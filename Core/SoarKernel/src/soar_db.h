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

#include "portability.h"

#include <list>
#include <assert.h>

#include "soar_module.h"
#include "sqlite3.h"

//#define DEBUG_SQL_ERRORS
//#define DEBUG_SQL_QUERIES

#ifdef DEBUG_SQL_QUERIES
//static void profile(void *context, const char *sql, sqlite3_uint64 ns) {
//fprintf(stderr, "Execution Time of %llu ms for: %s\n", ns / 1000000, sql);}
static void trace(void* /*arg*/, const char* query)
{
    fprintf(stderr, "Query: %s\n", query);
}
#endif

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

    // storage
    enum db_choices { memory, file };

    // performance
    enum page_choices { page_1k, page_2k, page_4k, page_8k, page_16k, page_32k, page_64k };
    enum opt_choices { opt_safety, opt_speed };


    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    template <typename T>
    class status_object
    {
        protected:
            T my_status;
            int my_errno;
            char* my_errmsg;

            inline void set_status(T new_status)
            {
                my_status = new_status;
            }
            inline void set_errno(int new_errno)
            {
                my_errno = new_errno;
            }

            inline void set_errmsg(const char* new_msg)
            {
                if (my_errmsg)
                {
                    delete my_errmsg;
                }

                if (new_msg)
                {
                    size_t my_len = strlen(new_msg);
                    my_errmsg = new char[ my_len + 1 ];
                    strcpy(my_errmsg, new_msg);
                    my_errmsg[ my_len ] = '\0';
                }
            }

        public:
            status_object(): my_errno(0), my_errmsg(NULL) {}
            virtual ~status_object()
            {
                set_errmsg(NULL);
            }

            //

            inline T get_status()
            {
                return my_status;
            }
            inline int get_errno()
            {
                return my_errno;
            }
            inline const char* get_errmsg()
            {
                return my_errmsg;
            }
    };


    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    class database: public status_object<db_status>
    {
        public:
            database()
            {
                set_status(disconnected);
            }
    };


    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    class statement: public status_object<statement_status>
    {
        protected:
            const char* sql;
            timer* query_timer;

            virtual exec_result _exec() = 0;
            virtual bool _prep() = 0;
            virtual void _reinit() = 0;
            virtual bool _destroy() = 0;

        public:
            statement(const char* new_sql, timer* new_query_timer): sql(new_sql), query_timer(new_query_timer)
            {
                set_status(unprepared);
            }

            virtual ~statement() {}

            //

            inline exec_result execute(statement_action post_action = op_none)
            {
                exec_result return_val = err;

                if (get_status() == ready)
                {
                    if (query_timer)
                    {
                        query_timer->start();
                    }

                    return_val = _exec();
                    assert(return_val != err);

                    if (query_timer)
                    {
                        query_timer->stop();
                    }

                    // post-action
                    switch (post_action)
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
                if (get_status() == unprepared)
                {
                    if (_prep())
                    {
                        set_status(ready);
                    }
                }
            }

            inline void reinitialize()
            {
                _reinit();
            }

            inline void clean()
            {
                if (get_status() != unprepared)
                {
                    if (_destroy())
                    {
                        set_status(unprepared);
                    }
                }
            }

            inline void set_timer(timer* new_query_timer)
            {
                query_timer = new_query_timer;
            }
    };


    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    class statement_container
    {
        protected:
            std::list<statement*>* statements;

        public:
            statement_container(): statements(new std::list<statement*>()) {}

            virtual ~statement_container()
            {
                for (std::list<statement*>::iterator p = statements->begin(); p != statements->end(); p++)
                {
                    delete(*p);
                }

                delete statements;
            }

            //

            void add(statement* new_statement)
            {
                statements->push_back(new_statement);
            }

            //

            void prepare()
            {
                for (std::list<statement*>::iterator p = statements->begin(); p != statements->end(); p++)
                {
                    (*p)->prepare();
                    assert((*p)->get_status() == ready);
                }
            }

            void clean()
            {
                for (std::list<statement*>::iterator p = statements->begin(); p != statements->end(); p++)
                {
                    (*p)->clean();
                    assert((*p)->get_status() == unprepared);
                }
            }
    };


    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    class sqlite_database: public database
    {
        protected:
            sqlite3* my_db;

        public:
            sqlite_database(): database(), my_db(NULL)
            {
                set_errno(SQLITE_OK);
            }
            virtual ~sqlite_database() {}

            inline sqlite3* get_db()
            {
                return my_db;
            }

            void connect(const char* file_name, int flags = (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE));
            void disconnect();
            bool backup(const char* file_name, std::string* err);
            bool print_table(const char* table_name);

            inline int64_t last_insert_rowid()
            {
                return static_cast<int64_t>(sqlite3_last_insert_rowid(my_db));
            }
            inline int64_t memory_usage()
            {
                return static_cast<int64_t>(sqlite3_memory_used());
            }
            inline int64_t memory_highwater()
            {
                return static_cast<int64_t>(sqlite3_memory_highwater(false));
            }
            inline const char* lib_version()
            {
                return sqlite3_libversion();
            }


            inline bool sql_execute(const char* sql);
            inline bool sql_simple_get_int(const char* sql, int64_t& return_value);
            inline bool sql_simple_get_float(const char* sql, double& return_value);
            inline bool sql_simple_get_string(const char* sql, std::string& return_value);
            inline bool sql_is_new_db(bool& return_value);
    };


    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    class sqlite_statement: public statement
    {
        protected:
            sqlite_database* my_db;
            sqlite3_stmt* my_stmt;

            inline void sqlite_err(int sqlite_res)
            {
                set_errno(sqlite3_errcode(my_db->get_db()));
                set_errmsg(sqlite3_errmsg(my_db->get_db()));

                //asm("int $3");

#ifdef DEBUG_SQL_ERRORS
                fprintf(stderr, "SoarDB| Unexpected sqlite result!  result = %d. error = %d (%s)\n", sqlite_res, sqlite3_errcode(my_db->get_db()),
                        sqlite3_errmsg(my_db->get_db()));
                fprintf(stderr, "SoarDB|...in SQL statement: %s\n", sql);
#endif
            }
            virtual bool _prep()
            {
                const char* tail;
                bool return_val = false;
                int sqlite_res = sqlite3_prepare_v2(my_db->get_db(), sql, SQLITE_PREP_STR_MAX, &(my_stmt), &tail);
                if (sqlite_res == SQLITE_OK)
                {
                    return_val = true;
                }
                else
                {
                    sqlite_err(sqlite_res);
                }

                return return_val;
            }

            virtual void _reinit()
            {
                sqlite3_reset(my_stmt);
            }

            virtual bool _destroy()
            {
                sqlite3_finalize(my_stmt);
                my_stmt = NULL;

                return true;
            }

            virtual exec_result _exec()
            {
                int sqlite_res = sqlite3_step(my_stmt);
                exec_result return_val = err;

                if ((sqlite_res != SQLITE_OK) &&
                        (sqlite_res != SQLITE_DONE) &&
                        (sqlite_res != SQLITE_ROW))
                {
                    sqlite_err(sqlite_res);
                }
                else
                {
                    return_val = ((sqlite_res == SQLITE_ROW) ? (row) : (ok));
                }

                return return_val;
            }


        public:
            sqlite_statement(sqlite_database* new_db, const char* new_sql, timer* new_query_timer = NULL): statement(new_sql, new_query_timer), my_db(new_db), my_stmt(NULL) {}

            virtual ~sqlite_statement()
            {
                if (my_stmt)
                {
                    _destroy();
                }
            }

            //

            inline void bind_int(int param, int64_t val)
            {
                sqlite3_bind_int64(my_stmt, param, static_cast<sqlite3_int64>(val));
            }

            inline void bind_double(int param, double val)
            {
                sqlite3_bind_double(my_stmt, param, val);
            }

            inline void bind_null(int param)
            {
                sqlite3_bind_null(my_stmt, param);
            }

            inline void bind_text(int param, const char* val)
            {
                sqlite3_bind_text(my_stmt, param, val, SQLITE_PREP_STR_MAX, SQLITE_STATIC);
            }

            //

            inline int64_t column_int(int col)
            {
                return sqlite3_column_int64(my_stmt, col);
            }

            inline double column_double(int col)
            {
                return sqlite3_column_double(my_stmt, col);
            }

            inline const char* column_text(int col)
            {
                return reinterpret_cast<const char*>(sqlite3_column_text(my_stmt, col));
            }

            inline value_type column_type(int col)
            {
                int col_type = sqlite3_column_type(my_stmt, col);
                value_type return_val = null_t;

                switch (col_type)
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

            inline int parameter(const char* paramName)
            {
                return sqlite3_bind_parameter_index(my_stmt, paramName);
            }
            inline int get_parameter_count()
            {
                return sqlite3_bind_parameter_count(my_stmt);
            }
    };


    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    class sqlite_statement_container: public statement_container
    {
        protected:
            std::list<const char*>* structures;

            sqlite_database* my_db;

        public:
            sqlite_statement_container(sqlite_database* new_db): statement_container(), structures(new std::list<const char*>()), my_db(new_db) {}

            virtual ~sqlite_statement_container()
            {
                delete structures;
            }

            //

            inline void add_structure(const char* new_structure)
            {
                structures->push_back(new_structure);
            }

            void structure()
            {
                sqlite_statement* temp_stmt;

                for (std::list<const char*>::iterator p = structures->begin(); p != structures->end(); p++)
                {
                    temp_stmt = new sqlite_statement(my_db, (*p));
                    exec_result execute_result = err;

                    temp_stmt->prepare();
                    assert(temp_stmt->get_status() == ready);
                    execute_result = temp_stmt->execute();
#ifdef DEBUG_SQL_ERRORS
                    if (execute_result == err)
                    {
                        fprintf(stderr, "SoarDB| Unexpected sqlite result in structure!  result = %d. error = %d (%s)\n", execute_result, temp_stmt->get_errno(), temp_stmt->get_errmsg());
                        fprintf(stderr, "SoarDB|...in SQL statement: %s\n", (*p));
                    }
#endif
                    delete temp_stmt;

                }
            }
    };


    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    class sqlite_statement_pool;

    class pooled_sqlite_statement: public sqlite_statement
    {
        protected:
            sqlite_statement_pool* stmt_pool;

        public:
            pooled_sqlite_statement(sqlite_statement_pool* new_pool, sqlite_database* new_db, const char* new_sql, timer* new_query_timer = NULL): sqlite_statement(new_db, new_sql, new_query_timer), stmt_pool(new_pool) {}

            inline sqlite_statement_pool* get_pool()
            {
                return stmt_pool;
            }
    };

    class sqlite_statement_pool
    {
        protected:
#ifdef USE_MEM_POOL_ALLOCATORS
            typedef std::list< pooled_sqlite_statement*, soar_memory_pool_allocator< pooled_sqlite_statement* > > sqlite_statement_pool_pool;
#else
            typedef std::list< pooled_sqlite_statement* > sqlite_statement_pool_pool;
#endif

            sqlite_statement_pool_pool* statements;

            sqlite_database* my_db;
            const char* my_sql;

        public:
            sqlite_statement_pool(agent* thisAgent, sqlite_database* new_db, const char* new_sql): my_db(new_db), my_sql(new_sql)
            {
#ifdef USE_MEM_POOL_ALLOCATORS
                statements = new sqlite_statement_pool_pool(thisAgent);
#else
                statements = new sqlite_statement_pool_pool();
#endif
            }

            ~sqlite_statement_pool()
            {
                for (sqlite_statement_pool_pool::iterator it = statements->begin(); it != statements->end(); it++)
                {
                    delete(*it);
                }

                delete statements;
            }

            void release(pooled_sqlite_statement* stmt)
            {
                stmt->reinitialize();
                statements->push_front(stmt);
            }

            pooled_sqlite_statement* request(timer* query_timer = NULL)
            {
                pooled_sqlite_statement* return_val = NULL;

                if (statements->empty())
                {
                    // make new (assigns timer)
                    return_val = new pooled_sqlite_statement(this, my_db, my_sql, query_timer);

                    // ready to use
                    return_val->prepare();
                }
                else
                {
                    return_val = statements->front();
                    statements->pop_front();

                    // assign timer
                    return_val->set_timer(query_timer);
                }

                return return_val;
            }
    };

    inline bool sqlite_database::sql_simple_get_int(const char* sql, int64_t& return_value)
    {
        soar_module::sqlite_statement* temp_q = new soar_module::sqlite_statement(this, sql);
        temp_q->prepare();
        bool result = (temp_q->execute() == soar_module::row);
        if (result)
        {
            return_value = temp_q->column_int(0);
        }
        delete temp_q;
        return result;
    }

    inline bool sqlite_database::sql_execute(const char* sql)
    {
        soar_module::sqlite_statement* temp_q = new soar_module::sqlite_statement(this, sql);
        exec_result execute_result = err;

        temp_q->prepare();

        execute_result = temp_q->execute();
#ifdef DEBUG_SQL_ERRORS
        if (execute_result == err)
        {
            fprintf(stderr, "SoarDB| Unexpected sqlite result in sql_execute!  result = %d. error = %d (%s)\n", execute_result, this->get_errno(), this->get_errmsg());
            fprintf(stderr, "SoarDB|...in SQL statement: %s\n", sql);
        }
#endif

        delete temp_q;
        return (execute_result == ok);
    }


    inline bool sqlite_database::sql_simple_get_float(const char* sql, double& return_value)
    {
        soar_module::sqlite_statement* temp_q = new soar_module::sqlite_statement(this, sql);
        temp_q->prepare();
        bool result = (temp_q->execute() == soar_module::row);
        if (result)
        {
            return_value = temp_q->column_double(0);
        }
        delete temp_q;
        return result;
    }

    inline bool sqlite_database::sql_simple_get_string(const char* sql, std::string& return_value)
    {
        soar_module::sqlite_statement* temp_q = new soar_module::sqlite_statement(this, sql);
        temp_q->prepare();
        bool result = (temp_q->execute() == soar_module::row);
        if (result)
        {
            return_value.assign(temp_q->column_text(0));
        }
        delete temp_q;
        return result;
    }
    inline bool sqlite_database::sql_is_new_db(bool& return_value)
    {
        int64_t numTables, value_retrieved;

        value_retrieved = sql_simple_get_int("SELECT count(*) FROM sqlite_master WHERE type='table'", numTables);
        if (value_retrieved)
        {
            return_value = (numTables == 0);
        }
        else
        {
            return_value = false;
        }
        return value_retrieved;
    }
}

#endif
