#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_kernelfactory.cpp 
*********************************************************************
* created:	   6/26/2002   16:00
*
* purpose: 
*********************************************************************/
#include "gSKI_KernelFactory.h"
#include "gSKI_Error.h"
#include "gSKI.h"
#include "gSKI_Kernel.h"
#include "gSKI_Iterator.h"

#include "kernel.h"

//#include "MegaUnitTest.h"
//DEF_EXPOSE(gSKI_KernelFactory);

using namespace std;

namespace gSKI {

   KernelFactory::tInstanceInfoVec::t KernelFactory::m_instances;
   KernelFactory::tKernelVec::t       KernelFactory::m_kernels;

   /*
   =========================
 _  __                    _ _____          _
| |/ /___ _ __ _ __   ___| |  ___|_ _  ___| |_ ___  _ __ _   _
| ' // _ \ '__| '_ \ / _ \ | |_ / _` |/ __| __/ _ \| '__| | | |
| . \  __/ |  | | | |  __/ |  _| (_| | (__| || (_) | |  | |_| |
|_|\_\___|_|  |_| |_|\___|_|_|  \__,_|\___|\__\___/|_|   \__, |
                                                         |___/
   =========================
   */
   KernelFactory::KernelFactory()
   {

   }


   /*
   =========================
  ____      _    ____ ____  _  _______     __            _
 / ___| ___| |_ / ___/ ___|| |/ /_ _\ \   / /__ _ __ ___(_) ___  _ __
| |  _ / _ \ __| |  _\___ \| ' / | | \ \ / / _ \ '__/ __| |/ _ \| '_ \
| |_| |  __/ |_| |_| |___) | . \ | |  \ V /  __/ |  \__ \ | (_) | | | |
 \____|\___|\__|\____|____/|_|\_\___|  \_/ \___|_|  |___/_|\___/|_| |_|
   =========================
   */
   Version KernelFactory::GetGSKIVersion(Error* err)const 
   {
      ClearError(err);
      Version v = {gSKI::MajorVersionNumber, gSKI::MinorVersionNumber, gSKI::MicroVersionNumber };
      return v;
   }

   /*
   =========================
  ____      _   _  __                    ___     __            _
 / ___| ___| |_| |/ /___ _ __ _ __   ___| \ \   / /__ _ __ ___(_) ___  _ __
| |  _ / _ \ __| ' // _ \ '__| '_ \ / _ \ |\ \ / / _ \ '__/ __| |/ _ \| '_ \
| |_| |  __/ |_| . \  __/ |  | | | |  __/ | \ V /  __/ |  \__ \ | (_) | | | |
 \____|\___|\__|_|\_\___|_|  |_| |_|\___|_|  \_/ \___|_|  |___/_|\___/|_| |_|
   =========================
   */
   Version KernelFactory::GetKernelVersion(Error* err) const 
   {
      ClearError(err); 
      Version v = {MAJOR_VERSION_NUMBER, MINOR_VERSION_NUMBER, MICRO_VERSION_NUMBER };
      return (v);
   }

   /*
   =========================
  ____             ____                _       ___      
 / ___|__ _ _ __  / ___|_ __ ___  __ _| |_ ___|_ _|_ __ 
| |   / _` | '_ \| |   | '__/ _ \/ _` | __/ _ \| || '_ \
| |__| (_| | | | | |___| | |  __/ (_| | ||  __/| || | | |
 \____\__,_|_| |_|\____|_|  \___|\__,_|\__\___|___|_| |_|
 ____
|  _ \ _ __ ___   ___ ___ ___ ___
| |_) | '__/ _ \ / __/ _ Y __/ __|
|  __/| | | (_) | (_|  __|__ \__ \
|_|   |_|  \___/ \___\___|___/___/

   =========================
   */
   bool KernelFactory::CanCreateInProcess(Error* err) const 
   {
      ClearError(err);
   
      //
      // Because this base API always need to be in-process,
      // this should always return true.
      return true;
   }

   /*
   =========================
  ____               _   _   _             _     _                    _
 / ___|__ _ _ __    / \ | |_| |_ __ _  ___| |__ | |    ___   ___ __ _| |
| |   / _` | '_ \  / _ \| __| __/ _` |/ __| '_ \| |   / _ \ / __/ _` | |
| |__| (_| | | | |/ ___ \ |_| || (_| | (__| | | | |__| (_) | (_| (_| | |
 \____\__,_|_| |_/_/   \_\__|\__\__,_|\___|_| |_|_____\___/ \___\__,_|_|
  ___        _    ___   __ ____
 / _ \ _   _| |_ / _ \ / _|  _ \ _ __ ___   ___ ___ ___ ___
| | | | | | | __| | | | |_| |_) | '__/ _ \ / __/ _ Y __/ __|
| |_| | |_| | |_| |_| |  _|  __/| | | (_) | (_|  __|__ \__ \
 \___/ \__,_|\__|\___/|_| |_|   |_|  \___/ \___\___|___/___/

   =========================
   */
   bool KernelFactory::CanAttachLocalOutOfProcess(Error* err) const 
   {
      ClearError(err);
   
      //
      // Because this base API always need to be in-process,
      // this should always return false.
      return false;
   }

   /*
   =========================
  ____             ____                _       _                    _ 
 / ___|__ _ _ __  / ___|_ __ ___  __ _| |_ ___| |    ___   ___ __ _| |
| |   / _` | '_ \| |   | '__/ _ \/ _` | __/ _ \ |   / _ \ / __/ _` | |
| |__| (_| | | | | |___| | |  __/ (_| | ||  __/ |__| (_) | (_| (_| | |
 \____\__,_|_| |_|\____|_|  \___|\__,_|\__\___|_____\___/ \___\__,_|_|
  ___        _    ___   __ ____
 / _ \ _   _| |_ / _ \ / _|  _ \ _ __ ___   ___ ___ ___ ___
| | | | | | | __| | | | |_| |_) | '__/ _ \ / __/ _ Y __/ __|
| |_| | |_| | |_| |_| |  _|  __/| | | (_) | (_|  __|__ \__ \
 \___/ \__,_|\__|\___/|_| |_|   |_|  \___/ \___\___|___/___/

   =========================
   */
   bool KernelFactory::CanCreateLocalOutOfProcess(Error* err) const 
   {
      ClearError(err);

      //
      // Because this base API always need to be in-process,
      // this should always return false.
      return false;
   }

   /*
   =========================
  ____               _   _   _             _    
 / ___|__ _ _ __    / \ | |_| |_ __ _  ___| |__ 
| |   / _` | '_ \  / _ \| __| __/ _` |/ __| '_ \
| |__| (_| | | | |/ ___ \ |_| || (_| | (__| | | |
 \____\__,_|_| |_/_/   \_\__|\__\__,_|\___|_| |_|
 ____                      _       _
|  _ \ ___ _ __ ___   ___ | |_ ___| |_   _
| |_) / _ \ '_ ` _ \ / _ \| __/ _ \ | | | |
|  _ <  __/ | | | | | (_) | ||  __/ | |_| |
|_| \_\___|_| |_| |_|\___/ \__\___|_|\__, |
                                     |___/                                      
   =========================
   */
   bool KernelFactory::CanAttachRemote(Error* err) const 
   {
      ClearError(err);
   
      //
      // Because this base API always need to be in-process,
      // this should always return false.
      return false;
   }

   /*
   =========================
  ____             ____                _       ____                      _       _
 / ___|__ _ _ __  / ___|_ __ ___  __ _| |_ ___|  _ \ ___ _ __ ___   ___ | |_ ___| |_   _
| |   / _` | '_ \| |   | '__/ _ \/ _` | __/ _ \ |_) / _ \ '_ ` _ \ / _ \| __/ _ \ | | | |
| |__| (_| | | | | |___| | |  __/ (_| | ||  __/  _ <  __/ | | | | | (_) | ||  __/ | |_| |
 \____\__,_|_| |_|\____|_|  \___|\__,_|\__\___|_| \_\___|_| |_| |_|\___/ \__\___|_|\__, |
   =========================                                                       |___/
   */
   bool KernelFactory::CanCreateRemote(Error* err) const 
   {
      ClearError(err);
   
      //
      // Because this base API always need to be in-process,
      // this should always return false.
      return false;
   }

   /*
   =========================
  ____             ____                _       __  __       _ _   _       _     
 / ___|__ _ _ __  / ___|_ __ ___  __ _| |_ ___|  \/  |_   _| | |_(_)_ __ | | ___
| |   / _` | '_ \| |   | '__/ _ \/ _` | __/ _ \ |\/| | | | | | __| | '_ \| |/ _ \
| |__| (_| | | | | |___| | |  __/ (_| | ||  __/ |  | | |_| | | |_| | |_) | |  __/
 \____\__,_|_| |_|\____|_|  \___|\__,_|\__\___|_|  |_|\__,_|_|\__|_| .__/|_|\___|
 ___           _                                                   |_|
|_ _|_ __  ___| |_ __ _ _ __   ___ ___ ___
 | || '_ \/ __| __/ _` | '_ \ / __/ _ Y __|
 | || | | \__ \ || (_| | | | | (_|  __|__ \
|___|_| |_|___/\__\__,_|_| |_|\___\___|___/                                                    
   =========================
   */
   bool KernelFactory::CanCreateMultipleInstances(Error* err) const 
   {
      ClearError(err);
      return true;
   }

   /*
   =========================
  ____      _   ___           _                       ___ _                 _
 / ___| ___| |_|_ _|_ __  ___| |_ __ _ _ __   ___ ___|_ _| |_ ___ _ __ __ _| |_ ___  _ __
| |  _ / _ \ __|| || '_ \/ __| __/ _` | '_ \ / __/ _ \| || __/ _ \ '__/ _` | __/ _ \| '__|
| |_| |  __/ |_ | || | | \__ \ || (_| | | | | (_|  __/| || ||  __/ | | (_| | || (_) | |
 \____|\___|\__|___|_| |_|___/\__\__,_|_| |_|\___\___|___|\__\___|_|  \__,_|\__\___/|_|
   =========================
   */
   tIInstanceInfoIterator* KernelFactory::GetInstanceIterator(Error* err) const
   {
      ClearError(err);

      tInstanceInfoIter *InstIter = new tInstanceInfoIter(m_instances);
   
      return InstIter;
   }

   /*
   =========================
  ____                _
 / ___|_ __ ___  __ _| |_ ___
| |   | '__/ _ \/ _` | __/ _ \
| |___| | |  __/ (_| | ||  __/
 \____|_|  \___|\__,_|\__\___|
   =========================
   */
   IKernel* KernelFactory::Create(const char*           szInstanceName,
                                  egSKIThreadingModel   eTModel, 
                                  egSKIProcessType      ePType, 
                                  const char*           szLocation, 
                                  const char*           szLogLocation,
                                  egSKILogActivityLevel eLogActivity,
                                  Error*                err) const
   {
      ClearError(err);

      IKernel* newKernel = new Kernel(this);

      m_kernels.push_back(const_cast<const IKernel *>(newKernel));
      m_instances.push_back(newKernel->GetInstanceInformation());

      return newKernel;
   }

   /*
   ==================================
 ____           _                   _  __                    _
|  _ \  ___ ___| |_ _ __ ___  _   _| |/ /___ _ __ _ __   ___| |
| | | |/ _ Y __| __| '__/ _ \| | | | ' // _ \ '__| '_ \ / _ \ |
| |_| |  __|__ \ |_| | | (_) | |_| | . \  __/ |  | | | |  __/ |
|____/ \___|___/\__|_|  \___/ \__, |_|\_\___|_|  |_| |_|\___|_|
                              |___/
   ==================================
   */
   void KernelFactory::DestroyKernel(IKernel *krnl, Error *err)
   {
      m_instances.erase(std::find(m_instances.begin(), m_instances.end(), krnl->GetInstanceInformation()));
      m_kernels.erase(std::find(m_kernels.begin(), m_kernels.end(), krnl));

	  // These two vectors are statics so they tend to leak memory on shutdown
	  // even if we erased all objects because the vectors are still reversing extra space.
	  // An explicit clear call helps make sure they aren't reported as leaking.
	  if (m_instances.size() == 0)
		  m_instances.clear() ;
	  if (m_kernels.size() == 0)
		  m_kernels.clear() ;

      delete(krnl);
   }


   /*
   =========================
    _   _   _             _
   / \ | |_| |_ __ _  ___| |__
  / _ \| __| __/ _` |/ __| '_ \
 / ___ \ |_| || (_| | (__| | | |
/_/   \_\__|\__\__,_|\___|_| |_|
   =========================
   */
   IKernel* KernelFactory::Attach(const IInstanceInfo* pInstanceInfo, 
                                  Error* err) const 
   {
      ClearError(err);
   
      return 0;
   }

   /*
   =========================
 ____           _
|  _ \  ___ ___| |_ _ __ ___  _   _
| | | |/ _ Y __| __| '__/ _ \| | | |
| |_| |  __|__ \ |_| | | (_) | |_| |
|____/ \___|___/\__|_|  \___/ \__, |
                              |___/
   =========================
   */
   bool KernelFactory::Release(Error* err)
   {
      delete(this);
	  return true ;
   }

  // TODO: Implement this function properly
  bool KernelFactory::IsClientOwned(Error* err) const
  { 
    MegaAssert(false, "Properly implement this method!");
    return false;
  }
    
}
