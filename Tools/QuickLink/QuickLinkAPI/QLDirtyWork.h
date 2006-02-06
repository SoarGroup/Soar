/*****************************************************
*  Author: Taylor Lafrinere
*  
*  Soar QuickLink
*
*  QLDirtyWork.h
*
*  The functions in this file control all of the printing
*  and behind-the-scenes-non-wme-manipulation functions
*
*  Start Date: 08.24.2005
*
*****************************************************/

#ifndef QLDIRTYWORK_H
#define QLDIRTYWORK_H

#include <fstream>
#include "QLMemory.h"

class QLDirtyWork 
{
public:

	/***Constructors***/
	QLDirtyWork( QLMemory* NewMem );

	/***Public Methods to be used by QLInterface***/

	bool SaveInput( ofstream& tempFile );  
	//saves input to a file

private:

	/***Member Data***/
	QLMemory* Memory;
};
#endif