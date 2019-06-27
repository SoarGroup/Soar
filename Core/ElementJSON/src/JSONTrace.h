#ifndef SOARJSON_JSONTRACE_H
#define SOARJSON_JSONTRACE_H

/////////////////////////////////////////////////////////////////
// JSONTrace class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : April 2005
//
// Represents a piece of structured trace information generated
// during the course of a run.
//
// This class is related to the ClientTraceJSON class.  This class
// is optimized for creating trace objects while the client class
// supports reading the trace information.
//
// This JSON structure is unusual because we want to add the same
// attributes to potentially a number of different tags.
// So we're looking for an API like this:
//
// beginTag(kWme);
// addAttribute(kTimeTag, mytimetag);
// addAttribute(kAtt, myatt);
// addAttribute(kVal, myval);
// endTag();
//
// where the "addAttribute" methods are working with the "current" object.
//
// JRV/RPM Update: The function definitions for this class' members are now included in the headers and there are no corresponding .cpp files.
// The reason for this is that the code that uses them needs to have these classes defined in that compilation unit so that the ElementJSON objects
// these units create are done so on their heap, and the ElementJSONImpl objects that actually do the work are created on the heap in the ElementJSON
// DLL. This is the Pimpl pattern.
//
/////////////////////////////////////////////////////////////////

#include "ElementJSON.h"
#include "soar_TraceNames.h"

#include <assert.h>

namespace soarjson
{

    class JSONTrace
    {
        private:
            // The root JSON object
            ElementJSON* m_JSON ;
            
            // The tag we're currently building
            ElementJSON* m_pCurrentTag ;
            
        public:
            JSONTrace()
            {
                m_JSON = new ElementJSON() ;
                m_JSON->SetTagName(soar_TraceNames::kTagTrace) ;
                
                m_pCurrentTag = new ElementJSON(m_JSON->GetJSONHandle()) ;
                m_pCurrentTag->AddRefOnHandle() ;
            }
            
            // Alternative constructor where we specify the base tag name
            JSONTrace(char const* pTagName)
            {
                m_JSON = new ElementJSON() ;
                m_JSON->SetTagName(pTagName) ;
                
                m_pCurrentTag = new ElementJSON(m_JSON->GetJSONHandle()) ;
                m_pCurrentTag->AddRefOnHandle() ;
            }
            
            virtual ~JSONTrace()
            {
                delete m_pCurrentTag ;
                m_pCurrentTag = NULL ;
                
                delete m_JSON ;
                m_JSON = NULL ;
            }
            
            /*************************************************************
            * @brief    Reinitialize this JSON trace object so we can
            *           re-use it.
            *************************************************************/
            void Reset()
            {
                delete m_pCurrentTag ;
                m_pCurrentTag = NULL ;
                
                delete m_JSON ;
                m_JSON = NULL ;
                
                m_JSON = new ElementJSON() ;
                m_JSON->SetTagName(soar_TraceNames::kTagTrace) ;
                
                m_pCurrentTag = new ElementJSON(m_JSON->GetJSONHandle()) ;
                m_pCurrentTag->AddRefOnHandle() ;
            }
            
            /*************************************************************
            * @brief    Returns true if this tag contains no children
            *           (i.e. it has just been reset).
            *************************************************************/
            bool IsEmpty()
            {
                return (m_JSON == NULL || m_JSON->GetNumberChildren() == 0) ;
            }
            
            /*************************************************************
            * @brief    Start a new tag.
            *
            * Subsequent calls to AddAttribute() will work with this tag
            * until EndTag() is called.
            *
            * NOTE: The tag name must remain in scope forever (i.e. it should be
            * a constant).  This allows us to save time by not copying the string.
            *************************************************************/
            void BeginTag(char const* pTagName)
            {
                ElementJSON* pChild = new ElementJSON() ;
                pChild->SetTagNameFast(pTagName) ;
                
                ElementJSON_Handle hChild = NULL ;
                
                // The new tag is created as a child of the current tag (if we have one)
                // or the root if we don't.  Adding the child deletes the pChild object
                // normally, so we have to create a second one.  Perhaps we should redesign AddChild?
                //  if (m_pCurrentTag == NULL)
                //      hChild = m_JSON->AddChild(pChild) ;
                //  else
                hChild = m_pCurrentTag->AddChild(pChild) ;
                
                delete m_pCurrentTag ;
                m_pCurrentTag = new ElementJSON(hChild) ;
                m_pCurrentTag->AddRefOnHandle() ;
            }
            
            /*************************************************************
            * @brief    Terminate the current tag.
            *
            * The tag name is just used for error checking to make sure
            * everything is properly balanced.
            *************************************************************/
            void EndTag(char const* pTagName)
            {
                assert(m_pCurrentTag) ;
                
                if (!m_pCurrentTag)
                {
                    return ;
                }
                
#ifdef _DEBUG
                // Make sure we're closing the tag we expect
                assert(m_pCurrentTag->IsTag(pTagName)) ;
#else
                (void)(pTagName); // quell compiler warning in VS.NET
#endif
                
                MoveCurrentToParent() ;
            }
            
            /*************************************************************
            * @brief    Adds an attribute to the current tag.
            *
            * NOTE: The attribute name must remain in scope forever (i.e. it should be
            * a constant).  This allows us to save time by not copying the string.
            * Naturally, the value doesn't have this restriction.
            *************************************************************/
            void AddAttribute(char const* pAttributeName, char const* pValue)
            {
                assert(m_pCurrentTag) ;
                
                if (!m_pCurrentTag)
                {
                    return ;
                }
                
                m_pCurrentTag->AddAttributeFast(pAttributeName, pValue) ;
            }
            
            /*************************************************************
            * @brief Releases ownership of the underlying JSON object.
            *
            *        The caller must call releaseRef() on this handle
            *        when it is done with it.
            *
            *       This should only be called when the message is actually
            *       being sent.  It shouldn't be used when building up the message.
            *
            *       NOTE: After doing this the JSONTrace object should either be
            *       deleted or Reset() should be called on it.
            *************************************************************/
            ElementJSON_Handle Detach()
            {
                ElementJSON_Handle hJSON = m_JSON->Detach() ;
                
                delete m_pCurrentTag ;
                m_pCurrentTag = NULL ;
                
                delete m_JSON ;
                m_JSON = NULL ;
                
                return hJSON ;
            }
            
            /*************************************************************
            * @brief Releases ownership of the underlying JSON object.
            *
            * As for Detach() but returns an object rather than a handle.
            *************************************************************/
            ElementJSON* DetatchObject()
            {
                delete m_pCurrentTag ;
                m_pCurrentTag = NULL ;
                
                ElementJSON* pResult = m_JSON ;
                m_JSON = NULL ;
                return pResult ;
            }
            
            /*************************************************************
            * @brief    Occassionally it's helpful to be able to back up
            *           in the JSON and add some extra elements.
            *
            *           These calls should only be used once a tag has been completed,
            *           so the sequence is something like:
            *           beginTag() ;
            *           ...
            *           endTag() ;
            *           moveToLastChild() ; // Goes back to previous tag
            *           add an extra attribute() ;
            *           moveToParent() ;    // Go back to parent
            *           ... continue on
            *************************************************************/
            bool MoveCurrentToParent()
            {
                assert(m_pCurrentTag) ;
                if (!m_pCurrentTag)
                {
                    return false ;
                }
                
                // Update the current tag to be its parent
                // (Note: GetParent stores return value in parameter passed in)
                bool hasParent = m_pCurrentTag->GetParent(m_pCurrentTag) ;
                
                assert(hasParent) ;
                
                return hasParent ;
            }
            bool MoveCurrentToChild(int index)
            {
                assert(m_pCurrentTag) ;
                if (!m_pCurrentTag)
                {
                    return false ;
                }
                
                return m_pCurrentTag->GetChild(m_pCurrentTag, index) ;
            }
            bool MoveCurrentToLastChild()
            {
                assert(m_pCurrentTag) ;
                if (!m_pCurrentTag)
                {
                    return false ;
                }
                
                return MoveCurrentToChild(m_pCurrentTag->GetNumberChildren() - 1) ;
            }
    } ;
    
} // namespace

#endif // SOARJSON_JSONTRACE_H
