using System;
using Microsoft.Dss.Core.Attributes;
using Microsoft.Dss.Core.Transforms;

#if NET_CF20
[assembly: ServiceDeclaration(DssServiceDeclaration.Transform, SourceAssemblyKey = @"cf.simpledashboard.y2006.m01, version=0.0.0.0, culture=neutral, publickeytoken=e2c165af91199ba2")]
#else
[assembly: ServiceDeclaration(DssServiceDeclaration.Transform, SourceAssemblyKey = @"simpledashboard.y2006.m01, version=0.0.0.0, culture=neutral, publickeytoken=e2c165af91199ba2")]
#endif
#if !URT_MINCLR
[assembly: System.Security.SecurityTransparent]
[assembly: System.Security.AllowPartiallyTrustedCallers]
#endif

namespace Dss.Transforms.TransformSimpleDashboard
{

    public class Transforms: TransformBase
    {

        public static object Transform_Microsoft_Robotics_Services_SimpleDashboard_Proxy_SimpleDashboardState_TO_Microsoft_Robotics_Services_SimpleDashboard_SimpleDashboardState(object transformFrom)
        {
            Microsoft.Robotics.Services.SimpleDashboard.SimpleDashboardState target = new Microsoft.Robotics.Services.SimpleDashboard.SimpleDashboardState();
            Microsoft.Robotics.Services.SimpleDashboard.Proxy.SimpleDashboardState from = transformFrom as Microsoft.Robotics.Services.SimpleDashboard.Proxy.SimpleDashboardState;
            target.Log = from.Log;
            target.LogFile = from.LogFile;
            return target;
        }


        public static object Transform_Microsoft_Robotics_Services_SimpleDashboard_SimpleDashboardState_TO_Microsoft_Robotics_Services_SimpleDashboard_Proxy_SimpleDashboardState(object transformFrom)
        {
            Microsoft.Robotics.Services.SimpleDashboard.Proxy.SimpleDashboardState target = new Microsoft.Robotics.Services.SimpleDashboard.Proxy.SimpleDashboardState();
            Microsoft.Robotics.Services.SimpleDashboard.SimpleDashboardState from = transformFrom as Microsoft.Robotics.Services.SimpleDashboard.SimpleDashboardState;
            target.Log = from.Log;
            target.LogFile = from.LogFile;
            return target;
        }

        static Transforms()
        {
            Register();
        }
        public static void Register()
        {
            AddProxyTransform(typeof(Microsoft.Robotics.Services.SimpleDashboard.Proxy.SimpleDashboardState), Transform_Microsoft_Robotics_Services_SimpleDashboard_Proxy_SimpleDashboardState_TO_Microsoft_Robotics_Services_SimpleDashboard_SimpleDashboardState);
            AddSourceTransform(typeof(Microsoft.Robotics.Services.SimpleDashboard.SimpleDashboardState), Transform_Microsoft_Robotics_Services_SimpleDashboard_SimpleDashboardState_TO_Microsoft_Robotics_Services_SimpleDashboard_Proxy_SimpleDashboardState);
        }
    }
}

