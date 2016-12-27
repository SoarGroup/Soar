/*************************************************************************
 * PLEASE SEE THE FILE "license.txt" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*************************************************************************
 *
 *  file:  database.cpp
 *
 * =======================================================================
 */

#include "output_db.h"

#include "output_manager.h"
#include "output_settings.h"

OM_DB::OM_DB(soar_module::sqlite_database* pDebugDB)
    : soar_module::sqlite_statement_container(pDebugDB)
{
    message_count = 0;
    m_OM = &Output_Manager::Get_OM();
    m_Debug_DB = pDebugDB;

    this->structure();
    this->prepare();
}

OM_DB::~OM_DB()
{
#ifdef DEBUG_REFCOUNT_DB
    compile_refcount_summary();
#endif
}

void OM_DB::create_db()
{
    init_db();

    /* May still want to drop tables even if in memory mode b/c we might be re-initialized.   -- */

    if ((m_OM->m_params->database->get_value() != soar_module::memory) &&
            (m_OM->m_params->append_db->get_value() == off))
    {
        drop_tables();
    }
    create_tables();
    create_indices();

    // Update the schema version number
    add_structure("INSERT OR REPLACE INTO versions (system, version_number) VALUES ('debug_schema'," DEBUG_SCHEMA_VERSION ")");

    create_statements();

}

void OM_DB::create_tables()
{

    add_structure("CREATE TABLE IF NOT EXISTS versions (system TEXT PRIMARY KEY,version_number TEXT)");

    /* -- Tables that store trace messages - */
    add_structure("CREATE TABLE IF NOT EXISTS message_ids (id INTEGER UNIQUE PRIMARY KEY, type INTEGER)");
    add_structure("CREATE TABLE IF NOT EXISTS trace (id INTEGER PRIMARY KEY, module TEXT, message TEXT)");
    add_structure("CREATE TABLE IF NOT EXISTS debug (id INTEGER PRIMARY KEY, module TEXT, message TEXT)");

    /* -- Tables that store reference count messages, final tallies and a problem report -- */
#ifdef DEBUG_REFCOUNT_DB
    add_structure("CREATE TABLE IF NOT EXISTS refcounts (id INTEGER PRIMARY KEY, symbol TEXT, "
                  "callers TEXT, old_ref INTEGER, new_ref INTEGER)");
    //  add_structure( "CREATE TABLE IF NOT EXISTS problems (id INTEGER PRIMARY KEY, symbol TEXT, "
    //                  "callers TEXT, old_ref INTEGER, new_ref INTEGER)" );
    add_structure("CREATE TABLE IF NOT EXISTS problems (id INTEGER, symbol TEXT, "
                  "callers TEXT, old_ref INTEGER, new_ref INTEGER)");
    add_structure("CREATE TABLE IF NOT EXISTS symbols (symbol TEXT PRIMARY KEY, adds INTEGER, removes INTEGER, difference INTEGER)");
#endif
}

void OM_DB::create_indices()
{

    add_structure("CREATE INDEX IF NOT EXISTS message_ids_type ON message_ids (type)");
    add_structure("CREATE INDEX IF NOT EXISTS trace_message ON trace (message)");
    add_structure("CREATE INDEX IF NOT EXISTS trace_module ON trace (module)");
    add_structure("CREATE INDEX IF NOT EXISTS debug_message ON debug (message)");
    add_structure("CREATE INDEX IF NOT EXISTS debug_module ON debug (module)");

#ifdef DEBUG_REFCOUNT_DB
    add_structure("CREATE INDEX IF NOT EXISTS refcounts_symbol ON refcounts (symbol)");
    add_structure("CREATE INDEX IF NOT EXISTS refcounts_caller ON refcounts (callers)");
    add_structure("CREATE INDEX IF NOT EXISTS refcounts_old_ref ON refcounts (old_ref)");
    add_structure("CREATE INDEX IF NOT EXISTS refcounts_new_ref ON refcounts (new_ref)");

    add_structure("CREATE INDEX IF NOT EXISTS refcounts_symbol ON problems (symbol)");
    add_structure("CREATE INDEX IF NOT EXISTS refcounts_caller ON problems (callers)");
    add_structure("CREATE INDEX IF NOT EXISTS refcounts_old_ref ON problems (old_ref)");
    add_structure("CREATE INDEX IF NOT EXISTS refcounts_new_ref ON problems (new_ref)");

    add_structure("CREATE INDEX IF NOT EXISTS refcounts_symbol ON symbols (symbol)");
    add_structure("CREATE INDEX IF NOT EXISTS refcounts_caller ON symbols (callers)");
    add_structure("CREATE INDEX IF NOT EXISTS refcounts_old_ref ON symbols (old_ref)");
    add_structure("CREATE INDEX IF NOT EXISTS refcounts_new_ref ON symbols (new_ref)");
#endif
}

void OM_DB::drop_tables()
{

    add_structure("DROP TABLE IF EXISTS message_ids");
    add_structure("DROP TABLE IF EXISTS versions");
    add_structure("DROP TABLE IF EXISTS trace");
    add_structure("DROP TABLE IF EXISTS debug");
    add_structure("DROP TABLE IF EXISTS refcounts");
    add_structure("DROP TABLE IF EXISTS refcount_summary");
    add_structure("DROP TABLE IF EXISTS symbols");
}
void OM_DB::init_tables()
{
    m_Debug_DB->sql_execute("DELETE FROM message_ids");
    m_Debug_DB->sql_execute("DELETE FROM versions");
    m_Debug_DB->sql_execute("DELETE FROM trace");
    m_Debug_DB->sql_execute("DELETE FROM debug");
    m_Debug_DB->sql_execute("DELETE FROM sqlite_sequence WHERE name='message_ids'");
    m_Debug_DB->sql_execute("DELETE FROM sqlite_sequence WHERE name='versions'");
    m_Debug_DB->sql_execute("DELETE FROM sqlite_sequence WHERE name='trace'");
    m_Debug_DB->sql_execute("DELETE FROM sqlite_sequence WHERE name='debug'");

#ifdef DEBUG_REFCOUNT_DB
    m_Debug_DB->sql_execute("DELETE FROM refcounts");
    m_Debug_DB->sql_execute("DELETE FROM refcount_summary");
    m_Debug_DB->sql_execute("DELETE FROM symbols");
    m_Debug_DB->sql_execute("DELETE FROM sqlite_sequence WHERE name='refcounts'");
    m_Debug_DB->sql_execute("DELETE FROM sqlite_sequence WHERE name='refcount_summary'");
    m_Debug_DB->sql_execute("DELETE FROM sqlite_sequence WHERE name='symbols'");
#endif
}

void OM_DB::create_statements()
{
    begin = new soar_module::sqlite_statement(m_Debug_DB, "BEGIN");
    add(begin);
    commit = new soar_module::sqlite_statement(m_Debug_DB, "COMMIT");
    add(commit);
    rollback = new soar_module::sqlite_statement(m_Debug_DB, "ROLLBACK");
    add(rollback);

    add_message_id = new soar_module::sqlite_statement(m_Debug_DB,
            "INSERT INTO message_ids (id, type) VALUES (?,?)");
    add(add_message_id);
    add_debug_message = new soar_module::sqlite_statement(m_Debug_DB,
            "INSERT INTO debug (id, module, message) VALUES (?,?,?)");
    add(add_debug_message);
    add_trace_message = new soar_module::sqlite_statement(m_Debug_DB,
            "INSERT INTO trace (id, module, message) VALUES (?,?,?)");
    add(add_trace_message);

#ifdef DEBUG_REFCOUNT_DB
    add_refcnt_message = new soar_module::sqlite_statement(m_Debug_DB,
            "INSERT INTO refcounts (id, symbol, callers, old_ref, new_ref) "
            "VALUES (?,?,?,?,?)");
    add(add_refcnt_message);
    add_refcnt_problem = new soar_module::sqlite_statement(m_Debug_DB,
            "INSERT INTO problems (id, symbol, callers, old_ref, new_ref) "
            "VALUES (?,?,?,?,?)");
    add(add_refcnt_problem);
    add_refcnt_totals = new soar_module::sqlite_statement(m_Debug_DB,
            "INSERT INTO symbols (symbol, adds, removes, difference) VALUES (?,?,?,?)");
    add(add_refcnt_totals);
    generate_symbols_seen = new soar_module::sqlite_statement(m_Debug_DB,
            "SELECT DISTINCT symbol FROM refcounts WHERE (symbol != '')");
    add(generate_symbols_seen);
    count_refs = new soar_module::sqlite_statement(m_Debug_DB,
            "select COUNT(*) from refcounts where (callers like ?) and (symbol = ?)");
    add(count_refs);
    get_entries_for_symbol = new soar_module::sqlite_statement(m_Debug_DB,
            "select * from refcounts where (symbol = ?)");
    add(get_entries_for_symbol);
#endif
}

void OM_DB::compile_refcount_summary()
{
    /* -- Compile refcount summary:
     *
     *    (1) Table of all symbols seen with total refcount adds and removed
     *    (2) Table of all functions that added and removed refcounts for problem symbols
     *
     *    A problem symbol is one that has a different number of adds than removes.
     *
     */
    const char* symString, *callers;
    int64_t message_cnt, old_ref, new_ref, add_count, rem_count;

    while (generate_symbols_seen->execute() == soar_module::row)
    {
        symString = generate_symbols_seen->column_text(0);

        /* -- Count total add_refs -- */
        count_refs->bind_text(1, "%add_ref%");
        count_refs->bind_text(2, symString);
        count_refs->execute();
        add_count = count_refs->column_int(0);
        count_refs->reinitialize();

        /* -- Count total remove_refs -- */
        count_refs->bind_text(1, "%remove_ref%");
        count_refs->bind_text(2, symString);
        count_refs->execute();
        rem_count = count_refs->column_int(0);
        count_refs->reinitialize();

        //    if (!add_count && !rem_count)
        //    {
        //      m_OM->print(thisAgent, "Debug| Could not find counts for symbol %s!", symString);
        //      assert(false);
        //    }
        add_refcnt_totals->bind_text(1, symString);
        add_refcnt_totals->bind_int(2, add_count);
        add_refcnt_totals->bind_int(3, rem_count);
        add_refcnt_totals->bind_int(4, (add_count - rem_count));
        add_refcnt_totals->execute(soar_module::op_reinit);

        if (add_count != rem_count)
        {
            /* -- Get refcount trace messages for this problem symbol -- */
            get_entries_for_symbol->bind_text(1, symString);
            while (get_entries_for_symbol->execute() == soar_module::row)
            {
                message_cnt = get_entries_for_symbol->column_int(0);
                callers = get_entries_for_symbol->column_text(2);
                old_ref = get_entries_for_symbol->column_int(3);
                new_ref = get_entries_for_symbol->column_int(4);

                /* -- Copy over all actual refcount trace messages to problem database -- */
                add_refcnt_problem->bind_int(1, message_cnt);
                add_refcnt_problem->bind_text(2, symString);
                add_refcnt_problem->bind_text(3, callers);
                add_refcnt_problem->bind_int(4, old_ref);
                add_refcnt_problem->bind_int(5, new_ref);
                add_refcnt_problem->execute(soar_module::op_reinit);
            }
            get_entries_for_symbol->reinitialize();
        }
    }
    generate_symbols_seen->reinitialize();
    close_db();
}


void OM_DB::clear()
{
    init_tables();
    message_count = 0;
}

void OM_DB::switch_to_memory_db(std::string& buf)
{
    m_OM->print(buf.c_str());
    m_OM->m_params->database->set_value(soar_module::memory);
    m_Debug_DB->disconnect();
    init_db();
}

void OM_DB::close_db()
{
    if (m_Debug_DB->get_status() == soar_module::connected)
    {
        m_OM->debug_print_sf(DT_DEBUG, "Closing database %s.\n", m_OM->m_params->path->get_value());
        // if lazy, commit
        if (m_OM->m_params->lazy_commit->get_value() == on)
        {
            commit->execute(soar_module::op_reinit);
        }

        m_Debug_DB->disconnect();
    }

}

void OM_DB::init_db()
{
    bool saved_db_mode = m_OM->db_mode;
    m_OM->db_mode = false;

    if (m_Debug_DB->get_status() != soar_module::disconnected)
    {
        m_OM->print("ERROR:  Cannot initialize debug database.  It is already connected!");
        m_OM->db_mode = saved_db_mode;
        return;
    }

    const char* db_path;
    if (m_OM->m_params->database->get_value() == soar_module::memory)
    {
        db_path = ":memory:";
        m_OM->print("Initializing debug database in cpu memory.\n");
    }
    else
    {
        db_path = m_OM->m_params->path->get_value();
        m_OM->print_sf("Initializing debug database at %s\n", db_path);
    }

    // attempt connection
    m_Debug_DB->connect(db_path);

    if (m_Debug_DB->get_status() == soar_module::problem)
    {
        m_OM->print_sf("Database Error: %s\n", m_Debug_DB->get_errmsg());
        /* -  Return and leaved db modes off -- */
        return;
    }
    else
    {
        soar_module::sqlite_statement* temp_q = NULL;

        // If the database is on file, make sure the database contents use the current schema
        // If it does not, switch to memory-based database

        if (strcmp(db_path, ":memory:")) // Only worry about database version if writing to disk
        {
            bool switch_to_memory, sql_is_new;
            std::string schema_version, version_error_message;

            switch_to_memory = true;

            if (m_Debug_DB->sql_is_new_db(sql_is_new))
            {
                if (sql_is_new)
                {
                    switch_to_memory = false;
                    m_OM->debug_print(DT_DEBUG, " ...debug database is new.\n");
                }
                else
                {
                    // Check if table exists already
                    temp_q = new soar_module::sqlite_statement(m_Debug_DB, "CREATE TABLE IF NOT EXISTS versions (system TEXT PRIMARY KEY,version_number TEXT)");
                    temp_q->prepare();
                    if (temp_q->get_status() == soar_module::ready)
                    {
                        if (m_Debug_DB->sql_simple_get_string("SELECT version_number FROM versions WHERE system = 'debug_schema'", schema_version))
                        {
                            if (schema_version != DEBUG_SCHEMA_VERSION)   // Incompatible version
                            {
                                version_error_message.assign("...Error:  Cannot load debug database with schema version ");
                                version_error_message.append(schema_version.c_str());
                                version_error_message.append(".\n...Please convert old database or start a new database by "
                                                             "setting a new database file path.\n...Switching to memory-based database.\n");
                            }
                            else     // Version is OK
                            {
                                m_OM->debug_print(DT_DEBUG, "...version of debug database ok.\n");
                                switch_to_memory = false;
                            }

                        }
                        else     // Some sort of error reading version info from version database
                        {
                            version_error_message.assign("...Error:  Cannot read version number from file-based debug database.\n"
                                                         "...Switching to memory-based database.\n");
                        }
                    }
                    else     // Non-empty database exists with no version table.  Probably schema 1.0
                    {
                        version_error_message.assign("...Error:  Cannot load an debug database with an old schema version.\n...Please convert "
                                                     "old database or start a new database by setting a new database file path.\n...Switching "
                                                     "to memory-based database.\n");
                    }
                    delete temp_q;
                    temp_q = NULL;
                }
            }
            else
            {
                version_error_message.assign("...Error:  Cannot read database meta info from file-based debug database.\n"
                                             "...Switching to memory-based database.\n");
            }
            if (switch_to_memory)
            {
                // Memory mode will be set on, database will be disconnected to and then init_db
                // will be called again to reinitialize database.
                switch_to_memory_db(version_error_message);
            }
        }
        if (m_OM->m_params->lazy_commit->get_value() == on)
        {
            begin->execute(soar_module::op_reinit);
        }
        m_OM->db_mode = saved_db_mode;
    }
}

void OM_DB::increment_message_count(MessageType msgType)
{
    message_count++;
    add_message_id->bind_int(1, message_count);
    add_message_id->bind_int(2, int(msgType));
    add_message_id->execute(soar_module::op_reinit);
}

void OM_DB::store_refcount(Symbol* sym, const char* callers, bool isAdd)
{
    increment_message_count(refcnt_msg);
    //  print_sf("Storing refcount %d %y %s\n", message_count, sym, callers);
    add_refcnt_message->bind_int(1, message_count);
    //  add_refcnt_message->bind_text( 2, sym->to_string());
    add_refcnt_message->bind_text(3, callers);
    //  add_refcnt_message->bind_int( 4, sym->reference_count );
    //  add_refcnt_message->bind_int( 5, (isAdd ? (sym->reference_count + 1) : (sym->reference_count - 1)));
    add_refcnt_message->execute(soar_module::op_reinit);
}


void OM_DB::print_db(MessageType msgType, const char* prefix, const char* msg)
{

    soar_module::sqlite_statement* target_statement = NIL;

    if (m_Debug_DB->get_status() == soar_module::connected)
    {
        increment_message_count(msgType);
        //print_sf(thisAgent, "Inserting msg %d %s| %s\n", message_count, mode_to_prefix(mode), msg);

        if (msgType == trace_msg)
        {
            target_statement = add_trace_message;
        }
        else if (msgType == debug_msg)
        {
            target_statement = add_debug_message;
        }

        if (target_statement)
        {
            target_statement->bind_int(1, message_count);
            target_statement->bind_text(2, prefix);
            target_statement->bind_text(3, msg);
            target_statement->execute(soar_module::op_reinit);
        }
    }
}
