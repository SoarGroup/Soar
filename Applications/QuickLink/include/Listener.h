/*****************************************************
*  Taylor Lafrinere
*  
*  Soar Quick Link
*
*  Listener.h
*
*  This class is used to listen to the input link and
*  store changes as a process for QuickLink
*
*  Start Date: 08.24.2005
*
*****************************************************/

#ifndef LISTENER_H
#define LISTENER_H

#include <iostream>
#include "sml_Client.h"

using namespace std;

class Listener 
{
public:

	//Constructor
	Listener( sml::Kernel* kernel , sml::Agent* agent );

	void printWMEs( sml::WMElement const* pRoot );
	//Taken from TestClientSML

private:

	//SML Variables
	sml::Kernel* pKernel;
	sml::Agent* pAgent;
	sml::Identifier* pInputLink;
};

#endif