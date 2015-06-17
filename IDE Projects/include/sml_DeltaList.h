/////////////////////////////////////////////////////////////////
// DeltaList class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : Sept 2004
//
// This class records the list of changes that have
// occured to working memory since it was last sent
// to the kernel (the "delta").
//
/////////////////////////////////////////////////////////////////

#ifndef SML_DELTA_LIST_H
#define SML_DELTA_LIST_H

#include <vector>
#include "Export.h"

namespace sml
{

    class WMElement ;
    class TagWme ;
    
    class EXPORT DeltaList
    {
        protected:
            std::vector<TagWme*>        m_DeltaList ;
            
        public:
            DeltaList() { }
            
            ~DeltaList()
            {
                Clear(true) ;
            }
            
            // We make deleting the contents optional as
            // we may just be moving the tags out of here
            // and into a message for sending, in which case
            // we won't delete them.
            void Clear(bool deleteContents) ;
            
            void RemoveWME(long long timeTag) ;
            
            void AddWME(WMElement* pWME) ;
            
            void UpdateWME(long long timeTagToRemove, WMElement* pWME)
            {
                // This is equivalent to a remove of the old value followed by an add of the new
                // We could choose to use a single tag for this later on.
                RemoveWME(timeTagToRemove) ;
                AddWME(pWME) ;
            }
            
            int GetSize()
            {
                return (int)m_DeltaList.size() ;
            }
            TagWme* GetDelta(int i)
            {
                return m_DeltaList[i] ;
            }
    };
    
}//closes namespace

#endif //SML_DELTA_LIST_H
