/*****************************************************
*  Author: Taylor Lafrinere
*  
*  Soar QuickLink
*
*  QLMemory.cpp
*
*  The functions in this file are called by QLInterface 
*  and they control the memory mainpulation of QL.
*
*  Start Date: 08.24.2005
*
*****************************************************/

#include "QLMemory.h"

QLMemory::QLMemory( sml::Kernel* kernel , sml::Agent* agent , sml::Identifier* IL , sml::Identifier* OL )
{
	pKernel = kernel;
	pAgent = agent;
	pInputLink = IL;
	pOutputLink = OL;

	_MEM_ADD_SUCCESS = "WME added successfully.";
	_MEM_CHANGE_SUCCESS = "WME changed successfully.";
	_MEM_NO_PARENT = "ERROR: Parent name not found.";
	_MEM_ALREADY_EXISTS = "ERROR: WME already exists.";
	_MEM_SOMETHING_DOESNT_EXIST = "ERROR: Either the parent, name or value specified does not exist.";
	_MEM_DELETE_SUCCESS = "WME deleted successfully.";
}

bool
QLMemory::ClearAll()
{
	DeleteChildren( "CLEAR" , "IL" , "IL" );  //deletes children on input-link
	if(IDs.size() > 0 || FEs.size() > 0 || SEs.size() > 0 || IEs.size() > 0) //used to guarantee clear
		DeleteChildren( "CLEAR" , "IL" , "IL" );

	return true;
}

string
QLMemory::CreateId( string parent, string attribute , string value )
{
	int sharedId = -1;
	bool tester = true;
	string toReturn = _MEM_ADD_SUCCESS;
	for(unsigned int i = 0; i < IDs.size() && tester; i++)  //checks to see if element already exists
	{
		if(IDparent[i] == parent && IDnames[i] == attribute && IDsoar[i] == value)
		{
			tester = false;
			toReturn = _MEM_ALREADY_EXISTS; //element not added because it already exists
		}
		if(IDsoar[i] == value)
			sharedId = i;  //checks to see if this uniqid already exists
	}
	if(tester)	//element should be added
	{
		if(parent == "IL")
		{
			if(sharedId != -1)  //If this needs to be a sharedIdWme
			{
				sml::Identifier* temp1;
				temp1 = pAgent->CreateSharedIdWME(pInputLink, attribute.c_str(), IDs[sharedId]);
				SharedParent.push_back(parent);  
				SharedNames.push_back(attribute);	 
				SharedValue.push_back(value);
				Shared.push_back(temp1);
			}
			else //regular identifier
			{
				sml::Identifier* temp1;
				temp1 = pAgent->CreateIdWME(pInputLink, attribute.c_str());
				IDnames.push_back(attribute);
				IDparent.push_back(parent);
				IDsoar.push_back(value);
				IDs.push_back(temp1);
			}

		}
		else //has parent other than input-link
		{
			int iI = -1;
			for(unsigned int i = 0; i < IDs.size(); i++)  //find the parent
			{
				if(IDsoar[i] == parent)
					iI = i;
			}
			if(iI == -1)  //parent doesn't exist
			{
				toReturn = _MEM_NO_PARENT;
				/*cout << endl << "ERROR: Parent name not found!" << endl << endl;
				WhenReady();*/
			}
			else //parent does exist
			{
				if(sharedId != -1)  //If this needs to be a sharedIdWme
				{
					sml::Identifier* temp1;
					temp1 = pAgent->CreateSharedIdWME(IDs[iI], attribute.c_str(), IDs[sharedId]);
					SharedParent.push_back(parent);  
					SharedNames.push_back(attribute);	 
					SharedValue.push_back(value);
					Shared.push_back(temp1);
				}
				else
				{
					sml::Identifier* temp1;
					temp1 = pAgent->CreateIdWME(IDs[iI], attribute.c_str());
					IDnames.push_back(attribute);
					IDparent.push_back(parent);
					IDsoar.push_back(value);
					IDs.push_back(temp1);
				}

			}					
		}
	}
	return toReturn;
}

string
QLMemory::DetermineValueAndCreate( string parent, string attribute , string value )
{
	bool mFloat = false;
	for(unsigned int i = 0; i < value.size(); i++)
	{
		if (value[i] == '.')  //if it has a period it might be a float, not and int
			mFloat = true;
	}
	if(mFloat && (isdigit(value[0]) || isdigit(value[1]))) //value is float
	{
		float Fvalue = static_cast<float>(atof(value.c_str()));
		bool tester = true;
		for(unsigned int i = 0; i < FEs.size() && tester; i++)  //checks to see if element already exists
		{
			if(FEparent[i] == parent && FEnames[i] == attribute && atof(FEvalue[i].c_str()) == Fvalue)
				return _MEM_ALREADY_EXISTS;
		}
		if(tester)	
			return CreateFloatWME( parent , attribute , Fvalue , value);				
	}
	else if(isdigit(value[0])) //value is int
	{
		int Ivalue = atoi(value.c_str());
		bool tester = true;
		for(unsigned int i = 0; i < IEs.size() && tester; i++)  //checks to see if element already exists
		{
			if(IEparent[i] == parent && IEnames[i] == attribute && atoi(IEvalue[i].c_str()) == Ivalue)
				return _MEM_ALREADY_EXISTS;
		}
		if(tester)
			return CreateIntWME( parent , attribute , Ivalue , value );
	}
	else //value is string
	{
		bool tester = true;
		for(unsigned int i = 0; i < SEs.size() && tester; i++)  //checks to see if element already exists
		{
			if(SEparent[i] == parent && SEnames[i] == attribute && SEvalue[i] == value)
				return _MEM_ALREADY_EXISTS;
		}
		if(tester)	
			return CreateStringWME( parent , attribute , value );
	}
	return "UNKNOWN ERROR.";
}

int
QLMemory::FindParentIndex( string parent )
{
	int pI = -1;

	for(unsigned int i = 0; i < IDs.size(); i++)  //find parent
	{
		if(IDsoar[i] == parent)
			pI = i;
	}

	return pI;
}

string
QLMemory::CreateIntWME( string parent , string attribute , int value , string Svalue )
{
	sml::IntElement* temp3;
	int pI = FindParentIndex( parent );
	if(pI == -1 && parent != "IL") //parent doesn't exist
		return _MEM_NO_PARENT;
	else
	{
		IEnames.push_back( attribute );
		IEvalue.push_back( Svalue );
		IEparent.push_back( parent );
		if(parent == "IL")
			temp3 = pAgent->CreateIntWME( pInputLink , attribute.c_str() , value );
		else
			temp3 = pAgent->CreateIntWME( IDs[pI], attribute.c_str() , value );

		IEs.push_back(temp3);
	}
	return _MEM_ADD_SUCCESS;
}

string
QLMemory::CreateFloatWME( string parent , string attribute , float value , string Svalue )
{
	sml::FloatElement* temp4;
	int pI = FindParentIndex( parent );
	if(pI == -1 && parent != "IL") //parent doesn't exist
		return _MEM_NO_PARENT;
	else
	{
		FEnames.push_back( attribute );
		FEvalue.push_back( Svalue );
		FEparent.push_back( parent );
		if(parent == "IL")
			temp4 = pAgent->CreateFloatWME( pInputLink , attribute.c_str() , value );
		else
			temp4 = pAgent->CreateFloatWME( IDs[pI], attribute.c_str() , value);
		
		FEs.push_back(temp4);
	}
	return _MEM_ADD_SUCCESS;
}

string
QLMemory::CreateStringWME( string parent , string attribute , string value )
{
	sml::StringElement* temp2;
	int pI = FindParentIndex( parent );
	if(pI == -1 && parent != "IL") //parent doesn't exist
		return _MEM_NO_PARENT;
	else
	{
		SEnames.push_back( attribute );
		SEvalue.push_back( value );
		SEparent.push_back( parent );
		if(parent == "IL")  //create on input-link
			temp2 = pAgent->CreateStringWME(pInputLink, attribute.c_str(), value.c_str());
		else
			temp2 = pAgent->CreateStringWME(IDs[pI], attribute.c_str(), value.c_str());
			
		SEs.push_back(temp2);
	}
	return _MEM_ADD_SUCCESS;
}

string
QLMemory::DetermineValueAndChange( string parent , string attribute , string oldValue , string newValue )
{
	int index = -1;
	bool mFloat = false;
	for(unsigned int i = 0; i < newValue.size(); i++)
	{
		if (newValue[i] == '.')
			mFloat = true;
	}
	if(mFloat && (isdigit(newValue[0]) || isdigit(newValue[1]))) //value is float
	{
		float OFvalue = static_cast<float>(atof(oldValue.c_str()));
		for(unsigned int i = 0; i < FEs.size(); i++)
		{
			if(FEparent[i] == parent && FEnames[i] == attribute && atof(FEvalue[i].c_str()) == OFvalue)
				index = i;
		}
		if (index == -1)
			return _MEM_SOMETHING_DOESNT_EXIST;
		else
		{
			float Fvalue = static_cast<float>(atof(newValue.c_str()));
			pAgent->Update(FEs[index],Fvalue);
			FEvalue[index] = newValue;
			return _MEM_CHANGE_SUCCESS;
		}					
	}
	else if(isdigit(newValue[0])) //value is int
	{
		int OIvalue = atoi(oldValue.c_str());
		for(unsigned int i = 0; i < IEs.size(); i++)
		{
			if(IEparent[i] == parent && IEnames[i] == attribute && atoi(IEvalue[i].c_str()) == OIvalue)
				index = i;
		}
		if (index == -1)
			return _MEM_SOMETHING_DOESNT_EXIST;
		else
		{
			int Ivalue = atoi(newValue.c_str());
			pAgent->Update(IEs[index],Ivalue);
			IEvalue[index] = newValue;
			return _MEM_CHANGE_SUCCESS;
		}					
	}
	else //value is string
	{
		for(unsigned int i = 0; i < SEs.size(); i++)
		{
			if(SEparent[i] == parent && SEnames[i] == attribute && SEvalue[i] == oldValue)
				index = i;
		}
		if (index == -1)
			return _MEM_SOMETHING_DOESNT_EXIST;
		else
		{
			pAgent->Update(SEs[index],newValue.c_str());
			SEvalue[index] = newValue;
			return _MEM_CHANGE_SUCCESS;
		}					
	}
}

string
QLMemory::DeleteId( string parent, string attribute , string value )
{

	int ind = -1;
	for ( unsigned int i = 0 ; i < IDs.size() ; i++ )  //find element
	{
		if( IDparent[i] == parent && IDnames[i] == attribute && IDsoar[i] == value )
			ind = i;
	}
	if ( ind == -1 )  //doesn't exist
		return _MEM_SOMETHING_DOESNT_EXIST;
	else
	{
		DeleteChildren( "NOTCLEAR" , IDsoar[ind] , IDsoar[ind] );
		DestroyIdentifier( "NOTCLEAR" , ind);
		return _MEM_DELETE_SUCCESS;
	}			
}

void
QLMemory::DeleteChildren( string flag , string father , string always )
{
	int ind = -1;

	for( unsigned int j = 0 ; j < IDs.size() ; j++ )  //Goes through all Identifiers
	{
		if( IDparent[j] == father ) //Finds ones who have this father
		{
			ind = j;
			DeleteChildren( flag , IDsoar[ind] , always );
			if( flag == "CLEAR" ) //Implemented for Bob, clear should not erase lower-level structures
			{
				if( IDparent[j] == always )
					pAgent->DestroyWME( IDs[ind] );
			}
			DestroyIdentifier( flag , ind );
			j--;  //needed because of way delete is made
		}
	}	
	ind = -1;
	for( unsigned int j = 0 ; j < IEs.size() ; j++ ) //Goes through all Int Elements 
	{
		if( IEparent[j] == father )  //Finds ones who have this father
		{
			ind = j;
			if( flag == "CLEAR" ) //Implemented for Bob, clear should not erase lower-level structures
			{
				if( IEparent[j] == always )
					pAgent->DestroyWME( IEs[ind] );
			}
			DestroyIntElement( flag , ind );
			j--;  //needed because of way delete is made
		}
	}
	ind = -1;
	for( unsigned int j = 0 ; j < FEs.size() ; j++ )  //Goes through all Float Elements
	{
		if( FEparent[j] == father ) //Finds ones who have this father
		{
			ind = j;
			if( flag == "CLEAR" ) //Implemented for Bob, clear should not erase lower-level structures
			{
				if( FEparent[j] == always )
					pAgent->DestroyWME( FEs[ind] );
			}
			DestroyFloatElement( flag , ind );
			j--;  //needed because of way delete is made
		}
	}
	ind = -1;
	for( unsigned int j = 0 ; j < SEs.size() ; j++ )  //Goes through all String Elements
	{
		if( SEparent[j] == father ) //Finds ones who have this father
		{
			ind = j;
			if( flag == "CLEAR" ) //Implemented for Bob, clear should not erase lower-level structures
			{
				if( SEparent[j] == always )
					pAgent->DestroyWME( SEs[ind] );
			}
			DestroyStringElement( flag , ind );
			j--;  //needed because of way delete is made
		}
	}
	ind = -1;
	for( unsigned int j = 0 ; j < Shared.size() ; j++ )  //Goes through all Shared Elements
	{
		if( SharedParent[j] == father || SharedValue[j] == father ) //Finds ones who have this father
		{
			ind = j;
			if( flag == "CLEAR" ) //Implemented for Bob, clear should not erase lower-level structures
			{
				if( SharedParent[j] == always )
					pAgent->DestroyWME( Shared[ind] );
			}
			DestroySharedId( flag , ind );
			j--;  //needed because of way delete is made
		}
	}
}

string
QLMemory::DetermineValueAndDelete( string parent, string attribute , string value )
{
	int ind = -1;
	bool mFloat = false;
	for(unsigned int i = 0; i < value.size(); i++)
	{
		if (value[i] == '.')
			mFloat = true;
	}
	if(mFloat && (isdigit(value[0]) || isdigit(value[1]))) //value is float
	{
		float Fvalue = static_cast<float>(atof(value.c_str()));
		for(unsigned int i = 0; i < FEs.size(); i++)
		{
			if(FEparent[i] == parent && FEnames[i] == attribute && atof(FEvalue[i].c_str()) == Fvalue)
				ind = i;
		}
		if (ind == -1)
			return _MEM_SOMETHING_DOESNT_EXIST;
		else
		{
			DestroyFloatElement( "NOTCLEAR" , ind);
			return _MEM_DELETE_SUCCESS;
		}

	}
	else if(isdigit(value[0])) //value is int
	{
		int Ivalue = static_cast<int>(atoi(value.c_str()));
		for(unsigned int i = 0; i < IEs.size(); i++)
		{
			if(IEparent[i] == parent && IEnames[i] == attribute && atoi(IEvalue[i].c_str()) == Ivalue)
				ind = i;
		}
		if (ind == -1)
			return _MEM_SOMETHING_DOESNT_EXIST;
		else
		{
			DestroyIntElement( "NOTCLEAR" , ind );
			return _MEM_DELETE_SUCCESS;
		}

	}
	else //value is string
	{
		for(unsigned int i = 0; i < SEs.size(); i++)
		{
			if(SEparent[i] == parent && SEnames[i] == attribute && SEvalue[i] == value)
				ind = i;
		}
		if (ind == -1)
			return _MEM_SOMETHING_DOESNT_EXIST;
		else
		{
			DestroyStringElement( "NOTCLEAR" , ind );
			return _MEM_DELETE_SUCCESS;
		}

	}
}

void
QLMemory::DestroyIdentifier( string flag , int index )
{
	if( flag != "CLEAR" ) //Implemented for Bob, clear should not erase lower-level structures
		pAgent->DestroyWME( IDs[index] );
	int last = IDs.size() - 1;  //the delete is made by swapping the element to be deleted with the last and using pop_back
	IDs[index] = IDs[last];
	IDnames[index] = IDnames[last];
	IDparent[index] = IDparent[last];
	IDsoar[index] = IDsoar[last];
	IDs.pop_back();
	IDnames.pop_back();
	IDparent.pop_back();
	IDsoar.pop_back();
}

void
QLMemory::DestroyFloatElement( string flag , int index )
{
	if( flag != "CLEAR" ) //Implemented for Bob, clear should not erase lower-level structures
		pAgent->DestroyWME( FEs[index] );
	int last = FEs.size() - 1;  //the delete is made by swapping the element to be deleted with the last and using pop_back
	FEs[index] = FEs[last];
	FEnames[index] = FEnames[last];
	FEparent[index] = FEparent[last];
	FEvalue[index] = FEvalue[last];
	FEs.pop_back();
	FEnames.pop_back();
	FEparent.pop_back();
	FEvalue.pop_back();
}

void
QLMemory::DestroyIntElement( string flag , int index )
{
	if( flag != "CLEAR" ) //Implemented for Bob, clear should not erase lower-level structures
		pAgent->DestroyWME( IEs[index] );
	int last = IEs.size() - 1;  //the delete is made by swapping the element to be deleted with the last and using pop_back
	IEs[index] = IEs[last];
	IEnames[index] = IEnames[last];
	IEparent[index] = IEparent[last];
	IEvalue[index] = IEvalue[last];
	IEs.pop_back();
	IEnames.pop_back();
	IEparent.pop_back();
	IEvalue.pop_back();
}

void
QLMemory::DestroySharedId( string flag , int index )
{
	if( flag != "CLEAR" ) //Implemented for Bob, clear should not erase lower-level structures
		pAgent->DestroyWME( Shared[index] );
	int last = Shared.size() - 1;  //the delete is made by swapping the element to be deleted with the last and using pop_back
	Shared[index] = Shared[last];
	SharedParent[index] = SharedParent[last];
	SharedNames[index] = SharedNames[last];
	SharedValue[index] = SharedValue[last];
	Shared.pop_back();
	SharedParent.pop_back();
	SharedNames.pop_back();
	SharedValue.pop_back();
}

void
QLMemory::DestroyStringElement( string flag , int index )
{
	if( flag != "CLEAR" ) //Implemented for Bob, clear should not erase lower-level structures
		pAgent->DestroyWME( SEs[index] );
	int last = SEs.size() - 1;  //the delete is made by swapping the element to be deleted with the last and using pop_back
	SEs[index] = SEs[last];
	SEnames[index] = SEnames[last];
	SEparent[index] = SEparent[last];
	SEvalue[index] = SEvalue[last];
	SEs.pop_back();
	SEnames.pop_back();
	SEparent.pop_back();
	SEvalue.pop_back();
}

bool
QLMemory::PurgeMemory()
{
	DeleteChildren( "NOTCLEAR" , "IL" , "IL" );
	IDnames.resize(0);
	IDparent.resize(0);
	IDsoar.resize(0);
	IDprint.resize(0);
	SEnames.resize(0);
	SEparent.resize(0);
	SEvalue.resize(0);
	SEprint.resize(0);
	SharedNames.resize(0);
	SharedParent.resize(0);
	SharedValue.resize(0);
	SharedPrint.resize(0);
	IEnames.resize(0);
	IEparent.resize(0);
	IEvalue.resize(0);
	IEprint.resize(0);
	FEnames.resize(0);
	FEparent.resize(0);
	FEvalue.resize(0);
	FEprint.resize(0);
	return true;
}

bool
QLMemory::ClearProcessMem()
{
	ProcessMemory.resize(0);
	return true;
}

string
QLMemory::GetInput()
{
	string toReturn = "";
	for( unsigned int i = 0 ; i < IDs.size() ; i++ )
		toReturn += (IDparent[i] + " ^" + IDnames[i] + " " + IDsoar[i]);
	for( unsigned int i = 0 ; i < Shared.size() ; i++ )
		toReturn += SharedParent[i] + " ^" + SharedNames[i] + " " + SharedValue[i];
	for( unsigned int i = 0 ; i < IEs.size() ; i++ )
		toReturn += IEparent[i] + " ^" + IEnames[i] + " " + IEvalue[i];
	for( unsigned int i = 0 ; i < SEs.size() ; i++ )
		toReturn += SEparent[i] + " ^" + SEnames[i] + " " + SEvalue[i];
	for( unsigned int i = 0 ; i < FEs.size() ; i++ )
		toReturn += FEparent[i] + " ^" + FEnames[i] + " " + FEvalue[i];

	return toReturn;
}