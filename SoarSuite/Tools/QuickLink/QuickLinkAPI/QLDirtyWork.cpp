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

#include "QLDirtyWork.h"

QLDirtyWork::QLDirtyWork( QLMemory* NewMem )
{
	Memory = NewMem;
}

bool
QLDirtyWork::SaveInput( ofstream& oFile )
{
	bool toReturn = true;
	if(!oFile)
	{
		toReturn = false;
	}
	else //write to file
	{	
		for(unsigned int i =0; i< Memory->IDs.size(); i++)  // all identifiers
		{
			oFile << "add " << Memory->IDparent[i] << " ^" << Memory->IDnames[i] << " /" << Memory->IDsoar[i] << endl;
		}
		for(unsigned int i =0; i< Memory->Shared.size(); i++) // all shared ids
		{
			oFile << "add " << Memory->SharedParent[i] << " ^" << Memory->SharedNames[i] << " /" << Memory->SharedValue[i] << endl;
		}
		for(unsigned int i =0; i< Memory->IEs.size(); i++)  // all int elements
		{
			oFile << "add " << Memory->IEparent[i] << " ^" << Memory->IEnames[i] << " " << Memory->IEvalue[i] << endl;
		}
		for(unsigned int i =0; i< Memory->FEs.size(); i++) // all float elements
		{
			oFile << "add " << Memory->FEparent[i] << " ^" << Memory->FEnames[i] << " " << Memory->FEvalue[i] << endl;
		}
		for(unsigned int i =0; i< Memory->SEs.size(); i++) // all string elements
		{
			oFile << "add " << Memory->SEparent[i] << " ^" << Memory->SEnames[i] << " " << Memory->SEvalue[i] << endl;
		}
	}

	return toReturn;
}
