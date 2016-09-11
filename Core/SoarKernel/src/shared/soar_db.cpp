/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

#include "soar_db.h"

/* ---------------- SQL Callback functions for sql debug modes-------------- */

#ifdef DEBUG_SQL_PROFILE
static void profile_sql(void* context, const char* sql, sqlite3_uint64 ns)
{
    fprintf(stderr, "Execution Time of %llu ms for: %s\n", ns / 1000000, sql);
}
#endif
#ifdef DEBUG_SQL_QUERIES
static void trace_sql(void* /*arg*/, const char* query)
{
    fprintf(stderr, "Query: %s\n", query);
}
#endif

namespace soar_module
{
    void sqlite_database::connect(const char* file_name, int flags)
    {
        int sqlite_err = sqlite3_open_v2(file_name, &(my_db), flags, NULL);
        
        if (sqlite_err == SQLITE_OK)
        {
            set_status(connected);
            
            set_errno(sqlite_err);
            set_errmsg(NULL);
            
            #ifdef DEBUG_SQL_PROFILE
                sqlite3_profile(my_db, &profile_sql, NULL);
            #endif
            #ifdef DEBUG_SQL_QUERIES
                sqlite3_trace(my_db, trace_sql, NULL);
            #endif
        }
        else
        {
            set_status(problem);
            
            set_errno(sqlite_err);
            set_errmsg(sqlite3_errmsg(my_db));
        }
    }
    
    void sqlite_database::disconnect()
    {
        if (get_status() == connected)
        {
            sqlite3_close(my_db);
            set_status(disconnected);
        }
    }
    
    bool sqlite_database::backup(const char* file_name, std::string* err)
    {
        sqlite3* backup_db;
        bool return_val = false;
        
        err->clear();
        
        if (get_status() == connected)
        {
            int sqlite_err = sqlite3_open_v2(file_name, &(backup_db), (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE), NULL);
            if (sqlite_err == SQLITE_OK)
            {
                sqlite3_backup* backup_h = sqlite3_backup_init(backup_db, "main", my_db, "main");
                if (backup_h)
                {
                    sqlite3_backup_step(backup_h, -1);
                    sqlite3_backup_finish(backup_h);
                }
                
                if (sqlite3_errcode(backup_db) == SQLITE_OK)
                {
                    return_val = true;
                }
                else
                {
                    err->assign("Error during backup: ");
                    err->append(sqlite3_errmsg(backup_db));
                }
            }
            else
            {
                err->assign("Error opening backup file: ");
                err->append(sqlite3_errmsg(backup_db));
            }
            sqlite3_close(backup_db);
        }
        else
        {
            err->assign("Database is not currently connected.");
        }
        
        return return_val;
    }
    
    bool sqlite_database::print_table(const char* table_name)
    {
        sqlite3_stmt* statement;
        std::string query;
        query.assign("select * from ");
        query.append(table_name);
        
        if (sqlite3_prepare(my_db, query.c_str(), -1, &statement, 0) == SQLITE_OK)
        {
            int ctotal = sqlite3_column_count(statement);
            int res = 0;
            const char* val;
//                  std::string val;
            
            fprintf(stderr, "----------------------------\n%s\n----------------------------\n", table_name);
            while (1)
            {
                res = sqlite3_step(statement);
                
                if (res == SQLITE_ROW)
                {
                    for (int i = 0; i < ctotal; i++)
                    {
//                              val.assign((const char *)sqlite3_column_text(statement, i));
//                              if (val)
//                                  fprintf(stderr, "%s ", val.c_str());
                        val = reinterpret_cast<const char*>(sqlite3_column_text(statement, i));
                        if (val)
                        {
                            fprintf(stderr, "%s ", val);
                        }
                        else
                        {
                            fprintf(stderr, "NULL ");
                        }
                    }
                    fprintf(stderr, "\n");
                }
                else if (res == SQLITE_DONE)
                {
                    fprintf(stderr, "Done.\n");
                    return true;
                    break;
                }
                else if (res == SQLITE_ERROR)
                {
                    fprintf(stderr, "{print_table error %d: %s\n", res, this->get_errmsg());
                }
            }
        }
        return false;
    }
}
