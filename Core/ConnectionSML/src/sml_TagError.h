/////////////////////////////////////////////////////////////////
// TagError class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// Represents an SML node which represents an error in a result with
// the tag <error>.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_TAG_ERROR_H
#define SML_TAG_ERROR_H

#include "ElementXML.h"
#include "sml_Names.h"
#include "misc.h"

namespace sml
{

    class TagError : public soarxml::ElementXML
    {
        public:
            TagError(void);
            ~TagError(void);
            
            void SetDescription(char const* pErrorMsg)
            {
                this->SetCharacterData(pErrorMsg) ;
            }
            
            void SetErrorCode(int error)
            {
                char buf[TO_C_STRING_BUFSIZE];
                this->AddAttributeFast(sml_Names::kErrorCode, to_c_string(error, buf)) ;
            }
            
    };
    
}

#endif  // SML_TAG_ERROR_H
