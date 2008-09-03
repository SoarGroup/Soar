/********************************************************************
* @file igski_agentperformancemonitor.h 
*********************************************************************
* @remarks Copyright (C) 2004 Soar Technology, All rights reserved. 
* The U.S. government has non-exclusive license to this software 
* for government purposes. 
*********************************************************************
* created:	   6/2/2004   10:40
*
* purpose: 
*********************************************************************/
#ifndef IGSKI_AGENTPERFORMANCEMONITOR_H
#define IGSKI_AGENTPERFORMANCEMONITOR_H

namespace gSKI {

   class IAgentPerformanceMonitor
   {
   public:
      virtual ~IAgentPerformanceMonitor() {}

      virtual bool GetStatsString(int argc, char* argv[], 
                                  const char** result) = 0;

   }; // IAgentPerformanceMonitor

}

#endif // IGSKI_AGENTPERFORMANCEMONITOR_H
