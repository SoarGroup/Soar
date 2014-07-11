/////////////////////////////////////////////////////////////////
// TagResult class
//
// Author: Douglas Pearson, www.threepenny.net
// Date  : August 2004
//
// Represents an SML node which represents the result of a command with
// the tag <result>.
//
/////////////////////////////////////////////////////////////////

#ifndef SML_TAG_RESULT_H
#define SML_TAG_RESULT_H

#include "ElementXML.h"
#include "sml_Names.h"

namespace sml
{

    class TagResult : public soarxml::ElementXML
    {
        public:
            TagResult(void);
            ~TagResult(void);
    };
    
}

#endif  // SML_TAG_ERROR_H
