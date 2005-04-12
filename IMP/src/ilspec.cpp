#include "ilspec.h"
#include <fstream>

/******************************************************************************
 * InputLinkSpec Class Function Definitions
 *
 *
 ******************************************************************************
 */

/* ImportDM
 *
 * This function creates an input link specification from the datamap
 * contained within "filename"
 * Returns true on success, false on failure.
 */
bool InputLinkSpec::ImportDM(string filename)
{
	fstream file;
	file.open(filename.c_str());



	file.close();
	return true;
}

/* ImportIL
 *
 * This function creates an input link specification from the IL file passed.
 * Returns true on success, false on failure.
 */
bool InputLinkSpec::ImportIL(string filename)
{
	fstream file;
	file.open(filename.c_str());



	file.close();
	return true;
}

