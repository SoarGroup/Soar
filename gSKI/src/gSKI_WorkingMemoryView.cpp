/********************************************************************
* @file gski_workingmemoryview.cpp
*********************************************************************
* @remarks Copyright (C) 2002 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   6/13/2002   15:00
*
* purpose: 
*********************************************************************/

#include "gSKI_WorkingMemoryView.h"
#include "gSKI_Error.h"
#include "MegaAssert.h"

//
// Explicit Export for this file.
//#include "MegaUnitTest.h"
//DEF_EXPOSE(gSKI_WorkingMemoryView);

namespace gSKI
{

  /*
    ===============================

    ===============================
  */

  WorkingMemoryView::~WorkingMemoryView() {}

  /*
    ===============================

    ===============================
  */

  IWMObject* WorkingMemoryView::GetObjectById(const char* idstring,
                                                Error* err) const
  {
    ClearError(err);

    MegaAssert(false, "NOT IMPLEMENTED YET!");

    return 0;
  }

  /*
    ===============================

    ===============================
  */

  tIWMObjectIterator* WorkingMemoryView::GetAllObjects(Error* err) const
  {
    ClearError(err);

    MegaAssert(false, "NOT IMPLEMENTED YET!");

    return 0;
  }

  /*
    ===============================

    ===============================
  */

  tIWmeIterator* WorkingMemoryView::GetAllWmes(Error* err) const
  {
    ClearError(err);

    MegaAssert(false, "NOT IMPLEMENTED YET!");

    return 0;
  }

  /*
    ===============================

    ===============================
  */

  tIWMObjectIterator* WorkingMemoryView::FindObjectsByCriteria(Error* err) const
  {
    ClearError(err);

    MegaAssert(false, "NOT IMPLEMENTED YET!");

    return 0;
  }

  /*
    ===============================

    ===============================
  */

  tIWmeIterator* WorkingMemoryView::FindWmesByCriteria(Error* err) const
  {
    ClearError(err);

    MegaAssert(false, "NOT IMPLEMENTED YET!");

    return 0;
  }

  /*
    ===============================

    ===============================
  */

  IWMStaticView* 
  WorkingMemoryView::CreateSubView(const IWMObject* rootobject,
                                     Error* err) const
  {
    ClearError(err);

    MegaAssert(false, "NOT IMPLEMENTED YET!");

    return 0;
  }

}
