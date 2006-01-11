/////////////////////////////////////////////////////////////////
// WorkingMemory class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// This class is used to represent Soar's working memory.
// We maintain a copy of this on the client so we can just
// send changes over to the kernel.
//
// Basic method is that working memory is stored as a tree
// of Element objects.
// When the client makes a change to the tree, we modify the tree
// and add the change to the list of changes to make to WM.
// At some point, we actually send that list of changes over.
// We should be able to be clever about collapsing changes together
// in the list of deltas (e.g. change value A->B->C can remove the
// A->B change (since it's overwritten by B->C).
//
/////////////////////////////////////////////////////////////////
#ifndef SML_WORKING_MEMORY_H
#define SML_WORKING_MEMORY_H

#include "sml_ObjectMap.h"
#include "sml_DeltaList.h"
#include "sml_OutputDeltaList.h"

#include <list>

namespace sml {

// Forward declarations
class Agent ;
class Connection ;
class StringElement ;
class IntElement ;
class FloatElement ;
class Identifier ;
class ElementXML ;
class AnalyzeXML ;

class WorkingMemory
{
protected:
	Agent*		m_Agent ;
	Identifier*	m_InputLink ;
	Identifier* m_OutputLink ;

	// List of changes that are pending to be sent to the kernel
	DeltaList	m_DeltaList ;

	// List of changes to output-link since last time client checked
	OutputDeltaList m_OutputDeltaList ;

	typedef std::list<WMElement*> WmeList ;
	typedef WmeList::iterator WmeListIter ;

	// A temporary list of wme's with no parent identifier
	// Should always be empty at the end of an output call from the kernel.
	WmeList		m_OutputOrphans ;

	void RecordAddition(WMElement* pWME) ;
	void RecordDeletion(WMElement* pWME) ;

	WMElement* SearchWmeListForID(WmeList* pWmeList, char const* pID, bool deleteFromList) ;
	Identifier* FindIdentifierInWmeList(WmeList* pWmeList, char const* pID) ;

public:
	WorkingMemory() ;

	virtual ~WorkingMemory();

	void			SetAgent(Agent* pAgent)	{ m_Agent = pAgent ; }
	Agent*			GetAgent() const		{ return m_Agent ; }
	char const*		GetAgentName() const ;
	Connection*		GetConnection()	const ;

	void			ClearOutputLinkChanges() ;

	OutputDeltaList* GetOutputLinkChanges() { return &m_OutputDeltaList ; }

	DeltaList*		GetInputDeltaList()		{ return &m_DeltaList ; }

	// Searches for an identifier object that matches this id.
	Identifier*		FindIdentifier(char const* pID, bool searchInput, bool searchOutput, int index = 0) ;

	// Create a new WME of the appropriate type based on this information.
	WMElement*		CreateWME(Identifier* pParent, char const* pID, char const* pAttribute, char const* pValue, char const* pType, long timeTag) ;

	// These functions are documented in the agent and handled here.
	Identifier*		GetInputLink() ;
	Identifier*		GetOutputLink() ;
	StringElement*	CreateStringWME(Identifier* parent, char const* pAttribute, char const* pValue);
	IntElement*		CreateIntWME(Identifier* parent, char const* pAttribute, int value) ;
	FloatElement*	CreateFloatWME(Identifier* parent, char const* pAttribute, double value) ;

	Identifier*		CreateIdWME(Identifier* parent, char const* pAttribute) ;
	Identifier*		CreateSharedIdWME(Identifier* parent, char const* pAttribute, Identifier* pSharedValue) ;

	void			UpdateString(StringElement* pWME, char const* pValue) ;
	void			UpdateInt(IntElement* pWME, int value) ;
	void			UpdateFloat(FloatElement* pWME, double value) ;

	bool			DestroyWME(WMElement* pWME) ;

	bool			ReceivedOutput(AnalyzeXML* pIncoming, ElementXML* pResponse) ;

	bool			SynchronizeInputLink() ;
	bool			SynchronizeOutputLink() ;

	long			GenerateTimeTag() ;
	void			GenerateNewID(char const* pAttribute, std::string* pID) ;

	void			Refresh() ;

	bool			Commit() ;
};

}//closes namespace

#endif //SML_WORKING_MEMORY_H
