/*****************************************************
*  Author: Taylor Lafrinere
*  
*  Soar QuickLink
*
*  QLMemory.h
*
*  The functions in this file are called by QLInterface 
*  and they control the memory mainpulation of QL.
*
*  Start Date: 08.24.2005
*
*****************************************************/

#ifndef QLMEMORY_H
#define QLMEMORY_H

#include <vector>
#include "sml_Client.h"

using namespace std;

class QLMemory 
{
	friend class QLDirtyWork;  //DirtyWork needs access to memory

public:

	/***Constructors***/
	QLMemory( sml::Kernel* kernel , sml::Agent* agent , sml::Identifier* IL , sml::Identifier* OL );

	/***Methods to be used by QLInterface***/

	bool ClearAll();
	//clears input link structure

	string CreateId( string parent, string attribute , string value );
	//creates identifier

	string DetermineValueAndCreate( string parent, string attribute , string value );
	//determines type of value and creates it

	string DetermineValueAndChange( string parent , string attribute , string oldValue , string newValue);
	//determines type of value and changes it

	string DeleteId( string parent, string attribute , string value );
	//deletes specified identifier

	string DetermineValueAndDelete( string parent, string attribute , string value );
	//used to delete non-identifier wme's

	bool PurgeMemory();
	//cleans up memory before exiting

	bool ClearProcessMem();
	//clear the process memory

	string GetInput();
	//converts input stored in vectors to a string



private:

	sml::Kernel* pKernel;
	sml::Agent* pAgent;
	sml::Identifier* pOutputLink;
	sml::Identifier* pInputLink;

	/***Storage for Current Input-Link Structure***/
	vector<sml::Identifier*> IDs;
	vector<string> IDnames;
	vector<string> IDparent;
	vector<string> IDsoar;
	vector<bool> IDprint;
	vector<sml::StringElement*> SEs;
	vector<string> SEnames;
	vector<string> SEparent;
	vector<string> SEvalue;
	vector<bool> SEprint;
	vector<sml::Identifier*> Shared;
	vector<string> SharedNames;
	vector<string> SharedParent;
	vector<string> SharedValue;
	vector<string> SharedPrint;
	vector<sml::IntElement*> IEs;
	vector<string> IEnames;
	vector<string> IEparent;
	vector<string> IEvalue;
	vector<bool> IEprint;
	vector<sml::FloatElement*> FEs;
	vector<string> FEnames;
	vector<string> FEparent;
	vector<string> FEvalue;
	vector<bool> FEprint;

	/***Storage for Printing Output***/
	struct triple
	{
		string name;
		string att;
		string val;
		bool printed;
	};

	vector<triple> storeO;

	/***Process Memory***/
	vector<string> ProcessMemory;

	/***Memory Constants***/
	string _MEM_NO_PARENT , _MEM_ALREADY_EXISTS , _MEM_ADD_SUCCESS, _MEM_SOMETHING_DOESNT_EXIST;
	string _MEM_CHANGE_SUCCESS , _MEM_DELETE_SUCCESS;

	/***Member Methods***/
	string CreateIntWME( string parent , string attribute , int value , string Svalue );
	//creates an int wme

	string CreateFloatWME( string parent , string attribute , float value , string Svalue );
	//creates a float wme

	string CreateStringWME( string parent , string attribute , string value );
	//creates a string wme

	int FindParentIndex( string parent );
	//finds the parent index

	void DeleteChildren( string flag , string father , string always );
	//deletes the children of an identifier

	void DestroyIdentifier( string flag , int index );
	//takes care of destruction of wme

	void DestroySharedId( string flag , int index );
	//takes care of destruction of wme

	void DestroyIntElement( string flag , int index );
	//takes care of destruction of wme

	void DestroyStringElement( string flag , int index );
	//takes care of destruction of wme

	void DestroyFloatElement( string flag , int index );
	//takes care of destruction of wme




	
};
#endif