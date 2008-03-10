using System;
using Microsoft.Dss.Core.Attributes;
using Microsoft.Dss.Core.Transforms;

#if NET_CF20
[assembly: ServiceDeclaration(DssServiceDeclaration.Transform, SourceAssemblyKey = @"cf.soarmsrservice.y2008.m03, version=1.0.0.0, culture=neutral, publickeytoken=5e5b29d51466eb05")]
#else
[assembly: ServiceDeclaration(DssServiceDeclaration.Transform, SourceAssemblyKey = @"soarmsrservice.y2008.m03, version=1.0.0.0, culture=neutral, publickeytoken=5e5b29d51466eb05")]
#endif
#if !URT_MINCLR
[assembly: System.Security.SecurityTransparent]
[assembly: System.Security.AllowPartiallyTrustedCallers]
#endif

namespace Dss.Transforms.TransformSoarMSRService
{

    public class Transforms: TransformBase
    {

        public static object Transform_Robotics_SoarMSR_Proxy_SoarMSRState_TO_Robotics_SoarMSR_SoarMSRState(object transformFrom)
        {
            Robotics.SoarMSR.SoarMSRState target = new Robotics.SoarMSR.SoarMSRState();
            Robotics.SoarMSR.Proxy.SoarMSRState from = transformFrom as Robotics.SoarMSR.Proxy.SoarMSRState;
            target.AgentName = from.AgentName;
            target.Productions = from.Productions;
            target.SpawnDebugger = from.SpawnDebugger;
            target.HasRandomSeed = from.HasRandomSeed;
            target.RandomSeed = from.RandomSeed;
            target.DrivePower = from.DrivePower;
            target.ReversePower = from.ReversePower;
            target.StopTimeout = from.StopTimeout;
            target.BackUpTimeout = from.BackUpTimeout;
            target.TurnTimeout = from.TurnTimeout;
            target.TimeoutVariance = from.TimeoutVariance;
            return target;
        }


        public static object Transform_Robotics_SoarMSR_SoarMSRState_TO_Robotics_SoarMSR_Proxy_SoarMSRState(object transformFrom)
        {
            Robotics.SoarMSR.Proxy.SoarMSRState target = new Robotics.SoarMSR.Proxy.SoarMSRState();
            Robotics.SoarMSR.SoarMSRState from = transformFrom as Robotics.SoarMSR.SoarMSRState;
            target.AgentName = from.AgentName;
            target.Productions = from.Productions;
            target.SpawnDebugger = from.SpawnDebugger;
            target.HasRandomSeed = from.HasRandomSeed;
            target.RandomSeed = from.RandomSeed;
            target.DrivePower = from.DrivePower;
            target.ReversePower = from.ReversePower;
            target.StopTimeout = from.StopTimeout;
            target.BackUpTimeout = from.BackUpTimeout;
            target.TurnTimeout = from.TurnTimeout;
            target.TimeoutVariance = from.TimeoutVariance;
            return target;
        }

        static Transforms()
        {
            Register();
        }
        public static void Register()
        {
            AddProxyTransform(typeof(Robotics.SoarMSR.Proxy.SoarMSRState), Transform_Robotics_SoarMSR_Proxy_SoarMSRState_TO_Robotics_SoarMSR_SoarMSRState);
            AddSourceTransform(typeof(Robotics.SoarMSR.SoarMSRState), Transform_Robotics_SoarMSR_SoarMSRState_TO_Robotics_SoarMSR_Proxy_SoarMSRState);
        }
    }
}

