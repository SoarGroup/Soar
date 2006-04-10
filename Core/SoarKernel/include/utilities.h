/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/* utilities.h */

#ifndef UTILITIES_H
#define UTILITIES_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <list>
#include "stl_support.h"

////////////////////////////////
// Returns a list of wmes that share a specified id
// The tc number of the id is checked against the tc number passed in
// If they match, then NULL is returned
// Otherwise the tc number of the id is set to the specified tc number
// To guarantee any existing wmes will be returned, use get_new_tc_num()
//  for the tc parameter
////////////////////////////////
extern SoarSTLWMEPoolList* get_augs_of_id(agent* thisAgent, Symbol * id, tc_number tc);

#ifdef __cplusplus
}
#endif

#endif //UTILITIES_H
