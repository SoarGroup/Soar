/*****************************************************
*  Taylor Lafrinere
*  
*  Soar Quick Link
*
*  Listener.cpp
*
*  This class is used to listen to the input link and
*  store changes as a process for QuickLink
*
*  Start Date: 08.24.2005
*
*****************************************************/

#include "Listener.h"

using namespace std;

Listener::Listener( sml::Kernel* kernel , sml::Agent* agent )
{
	pKernel = kernel;
	pAgent = agent;
	bool synch = pAgent->SynchronizeInputLink();
	
	//check to make sure synch worked
	if(!synch)
		cout << "Synchronization did not work." << endl << endl;

	printWMEs( pAgent->GetInputLink() );

	
}

void
Listener::printWMEs( sml::WMElement const* pRoot )
{
	if (pRoot->GetParent() == NULL)
		cout << "Top Identifier " << pRoot->GetValueAsString() << endl ;
	else
	{
		cout << "(" << pRoot->GetParent()->GetIdentifierSymbol() << " ^" << pRoot->GetAttribute() << " " << pRoot->GetValueAsString() << ")" << endl ;
	}

	if (pRoot->IsIdentifier())
	{
		sml::Identifier* pID = (sml::Identifier*)pRoot ;
		int size = pID->GetNumberChildren() ;

		for (int i = 0 ; i < size ; i++)
		{
			sml::WMElement const* pWME = pID->GetChild(i) ;

			printWMEs(pWME) ;
		}
	}
}
