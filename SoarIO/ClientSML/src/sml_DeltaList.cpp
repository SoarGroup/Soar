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

#include "sml_DeltaList.h"
#include "sml_ClientWMElement.h"

using namespace sml ;

// Delete the wm because we do own this one.
RemoveDelta::~RemoveDelta() { delete m_Element ; }
