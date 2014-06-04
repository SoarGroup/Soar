/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  output_manager_params.h
 *
 * =======================================================================
 */

#ifndef OUTPUT_MANAGER_DB_H_
#define OUTPUT_MANAGER_DB_H_

#include "soar_db.h"

class Output_Manager;

class OM_DB: public soar_module::sqlite_statement_container
{
  public:

    OM_DB(soar_module::sqlite_database *pDebugDB);
    virtual ~OM_DB();

    void print_db(MessageType msgType, const char *prefix, const char *msg);
    void create_db();
    void store_refcount(Symbol *sym, const char *trace, bool isAdd);

  private:
    soar_module::sqlite_database    *m_Debug_DB;
    Output_Manager                  *m_OM;

    soar_module::sqlite_statement   *begin, *commit, *rollback;
    soar_module::sqlite_statement   *add_message_id;
    soar_module::sqlite_statement   *add_debug_message;
    soar_module::sqlite_statement   *add_trace_message;
    soar_module::sqlite_statement   *add_refcnt_message;
    soar_module::sqlite_statement   *add_refcnt_problem;
    soar_module::sqlite_statement   *generate_symbols_seen;
    soar_module::sqlite_statement   *count_refs;
    soar_module::sqlite_statement   *add_refcnt_totals;
    soar_module::sqlite_statement   *get_entries_for_symbol;

    int64_t                         message_count;

    void create_tables();
    void create_indices();
    void create_statements();
    void drop_tables();
    void init_tables();
    void clear();
    void init_db();
    void close_db();
    void switch_to_memory_db(std::string& buf);

    void compile_refcount_summary();

    void increment_message_count(MessageType msgType);

};

#endif /* OUTPUT_MANAGER_DB_H_ */
