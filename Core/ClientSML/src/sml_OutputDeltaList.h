/////////////////////////////////////////////////////////////////
// OutputDeltaList class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// This class records the list of changes that have
// occured to the output-link since the client
// last asked for them.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_OUTPUT_DELTA_LIST_H
#define SML_OUTPUT_DELTA_LIST_H

#include <vector>
#include "sml_ClientIdentifier.h"
#include "sml_ClientWMElement.h"
#include "Export.h"

namespace sml
{

    class WMElement ;
    
    class EXPORT WMDelta
    {
        public:
            enum ChangeType { kAdded = 1, kRemoved } ;
            
        protected:
            ChangeType  m_ChangeType ;
            
            // If this is an element that has been removed then
            // we actually own this pointer.  Otherwise we don't.  Be careful.
            WMElement*  m_pWME ;
            
        public:
            WMDelta(ChangeType change, WMElement* pWME)
            {
                m_ChangeType = change ;
                m_pWME       = pWME ;
                
                //std::string wmeString;
                //pWME->DebugString(wmeString);
                //std::cout << "New delta: " << wmeString;
                //std::cout << ((change == kAdded) ? " add" : " remove") << std::endl;
            }
            
            ~WMDelta() ;
            
            ChangeType getChangeType()
            {
                return m_ChangeType ;
            }
            WMElement* getWME()
            {
                return m_pWME ;
            }
    } ;
    
    class EXPORT OutputDeltaList
    {
        protected:
            std::vector<WMDelta*>       m_DeltaList ;
            
        public:
            OutputDeltaList() { }
            
            ~OutputDeltaList()
            {
                Clear(true) ;
            }
            
            void Clear(bool deleteContents, bool clearJustAdded = false, bool clearChildrenModified = false)
            {
                //std::cout << "Clearing delta list " << deleteContents << clearJustAdded << clearChildrenModified;
                if (m_DeltaList.empty())
                {
                    //std::cout << " (empty)" << std::endl;
                    return;
                }
                
                if (clearJustAdded || clearChildrenModified)
                {
                    for (std::vector<WMDelta*>::iterator iter = m_DeltaList.begin(); iter != m_DeltaList.end(); ++iter)
                    {
                        WMElement* pWME = (*iter)->getWME();
                        //std::string wmeString;
                        //pWME->DebugString(wmeString);
                        //std::cout << std::endl << wmeString;
                        if (clearJustAdded)
                        {
                            pWME->SetJustAdded(false);
                        }
                        if (clearChildrenModified)
                        {
                            if (pWME->IsIdentifier())
                            {
                                Identifier* pID = static_cast< Identifier* >(pWME);
                                pID->m_pSymbol->SetAreChildrenModified(false);
                            }
                        }
                    }
                }
                
                if (deleteContents)
                {
                    int size = (int)m_DeltaList.size() ;
                    for (int i = 0 ; i < size ; i++)
                    {
                        delete m_DeltaList[i] ;
                    }
                }
                
                m_DeltaList.clear() ;
                //std::cout << std::endl;
            }
            
            void RemoveWME(WMElement* pWME)
            {
                WMDelta* pDelta = new WMDelta(WMDelta::kRemoved, pWME) ;
                m_DeltaList.push_back(pDelta) ;
            }
            
            void AddWME(WMElement* pWME)
            {
                WMDelta* pDelta = new WMDelta(WMDelta::kAdded, pWME) ;
                m_DeltaList.push_back(pDelta) ;
            }
            
            int GetSize()
            {
                return (int)m_DeltaList.size() ;
            }
            WMDelta* GetDeltaWME(int i)
            {
                return m_DeltaList[i] ;
            }
    };
    
}//closes namespace

#endif //SML_OUTPUT_DELTA_LIST_H
