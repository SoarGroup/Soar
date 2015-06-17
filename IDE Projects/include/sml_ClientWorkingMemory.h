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
#include "sml_ClientDirect.h" // SML_DIRECT defined here
#include "sml_Events.h"
#include "Export.h"

#include <list>
#include <map>

namespace soarxml
{
    class ElementXML ;
}

namespace sml
{

// Forward declarations
    class Agent ;
    class Connection ;
    class StringElement ;
    class IntElement ;
    class FloatElement ;
    class Identifier ;
    class IdentifierSymbol ;
    class AnalyzeXML ;
    
    class EXPORT WorkingMemory
    {
            friend class Identifier;
            
        protected:
        
#ifdef SML_DIRECT
            Direct_AgentSML_Handle m_AgentSMLHandle;
#endif // SML_DIRECT
            
            Agent*      m_Agent ;
            Identifier* m_InputLink ;
            Identifier* m_OutputLink ;
            
            // List of changes that are pending to be sent to the kernel
            DeltaList   m_DeltaList ;
            
            // List of changes to output-link since last time client checked
            OutputDeltaList m_OutputDeltaList ;
            enum ChangeListMode { CHANGE_LIST_AUTO_DISABLED = -1, CHANGE_LIST_USER_DISABLED = -2 };
            int m_changeListHandlerId;
            static void ClearHandlerStatic(sml::smlRunEventId id, void* pUserData, sml::Agent* pAgent, sml::smlPhase phase);
            
            typedef std::list<WMElement*> WmeList ;
            typedef WmeList::iterator WmeListIter ;
            
            // A temporary list of wme's with no parent identifier
            // Should always be empty at the end of an output call from the kernel.
            WmeList     m_OutputOrphans ;
            
            void RecordAddition(WMElement* pWME) ;
            void RecordDeletion(WMElement* pWME) ;
            
            WMElement* SearchWmeListForID(WmeList* pWmeList, char const* pID, bool deleteFromList) ;
            
            typedef std::map<std::string, IdentifierSymbol*> IdSymbolMap ;
            typedef IdSymbolMap::iterator IdSymbolMapIter ;
            
            IdSymbolMap     m_IdSymbolMap;
            
            typedef std::map< long long, WMElement* > TimeTagWMEMap ;
            typedef TimeTagWMEMap::iterator TimeTagWMEMapIter ;
            
            TimeTagWMEMap       m_TimeTagWMEMap;
            
            // Searches for an identifier object that matches this id.
            IdentifierSymbol*   FindIdentifierSymbol(char const* pID);
            void                RecordSymbolInMap(IdentifierSymbol* pSymbol);
            void                RemoveSymbolFromMap(IdentifierSymbol* pSymbol);
            bool                m_Deleting; // used when we're being deleted and the maps shouldn't be updated
            
            // Create a new WME of the appropriate type based on this information.
            WMElement*          CreateWME(IdentifierSymbol* pParentSymbol, char const* pID, char const* pAttribute, char const* pValue, char const* pType, long long timeTag) ;
            
        public:
            WorkingMemory() ;
            
            virtual ~WorkingMemory();
            
            void            SetAgent(Agent* pAgent);
            Agent*          GetAgent() const
            {
                return m_Agent ;
            }
            char const*     GetAgentName() const ;
            Connection*     GetConnection() const ;
            
            void            SetOutputLinkChangeTracking(bool setting);
            bool            IsTrackingOutputLinkChanges()
            {
                return m_changeListHandlerId > 0;
            }
            void            ClearOutputLinkChanges() ;
            
            OutputDeltaList* GetOutputLinkChanges()
            {
                return &m_OutputDeltaList ;
            }
            
            DeltaList*      GetInputDeltaList()
            {
                return &m_DeltaList ;
            }
            
            // These functions are documented in the agent and handled here.
            Identifier*     GetInputLink() ;
            Identifier*     GetOutputLink() ;
            StringElement*  CreateStringWME(Identifier* parent, char const* pAttribute, char const* pValue);
            IntElement*     CreateIntWME(Identifier* parent, char const* pAttribute, long long value) ;
            FloatElement*   CreateFloatWME(Identifier* parent, char const* pAttribute, double value) ;
            
            Identifier*     CreateIdWME(Identifier* parent, char const* pAttribute) ;
            Identifier*     CreateSharedIdWME(Identifier* parent, char const* pAttribute, Identifier* pSharedValue) ;
            
            void            UpdateString(StringElement* pWME, char const* pValue) ;
            void            UpdateInt(IntElement* pWME, long long value) ;
            void            UpdateFloat(FloatElement* pWME, double value) ;
            
            bool            DestroyWME(WMElement* pWME) ;
            
            bool            TryToAttachOrphanedChildren(Identifier* pPossibleParent) ;
            bool            ReceivedOutputRemoval(soarxml::ElementXML* pWmeXML, bool tracing) ;
            bool            ReceivedOutputAddition(soarxml::ElementXML* pWmeXML, bool tracing) ;
            bool            ReceivedOutput(AnalyzeXML* pIncoming, soarxml::ElementXML* pResponse) ;
            
            bool            SynchronizeInputLink() ;
            bool            SynchronizeOutputLink() ;
            
            long long       GenerateTimeTag() ;
            void            GenerateNewID(char const* pAttribute, std::string* pID) ;
            
            void            Refresh() ;
            void            InvalidateOutputLink(); // During init-soar, the output-link identifier could change
            
            bool            IsCommitRequired() ;
            bool            Commit() ;
            bool            IsAutoCommitEnabled() ;
            
    };
    
}//closes namespace

#endif //SML_WORKING_MEMORY_H
