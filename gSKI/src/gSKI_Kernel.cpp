#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H
#include "portability.h"

/*************************************************************************
 * PLEASE SEE THE FILE "COPYING" (INCLUDED WITH THIS SOFTWARE PACKAGE)
 * FOR LICENSE AND COPYRIGHT INFORMATION. 
 *************************************************************************/

/********************************************************************
* @file gski_kernel.cpp 
*********************************************************************
* created:	   6/27/2002   10:44
*
* purpose: 
*********************************************************************/
#include "gSKI_Kernel.h"
#include "gSKI_Error.h"
#include "gSKI_InstanceInfo.h"
#include "gSKI_AgentManager.h"
#include "gSKI_EnumRemapping.h"
#include "kernel_struct.h"
#include "MegaAssert.h"

//#include "MegaUnitTest.h"
//DEF_EXPOSE(gSKI_Kernel);

//
// HACK!
#include "gSKI_DoNotTouch.h"

namespace gSKI
{

   /*
   =========================
 _  __                    _
| |/ /___ _ __ _ __   ___| |
| ' // _ \ '__| '_ \ / _ \ |
| . \  __/ |  | | | |  __/ |
|_|\_\___|_|  |_| |_|\___|_|
   =========================
   */
   Kernel::Kernel(const KernelFactory* kf) : m_kF(kf)
   {
      m_soarKernel   = create_kernel();
	  m_InterruptCheckRate = 10 ;
	  m_stopPoint    = gSKI_INPUT_PHASE;
      init_soar( m_soarKernel );
      m_instanceInfo = new InstanceInfo("test1", "test2", gSKI_IN_PROCESS, gSKI_SINGLE_THREAD);
      m_agentMgr     = new AgentManager(this);
   }

   /*
   ==================================
 /\/|  __                    _
|/\/ |/ /___ _ __ _ __   ___| |
   | ' // _ \ '__| '_ \ / _ \ |
   | . \  __/ |  | | | |  __/ |
   |_|\_\___|_|  |_| |_|\___|_|
   ==================================
   */
   Kernel::~Kernel()
   {
      delete m_agentMgr; m_agentMgr = 0;
      delete m_instanceInfo; m_instanceInfo = 0;
      destroy_kernel(m_soarKernel);
      m_soarKernel = 0;
   }

   /*
   =========================
  ____      _      _                    _   __  __
 / ___| ___| |_   / \   __ _  ___ _ __ | |_|  \/  | __ _ _ __   __ _  __ _  ___ _ __
| |  _ / _ \ __| / _ \ / _` |/ _ \ '_ \| __| |\/| |/ _` | '_ \ / _` |/ _` |/ _ \ '__|
| |_| |  __/ |_ / ___ \ (_| |  __/ | | | |_| |  | | (_| | | | | (_| | (_| |  __/ |
 \____|\___|\__/_/   \_\__, |\___|_| |_|\__|_|  |_|\__,_|_| |_|\__,_|\__, |\___|_|
                       |___/                                         |___/
   =========================
   */
   AgentManager* Kernel::GetAgentManager(Error* err)
   {
      ClearError(err);

      return(m_agentMgr); 
   }


   /*
   =========================
  ____      _   _  __                    _ _____          _
 / ___| ___| |_| |/ /___ _ __ _ __   ___| |  ___|_ _  ___| |_ ___  _ __ _   _
| |  _ / _ \ __| ' // _ \ '__| '_ \ / _ \ | |_ / _` |/ __| __/ _ \| '__| | | |
| |_| |  __/ |_| . \  __/ |  | | | |  __/ |  _| (_| | (__| || (_) | |  | |_| |
 \____|\___|\__|_|\_\___|_|  |_| |_|\___|_|_|  \__,_|\___|\__\___/|_|   \__, |
                                                                        |___/
   =========================
   */
   const KernelFactory* Kernel::GetKernelFactory(Error* err) const
   {
      return m_kF;
   }


   /*
   =========================
  ____      _   ____            __
 / ___| ___| |_|  _ \ ___ _ __ / _| ___  _ __ _ __ ___   __ _ _ __   ___ ___
| |  _ / _ \ __| |_) / _ \ '__| |_ / _ \| '__| '_ ` _ \ / _` | '_ \ / __/ _ \
| |_| |  __/ |_|  __/  __/ |  |  _| (_) | |  | | | | | | (_| | | | | (_|  __/
 \____|\___|\__|_|  _\___|_|  |_|  \___/|_|  |_| |_| |_|\__,_|_| |_|\___\___|
|  \/  | ___  _ __ (_) |_ ___  _ __
| |\/| |/ _ \| '_ \| | __/ _ \| '__|
| |  | | (_) | | | | | || (_) | |
|_|  |_|\___/|_| |_|_|\__\___/|_|
   =========================
   */
   IPerformanceMonitor* Kernel::GetPerformanceMonitor(Error* err)
   {
      ClearError(err);
   
      return 0;
   }

   /*
   =========================
  ____      _   ___           _
 / ___| ___| |_|_ _|_ __  ___| |_ __ _ _ __   ___ ___
| |  _ / _ \ __|| || '_ \/ __| __/ _` | '_ \ / __/ _ \
| |_| |  __/ |_ | || | | \__ \ || (_| | | | | (_|  __/
 \____|\___|\__|___|_| |_|___/\__\__,_|_| |_|\___\___|
|_ _|_ __  / _| ___  _ __ _ __ ___   __ _| |_(_) ___  _ __
 | || '_ \| |_ / _ \| '__| '_ ` _ \ / _` | __| |/ _ \| '_ \
 | || | | |  _| (_) | |  | | | | | | (_| | |_| | (_) | | | |
|___|_| |_|_|  \___/|_|  |_| |_| |_|\__,_|\__|_|\___/|_| |_|
      =========================
   */
   InstanceInfo* Kernel::GetInstanceInformation(Error* err)
   {
      ClearError(err);
   
      return m_instanceInfo;
   }




   /*
   =========================
    _       _     _ ____            _                 _     _     _
   / \   __| | __| / ___| _   _ ___| |_ ___ _ __ ___ | |   (_)___| |_ ___ _ __   ___ _ __
  / _ \ / _` |/ _` \___ \| | | / __| __/ _ \ '_ ` _ \| |   | / __| __/ _ \ '_ \ / _ \ '__|
 / ___ \ (_| | (_| |___) | |_| \__ \ ||  __/ | | | | | |___| \__ \ ||  __/ | | |  __/ |
/_/   \_\__,_|\__,_|____/ \__, |___/\__\___|_| |_| |_|_____|_|___/\__\___|_| |_|\___|_|
                          |___/
      =========================
      */
      void Kernel::AddSystemListener(egSKISystemEventId  nEventId, 
                             ISystemListener*    pListener, 
                             bool                bAllowAsynch,
                             Error*              err)
      {
         ClearError(err);
         m_systemListeners.AddListener(nEventId, pListener);
      }


   /*
   =========================
 ____                               ____            _
|  _ \ ___ _ __ ___   _____   _____/ ___| _   _ ___| |_ ___ _ __ ___
| |_) / _ \ '_ ` _ \ / _ \ \ / / _ \___ \| | | / __| __/ _ \ '_ ` _ \
|  _ <  __/ | | | | | (_) \ V /  __/___) | |_| \__ \ ||  __/ | | | | |
|_| \_\___|_|_|_| |_|\___/ \_/ \___|____/ \__, |___/\__\___|_| |_| |_|
| |   (_)___| |_ ___ _ __   ___ _ __      |___/
| |   | / __| __/ _ \ '_ \ / _ \ '__|
| |___| \__ \ ||  __/ | | |  __/ |
|_____|_|___/\__\___|_| |_|\___|_|
      =========================
      */
      void Kernel::RemoveSystemListener(egSKISystemEventId  nEventId,
                                ISystemListener*     pListener,
                                Error*               err)
      {
         ClearError(err);
         m_systemListeners.RemoveListener(nEventId, pListener);
      }

	  /**************************************************
	   *
	   * Listen for rhs user function firings
	   *
	   **************************************************/
	  void Kernel::AddRhsListener(egSKIRhsEventId   nEventId, 
                             IRhsListener*       pListener, 
                             bool                bAllowAsynch,
                             Error*              err)
      {
         ClearError(err);
         m_rhsListeners.AddListener(nEventId, pListener);
      }

	  /**************************************************
	   *
	   * Stop listening for rhs user function firings
	   *
	   **************************************************/
      void Kernel::RemoveRhsListener(egSKIRhsEventId    nEventId,
                                IRhsListener*        pListener,
                                Error*               err)
      {
         ClearError(err);
         m_rhsListeners.RemoveListener(nEventId, pListener);
      }

	  /**************************************************
	   *
	   * Notify listeners about a RHS user function firing.
	   * The listeners can provide the return value for this function.
	   *
	   * If this function returns true, pReturnValue should be filled in with the return value.
	   * maxReturnValueLength indicates the size of the pReturnValue buffer (which is allocated by the
	   * caller of this function not the listener who responds).
	   *
	   * If "commandLine" is true then we will execute this with the command line processor
	   * rather than a generic user function that the user provides.
	   *
	   **************************************************/
	  bool Kernel::FireRhsNotification(Agent* pAgent, bool commandLine, char const* pFunctionName, char const* pArgument,
									   int maxReturnValueLength, char* pReturnValue)
	  {
			RhsNotifier rhs(pAgent, commandLine, pFunctionName, pArgument, maxReturnValueLength, pReturnValue) ;
			bool result = m_rhsListeners.NotifyGetResult(gSKIEVENT_RHS_USER_FUNCTION, rhs) ;

			return result ;
	  }

	  /** Notify listeners that they should start the "system" (the simulation usually) */
	  void Kernel::FireSystemStart()
	  {
		  SystemNotifier sys(this) ;
		  m_systemListeners.Notify(gSKIEVENT_SYSTEM_START, sys) ;
	  }

	  /** Notify listeners that they should stop the "system" (the simulation usually) */
	  void Kernel::FireSystemStop()
	  {
		  SystemNotifier sys(this) ;
		  m_systemListeners.Notify(gSKIEVENT_SYSTEM_STOP, sys) ;
	  }

	  /** Notify listeners that Soar is running and give them a chance to interrupt it (without having to start up separate threads etc.) */
	  void Kernel::FireInterruptCheckEvent()
	  {
		  SystemNotifier sys(this) ;
		  m_systemListeners.Notify(gSKIEVENT_INTERRUPT_CHECK, sys) ;
	  }

	  /** Notify listeners that a property has changed within the system.  We're not defining which property at this point so a client
	      who wishes to know about a specific property listens for this event and then queries for the property they care about (which may or
		  may not have changed). */
	  void Kernel::FireSystemPropertyChangedEvent()
	  {
		  SystemNotifier sys(this) ;
		  m_systemListeners.Notify(gSKIEVENT_SYSTEM_PROPERTY_CHANGED, sys) ;
	  }


   /*
   =========================
    _       _     _  ____                            _   _
   / \   __| | __| |/ ___|___  _ __  _ __   ___  ___| |_(_) ___  _ __
  / _ \ / _` |/ _` | |   / _ \| '_ \| '_ \ / _ \/ __| __| |/ _ \| '_ \
 / ___ \ (_| | (_| | |__| (_) | | | | | | |  __/ (__| |_| | (_) | | | |
/_/   \_\__,_|\__,_|\____\___/|_| |_|_| |_|\___|\___|\__|_|\___/|_| |_|
| |    ___  ___| |_| |   (_)___| |_ ___ _ __   ___ _ __
| |   / _ \/ __| __| |   | / __| __/ _ \ '_ \ / _ \ '__|
| |__| (_) \__ \ |_| |___| \__ \ ||  __/ | | |  __/ |
|_____\___/|___/\__|_____|_|___/\__\___|_| |_|\___|_|
      =========================
      */
      void Kernel::AddConnectionLostListener(egSKISystemEventId       nEventId, 
                                             IConnectionLostListener* pListener, 
                                             bool                     bAllowAsynch,
                                             Error*                   err)
      {
         ClearError(err);
      
      }


   /*
   =========================
 ____                                ____                            _   _
|  _ \ ___ _ __ ___   _____   _____ / ___|___  _ __  _ __   ___  ___| |_(_) ___  _ __
| |_) / _ \ '_ ` _ \ / _ \ \ / / _ \ |   / _ \| '_ \| '_ \ / _ \/ __| __| |/ _ \| '_ \
|  _ <  __/ | | | | | (_) \ V /  __/ |__| (_) | | | | | | |  __/ (__| |_| | (_) | | | |
|_| \_\___|_| |_| |_|\___/_\_/ \___|\____\___/|_| |_|_| |_|\___|\___|\__|_|\___/|_| |_|
| |    ___  ___| |_| |   (_)___| |_ ___ _ __   ___ _ __
| |   / _ \/ __| __| |   | / __| __/ _ \ '_ \ / _ \ '__|
| |__| (_) \__ \ |_| |___| \__ \ ||  __/ | | |  __/ |
|_____\___/|___/\__|_____|_|___/\__\___|_| |_|\___|_|
      =========================
      */
      void Kernel::RemoveConnectionLostListener(egSKISystemEventId       nEventId,
                                                IConnectionLostListener* pListener,
                                                Error*                   err)
      {
         ClearError(err);
      
      }

   /*
   ==================================
 ____      _
|  _ \ ___| | ___  __ _ ___  ___
| |_) / _ \ |/ _ \/ _` / __|/ _ \
|  _ <  __/ |  __/ (_| \__ \  __/
|_| \_\___|_|\___|\__,_|___/\___|
   ==================================
   */
   void Kernel::Release(Error* err)
   {
      delete(this);
   }

   /*
   ==================================
   
   ==================================
   */
  EvilBackDoor::TgDWorkArounds* Kernel::getWorkaroundObject()
  {
    static  EvilBackDoor::TgDWorkArounds evilBackDoor;
    return &evilBackDoor;
  }

  int  Kernel::GetInterruptCheckRate() const			{ return m_InterruptCheckRate ; }
  void Kernel::SetInterruptCheckRate(int newRate)	{ if (newRate >= 1) m_InterruptCheckRate = newRate ; }

  unsigned long Kernel::GetStopPoint()                 {return m_stopPoint ; }
  void Kernel::SetStopPoint(egSKIRunType runStepSize, egSKIPhaseType m_StopBeforePhase)
  {
	  if ((gSKI_RUN_DECISION_CYCLE == runStepSize) || (gSKI_RUN_FOREVER == runStepSize)) {
		  m_stopPoint = m_StopBeforePhase ;
	  } else  {
		  m_stopPoint = gSKI_INPUT_PHASE ;
	  }
  }
}
