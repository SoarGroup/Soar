/////////////////////////////////////////////////////////////////
// Identifier class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// Working memory element that has an ID as its value.
//
// Also contains "IdentifierSymbol".  This represents the value.
//
// So for (I6 ^name N1) the triplet is the WME (Identifier class)
// and "N1" is the IdentifierSymbol.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_IDENTIFIER_H
#define SML_IDENTIFIER_H

#include "sml_ClientWMElement.h"
#include "sml_ClientDirect.h"

#include <string>
#include <list>
#include <set>
#include <iostream>
#include "Export.h"

namespace sml
{

    class StringElement ;
    class WorkingMemory ;
    class Identifier ;
    
// Two identifiers (two wmes) can have the same value because this is a graph not a tree
// so we need to represent that separately.
    class EXPORT IdentifierSymbol
    {
            friend class Identifier ;           // Provide direct access to children.
            
        protected:
            // The value for this id, which is a string identifier (e.g. I3)
            // We'll use upper case for Soar IDs and lower case for client IDs
            // (sometimes the client has to generate these before they are assigned by the kernel)
            std::string m_Symbol ;
            
            // The list of WMEs owned by this identifier.
            // (When we delete this identifier we'll delete all these automatically)
            std::list<WMElement*>       m_Children ;
            
            // The list of WMEs that are using this symbol as their identifier
            // (Usually just one value in this list)
            std::list<Identifier*>      m_UsedBy ;
            
            // This is true if the list of children of this identifier was changed.  The client chooses when to clear these flags.
            bool m_AreChildrenModified ;
            
        public:
            IdentifierSymbol(Identifier* pIdentifier) ;
            ~IdentifierSymbol() ;
            
            char const* GetIdentifierSymbol()
            {
                return m_Symbol.c_str() ;
            }
            void SetIdentifierSymbol(char const* pID);
            
            bool AreChildrenModified()
            {
                return m_AreChildrenModified ;
            }
            void SetAreChildrenModified(bool state)
            {
                m_AreChildrenModified = state ;
            }
            
            // Indicates that an identifier is no longer using this as its value
            void NoLongerUsedBy(Identifier* pIdentifier);
            void UsedBy(Identifier* pIdentifier);
            
            bool IsFirstUser(Identifier* pIdentifier)
            {
                if (m_UsedBy.size() == 0)
                {
                    return false ;
                }
                
                Identifier* front = m_UsedBy.front() ;
                return (front == pIdentifier) ;
            }
            
            Identifier* GetFirstUser()
            {
                return m_UsedBy.front() ;
            }
            
            int  GetNumberUsing()
            {
                return (int)m_UsedBy.size() ;
            }
            
            // Have this identifier take ownership of this WME.  So when the identifier is deleted
            // it will delete the WME.
            void AddChild(WMElement* pWME) ;
            WMElement* GetChildByTimeTag(long long timeTag);
            void TransferChildren(IdentifierSymbol* pDestination);
            void DeleteAllChildren() ;
            
            void RemoveChild(WMElement* pWME) ;
            
            void DebugString(std::string& result);
            
        private:
            std::list<WMElement*>::iterator FindChildByTimeTag(long long timeTag);
    } ;
    
//
// Special note about output-link WMEs: The agent is
// free to remove WMEs from the output-link at any time.
// If you retain a WME for multiple decision cycles,
// you must check output link changes (using
// GetNumOutputLinkChanges, GetOutputLinkChange, and
// IsOutputLinkAdd) to check if the WMEs you have were
// removed during the last decision cycle. Dereferencing
// a removed WME causes a segmentation fault.
    class EXPORT Identifier : public WMElement
    {
            // Make the members all protected, so users dont' access them by accident.
            // Instead, only open them up to the working memory class to use.
            friend class WorkingMemory ;
            friend class WMElement ;
            friend class Agent ;
            friend class StringElement ;
            friend class IntElement ;
            friend class FloatElement ;
            friend class OutputDeltaList ;      // Allow it to clear are children modified
            
        public:
            typedef std::list<WMElement*>::iterator ChildrenIter ;
            typedef std::list<WMElement*>::const_iterator ChildrenConstIter ;
            
            ChildrenIter GetChildrenBegin()
            {
                return m_pSymbol->m_Children.begin() ;
            }
            ChildrenIter GetChildrenEnd()
            {
                return m_pSymbol->m_Children.end() ;
            }
            
        private:
            void ReleaseSymbol();
            
        protected:
            // Two identifiers (i.e. two wmes) can share the same identifier value
            // So each identifier has a pointer to a symbol object, but two could share the same object.
            IdentifierSymbol* m_pSymbol ;
            
            IdentifierSymbol* GetSymbol()
            {
                return m_pSymbol ;
            }
            void UpdateSymbol(IdentifierSymbol* pSymbol);
            
            void SetSymbol(IdentifierSymbol* p_ID) ;
            
            void RecordSymbolInMap();
            
        public:
            virtual char const* GetValueType() const ;
            
            // Returns a string form of the value stored here.
            virtual char const* GetValueAsString() const
            {
                return m_pSymbol->GetIdentifierSymbol() ;
            }
            
            // Returns a string form of the value, uses buf to create the string
            virtual char const* GetValueAsString(std::string& buf) const
            {
                buf.assign(m_pSymbol->GetIdentifierSymbol());
                return buf.c_str();
            }
            
            // The Identifier class overrides this to return true.  (The poor man's RTTI).
            virtual bool IsIdentifier() const
            {
                return true ;
            }
            
            virtual Identifier* ConvertToIdentifier()
            {
                return this;
            }
            
            /*************************************************************
            * @brief Searches for a child of this identifier that has the given
            *        time tag.
            *        (The search is recursive over all children).
            *
            * @param timeTag    The tag to look for (e.g. +12 for kernel side or -15 for client side)
            *************************************************************/
            WMElement* FindFromTimeTag(long long timeTag) const ;
            
            /*************************************************************
            * @brief Returns the n-th WME that has the given attribute
            *        and this identifier as its parent (or NULL).
            *        (<ID> ^attribute <WME>) - returns "WME"
            *
            * @param pAttribute     The name of the attribute to match
            * @param index          0 based index of values for this attribute
            *                      (> 0 only needed for multi-valued attributes)
            *************************************************************/
            WMElement* FindByAttribute(char const* pAttribute, int index) const ;
            
            /*************************************************************
            * @brief Returns the value (as a string) of the first WME with this attribute.
            *       (<ID> ^attribute value) - returns "value"
            *
            * @param pAttribute     The name of the attribute to match
            *************************************************************/
            char const* GetParameterValue(char const* pAttribute) const
            {
                WMElement* pWME = FindByAttribute(pAttribute, 0) ;
                return pWME ? pWME->GetValueAsString() : NULL ;
            }
            
            /*************************************************************
            * @brief Returns the "command name" for a top-level identifier on the output-link.
            *        That is for output-link O1 (O1 ^move M3) returns "move".
            *************************************************************/
            char const* GetCommandName() const
            {
                return this->GetAttribute() ;
            }
            
            /*************************************************************
            * @brief Adds "^status complete" as a child of this identifier.
            *************************************************************/
            void AddStatusComplete() ;
            
            /*************************************************************
            * @brief Adds "^status error" as a child of this identifier.
            *************************************************************/
            void AddStatusError() ;
            
            /*************************************************************
            * @brief Adds "^error-code <code>" as a child of this identifier.
            *************************************************************/
            void AddErrorCode(int errorCode) ;
            
            /*************************************************************
            * @brief Returns the number of children
            *************************************************************/
            int GetNumberChildren()
            {
                return (int)m_pSymbol->m_Children.size() ;
            }
            
            /*************************************************************
            * @brief Gets the n-th child.
            *        Ownership of this WME is retained by the agent.
            *
            *        This is an O(n) operation.  We could expose the iterator directly
            *        but we want to export this interface to Java/Tcl etc. and this is easier.
            *************************************************************/
            WMElement* GetChild(int index) ;
            
            /*************************************************************
            * @brief This is true if the list of children of this identifier has changed.
            *        The client chooses when to clear these flags.
            *************************************************************/
            bool AreChildrenModified()
            {
                return m_pSymbol->AreChildrenModified() ;
            }
            
            StringElement*  CreateStringWME(char const* pAttribute, char const* pValue);
            IntElement*     CreateIntWME(char const* pAttribute, long long value) ;
            FloatElement*   CreateFloatWME(char const* pAttribute, double value) ;
            Identifier*     CreateIdWME(char const* pAttribute) ;
            Identifier*     CreateSharedIdWME(char const* pAttribute, Identifier* pSharedValue) ;
            
        protected:
            // This version is only needed at the top of the tree (e.g. the input link)
            Identifier(Agent* pAgent, char const* pAttributeName, char const* pIdentifier, long long timeTag);
            
            // The normal case (where there is a parent id)
            Identifier(Agent* pAgent, Identifier* pParent, char const* pID, char const* pAttributeName, char const* pIdentifier, long long timeTag) ;
            Identifier(Agent* pAgent, IdentifierSymbol* pParentSymbol, char const* pID, char const* pAttributeName, char const* pIdentifier, long long timeTag) ;
            
            // The shared id case (where there is a parent id and value is an identifier that already exists)
            Identifier(Agent* pAgent, Identifier* pParent, char const* pID, char const* pAttributeName, Identifier* pLinkedIdentifier, long long timeTag) ;
            Identifier(Agent* pAgent, IdentifierSymbol* pParentSymbol, char const* pID, char const* pAttributeName, IdentifierSymbol* pLinkedIdentifierSymbol, long long timeTag) ;
            
            virtual ~Identifier(void);
            
            void AddChild(WMElement* pWME)
            {
                m_pSymbol->AddChild(pWME) ;
            }
            
            void RemoveChild(WMElement* pWME)
            {
                m_pSymbol->RemoveChild(pWME) ;
            }
            
#ifdef SML_DIRECT
            virtual void DirectAdd(Direct_AgentSML_Handle pAgentSML, long long timeTag) ;
#endif
            
            // Send over to the kernel again
            virtual void Refresh() ;
            
        private:
            // NOT IMPLEMENTED
            Identifier(const Identifier& rhs);
            Identifier& operator=(const Identifier& rhs);
            
    };
    
}   // namespace

#endif // SML_IDENTIFIER_H
