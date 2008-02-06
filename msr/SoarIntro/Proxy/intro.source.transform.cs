using System;
using Microsoft.Dss.Core.Attributes;
using Microsoft.Dss.Core.Transforms;

#if NET_CF20
[assembly: ServiceDeclaration(DssServiceDeclaration.Transform, SourceAssemblyKey = @"cf.intro.y2006.m08, version=1.8.0.0, culture=neutral, publickeytoken=d183f0e3c2276485")]
#else
[assembly: ServiceDeclaration(DssServiceDeclaration.Transform, SourceAssemblyKey = @"intro.y2006.m08, version=1.8.0.0, culture=neutral, publickeytoken=d183f0e3c2276485")]
#endif
#if !URT_MINCLR
[assembly: System.Security.SecurityTransparent]
[assembly: System.Security.AllowPartiallyTrustedCallers]
#endif

namespace Dss.Transforms.TransformIntro
{

    public class Transforms: TransformBase
    {

        public static object Transform_Robotics_Intro_Proxy_IntroState_TO_Robotics_Intro_IntroState(object transformFrom)
        {
            Robotics.Intro.IntroState target = new Robotics.Intro.IntroState();
            Robotics.Intro.Proxy.IntroState from = transformFrom as Robotics.Intro.Proxy.IntroState;
            target.MotorOn = from.MotorOn;
            target.ForwardMovesOnly = from.ForwardMovesOnly;
            target.StopTimeout = from.StopTimeout;
            target.BackUpTimeout = from.BackUpTimeout;
            target.TurnTimeout = from.TurnTimeout;
            target.MinimumDriveTimeout = from.MinimumDriveTimeout;
            target.MinimumPower = from.MinimumPower;
            target.MaximumPower = from.MaximumPower;
            target.BackUpPower = from.BackUpPower;
            target.nextTimestamp = from.nextTimestamp;
            target.lastBumperNum = from.lastBumperNum;
            target.insideBehavior = from.insideBehavior;
            return target;
        }


        public static object Transform_Robotics_Intro_IntroState_TO_Robotics_Intro_Proxy_IntroState(object transformFrom)
        {
            Robotics.Intro.Proxy.IntroState target = new Robotics.Intro.Proxy.IntroState();
            Robotics.Intro.IntroState from = transformFrom as Robotics.Intro.IntroState;
            target.MotorOn = from.MotorOn;
            target.ForwardMovesOnly = from.ForwardMovesOnly;
            target.StopTimeout = from.StopTimeout;
            target.BackUpTimeout = from.BackUpTimeout;
            target.TurnTimeout = from.TurnTimeout;
            target.MinimumDriveTimeout = from.MinimumDriveTimeout;
            target.MinimumPower = from.MinimumPower;
            target.MaximumPower = from.MaximumPower;
            target.BackUpPower = from.BackUpPower;
            target.nextTimestamp = from.nextTimestamp;
            target.lastBumperNum = from.lastBumperNum;
            target.insideBehavior = from.insideBehavior;
            return target;
        }


        public static object Transform_Robotics_Intro_Proxy_MoveStates_TO_Robotics_Intro_MoveStates(object transformFrom)
        {
            Robotics.Intro.MoveStates target = new Robotics.Intro.MoveStates();
            return target;
        }


        public static object Transform_Robotics_Intro_MoveStates_TO_Robotics_Intro_Proxy_MoveStates(object transformFrom)
        {
            Robotics.Intro.Proxy.MoveStates target = new Robotics.Intro.Proxy.MoveStates();
            return target;
        }

        static Transforms()
        {
            Register();
        }
        public static void Register()
        {
            AddProxyTransform(typeof(Robotics.Intro.Proxy.IntroState), Transform_Robotics_Intro_Proxy_IntroState_TO_Robotics_Intro_IntroState);
            AddSourceTransform(typeof(Robotics.Intro.IntroState), Transform_Robotics_Intro_IntroState_TO_Robotics_Intro_Proxy_IntroState);
            AddProxyTransform(typeof(Robotics.Intro.Proxy.MoveStates), Transform_Robotics_Intro_Proxy_MoveStates_TO_Robotics_Intro_MoveStates);
            AddSourceTransform(typeof(Robotics.Intro.MoveStates), Transform_Robotics_Intro_MoveStates_TO_Robotics_Intro_Proxy_MoveStates);
        }
    }
}

