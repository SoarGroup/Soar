/////////////////////////////////////////////////////////////////
// WMElement class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// This is the base class for all working memory elements.
// Every WME consists of an ID, attribute and value.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_WORKING_MEMORY_ELEMENT_H
#define SML_WORKING_MEMORY_ELEMENT_H

#include <string>
#include "Export.h"

#include "sml_ClientDirect.h"

namespace sml
{

    class Agent ;
    class Identifier ;
    class IdentifierSymbol ;
    class IntElement ;
    class FloatElement ;
    class StringElement ;
    class WorkingMemory ;
    class RemoveDelta ;
    class WMDelta ;
    
    class EXPORT WMElement
    {
            // Making most methods protected, so users don't use them directly by accident.
            // But allow working memory to work with them directly.
            friend class WorkingMemory ;
            friend class Identifier ;           // Access to just added information
            friend class WMDelta ;              // Allow it to destroy WMEs
            friend class IdentifierSymbol ;     // Allow it to destroy WMEs
            friend class OutputDeltaList ;      // Allow it to clear just added
            
        protected:
            // The agent which owns this WME.
            Agent*  m_Agent ;
            
            // The time tag (a unique id for this WME)
            // We used negative values so it's clear that this time tag is a client side tag.
            long long   m_TimeTag ;
            
            // The identifier symbol as a string.  This can be necessary when connecting up
            // disconnected segments of a graph.
            std::string         m_IDName ;
            
            // The id for this wme (can be NULL if we're at the top of the tree)
            IdentifierSymbol*   m_ID ;
            
            // The attribute name for this wme (the value is owned by the derived class)
            std::string m_AttributeName ;
            
            // This is true if the wme was just added.  The client chooses when to clear these flags.
            bool    m_JustAdded ;
            
        public:
            // This is true if the wme was just added.  The client chooses when to clear these flags.
            // This is only maintained for output wmes (the client controls input wmes).
            bool    IsJustAdded()
            {
                return m_JustAdded ;
            }
            
            // Two accessors for the ID as people think about it in different ways
            IdentifierSymbol*       GetParent()     const
            {
                return m_ID ;
            }
            IdentifierSymbol*       GetIdentifier() const
            {
                return m_ID ;
            }
            
            // This will always be valid even if we no identifier symbol
            char const* GetIdentifierName() const
            {
                return m_IDName.c_str() ;
            }
            
            char const* GetAttribute() const
            {
                return m_AttributeName.c_str() ;
            }
            
            // Returns the type of the value stored here (e.g. "string" or "int" etc.)
            virtual char const* GetValueType() const = 0 ;
            
            // Returns a string form of the value stored here.
            virtual char const* GetValueAsString() const = 0 ;
            
            // Returns a string form of the value, uses buf to create the string
            virtual char const* GetValueAsString(std::string& buf) const = 0 ;
            
            long long GetTimeTag() const
            {
                return m_TimeTag ;
            }
            
            // The Identifier class overrides this to return true.  (The poor man's RTTI).
            virtual bool IsIdentifier() const
            {
                return false ;
            }
            
            // Class conversions for SWIG language bridges. (The even poorer man's RTTI).
            // Each subclass overrides the appropriate method.
            // Note: In SWIG-wrapped languages such as Java, Python, Tcl, CSHarp:
            //       The reference returned by this command will not pass a reference
            //       equality test with other references to other WMEs pointing to
            //       the same working memory element, even though in C++ these
            //       pointers are the same. Workaround: use GetTimeTag to test
            //       if the returned WMElements are indeed the same WME.
            virtual Identifier* ConvertToIdentifier()
            {
                return NULL;
            }
            virtual IntElement* ConvertToIntElement()
            {
                return NULL;
            }
            virtual FloatElement* ConvertToFloatElement()
            {
                return NULL;
            }
            virtual StringElement* ConvertToStringElement()
            {
                return NULL;
            }
            
            Agent*      GetAgent()
            {
                return m_Agent ;
            }
            
            /*************************************************************
            * @brief Schedules a WME from deletion from the input link and removes
            *        it from the client's model of working memory.
            *
            *        If this is an identifier then all of its children will be
            *        deleted too (assuming it's the only parent -- i.e. part of a
            *        tree not a full graph). Disconnecting WMEs that are still
            *        linked to each other will cause a memory leak until
            *        the agent is destroyed.
            *
            *        The caller should not access this WME after calling
            *        DestroyWME() or any of its children if this is an identifier.
            *        If "auto commit" is turned off in ClientKernel,
            *        the WME is not removed from the input link until
            *        the client calls "Commit"
            *
            *        Special note about output-link WMEs: The agent is
            *        free to remove WMEs from the output-link at any time.
            *        If you retain a WME for multiple decision cycles,
            *        you must check output link changes (using
            *        GetNumOutputLinkChanges, GetOutputLinkChange, and
            *        IsOutputLinkAdd) to check if the WMEs you have were
            *        removed during the last decision cycle. Dereferencing
            *        a removed WME causes a segmentation fault.
            *************************************************************/
            bool DestroyWME();
            
            void DebugString(std::string& result);
            
        protected:
            // Keep these protected, so user can only create and destroy WMEs through
            // the methods exposed in the agent class.  This makes it clear that the
            // agent owns all objects.
            WMElement(Agent* pAgent, IdentifierSymbol* pParentSymbol, char const* pID, char const* pAttributeName, long long timeTag);
            virtual ~WMElement(void);
            
            void    SetJustAdded(bool state)
            {
                m_JustAdded = state ;
            }
            
            // We should only set the parent symbol once and only to the same
            // string value as the ID we used when we constructed the WME.
            // (This method is only called when we have dangling parts of a graph while
            //  it's being built).
            // This can also be re-assigned when IdentifierSymbols' children transfer
            // ownership when converting from client side (e.g. h2) to kernel side (H1)
            void SetSymbol(IdentifierSymbol* p_ID) ;
            
            // If we update the value we need to assign a new time tag to this WME.
            // That's because we're really doing a delete followed by an add
            // and the add would create a new time tag.
            void GenerateNewTimeTag() ;
            
            // Send over to the kernel again
            virtual void Refresh() ;
            
#ifdef SML_DIRECT
            virtual void DirectAdd(Direct_AgentSML_Handle pAgentSML, long long timeTag) = 0 ;
#endif
            
        private:
            // NOT IMPLEMENTED
            WMElement(const WMElement& rhs);
            WMElement& operator=(const WMElement& rhs);
            
    };
    
}   // namespace

#endif // SML_WORKING_MEMORY_ELEMENT_H
