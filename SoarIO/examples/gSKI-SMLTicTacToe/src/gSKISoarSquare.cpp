#include "gSKISoarSquare.h"
#include "board.h"

#include "gSKISoarAgent.h"
/*
#include "sml_ClientAgent.h"
#include "sml_ClientInputLink.h"
#include "sml_ClientOutputLink.h"
#include "sml_ClientWme.h"
#include "sml_ClientSymbol.h"
*/
/*
#include "IgSKI_Agent.h"
#include "IgSKI_InputLink.h"
#include "IgSKI_OutputLink.h"
#include "IgSKI_Wme.h"
#include "IgSKI_Symbol.h"
#include "gSKI_Structures.h"
*/
#include <iostream>
#include <string>

using std::string;
using std::cout;
using std::endl;

using namespace sml;


/**************************************/
/*   Soar Square Function Definitions */
/**************************************/
void SoarSquare::Attach(IWorkingMemory* wmemory, Marker m, int inRow, int inCol)
{
	IInputLink* iLink = wmemory->GetAgent()->GetInputLink();
	IWMObject* iLinkRootObject;
	iLink->GetRootObject(&iLinkRootObject);

	//create an identifier to bind row, col, and symbol to
	squareWMEStructure = wmemory->AddWmeNewObject(iLinkRootObject, "square");

	const ISymbol* parentSymbol = squareWMEStructure->GetValue();
	IWMObject* parentObject = parentSymbol->GetObject();

	row = wmemory->AddWmeInt(parentObject, "row", inRow);
	col = wmemory->AddWmeInt(parentObject, "col", inCol);
	symbol = wmemory->AddWmeString(parentObject, "content", MarkerToString(m));

	//SGIO way
	//soarId = mem->CreateIdWME(mem->GetILink(),"square"); 
	//row = mem->CreateIntWME(soarId,"row",inRow);
	//col = mem->CreateIntWME(soarId,"col",inCol);
	//symbol = mem->CreateStringWME(soarId,"content",MarkerToString(m));
}


void SoarSquare::Detach(IWorkingMemory* mem)
{	//by removing the base (identifier) object, all child wmes hanging off are removed too
	const ISymbol* parentSymbol = squareWMEStructure->GetValue();
	IWMObject* parentObject = parentSymbol->GetObject();
	mem->RemoveObject(parentObject);

	//SGIO way
	//mem->DestroyWME(soarId);
}

void SoarSquare::Update(IWorkingMemory* wMemory, IWMObject* object)
{
	//cout << "Square[" << this->row->GetValue()->GetInt() << "][" << this->col->GetValue()->GetInt() << "] = ";
	//cout << MarkerToString(board->GetAt(this->row->GetValue()->GetInt(), this->col->GetValue()->GetInt())) << endl;

	// Get List of objects referencing this object with attribute "content"
	tIWmeIterator* contentIter = object->GetWMEs("content");

	string currentMarker = MarkerToString(board->GetAt(this->row->GetValue()->GetInt(), this->col->GetValue()->GetInt()));
	if (contentIter->IsValid())
	{
		// Get the content value
		IWme* wmeOldContent = contentIter->GetVal();

		// Replace the wme attribute "content" with the new value
		this->symbol = wMemory->ReplaceStringWme(wmeOldContent, currentMarker.c_str());
		if (this->symbol == NULL)
			throw "ReplaceStringWme returned NULL in SoarSquare::Update()";

		// Look up the value we got back.
		const char* str = (symbol->GetValue())->GetString();
	}
	else
	{
		throw "The content Iterator returned was not valid in SoarSquare::Update()";
	}

}

IWMObject* SoarSquare::GetSquareIdentifier()
{
	const ISymbol* parentSymbol = squareWMEStructure->GetValue();
	return parentSymbol->GetObject();
}

void SoarSquare::PrintMarker()
{
	const char* str = (symbol->GetValue())->GetString();
	if (!strcmp(str,"EMPTY"))
		cout << " ";
	else
		cout << str;
}


void SoarSquare::Initialize(Board* inboard){board = inboard;}