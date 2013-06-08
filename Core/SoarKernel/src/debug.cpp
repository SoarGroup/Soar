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
#include "soar_db.h"
#include "gsysparam.h"
#include "sqlite3.h"

soar_module::sqlite_database	*db_err_epmem_db=NULL,
								*db_err_smem_db=NULL;

extern void debug_print_db_err()
{
	if (!db_err_epmem_db)
	{
		fprintf(stderr, "MemCon| Cannot access epmem database because wmg not yet initialized.\n");
	}
	else
	{
		fprintf(stderr, "MemCon| SQL Error #%d: %s\n", sqlite3_errcode( db_err_epmem_db->get_db() ),
													   sqlite3_errmsg( db_err_epmem_db->get_db() ));
	}
	if (!db_err_smem_db)
	{
		fprintf(stderr, "MemCon| Cannot access smem database because wmg not yet initialized.\n");
	}
	else
	{
		fprintf(stderr, "MemCon| SQL Error #%d: %s\n", sqlite3_errcode( db_err_smem_db->get_db() ),
													   sqlite3_errmsg( db_err_smem_db->get_db() ));
	}
}

extern void debug_init_db( agent *my_agent)
{
	if ( my_agent->epmem_db->get_status() == soar_module::disconnected )
	{
		epmem_init_db( my_agent );
	}

	if ( my_agent->smem_db->get_status() == soar_module::disconnected )
	{
		smem_init_db( my_agent );
	}

	db_err_epmem_db = my_agent->epmem_db;
	db_err_smem_db = my_agent->smem_db;
}
