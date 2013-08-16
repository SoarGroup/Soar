/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION.
 *************************************************************************/

/*------------------------------------------------------------------
					   debug.cpp

   @brief debug.cpp provides some utility functions for inspecting and
   	   	  manipulating the data structures of the Soar kernel at run
   	   	  time.

   	   	  (Not much here now.  Will move some other utility stuff from
   	   	   experimental chunking and memory consolidation branches
   	   	   later.)

------------------------------------------------------------------ */

#include "debug.h"

#ifdef SOAR_DEBUG_UTILITIES
#include "soar_db.h"
#include "gsysparam.h"
#include "sqlite3.h"
#include <typeinfo>

soar_module::sqlite_database	*db_err_epmem_db=NULL,
								*db_err_smem_db=NULL;
agent *debug_agent=NULL;
#define DEBUG_BUFFER_SIZE 5000

// The following functions use a global debug_agent variable so that they can be called from gdb at any time
// after the global is initialized

// gdb can't seem to find this function because of the variadic parameters.  Figure out later.

extern void debug_print(const char *format, ...)
{
	va_list args;
	char debug_buffer[DEBUG_BUFFER_SIZE];

	va_start (args, format);
	vsprintf (debug_buffer, format, args);
	va_end (args);
	print (debug_agent, debug_buffer);
	if (debug_buffer[strlen(debug_buffer)-1] != '\n')
	{
		print(debug_agent, "\n");
	}
	print(debug_agent, "Type is %s", typeid(debug_agent).name());
}

// gdb can't find this function at all.  Figure out later.

template <typename T> extern void dp2(T item)
{
	print(debug_agent, "Type is %s", typeid(item).name());

}

// test function for typeid
extern void dp()
{
	print(debug_agent, "Type is %s", typeid(debug_agent).name());
}

extern void debug_print_db_err()
{
	if (debug_agent)
	{
		print_trace (debug_agent,0, "Debug| Printing database status/errors...\n");
		if (debug_agent->debug_params->epmem_commands->get_value() == soar_module::on)
		{
			if (!db_err_epmem_db)
			{
				print_trace (debug_agent,0, "Debug| Cannot access epmem database because wmg not yet initialized.\n");
			}
			else
			{
				print_trace (debug_agent,0, "Debug| EpMem DB: %d - %s\n", sqlite3_errcode( db_err_epmem_db->get_db() ),
						sqlite3_errmsg( db_err_epmem_db->get_db() ));
			}
		}
		if (debug_agent->debug_params->smem_commands->get_value() == soar_module::on)
		{
			if (!db_err_smem_db)
			{
				print_trace (debug_agent,0, "Debug| Cannot access smem database because wmg not yet initialized.\n");
			}
			else
			{
				print_trace (debug_agent,0, "Debug| SMem DB: %d - %s\n", sqlite3_errcode( db_err_smem_db->get_db() ),
						sqlite3_errmsg( db_err_smem_db->get_db() ));
			}
		}
	}
}

extern void debug_print_epmem_table(const char *table_name)
{
	if (debug_agent)
	{
		if (!db_err_epmem_db)
		{
			if ((debug_agent->epmem_db) && ( debug_agent->epmem_db->get_status() == soar_module::connected ))
			{
				db_err_epmem_db = debug_agent->epmem_db;
				debug_agent->debug_params->epmem_commands->set_value(soar_module::on);
			}
			else
			{
				print_trace (debug_agent,0, "Debug| Cannot access epmem database because database not yet initialized.\n");
				return;
			}
		}

		db_err_epmem_db->print_table(table_name);
	}
}

extern void debug_print_smem_table(const char *table_name)
{
	if (debug_agent)
	{
		if (!db_err_smem_db)
		{
			if (debug_agent->smem_db && ( debug_agent->smem_db->get_status() == soar_module::connected ))
			{
				db_err_smem_db = debug_agent->smem_db;
				debug_agent->debug_params->smem_commands->set_value(soar_module::on);
			}
			else
			{
				print_trace (debug_agent,0, "Debug| Cannot access smem database because database not yet initialized.\n");
				return;
			}
		}
		db_err_smem_db->print_table(table_name);
	}
}

extern void debug_init_db( agent *my_agent)
{
	if (!debug_agent)
	{
		debug_agent = my_agent;
	}
	else
	{
		if ((my_agent->debug_params->epmem_commands->get_value() == soar_module::on) && ( my_agent->epmem_db->get_status() == soar_module::disconnected ))
		{
			epmem_init_db( my_agent );
			db_err_epmem_db = my_agent->epmem_db;
		}

		if ((my_agent->debug_params->smem_commands->get_value() == soar_module::on) && ( my_agent->smem_db->get_status() == soar_module::disconnected ))
		{
			smem_init_db( my_agent );
			db_err_smem_db = my_agent->smem_db;
		}
	}
}

#endif
