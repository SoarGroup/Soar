#ifndef COMMAND_IDS_H
#define COMMAND_IDS_H

/* Command IDs */
/* The ranges are not important to the software */
/* but may make debugging easier. */

/* From Tool to Runtime 1-1000 */
const long STI_kSendProduction		= 1 ;
const long STI_kSendFile			= 2 ;
const long STI_kProductionMatches	= 3 ;
const long STI_kSendRawCommand		= 4 ;
const long STI_kExciseProduction	= 5 ;

/* From Runtime to Tool 1001-2000 */
const long STI_kEditProduction	  = 1001 ;

/* System messages 2001-3000 */
const long STI_kSystemNameCommand = 2001 ;

#endif /* COMMAND_IDS_H */
