using System;
using Microsoft.Dss.Core.Attributes;
using Microsoft.Dss.Core.Transforms;

#if NET_CF20
[assembly: ServiceDeclaration(DssServiceDeclaration.Transform, SourceAssemblyKey = @"cf.soarmazesimulator.y2006.m08, version=1.8.0.0, culture=neutral, publickeytoken=5e5b29d51466eb05")]
#else
[assembly: ServiceDeclaration(DssServiceDeclaration.Transform, SourceAssemblyKey = @"soarmazesimulator.y2006.m08, version=1.8.0.0, culture=neutral, publickeytoken=5e5b29d51466eb05")]
#endif
#if !URT_MINCLR
[assembly: System.Security.SecurityTransparent]
[assembly: System.Security.AllowPartiallyTrustedCallers]
#endif

namespace Dss.Transforms.TransformSoarMazeSimulator
{

    public class Transforms: TransformBase
    {

        public static object Transform_Robotics_SoarMazeSimulator_Proxy_SoarMazeSimulatorState_TO_Robotics_SoarMazeSimulator_SoarMazeSimulatorState(object transformFrom)
        {
            Robotics.SoarMazeSimulator.SoarMazeSimulatorState target = new Robotics.SoarMazeSimulator.SoarMazeSimulatorState();
            Robotics.SoarMazeSimulator.Proxy.SoarMazeSimulatorState from = transformFrom as Robotics.SoarMazeSimulator.Proxy.SoarMazeSimulatorState;
            target.Maze = from.Maze;
            target.GroundTexture = from.GroundTexture;

            // copy System.String[] target.WallTextures = from.WallTextures
            if (from.WallTextures != null)
            {
                target.WallTextures = new System.String[from.WallTextures.GetLength(0)];

                from.WallTextures.CopyTo(target.WallTextures, 0);
            }

            // copy Microsoft.Robotics.PhysicalModel.Vector3[] target.WallColors = from.WallColors
            if (from.WallColors != null)
            {
                target.WallColors = new Microsoft.Robotics.PhysicalModel.Vector3[from.WallColors.GetLength(0)];

                for (int d0 = 0; d0 < from.WallColors.GetLength(0); d0++)
                    target.WallColors[d0] = (Microsoft.Robotics.PhysicalModel.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Vector3_TO_Microsoft_Robotics_PhysicalModel_Vector3(from.WallColors[d0]);
            }

            // copy System.Single[] target.HeightMap = from.HeightMap
            if (from.HeightMap != null)
            {
                target.HeightMap = new System.Single[from.HeightMap.GetLength(0)];

                from.HeightMap.CopyTo(target.HeightMap, 0);
            }

            // copy System.Single[] target.MassMap = from.MassMap
            if (from.MassMap != null)
            {
                target.MassMap = new System.Single[from.MassMap.GetLength(0)];

                from.MassMap.CopyTo(target.MassMap, 0);
            }

            // copy System.Boolean[] target.UseSphere = from.UseSphere
            if (from.UseSphere != null)
            {
                target.UseSphere = new System.Boolean[from.UseSphere.GetLength(0)];

                from.UseSphere.CopyTo(target.UseSphere, 0);
            }
            target.WallBoxSize = from.WallBoxSize;
            target.GridSpacing = from.GridSpacing;
            target.HeightScale = from.HeightScale;
            target.DefaultHeight = from.DefaultHeight;
            target.SphereScale = from.SphereScale;
            target.RobotStartCellRow = from.RobotStartCellRow;
            target.RobotStartCellCol = from.RobotStartCellCol;
            target.RobotType = from.RobotType;
            return target;
        }


        public static object Transform_Robotics_SoarMazeSimulator_SoarMazeSimulatorState_TO_Robotics_SoarMazeSimulator_Proxy_SoarMazeSimulatorState(object transformFrom)
        {
            Robotics.SoarMazeSimulator.Proxy.SoarMazeSimulatorState target = new Robotics.SoarMazeSimulator.Proxy.SoarMazeSimulatorState();
            Robotics.SoarMazeSimulator.SoarMazeSimulatorState from = transformFrom as Robotics.SoarMazeSimulator.SoarMazeSimulatorState;
            target.Maze = from.Maze;
            target.GroundTexture = from.GroundTexture;

            // copy System.String[] target.WallTextures = from.WallTextures
            if (from.WallTextures != null)
            {
                target.WallTextures = new System.String[from.WallTextures.GetLength(0)];

                from.WallTextures.CopyTo(target.WallTextures, 0);
            }

            // copy Microsoft.Robotics.PhysicalModel.Proxy.Vector3[] target.WallColors = from.WallColors
            if (from.WallColors != null)
            {
                target.WallColors = new Microsoft.Robotics.PhysicalModel.Proxy.Vector3[from.WallColors.GetLength(0)];

                for (int d0 = 0; d0 < from.WallColors.GetLength(0); d0++)
                    target.WallColors[d0] = (Microsoft.Robotics.PhysicalModel.Proxy.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Vector3_TO_Microsoft_Robotics_PhysicalModel_Proxy_Vector3(from.WallColors[d0]);
            }

            // copy System.Single[] target.HeightMap = from.HeightMap
            if (from.HeightMap != null)
            {
                target.HeightMap = new System.Single[from.HeightMap.GetLength(0)];

                from.HeightMap.CopyTo(target.HeightMap, 0);
            }

            // copy System.Single[] target.MassMap = from.MassMap
            if (from.MassMap != null)
            {
                target.MassMap = new System.Single[from.MassMap.GetLength(0)];

                from.MassMap.CopyTo(target.MassMap, 0);
            }

            // copy System.Boolean[] target.UseSphere = from.UseSphere
            if (from.UseSphere != null)
            {
                target.UseSphere = new System.Boolean[from.UseSphere.GetLength(0)];

                from.UseSphere.CopyTo(target.UseSphere, 0);
            }
            target.WallBoxSize = from.WallBoxSize;
            target.GridSpacing = from.GridSpacing;
            target.HeightScale = from.HeightScale;
            target.DefaultHeight = from.DefaultHeight;
            target.SphereScale = from.SphereScale;
            target.RobotStartCellRow = from.RobotStartCellRow;
            target.RobotStartCellCol = from.RobotStartCellCol;
            target.RobotType = from.RobotType;
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_Proxy_Vector3_TO_Microsoft_Robotics_PhysicalModel_Vector3(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.Vector3 target = new Microsoft.Robotics.PhysicalModel.Vector3();
            Microsoft.Robotics.PhysicalModel.Proxy.Vector3 from = (Microsoft.Robotics.PhysicalModel.Proxy.Vector3)transformFrom;
            target.X = from.X;
            target.Y = from.Y;
            target.Z = from.Z;
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_Vector3_TO_Microsoft_Robotics_PhysicalModel_Proxy_Vector3(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.Proxy.Vector3 target = new Microsoft.Robotics.PhysicalModel.Proxy.Vector3();
            Microsoft.Robotics.PhysicalModel.Vector3 from = (Microsoft.Robotics.PhysicalModel.Vector3)transformFrom;
            target.X = from.X;
            target.Y = from.Y;
            target.Z = from.Z;
            return target;
        }

        static Transforms()
        {
            Register();
        }
        public static void Register()
        {
            AddProxyTransform(typeof(Robotics.SoarMazeSimulator.Proxy.SoarMazeSimulatorState), Transform_Robotics_SoarMazeSimulator_Proxy_SoarMazeSimulatorState_TO_Robotics_SoarMazeSimulator_SoarMazeSimulatorState);
            AddSourceTransform(typeof(Robotics.SoarMazeSimulator.SoarMazeSimulatorState), Transform_Robotics_SoarMazeSimulator_SoarMazeSimulatorState_TO_Robotics_SoarMazeSimulator_Proxy_SoarMazeSimulatorState);
            AddProxyTransform(typeof(Microsoft.Robotics.PhysicalModel.Proxy.Vector3), Transform_Microsoft_Robotics_PhysicalModel_Proxy_Vector3_TO_Microsoft_Robotics_PhysicalModel_Vector3);
            AddSourceTransform(typeof(Microsoft.Robotics.PhysicalModel.Vector3), Transform_Microsoft_Robotics_PhysicalModel_Vector3_TO_Microsoft_Robotics_PhysicalModel_Proxy_Vector3);
        }
    }
}

