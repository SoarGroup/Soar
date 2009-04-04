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
#include "sqlite3.h"
#include "agent.h"

#include <assert.h>

namespace soar_module
{
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