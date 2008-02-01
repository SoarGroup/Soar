using System;
using Microsoft.Dss.Core.Attributes;
using Microsoft.Dss.Core.Transforms;

#if NET_CF20
[assembly: ServiceDeclaration(DssServiceDeclaration.Transform, SourceAssemblyKey = @"cf.simulationtutorial2.y2006.m06, version=0.0.0.0, culture=neutral, publickeytoken=e2c165af91199ba2")]
#else
[assembly: ServiceDeclaration(DssServiceDeclaration.Transform, SourceAssemblyKey = @"simulationtutorial2.y2006.m06, version=0.0.0.0, culture=neutral, publickeytoken=e2c165af91199ba2")]
#endif
#if !URT_MINCLR
[assembly: System.Security.SecurityTransparent]
[assembly: System.Security.AllowPartiallyTrustedCallers]
#endif

namespace Dss.Transforms.TransformSimulationTutorial2
{

    public class Transforms: TransformBase
    {

        public static object Transform_Robotics_SimulationTutorial2_Proxy_State_TO_Robotics_SimulationTutorial2_State(object transformFrom)
        {
            Robotics.SimulationTutorial2.State target = new Robotics.SimulationTutorial2.State();
            Robotics.SimulationTutorial2.Proxy.State from = transformFrom as Robotics.SimulationTutorial2.Proxy.State;
            target.Message = from.Message;
            target.TableCount = from.TableCount;
            return target;
        }


        public static object Transform_Robotics_SimulationTutorial2_State_TO_Robotics_SimulationTutorial2_Proxy_State(object transformFrom)
        {
            Robotics.SimulationTutorial2.Proxy.State target = new Robotics.SimulationTutorial2.Proxy.State();
            Robotics.SimulationTutorial2.State from = transformFrom as Robotics.SimulationTutorial2.State;
            target.Message = from.Message;
            target.TableCount = from.TableCount;
            return target;
        }


        public static object Transform_Robotics_SimulationTutorial2_Proxy_TableEntity_TO_Robotics_SimulationTutorial2_TableEntity(object transformFrom)
        {
            Robotics.SimulationTutorial2.TableEntity target = new Robotics.SimulationTutorial2.TableEntity();
            Robotics.SimulationTutorial2.Proxy.TableEntity from = transformFrom as Robotics.SimulationTutorial2.Proxy.TableEntity;

            // copy IEnumerable BoxShapes
            if (from.BoxShapes != null)
            {
                target.BoxShapes = new System.Collections.Generic.List<Microsoft.Robotics.Simulation.Physics.BoxShape>();
                foreach(Microsoft.Robotics.Simulation.Physics.Proxy.BoxShape elem in from.BoxShapes)
                {
                    target.BoxShapes.Add((elem == null) ? null : (Microsoft.Robotics.Simulation.Physics.BoxShape)Transform_Microsoft_Robotics_Simulation_Physics_Proxy_BoxShape_TO_Microsoft_Robotics_Simulation_Physics_BoxShape(elem));
                }
            }

            // copy IEnumerable SphereShapes
            if (from.SphereShapes != null)
            {
                target.SphereShapes = new System.Collections.Generic.List<Microsoft.Robotics.Simulation.Physics.SphereShape>();
                foreach(Microsoft.Robotics.Simulation.Physics.Proxy.SphereShape elem in from.SphereShapes)
                {
                    target.SphereShapes.Add((elem == null) ? null : (Microsoft.Robotics.Simulation.Physics.SphereShape)Transform_Microsoft_Robotics_Simulation_Physics_Proxy_SphereShape_TO_Microsoft_Robotics_Simulation_Physics_SphereShape(elem));
                }
            }
            target.Flags = (Microsoft.Robotics.Simulation.Engine.VisualEntityProperties)((System.Int32)from.Flags);
            target.ChildCount = from.ChildCount;
            target.ParentJoint = (from.ParentJoint == null) ? null : (Microsoft.Robotics.PhysicalModel.Joint)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Joint_TO_Microsoft_Robotics_PhysicalModel_Joint(from.ParentJoint);
            target.ReferenceFrame = (Microsoft.Robotics.Simulation.Engine.VisualEntity.ReferenceFrames)((System.Int32)from.ReferenceFrame);
            target.ServiceContract = from.ServiceContract;
            target.MeshScale = (Microsoft.Robotics.PhysicalModel.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Vector3_TO_Microsoft_Robotics_PhysicalModel_Vector3(from.MeshScale);
            target.MeshRotation = (Microsoft.Robotics.PhysicalModel.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Vector3_TO_Microsoft_Robotics_PhysicalModel_Vector3(from.MeshRotation);
            target.MeshTranslation = (Microsoft.Robotics.PhysicalModel.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Vector3_TO_Microsoft_Robotics_PhysicalModel_Vector3(from.MeshTranslation);
            target.State = (from.State == null) ? null : (Microsoft.Robotics.Simulation.EntityState)Transform_Microsoft_Robotics_Simulation_Proxy_EntityState_TO_Microsoft_Robotics_Simulation_EntityState(from.State);
            return target;
        }


        public static object Transform_Robotics_SimulationTutorial2_TableEntity_TO_Robotics_SimulationTutorial2_Proxy_TableEntity(object transformFrom)
        {
            Robotics.SimulationTutorial2.Proxy.TableEntity target = new Robotics.SimulationTutorial2.Proxy.TableEntity();
            Robotics.SimulationTutorial2.TableEntity from = transformFrom as Robotics.SimulationTutorial2.TableEntity;

            // copy IEnumerable BoxShapes
            if (from.BoxShapes != null)
            {
                target.BoxShapes = new System.Collections.Generic.List<Microsoft.Robotics.Simulation.Physics.Proxy.BoxShape>();
                foreach(Microsoft.Robotics.Simulation.Physics.BoxShape elem in from.BoxShapes)
                {
                    target.BoxShapes.Add((elem == null) ? null : (Microsoft.Robotics.Simulation.Physics.Proxy.BoxShape)Transform_Microsoft_Robotics_Simulation_Physics_BoxShape_TO_Microsoft_Robotics_Simulation_Physics_Proxy_BoxShape(elem));
                }
            }

            // copy IEnumerable SphereShapes
            if (from.SphereShapes != null)
            {
                target.SphereShapes = new System.Collections.Generic.List<Microsoft.Robotics.Simulation.Physics.Proxy.SphereShape>();
                foreach(Microsoft.Robotics.Simulation.Physics.SphereShape elem in from.SphereShapes)
                {
                    target.SphereShapes.Add((elem == null) ? null : (Microsoft.Robotics.Simulation.Physics.Proxy.SphereShape)Transform_Microsoft_Robotics_Simulation_Physics_SphereShape_TO_Microsoft_Robotics_Simulation_Physics_Proxy_SphereShape(elem));
                }
            }
            target.Flags = (Microsoft.Robotics.Simulation.Engine.Proxy.VisualEntityProperties)((System.Int32)from.Flags);
            target.ChildCount = from.ChildCount;
            target.ParentJoint = (from.ParentJoint == null) ? null : (Microsoft.Robotics.PhysicalModel.Proxy.Joint)Transform_Microsoft_Robotics_PhysicalModel_Joint_TO_Microsoft_Robotics_PhysicalModel_Proxy_Joint(from.ParentJoint);
            target.ReferenceFrame = (Microsoft.Robotics.Simulation.Engine.Proxy.VisualEntity.ReferenceFrames)((System.Int32)from.ReferenceFrame);
            target.ServiceContract = from.ServiceContract;
            target.MeshScale = (Microsoft.Robotics.PhysicalModel.Proxy.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Vector3_TO_Microsoft_Robotics_PhysicalModel_Proxy_Vector3(from.MeshScale);
            target.MeshRotation = (Microsoft.Robotics.PhysicalModel.Proxy.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Vector3_TO_Microsoft_Robotics_PhysicalModel_Proxy_Vector3(from.MeshRotation);
            target.MeshTranslation = (Microsoft.Robotics.PhysicalModel.Proxy.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Vector3_TO_Microsoft_Robotics_PhysicalModel_Proxy_Vector3(from.MeshTranslation);
            target.State = (from.State == null) ? null : (Microsoft.Robotics.Simulation.Proxy.EntityState)Transform_Microsoft_Robotics_Simulation_EntityState_TO_Microsoft_Robotics_Simulation_Proxy_EntityState(from.State);
            return target;
        }


        public static object Transform_Microsoft_Robotics_Simulation_Physics_Proxy_BoxShape_TO_Microsoft_Robotics_Simulation_Physics_BoxShape(object transformFrom)
        {
            Microsoft.Robotics.Simulation.Physics.BoxShape target = new Microsoft.Robotics.Simulation.Physics.BoxShape();
            Microsoft.Robotics.Simulation.Physics.Proxy.BoxShape from = transformFrom as Microsoft.Robotics.Simulation.Physics.Proxy.BoxShape;
            target.BoxState = (from.BoxState == null) ? null : (Microsoft.Robotics.Simulation.Physics.BoxShapeProperties)Transform_Microsoft_Robotics_Simulation_Physics_Proxy_BoxShapeProperties_TO_Microsoft_Robotics_Simulation_Physics_BoxShapeProperties(from.BoxState);
            return target;
        }


        public static object Transform_Microsoft_Robotics_Simulation_Physics_BoxShape_TO_Microsoft_Robotics_Simulation_Physics_Proxy_BoxShape(object transformFrom)
        {
            Microsoft.Robotics.Simulation.Physics.Proxy.BoxShape target = new Microsoft.Robotics.Simulation.Physics.Proxy.BoxShape();
            Microsoft.Robotics.Simulation.Physics.BoxShape from = transformFrom as Microsoft.Robotics.Simulation.Physics.BoxShape;
            target.BoxState = (from.BoxState == null) ? null : (Microsoft.Robotics.Simulation.Physics.Proxy.BoxShapeProperties)Transform_Microsoft_Robotics_Simulation_Physics_BoxShapeProperties_TO_Microsoft_Robotics_Simulation_Physics_Proxy_BoxShapeProperties(from.BoxState);
            return target;
        }


        public static object Transform_Microsoft_Robotics_Simulation_Physics_Proxy_BoxShapeProperties_TO_Microsoft_Robotics_Simulation_Physics_BoxShapeProperties(object transformFrom)
        {
            Microsoft.Robotics.Simulation.Physics.BoxShapeProperties target = new Microsoft.Robotics.Simulation.Physics.BoxShapeProperties();
            Microsoft.Robotics.Simulation.Physics.Proxy.BoxShapeProperties from = transformFrom as Microsoft.Robotics.Simulation.Physics.Proxy.BoxShapeProperties;
            target.Name = from.Name;
            target.ShapeId = (Microsoft.Robotics.Simulation.Physics.Shapes)((System.Int32)from.ShapeId);
            target.Dimensions = (Microsoft.Robotics.PhysicalModel.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Vector3_TO_Microsoft_Robotics_PhysicalModel_Vector3(from.Dimensions);
            target.Radius = from.Radius;
            target.Material = (from.Material == null) ? null : (Microsoft.Robotics.Simulation.Physics.MaterialProperties)Transform_Microsoft_Robotics_Simulation_Physics_Proxy_MaterialProperties_TO_Microsoft_Robotics_Simulation_Physics_MaterialProperties(from.Material);
            target.MassDensity = (from.MassDensity == null) ? null : (Microsoft.Robotics.Simulation.Physics.MassDensity)Transform_Microsoft_Robotics_Simulation_Physics_Proxy_MassDensity_TO_Microsoft_Robotics_Simulation_Physics_MassDensity(from.MassDensity);
            target.Advanced = (from.Advanced == null) ? null : (Microsoft.Robotics.Simulation.Physics.ShapeAdvancedProperties)Transform_Microsoft_Robotics_Simulation_Physics_Proxy_ShapeAdvancedProperties_TO_Microsoft_Robotics_Simulation_Physics_ShapeAdvancedProperties(from.Advanced);
            target.LocalPose = (Microsoft.Robotics.PhysicalModel.Pose)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Pose_TO_Microsoft_Robotics_PhysicalModel_Pose(from.LocalPose);
            target.TextureFileName = from.TextureFileName;
            target.DiffuseColor = (Microsoft.Robotics.PhysicalModel.Vector4)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Vector4_TO_Microsoft_Robotics_PhysicalModel_Vector4(from.DiffuseColor);
            target.EnableContactNotifications = from.EnableContactNotifications;
            target.ContactFilter = (from.ContactFilter == null) ? null : (Microsoft.Robotics.Simulation.Physics.ContactNotificationFilter)Transform_Microsoft_Robotics_Simulation_Physics_Proxy_ContactNotificationFilter_TO_Microsoft_Robotics_Simulation_Physics_ContactNotificationFilter(from.ContactFilter);
            return target;
        }


        public static object Transform_Microsoft_Robotics_Simulation_Physics_BoxShapeProperties_TO_Microsoft_Robotics_Simulation_Physics_Proxy_BoxShapeProperties(object transformFrom)
        {
            Microsoft.Robotics.Simulation.Physics.Proxy.BoxShapeProperties target = new Microsoft.Robotics.Simulation.Physics.Proxy.BoxShapeProperties();
            Microsoft.Robotics.Simulation.Physics.BoxShapeProperties from = transformFrom as Microsoft.Robotics.Simulation.Physics.BoxShapeProperties;
            target.Name = from.Name;
            target.ShapeId = (Microsoft.Robotics.Simulation.Physics.Proxy.Shapes)((System.Int32)from.ShapeId);
            target.Dimensions = (Microsoft.Robotics.PhysicalModel.Proxy.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Vector3_TO_Microsoft_Robotics_PhysicalModel_Proxy_Vector3(from.Dimensions);
            target.Radius = from.Radius;
            target.Material = (from.Material == null) ? null : (Microsoft.Robotics.Simulation.Physics.Proxy.MaterialProperties)Transform_Microsoft_Robotics_Simulation_Physics_MaterialProperties_TO_Microsoft_Robotics_Simulation_Physics_Proxy_MaterialProperties(from.Material);
            target.MassDensity = (from.MassDensity == null) ? null : (Microsoft.Robotics.Simulation.Physics.Proxy.MassDensity)Transform_Microsoft_Robotics_Simulation_Physics_MassDensity_TO_Microsoft_Robotics_Simulation_Physics_Proxy_MassDensity(from.MassDensity);
            target.Advanced = (from.Advanced == null) ? null : (Microsoft.Robotics.Simulation.Physics.Proxy.ShapeAdvancedProperties)Transform_Microsoft_Robotics_Simulation_Physics_ShapeAdvancedProperties_TO_Microsoft_Robotics_Simulation_Physics_Proxy_ShapeAdvancedProperties(from.Advanced);
            target.LocalPose = (Microsoft.Robotics.PhysicalModel.Proxy.Pose)Transform_Microsoft_Robotics_PhysicalModel_Pose_TO_Microsoft_Robotics_PhysicalModel_Proxy_Pose(from.LocalPose);
            target.TextureFileName = from.TextureFileName;
            target.DiffuseColor = (Microsoft.Robotics.PhysicalModel.Proxy.Vector4)Transform_Microsoft_Robotics_PhysicalModel_Vector4_TO_Microsoft_Robotics_PhysicalModel_Proxy_Vector4(from.DiffuseColor);
            target.EnableContactNotifications = from.EnableContactNotifications;
            target.ContactFilter = (from.ContactFilter == null) ? null : (Microsoft.Robotics.Simulation.Physics.Proxy.ContactNotificationFilter)Transform_Microsoft_Robotics_Simulation_Physics_ContactNotificationFilter_TO_Microsoft_Robotics_Simulation_Physics_Proxy_ContactNotificationFilter(from.ContactFilter);
            return target;
        }


        public static object Transform_Microsoft_Robotics_Simulation_Physics_Proxy_MaterialProperties_TO_Microsoft_Robotics_Simulation_Physics_MaterialProperties(object transformFrom)
        {
            Microsoft.Robotics.Simulation.Physics.MaterialProperties target = new Microsoft.Robotics.Simulation.Physics.MaterialProperties();
            Microsoft.Robotics.Simulation.Physics.Proxy.MaterialProperties from = transformFrom as Microsoft.Robotics.Simulation.Physics.Proxy.MaterialProperties;
            target.Name = from.Name;
            target.Restitution = from.Restitution;
            target.DynamicFriction = from.DynamicFriction;
            target.StaticFriction = from.StaticFriction;
            target.MaterialIndex = from.MaterialIndex;
            target.Advanced = (from.Advanced == null) ? null : (Microsoft.Robotics.Simulation.Physics.MaterialAdvancedProperties)Transform_Microsoft_Robotics_Simulation_Physics_Proxy_MaterialAdvancedProperties_TO_Microsoft_Robotics_Simulation_Physics_MaterialAdvancedProperties(from.Advanced);
            return target;
        }


        public static object Transform_Microsoft_Robotics_Simulation_Physics_MaterialProperties_TO_Microsoft_Robotics_Simulation_Physics_Proxy_MaterialProperties(object transformFrom)
        {
            Microsoft.Robotics.Simulation.Physics.Proxy.MaterialProperties target = new Microsoft.Robotics.Simulation.Physics.Proxy.MaterialProperties();
            Microsoft.Robotics.Simulation.Physics.MaterialProperties from = transformFrom as Microsoft.Robotics.Simulation.Physics.MaterialProperties;
            target.Name = from.Name;
            target.Restitution = from.Restitution;
            target.DynamicFriction = from.DynamicFriction;
            target.StaticFriction = from.StaticFriction;
            target.MaterialIndex = from.MaterialIndex;
            target.Advanced = (from.Advanced == null) ? null : (Microsoft.Robotics.Simulation.Physics.Proxy.MaterialAdvancedProperties)Transform_Microsoft_Robotics_Simulation_Physics_MaterialAdvancedProperties_TO_Microsoft_Robotics_Simulation_Physics_Proxy_MaterialAdvancedProperties(from.Advanced);
            return target;
        }


        public static object Transform_Microsoft_Robotics_Simulation_Physics_Proxy_MaterialAdvancedProperties_TO_Microsoft_Robotics_Simulation_Physics_MaterialAdvancedProperties(object transformFrom)
        {
            Microsoft.Robotics.Simulation.Physics.MaterialAdvancedProperties target = new Microsoft.Robotics.Simulation.Physics.MaterialAdvancedProperties();
            Microsoft.Robotics.Simulation.Physics.Proxy.MaterialAdvancedProperties from = transformFrom as Microsoft.Robotics.Simulation.Physics.Proxy.MaterialAdvancedProperties;
            target.AnisotropicStaticFriction = from.AnisotropicStaticFriction;
            target.AnisotropicDynamicFriction = from.AnisotropicDynamicFriction;
            target.AnisotropyDirection = (Microsoft.Robotics.PhysicalModel.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Vector3_TO_Microsoft_Robotics_PhysicalModel_Vector3(from.AnisotropyDirection);
            target.Spring = (from.Spring == null) ? null : (Microsoft.Robotics.PhysicalModel.SpringProperties)Transform_Microsoft_Robotics_PhysicalModel_Proxy_SpringProperties_TO_Microsoft_Robotics_PhysicalModel_SpringProperties(from.Spring);
            target.RestitutionCombineMode = (Microsoft.Robotics.Simulation.Physics.CoefficientsCombineMode)((System.Int32)from.RestitutionCombineMode);
            target.FrictionCombineMode = (Microsoft.Robotics.Simulation.Physics.CoefficientsCombineMode)((System.Int32)from.FrictionCombineMode);

            // copy IEnumerable Optical
            if (from.Optical != null)
            {
                target.Optical = new System.Collections.Generic.List<Microsoft.Robotics.Simulation.Physics.MaterialOpticalProperties>();
                foreach(Microsoft.Robotics.Simulation.Physics.Proxy.MaterialOpticalProperties elem in from.Optical)
                {
                    target.Optical.Add((elem == null) ? null : (Microsoft.Robotics.Simulation.Physics.MaterialOpticalProperties)Transform_Microsoft_Robotics_Simulation_Physics_Proxy_MaterialOpticalProperties_TO_Microsoft_Robotics_Simulation_Physics_MaterialOpticalProperties(elem));
                }
            }
            return target;
        }


        public static object Transform_Microsoft_Robotics_Simulation_Physics_MaterialAdvancedProperties_TO_Microsoft_Robotics_Simulation_Physics_Proxy_MaterialAdvancedProperties(object transformFrom)
        {
            Microsoft.Robotics.Simulation.Physics.Proxy.MaterialAdvancedProperties target = new Microsoft.Robotics.Simulation.Physics.Proxy.MaterialAdvancedProperties();
            Microsoft.Robotics.Simulation.Physics.MaterialAdvancedProperties from = transformFrom as Microsoft.Robotics.Simulation.Physics.MaterialAdvancedProperties;
            target.AnisotropicStaticFriction = from.AnisotropicStaticFriction;
            target.AnisotropicDynamicFriction = from.AnisotropicDynamicFriction;
            target.AnisotropyDirection = (Microsoft.Robotics.PhysicalModel.Proxy.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Vector3_TO_Microsoft_Robotics_PhysicalModel_Proxy_Vector3(from.AnisotropyDirection);
            target.Spring = (from.Spring == null) ? null : (Microsoft.Robotics.PhysicalModel.Proxy.SpringProperties)Transform_Microsoft_Robotics_PhysicalModel_SpringProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_SpringProperties(from.Spring);
            target.RestitutionCombineMode = (Microsoft.Robotics.Simulation.Physics.Proxy.CoefficientsCombineMode)((System.Int32)from.RestitutionCombineMode);
            target.FrictionCombineMode = (Microsoft.Robotics.Simulation.Physics.Proxy.CoefficientsCombineMode)((System.Int32)from.FrictionCombineMode);

            // copy IEnumerable Optical
            if (from.Optical != null)
            {
                target.Optical = new System.Collections.Generic.List<Microsoft.Robotics.Simulation.Physics.Proxy.MaterialOpticalProperties>();
                foreach(Microsoft.Robotics.Simulation.Physics.MaterialOpticalProperties elem in from.Optical)
                {
                    target.Optical.Add((elem == null) ? null : (Microsoft.Robotics.Simulation.Physics.Proxy.MaterialOpticalProperties)Transform_Microsoft_Robotics_Simulation_Physics_MaterialOpticalProperties_TO_Microsoft_Robotics_Simulation_Physics_Proxy_MaterialOpticalProperties(elem));
                }
            }
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_Proxy_SpringProperties_TO_Microsoft_Robotics_PhysicalModel_SpringProperties(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.SpringProperties target = new Microsoft.Robotics.PhysicalModel.SpringProperties();
            Microsoft.Robotics.PhysicalModel.Proxy.SpringProperties from = transformFrom as Microsoft.Robotics.PhysicalModel.Proxy.SpringProperties;
            target.SpringCoefficient = from.SpringCoefficient;
            target.DamperCoefficient = from.DamperCoefficient;
            target.EquilibriumPosition = from.EquilibriumPosition;
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_SpringProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_SpringProperties(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.Proxy.SpringProperties target = new Microsoft.Robotics.PhysicalModel.Proxy.SpringProperties();
            Microsoft.Robotics.PhysicalModel.SpringProperties from = transformFrom as Microsoft.Robotics.PhysicalModel.SpringProperties;
            target.SpringCoefficient = from.SpringCoefficient;
            target.DamperCoefficient = from.DamperCoefficient;
            target.EquilibriumPosition = from.EquilibriumPosition;
            return target;
        }


        public static object Transform_Microsoft_Robotics_Simulation_Physics_Proxy_SphereShape_TO_Microsoft_Robotics_Simulation_Physics_SphereShape(object transformFrom)
        {
            Microsoft.Robotics.Simulation.Physics.SphereShape target = new Microsoft.Robotics.Simulation.Physics.SphereShape();
            Microsoft.Robotics.Simulation.Physics.Proxy.SphereShape from = transformFrom as Microsoft.Robotics.Simulation.Physics.Proxy.SphereShape;
            target.SphereState = (from.SphereState == null) ? null : (Microsoft.Robotics.Simulation.Physics.SphereShapeProperties)Transform_Microsoft_Robotics_Simulation_Physics_Proxy_SphereShapeProperties_TO_Microsoft_Robotics_Simulation_Physics_SphereShapeProperties(from.SphereState);
            return target;
        }


        public static object Transform_Microsoft_Robotics_Simulation_Physics_SphereShape_TO_Microsoft_Robotics_Simulation_Physics_Proxy_SphereShape(object transformFrom)
        {
            Microsoft.Robotics.Simulation.Physics.Proxy.SphereShape target = new Microsoft.Robotics.Simulation.Physics.Proxy.SphereShape();
            Microsoft.Robotics.Simulation.Physics.SphereShape from = transformFrom as Microsoft.Robotics.Simulation.Physics.SphereShape;
            target.SphereState = (from.SphereState == null) ? null : (Microsoft.Robotics.Simulation.Physics.Proxy.SphereShapeProperties)Transform_Microsoft_Robotics_Simulation_Physics_SphereShapeProperties_TO_Microsoft_Robotics_Simulation_Physics_Proxy_SphereShapeProperties(from.SphereState);
            return target;
        }


        public static object Transform_Microsoft_Robotics_Simulation_Physics_Proxy_SphereShapeProperties_TO_Microsoft_Robotics_Simulation_Physics_SphereShapeProperties(object transformFrom)
        {
            Microsoft.Robotics.Simulation.Physics.SphereShapeProperties target = new Microsoft.Robotics.Simulation.Physics.SphereShapeProperties();
            Microsoft.Robotics.Simulation.Physics.Proxy.SphereShapeProperties from = transformFrom as Microsoft.Robotics.Simulation.Physics.Proxy.SphereShapeProperties;
            target.Name = from.Name;
            target.ShapeId = (Microsoft.Robotics.Simulation.Physics.Shapes)((System.Int32)from.ShapeId);
            target.Dimensions = (Microsoft.Robotics.PhysicalModel.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Vector3_TO_Microsoft_Robotics_PhysicalModel_Vector3(from.Dimensions);
            target.Radius = from.Radius;
            target.Material = (from.Material == null) ? null : (Microsoft.Robotics.Simulation.Physics.MaterialProperties)Transform_Microsoft_Robotics_Simulation_Physics_Proxy_MaterialProperties_TO_Microsoft_Robotics_Simulation_Physics_MaterialProperties(from.Material);
            target.MassDensity = (from.MassDensity == null) ? null : (Microsoft.Robotics.Simulation.Physics.MassDensity)Transform_Microsoft_Robotics_Simulation_Physics_Proxy_MassDensity_TO_Microsoft_Robotics_Simulation_Physics_MassDensity(from.MassDensity);
            target.Advanced = (from.Advanced == null) ? null : (Microsoft.Robotics.Simulation.Physics.ShapeAdvancedProperties)Transform_Microsoft_Robotics_Simulation_Physics_Proxy_ShapeAdvancedProperties_TO_Microsoft_Robotics_Simulation_Physics_ShapeAdvancedProperties(from.Advanced);
            target.LocalPose = (Microsoft.Robotics.PhysicalModel.Pose)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Pose_TO_Microsoft_Robotics_PhysicalModel_Pose(from.LocalPose);
            target.TextureFileName = from.TextureFileName;
            target.DiffuseColor = (Microsoft.Robotics.PhysicalModel.Vector4)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Vector4_TO_Microsoft_Robotics_PhysicalModel_Vector4(from.DiffuseColor);
            target.EnableContactNotifications = from.EnableContactNotifications;
            target.ContactFilter = (from.ContactFilter == null) ? null : (Microsoft.Robotics.Simulation.Physics.ContactNotificationFilter)Transform_Microsoft_Robotics_Simulation_Physics_Proxy_ContactNotificationFilter_TO_Microsoft_Robotics_Simulation_Physics_ContactNotificationFilter(from.ContactFilter);
            return target;
        }


        public static object Transform_Microsoft_Robotics_Simulation_Physics_SphereShapeProperties_TO_Microsoft_Robotics_Simulation_Physics_Proxy_SphereShapeProperties(object transformFrom)
        {
            Microsoft.Robotics.Simulation.Physics.Proxy.SphereShapeProperties target = new Microsoft.Robotics.Simulation.Physics.Proxy.SphereShapeProperties();
            Microsoft.Robotics.Simulation.Physics.SphereShapeProperties from = transformFrom as Microsoft.Robotics.Simulation.Physics.SphereShapeProperties;
            target.Name = from.Name;
            target.ShapeId = (Microsoft.Robotics.Simulation.Physics.Proxy.Shapes)((System.Int32)from.ShapeId);
            target.Dimensions = (Microsoft.Robotics.PhysicalModel.Proxy.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Vector3_TO_Microsoft_Robotics_PhysicalModel_Proxy_Vector3(from.Dimensions);
            target.Radius = from.Radius;
            target.Material = (from.Material == null) ? null : (Microsoft.Robotics.Simulation.Physics.Proxy.MaterialProperties)Transform_Microsoft_Robotics_Simulation_Physics_MaterialProperties_TO_Microsoft_Robotics_Simulation_Physics_Proxy_MaterialProperties(from.Material);
            target.MassDensity = (from.MassDensity == null) ? null : (Microsoft.Robotics.Simulation.Physics.Proxy.MassDensity)Transform_Microsoft_Robotics_Simulation_Physics_MassDensity_TO_Microsoft_Robotics_Simulation_Physics_Proxy_MassDensity(from.MassDensity);
            target.Advanced = (from.Advanced == null) ? null : (Microsoft.Robotics.Simulation.Physics.Proxy.ShapeAdvancedProperties)Transform_Microsoft_Robotics_Simulation_Physics_ShapeAdvancedProperties_TO_Microsoft_Robotics_Simulation_Physics_Proxy_ShapeAdvancedProperties(from.Advanced);
            target.LocalPose = (Microsoft.Robotics.PhysicalModel.Proxy.Pose)Transform_Microsoft_Robotics_PhysicalModel_Pose_TO_Microsoft_Robotics_PhysicalModel_Proxy_Pose(from.LocalPose);
            target.TextureFileName = from.TextureFileName;
            target.DiffuseColor = (Microsoft.Robotics.PhysicalModel.Proxy.Vector4)Transform_Microsoft_Robotics_PhysicalModel_Vector4_TO_Microsoft_Robotics_PhysicalModel_Proxy_Vector4(from.DiffuseColor);
            target.EnableContactNotifications = from.EnableContactNotifications;
            target.ContactFilter = (from.ContactFilter == null) ? null : (Microsoft.Robotics.Simulation.Physics.Proxy.ContactNotificationFilter)Transform_Microsoft_Robotics_Simulation_Physics_ContactNotificationFilter_TO_Microsoft_Robotics_Simulation_Physics_Proxy_ContactNotificationFilter(from.ContactFilter);
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_Proxy_Joint_TO_Microsoft_Robotics_PhysicalModel_Joint(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.Joint target = new Microsoft.Robotics.PhysicalModel.Joint();
            Microsoft.Robotics.PhysicalModel.Proxy.Joint from = transformFrom as Microsoft.Robotics.PhysicalModel.Proxy.Joint;
            target.State = (from.State == null) ? null : (Microsoft.Robotics.PhysicalModel.JointProperties)Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointProperties_TO_Microsoft_Robotics_PhysicalModel_JointProperties(from.State);
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_Joint_TO_Microsoft_Robotics_PhysicalModel_Proxy_Joint(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.Proxy.Joint target = new Microsoft.Robotics.PhysicalModel.Proxy.Joint();
            Microsoft.Robotics.PhysicalModel.Joint from = transformFrom as Microsoft.Robotics.PhysicalModel.Joint;
            target.State = (from.State == null) ? null : (Microsoft.Robotics.PhysicalModel.Proxy.JointProperties)Transform_Microsoft_Robotics_PhysicalModel_JointProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointProperties(from.State);
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointProperties_TO_Microsoft_Robotics_PhysicalModel_JointProperties(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.JointProperties target = new Microsoft.Robotics.PhysicalModel.JointProperties();
            Microsoft.Robotics.PhysicalModel.Proxy.JointProperties from = transformFrom as Microsoft.Robotics.PhysicalModel.Proxy.JointProperties;
            target.Name = from.Name;

            // copy Microsoft.Robotics.PhysicalModel.EntityJointConnector[] target.Connectors = from.Connectors
            if (from.Connectors != null)
            {
                target.Connectors = new Microsoft.Robotics.PhysicalModel.EntityJointConnector[from.Connectors.GetLength(0)];

                for (int d0 = 0; d0 < from.Connectors.GetLength(0); d0++)
                    target.Connectors[d0] = (from.Connectors[d0] == null) ? null : (Microsoft.Robotics.PhysicalModel.EntityJointConnector)Transform_Microsoft_Robotics_PhysicalModel_Proxy_EntityJointConnector_TO_Microsoft_Robotics_PhysicalModel_EntityJointConnector(from.Connectors[d0]);
            }
            target.MaximumForce = from.MaximumForce;
            target.MaximumTorque = from.MaximumTorque;
            target.EnableCollisions = from.EnableCollisions;
            target.Projection = (from.Projection == null) ? null : (Microsoft.Robotics.PhysicalModel.JointProjectionProperties)Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointProjectionProperties_TO_Microsoft_Robotics_PhysicalModel_JointProjectionProperties(from.Projection);
            target.Linear = (from.Linear == null) ? null : (Microsoft.Robotics.PhysicalModel.JointLinearProperties)Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointLinearProperties_TO_Microsoft_Robotics_PhysicalModel_JointLinearProperties(from.Linear);
            target.Angular = (from.Angular == null) ? null : (Microsoft.Robotics.PhysicalModel.JointAngularProperties)Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointAngularProperties_TO_Microsoft_Robotics_PhysicalModel_JointAngularProperties(from.Angular);
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_JointProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointProperties(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.Proxy.JointProperties target = new Microsoft.Robotics.PhysicalModel.Proxy.JointProperties();
            Microsoft.Robotics.PhysicalModel.JointProperties from = transformFrom as Microsoft.Robotics.PhysicalModel.JointProperties;
            target.Name = from.Name;

            // copy Microsoft.Robotics.PhysicalModel.Proxy.EntityJointConnector[] target.Connectors = from.Connectors
            if (from.Connectors != null)
            {
                target.Connectors = new Microsoft.Robotics.PhysicalModel.Proxy.EntityJointConnector[from.Connectors.GetLength(0)];

                for (int d0 = 0; d0 < from.Connectors.GetLength(0); d0++)
                    target.Connectors[d0] = (from.Connectors[d0] == null) ? null : (Microsoft.Robotics.PhysicalModel.Proxy.EntityJointConnector)Transform_Microsoft_Robotics_PhysicalModel_EntityJointConnector_TO_Microsoft_Robotics_PhysicalModel_Proxy_EntityJointConnector(from.Connectors[d0]);
            }
            target.MaximumForce = from.MaximumForce;
            target.MaximumTorque = from.MaximumTorque;
            target.EnableCollisions = from.EnableCollisions;
            target.Projection = (from.Projection == null) ? null : (Microsoft.Robotics.PhysicalModel.Proxy.JointProjectionProperties)Transform_Microsoft_Robotics_PhysicalModel_JointProjectionProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointProjectionProperties(from.Projection);
            target.Linear = (from.Linear == null) ? null : (Microsoft.Robotics.PhysicalModel.Proxy.JointLinearProperties)Transform_Microsoft_Robotics_PhysicalModel_JointLinearProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointLinearProperties(from.Linear);
            target.Angular = (from.Angular == null) ? null : (Microsoft.Robotics.PhysicalModel.Proxy.JointAngularProperties)Transform_Microsoft_Robotics_PhysicalModel_JointAngularProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointAngularProperties(from.Angular);
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointLinearProperties_TO_Microsoft_Robotics_PhysicalModel_JointLinearProperties(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.JointLinearProperties target = new Microsoft.Robotics.PhysicalModel.JointLinearProperties();
            Microsoft.Robotics.PhysicalModel.Proxy.JointLinearProperties from = transformFrom as Microsoft.Robotics.PhysicalModel.Proxy.JointLinearProperties;
            target.XMotionMode = (Microsoft.Robotics.PhysicalModel.JointDOFMode)((System.Int32)from.XMotionMode);
            target.YMotionMode = (Microsoft.Robotics.PhysicalModel.JointDOFMode)((System.Int32)from.YMotionMode);
            target.ZMotionMode = (Microsoft.Robotics.PhysicalModel.JointDOFMode)((System.Int32)from.ZMotionMode);
            target.XDrive = (from.XDrive == null) ? null : (Microsoft.Robotics.PhysicalModel.JointDriveProperties)Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointDriveProperties_TO_Microsoft_Robotics_PhysicalModel_JointDriveProperties(from.XDrive);
            target.YDrive = (from.YDrive == null) ? null : (Microsoft.Robotics.PhysicalModel.JointDriveProperties)Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointDriveProperties_TO_Microsoft_Robotics_PhysicalModel_JointDriveProperties(from.YDrive);
            target.ZDrive = (from.ZDrive == null) ? null : (Microsoft.Robotics.PhysicalModel.JointDriveProperties)Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointDriveProperties_TO_Microsoft_Robotics_PhysicalModel_JointDriveProperties(from.ZDrive);
            target.MotionLimit = (from.MotionLimit == null) ? null : (Microsoft.Robotics.PhysicalModel.JointLimitProperties)Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointLimitProperties_TO_Microsoft_Robotics_PhysicalModel_JointLimitProperties(from.MotionLimit);
            target.DriveTargetPosition = (Microsoft.Robotics.PhysicalModel.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Vector3_TO_Microsoft_Robotics_PhysicalModel_Vector3(from.DriveTargetPosition);
            target.DriveTargetVelocity = (Microsoft.Robotics.PhysicalModel.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Vector3_TO_Microsoft_Robotics_PhysicalModel_Vector3(from.DriveTargetVelocity);
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_JointLinearProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointLinearProperties(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.Proxy.JointLinearProperties target = new Microsoft.Robotics.PhysicalModel.Proxy.JointLinearProperties();
            Microsoft.Robotics.PhysicalModel.JointLinearProperties from = transformFrom as Microsoft.Robotics.PhysicalModel.JointLinearProperties;
            target.XMotionMode = (Microsoft.Robotics.PhysicalModel.Proxy.JointDOFMode)((System.Int32)from.XMotionMode);
            target.YMotionMode = (Microsoft.Robotics.PhysicalModel.Proxy.JointDOFMode)((System.Int32)from.YMotionMode);
            target.ZMotionMode = (Microsoft.Robotics.PhysicalModel.Proxy.JointDOFMode)((System.Int32)from.ZMotionMode);
            target.XDrive = (from.XDrive == null) ? null : (Microsoft.Robotics.PhysicalModel.Proxy.JointDriveProperties)Transform_Microsoft_Robotics_PhysicalModel_JointDriveProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointDriveProperties(from.XDrive);
            target.YDrive = (from.YDrive == null) ? null : (Microsoft.Robotics.PhysicalModel.Proxy.JointDriveProperties)Transform_Microsoft_Robotics_PhysicalModel_JointDriveProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointDriveProperties(from.YDrive);
            target.ZDrive = (from.ZDrive == null) ? null : (Microsoft.Robotics.PhysicalModel.Proxy.JointDriveProperties)Transform_Microsoft_Robotics_PhysicalModel_JointDriveProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointDriveProperties(from.ZDrive);
            target.MotionLimit = (from.MotionLimit == null) ? null : (Microsoft.Robotics.PhysicalModel.Proxy.JointLimitProperties)Transform_Microsoft_Robotics_PhysicalModel_JointLimitProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointLimitProperties(from.MotionLimit);
            target.DriveTargetPosition = (Microsoft.Robotics.PhysicalModel.Proxy.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Vector3_TO_Microsoft_Robotics_PhysicalModel_Proxy_Vector3(from.DriveTargetPosition);
            target.DriveTargetVelocity = (Microsoft.Robotics.PhysicalModel.Proxy.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Vector3_TO_Microsoft_Robotics_PhysicalModel_Proxy_Vector3(from.DriveTargetVelocity);
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointDriveProperties_TO_Microsoft_Robotics_PhysicalModel_JointDriveProperties(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.JointDriveProperties target = new Microsoft.Robotics.PhysicalModel.JointDriveProperties();
            Microsoft.Robotics.PhysicalModel.Proxy.JointDriveProperties from = transformFrom as Microsoft.Robotics.PhysicalModel.Proxy.JointDriveProperties;
            target.Mode = (Microsoft.Robotics.PhysicalModel.JointDriveMode)((System.Int32)from.Mode);
            target.Spring = (from.Spring == null) ? null : (Microsoft.Robotics.PhysicalModel.SpringProperties)Transform_Microsoft_Robotics_PhysicalModel_Proxy_SpringProperties_TO_Microsoft_Robotics_PhysicalModel_SpringProperties(from.Spring);
            target.ForceLimit = from.ForceLimit;
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_JointDriveProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointDriveProperties(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.Proxy.JointDriveProperties target = new Microsoft.Robotics.PhysicalModel.Proxy.JointDriveProperties();
            Microsoft.Robotics.PhysicalModel.JointDriveProperties from = transformFrom as Microsoft.Robotics.PhysicalModel.JointDriveProperties;
            target.Mode = (Microsoft.Robotics.PhysicalModel.Proxy.JointDriveMode)((System.Int32)from.Mode);
            target.Spring = (from.Spring == null) ? null : (Microsoft.Robotics.PhysicalModel.Proxy.SpringProperties)Transform_Microsoft_Robotics_PhysicalModel_SpringProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_SpringProperties(from.Spring);
            target.ForceLimit = from.ForceLimit;
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointProjectionProperties_TO_Microsoft_Robotics_PhysicalModel_JointProjectionProperties(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.JointProjectionProperties target = new Microsoft.Robotics.PhysicalModel.JointProjectionProperties();
            Microsoft.Robotics.PhysicalModel.Proxy.JointProjectionProperties from = transformFrom as Microsoft.Robotics.PhysicalModel.Proxy.JointProjectionProperties;
            target.ProjectionMode = (Microsoft.Robotics.PhysicalModel.JointProjectionMode)((System.Int32)from.ProjectionMode);
            target.ProjectionDistanceThreshold = from.ProjectionDistanceThreshold;
            target.ProjectionAngleThreshold = from.ProjectionAngleThreshold;
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_JointProjectionProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointProjectionProperties(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.Proxy.JointProjectionProperties target = new Microsoft.Robotics.PhysicalModel.Proxy.JointProjectionProperties();
            Microsoft.Robotics.PhysicalModel.JointProjectionProperties from = transformFrom as Microsoft.Robotics.PhysicalModel.JointProjectionProperties;
            target.ProjectionMode = (Microsoft.Robotics.PhysicalModel.Proxy.JointProjectionMode)((System.Int32)from.ProjectionMode);
            target.ProjectionDistanceThreshold = from.ProjectionDistanceThreshold;
            target.ProjectionAngleThreshold = from.ProjectionAngleThreshold;
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_Proxy_EntityJointConnector_TO_Microsoft_Robotics_PhysicalModel_EntityJointConnector(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.EntityJointConnector target = new Microsoft.Robotics.PhysicalModel.EntityJointConnector();
            Microsoft.Robotics.PhysicalModel.Proxy.EntityJointConnector from = transformFrom as Microsoft.Robotics.PhysicalModel.Proxy.EntityJointConnector;
            target.EntityName = from.EntityName;
            target.JointNormal = (Microsoft.Robotics.PhysicalModel.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Vector3_TO_Microsoft_Robotics_PhysicalModel_Vector3(from.JointNormal);
            target.JointAxis = (Microsoft.Robotics.PhysicalModel.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Vector3_TO_Microsoft_Robotics_PhysicalModel_Vector3(from.JointAxis);
            target.JointConnectPoint = (Microsoft.Robotics.PhysicalModel.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Vector3_TO_Microsoft_Robotics_PhysicalModel_Vector3(from.JointConnectPoint);
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_EntityJointConnector_TO_Microsoft_Robotics_PhysicalModel_Proxy_EntityJointConnector(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.Proxy.EntityJointConnector target = new Microsoft.Robotics.PhysicalModel.Proxy.EntityJointConnector();
            Microsoft.Robotics.PhysicalModel.EntityJointConnector from = transformFrom as Microsoft.Robotics.PhysicalModel.EntityJointConnector;
            target.EntityName = from.EntityName;
            target.JointNormal = (Microsoft.Robotics.PhysicalModel.Proxy.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Vector3_TO_Microsoft_Robotics_PhysicalModel_Proxy_Vector3(from.JointNormal);
            target.JointAxis = (Microsoft.Robotics.PhysicalModel.Proxy.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Vector3_TO_Microsoft_Robotics_PhysicalModel_Proxy_Vector3(from.JointAxis);
            target.JointConnectPoint = (Microsoft.Robotics.PhysicalModel.Proxy.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Vector3_TO_Microsoft_Robotics_PhysicalModel_Proxy_Vector3(from.JointConnectPoint);
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


        public static object Transform_Microsoft_Robotics_Simulation_Proxy_EntityState_TO_Microsoft_Robotics_Simulation_EntityState(object transformFrom)
        {
            Microsoft.Robotics.Simulation.EntityState target = new Microsoft.Robotics.Simulation.EntityState();
            Microsoft.Robotics.Simulation.Proxy.EntityState from = transformFrom as Microsoft.Robotics.Simulation.Proxy.EntityState;
            target.Name = from.Name;
            target.Assets = (from.Assets == null) ? null : (Microsoft.Robotics.Simulation.RenderingAssets)Transform_Microsoft_Robotics_Simulation_Proxy_RenderingAssets_TO_Microsoft_Robotics_Simulation_RenderingAssets(from.Assets);
            target.Pose = (Microsoft.Robotics.PhysicalModel.Pose)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Pose_TO_Microsoft_Robotics_PhysicalModel_Pose(from.Pose);
            target.Velocity = (Microsoft.Robotics.PhysicalModel.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Vector3_TO_Microsoft_Robotics_PhysicalModel_Vector3(from.Velocity);
            target.AngularVelocity = (Microsoft.Robotics.PhysicalModel.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Vector3_TO_Microsoft_Robotics_PhysicalModel_Vector3(from.AngularVelocity);
            target.MassDensity = (from.MassDensity == null) ? null : (Microsoft.Robotics.Simulation.Physics.MassDensity)Transform_Microsoft_Robotics_Simulation_Physics_Proxy_MassDensity_TO_Microsoft_Robotics_Simulation_Physics_MassDensity(from.MassDensity);
            target.Flags = (Microsoft.Robotics.Simulation.Physics.EntitySimulationModifiers)((System.Int32)from.Flags);
            return target;
        }


        public static object Transform_Microsoft_Robotics_Simulation_EntityState_TO_Microsoft_Robotics_Simulation_Proxy_EntityState(object transformFrom)
        {
            Microsoft.Robotics.Simulation.Proxy.EntityState target = new Microsoft.Robotics.Simulation.Proxy.EntityState();
            Microsoft.Robotics.Simulation.EntityState from = transformFrom as Microsoft.Robotics.Simulation.EntityState;
            target.Name = from.Name;
            target.Assets = (from.Assets == null) ? null : (Microsoft.Robotics.Simulation.Proxy.RenderingAssets)Transform_Microsoft_Robotics_Simulation_RenderingAssets_TO_Microsoft_Robotics_Simulation_Proxy_RenderingAssets(from.Assets);
            target.Pose = (Microsoft.Robotics.PhysicalModel.Proxy.Pose)Transform_Microsoft_Robotics_PhysicalModel_Pose_TO_Microsoft_Robotics_PhysicalModel_Proxy_Pose(from.Pose);
            target.Velocity = (Microsoft.Robotics.PhysicalModel.Proxy.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Vector3_TO_Microsoft_Robotics_PhysicalModel_Proxy_Vector3(from.Velocity);
            target.AngularVelocity = (Microsoft.Robotics.PhysicalModel.Proxy.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Vector3_TO_Microsoft_Robotics_PhysicalModel_Proxy_Vector3(from.AngularVelocity);
            target.MassDensity = (from.MassDensity == null) ? null : (Microsoft.Robotics.Simulation.Physics.Proxy.MassDensity)Transform_Microsoft_Robotics_Simulation_Physics_MassDensity_TO_Microsoft_Robotics_Simulation_Physics_Proxy_MassDensity(from.MassDensity);
            target.Flags = (Microsoft.Robotics.Simulation.Physics.Proxy.EntitySimulationModifiers)((System.Int32)from.Flags);
            return target;
        }


        public static object Transform_Microsoft_Robotics_Simulation_Proxy_RenderingAssets_TO_Microsoft_Robotics_Simulation_RenderingAssets(object transformFrom)
        {
            Microsoft.Robotics.Simulation.RenderingAssets target = new Microsoft.Robotics.Simulation.RenderingAssets();
            Microsoft.Robotics.Simulation.Proxy.RenderingAssets from = transformFrom as Microsoft.Robotics.Simulation.Proxy.RenderingAssets;
            target.Mesh = from.Mesh;
            target.DefaultTexture = from.DefaultTexture;
            target.Effect = from.Effect;
            return target;
        }


        public static object Transform_Microsoft_Robotics_Simulation_RenderingAssets_TO_Microsoft_Robotics_Simulation_Proxy_RenderingAssets(object transformFrom)
        {
            Microsoft.Robotics.Simulation.Proxy.RenderingAssets target = new Microsoft.Robotics.Simulation.Proxy.RenderingAssets();
            Microsoft.Robotics.Simulation.RenderingAssets from = transformFrom as Microsoft.Robotics.Simulation.RenderingAssets;
            target.Mesh = from.Mesh;
            target.DefaultTexture = from.DefaultTexture;
            target.Effect = from.Effect;
            return target;
        }


        public static object Transform_Microsoft_Robotics_Simulation_Physics_Proxy_MassDensity_TO_Microsoft_Robotics_Simulation_Physics_MassDensity(object transformFrom)
        {
            Microsoft.Robotics.Simulation.Physics.MassDensity target = new Microsoft.Robotics.Simulation.Physics.MassDensity();
            Microsoft.Robotics.Simulation.Physics.Proxy.MassDensity from = transformFrom as Microsoft.Robotics.Simulation.Physics.Proxy.MassDensity;
            target.Mass = from.Mass;
            target.InertiaTensor = (Microsoft.Robotics.PhysicalModel.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Vector3_TO_Microsoft_Robotics_PhysicalModel_Vector3(from.InertiaTensor);
            target.CenterOfMass = (Microsoft.Robotics.PhysicalModel.Pose)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Pose_TO_Microsoft_Robotics_PhysicalModel_Pose(from.CenterOfMass);
            target.Density = from.Density;
            target.LinearDamping = from.LinearDamping;
            target.AngularDamping = from.AngularDamping;
            return target;
        }


        public static object Transform_Microsoft_Robotics_Simulation_Physics_MassDensity_TO_Microsoft_Robotics_Simulation_Physics_Proxy_MassDensity(object transformFrom)
        {
            Microsoft.Robotics.Simulation.Physics.Proxy.MassDensity target = new Microsoft.Robotics.Simulation.Physics.Proxy.MassDensity();
            Microsoft.Robotics.Simulation.Physics.MassDensity from = transformFrom as Microsoft.Robotics.Simulation.Physics.MassDensity;
            target.Mass = from.Mass;
            target.InertiaTensor = (Microsoft.Robotics.PhysicalModel.Proxy.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Vector3_TO_Microsoft_Robotics_PhysicalModel_Proxy_Vector3(from.InertiaTensor);
            target.CenterOfMass = (Microsoft.Robotics.PhysicalModel.Proxy.Pose)Transform_Microsoft_Robotics_PhysicalModel_Pose_TO_Microsoft_Robotics_PhysicalModel_Proxy_Pose(from.CenterOfMass);
            target.Density = from.Density;
            target.LinearDamping = from.LinearDamping;
            target.AngularDamping = from.AngularDamping;
            return target;
        }


        public static object Transform_Microsoft_Robotics_Simulation_Physics_Proxy_ShapeAdvancedProperties_TO_Microsoft_Robotics_Simulation_Physics_ShapeAdvancedProperties(object transformFrom)
        {
            Microsoft.Robotics.Simulation.Physics.ShapeAdvancedProperties target = new Microsoft.Robotics.Simulation.Physics.ShapeAdvancedProperties();
            Microsoft.Robotics.Simulation.Physics.Proxy.ShapeAdvancedProperties from = transformFrom as Microsoft.Robotics.Simulation.Physics.Proxy.ShapeAdvancedProperties;
            target.MassSpaceIntertiaTensor = (Microsoft.Robotics.PhysicalModel.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Vector3_TO_Microsoft_Robotics_PhysicalModel_Vector3(from.MassSpaceIntertiaTensor);
            target.LinearDamping = from.LinearDamping;
            target.AngularDamping = from.AngularDamping;
            target.PhysicsCalculationPasses = from.PhysicsCalculationPasses;
            target.IsTrigger = from.IsTrigger;
            return target;
        }


        public static object Transform_Microsoft_Robotics_Simulation_Physics_ShapeAdvancedProperties_TO_Microsoft_Robotics_Simulation_Physics_Proxy_ShapeAdvancedProperties(object transformFrom)
        {
            Microsoft.Robotics.Simulation.Physics.Proxy.ShapeAdvancedProperties target = new Microsoft.Robotics.Simulation.Physics.Proxy.ShapeAdvancedProperties();
            Microsoft.Robotics.Simulation.Physics.ShapeAdvancedProperties from = transformFrom as Microsoft.Robotics.Simulation.Physics.ShapeAdvancedProperties;
            target.MassSpaceIntertiaTensor = (Microsoft.Robotics.PhysicalModel.Proxy.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Vector3_TO_Microsoft_Robotics_PhysicalModel_Proxy_Vector3(from.MassSpaceIntertiaTensor);
            target.LinearDamping = from.LinearDamping;
            target.AngularDamping = from.AngularDamping;
            target.PhysicsCalculationPasses = from.PhysicsCalculationPasses;
            target.IsTrigger = from.IsTrigger;
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_Proxy_Pose_TO_Microsoft_Robotics_PhysicalModel_Pose(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.Pose target = new Microsoft.Robotics.PhysicalModel.Pose();
            Microsoft.Robotics.PhysicalModel.Proxy.Pose from = (Microsoft.Robotics.PhysicalModel.Proxy.Pose)transformFrom;
            target.Position = (Microsoft.Robotics.PhysicalModel.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Vector3_TO_Microsoft_Robotics_PhysicalModel_Vector3(from.Position);
            target.Orientation = (Microsoft.Robotics.PhysicalModel.Quaternion)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Quaternion_TO_Microsoft_Robotics_PhysicalModel_Quaternion(from.Orientation);
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_Pose_TO_Microsoft_Robotics_PhysicalModel_Proxy_Pose(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.Proxy.Pose target = new Microsoft.Robotics.PhysicalModel.Proxy.Pose();
            Microsoft.Robotics.PhysicalModel.Pose from = (Microsoft.Robotics.PhysicalModel.Pose)transformFrom;
            target.Position = (Microsoft.Robotics.PhysicalModel.Proxy.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Vector3_TO_Microsoft_Robotics_PhysicalModel_Proxy_Vector3(from.Position);
            target.Orientation = (Microsoft.Robotics.PhysicalModel.Proxy.Quaternion)Transform_Microsoft_Robotics_PhysicalModel_Quaternion_TO_Microsoft_Robotics_PhysicalModel_Proxy_Quaternion(from.Orientation);
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_Proxy_Quaternion_TO_Microsoft_Robotics_PhysicalModel_Quaternion(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.Quaternion target = new Microsoft.Robotics.PhysicalModel.Quaternion();
            Microsoft.Robotics.PhysicalModel.Proxy.Quaternion from = (Microsoft.Robotics.PhysicalModel.Proxy.Quaternion)transformFrom;
            target.X = from.X;
            target.Y = from.Y;
            target.Z = from.Z;
            target.W = from.W;
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_Quaternion_TO_Microsoft_Robotics_PhysicalModel_Proxy_Quaternion(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.Proxy.Quaternion target = new Microsoft.Robotics.PhysicalModel.Proxy.Quaternion();
            Microsoft.Robotics.PhysicalModel.Quaternion from = (Microsoft.Robotics.PhysicalModel.Quaternion)transformFrom;
            target.X = from.X;
            target.Y = from.Y;
            target.Z = from.Z;
            target.W = from.W;
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_Proxy_Vector4_TO_Microsoft_Robotics_PhysicalModel_Vector4(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.Vector4 target = new Microsoft.Robotics.PhysicalModel.Vector4();
            Microsoft.Robotics.PhysicalModel.Proxy.Vector4 from = (Microsoft.Robotics.PhysicalModel.Proxy.Vector4)transformFrom;
            target.X = from.X;
            target.Y = from.Y;
            target.Z = from.Z;
            target.W = from.W;
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_Vector4_TO_Microsoft_Robotics_PhysicalModel_Proxy_Vector4(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.Proxy.Vector4 target = new Microsoft.Robotics.PhysicalModel.Proxy.Vector4();
            Microsoft.Robotics.PhysicalModel.Vector4 from = (Microsoft.Robotics.PhysicalModel.Vector4)transformFrom;
            target.X = from.X;
            target.Y = from.Y;
            target.Z = from.Z;
            target.W = from.W;
            return target;
        }


        public static object Transform_Microsoft_Robotics_Simulation_Physics_Proxy_ContactNotificationFilter_TO_Microsoft_Robotics_Simulation_Physics_ContactNotificationFilter(object transformFrom)
        {
            Microsoft.Robotics.Simulation.Physics.ContactNotificationFilter target = new Microsoft.Robotics.Simulation.Physics.ContactNotificationFilter();
            Microsoft.Robotics.Simulation.Physics.Proxy.ContactNotificationFilter from = transformFrom as Microsoft.Robotics.Simulation.Physics.Proxy.ContactNotificationFilter;
            target.Stages = (Microsoft.Robotics.Simulation.Physics.ContactNotificationStage)((System.Int32)from.Stages);
            target.NormalForceThreshold = from.NormalForceThreshold;
            target.FrictionForceThreshold = from.FrictionForceThreshold;
            return target;
        }


        public static object Transform_Microsoft_Robotics_Simulation_Physics_ContactNotificationFilter_TO_Microsoft_Robotics_Simulation_Physics_Proxy_ContactNotificationFilter(object transformFrom)
        {
            Microsoft.Robotics.Simulation.Physics.Proxy.ContactNotificationFilter target = new Microsoft.Robotics.Simulation.Physics.Proxy.ContactNotificationFilter();
            Microsoft.Robotics.Simulation.Physics.ContactNotificationFilter from = transformFrom as Microsoft.Robotics.Simulation.Physics.ContactNotificationFilter;
            target.Stages = (Microsoft.Robotics.Simulation.Physics.Proxy.ContactNotificationStage)((System.Int32)from.Stages);
            target.NormalForceThreshold = from.NormalForceThreshold;
            target.FrictionForceThreshold = from.FrictionForceThreshold;
            return target;
        }


        public static object Transform_Microsoft_Robotics_Simulation_Physics_Proxy_MaterialOpticalProperties_TO_Microsoft_Robotics_Simulation_Physics_MaterialOpticalProperties(object transformFrom)
        {
            Microsoft.Robotics.Simulation.Physics.MaterialOpticalProperties target = new Microsoft.Robotics.Simulation.Physics.MaterialOpticalProperties();
            Microsoft.Robotics.Simulation.Physics.Proxy.MaterialOpticalProperties from = transformFrom as Microsoft.Robotics.Simulation.Physics.Proxy.MaterialOpticalProperties;
            target.WaveLength = from.WaveLength;
            target.DiffuseReflectance = from.DiffuseReflectance;
            target.SpecularReflectance = from.SpecularReflectance;
            target.Transmittance = from.Transmittance;
            return target;
        }


        public static object Transform_Microsoft_Robotics_Simulation_Physics_MaterialOpticalProperties_TO_Microsoft_Robotics_Simulation_Physics_Proxy_MaterialOpticalProperties(object transformFrom)
        {
            Microsoft.Robotics.Simulation.Physics.Proxy.MaterialOpticalProperties target = new Microsoft.Robotics.Simulation.Physics.Proxy.MaterialOpticalProperties();
            Microsoft.Robotics.Simulation.Physics.MaterialOpticalProperties from = transformFrom as Microsoft.Robotics.Simulation.Physics.MaterialOpticalProperties;
            target.WaveLength = from.WaveLength;
            target.DiffuseReflectance = from.DiffuseReflectance;
            target.SpecularReflectance = from.SpecularReflectance;
            target.Transmittance = from.Transmittance;
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointAngularProperties_TO_Microsoft_Robotics_PhysicalModel_JointAngularProperties(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.JointAngularProperties target = new Microsoft.Robotics.PhysicalModel.JointAngularProperties();
            Microsoft.Robotics.PhysicalModel.Proxy.JointAngularProperties from = transformFrom as Microsoft.Robotics.PhysicalModel.Proxy.JointAngularProperties;
            target.Swing1Mode = (Microsoft.Robotics.PhysicalModel.JointDOFMode)((System.Int32)from.Swing1Mode);
            target.Swing2Mode = (Microsoft.Robotics.PhysicalModel.JointDOFMode)((System.Int32)from.Swing2Mode);
            target.TwistMode = (Microsoft.Robotics.PhysicalModel.JointDOFMode)((System.Int32)from.TwistMode);
            target.Swing1Limit = (from.Swing1Limit == null) ? null : (Microsoft.Robotics.PhysicalModel.JointLimitProperties)Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointLimitProperties_TO_Microsoft_Robotics_PhysicalModel_JointLimitProperties(from.Swing1Limit);
            target.Swing2Limit = (from.Swing2Limit == null) ? null : (Microsoft.Robotics.PhysicalModel.JointLimitProperties)Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointLimitProperties_TO_Microsoft_Robotics_PhysicalModel_JointLimitProperties(from.Swing2Limit);
            target.UpperTwistLimit = (from.UpperTwistLimit == null) ? null : (Microsoft.Robotics.PhysicalModel.JointLimitProperties)Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointLimitProperties_TO_Microsoft_Robotics_PhysicalModel_JointLimitProperties(from.UpperTwistLimit);
            target.LowerTwistLimit = (from.LowerTwistLimit == null) ? null : (Microsoft.Robotics.PhysicalModel.JointLimitProperties)Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointLimitProperties_TO_Microsoft_Robotics_PhysicalModel_JointLimitProperties(from.LowerTwistLimit);
            target.SwingDrive = (from.SwingDrive == null) ? null : (Microsoft.Robotics.PhysicalModel.JointDriveProperties)Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointDriveProperties_TO_Microsoft_Robotics_PhysicalModel_JointDriveProperties(from.SwingDrive);
            target.TwistDrive = (from.TwistDrive == null) ? null : (Microsoft.Robotics.PhysicalModel.JointDriveProperties)Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointDriveProperties_TO_Microsoft_Robotics_PhysicalModel_JointDriveProperties(from.TwistDrive);
            target.SlerpDrive = (from.SlerpDrive == null) ? null : (Microsoft.Robotics.PhysicalModel.JointDriveProperties)Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointDriveProperties_TO_Microsoft_Robotics_PhysicalModel_JointDriveProperties(from.SlerpDrive);
            target.DriveTargetOrientation = (Microsoft.Robotics.PhysicalModel.Quaternion)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Quaternion_TO_Microsoft_Robotics_PhysicalModel_Quaternion(from.DriveTargetOrientation);
            target.DriveTargetVelocity = (Microsoft.Robotics.PhysicalModel.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Proxy_Vector3_TO_Microsoft_Robotics_PhysicalModel_Vector3(from.DriveTargetVelocity);
            target.GearRatio = from.GearRatio;
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_JointAngularProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointAngularProperties(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.Proxy.JointAngularProperties target = new Microsoft.Robotics.PhysicalModel.Proxy.JointAngularProperties();
            Microsoft.Robotics.PhysicalModel.JointAngularProperties from = transformFrom as Microsoft.Robotics.PhysicalModel.JointAngularProperties;
            target.Swing1Mode = (Microsoft.Robotics.PhysicalModel.Proxy.JointDOFMode)((System.Int32)from.Swing1Mode);
            target.Swing2Mode = (Microsoft.Robotics.PhysicalModel.Proxy.JointDOFMode)((System.Int32)from.Swing2Mode);
            target.TwistMode = (Microsoft.Robotics.PhysicalModel.Proxy.JointDOFMode)((System.Int32)from.TwistMode);
            target.Swing1Limit = (from.Swing1Limit == null) ? null : (Microsoft.Robotics.PhysicalModel.Proxy.JointLimitProperties)Transform_Microsoft_Robotics_PhysicalModel_JointLimitProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointLimitProperties(from.Swing1Limit);
            target.Swing2Limit = (from.Swing2Limit == null) ? null : (Microsoft.Robotics.PhysicalModel.Proxy.JointLimitProperties)Transform_Microsoft_Robotics_PhysicalModel_JointLimitProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointLimitProperties(from.Swing2Limit);
            target.UpperTwistLimit = (from.UpperTwistLimit == null) ? null : (Microsoft.Robotics.PhysicalModel.Proxy.JointLimitProperties)Transform_Microsoft_Robotics_PhysicalModel_JointLimitProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointLimitProperties(from.UpperTwistLimit);
            target.LowerTwistLimit = (from.LowerTwistLimit == null) ? null : (Microsoft.Robotics.PhysicalModel.Proxy.JointLimitProperties)Transform_Microsoft_Robotics_PhysicalModel_JointLimitProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointLimitProperties(from.LowerTwistLimit);
            target.SwingDrive = (from.SwingDrive == null) ? null : (Microsoft.Robotics.PhysicalModel.Proxy.JointDriveProperties)Transform_Microsoft_Robotics_PhysicalModel_JointDriveProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointDriveProperties(from.SwingDrive);
            target.TwistDrive = (from.TwistDrive == null) ? null : (Microsoft.Robotics.PhysicalModel.Proxy.JointDriveProperties)Transform_Microsoft_Robotics_PhysicalModel_JointDriveProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointDriveProperties(from.TwistDrive);
            target.SlerpDrive = (from.SlerpDrive == null) ? null : (Microsoft.Robotics.PhysicalModel.Proxy.JointDriveProperties)Transform_Microsoft_Robotics_PhysicalModel_JointDriveProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointDriveProperties(from.SlerpDrive);
            target.DriveTargetOrientation = (Microsoft.Robotics.PhysicalModel.Proxy.Quaternion)Transform_Microsoft_Robotics_PhysicalModel_Quaternion_TO_Microsoft_Robotics_PhysicalModel_Proxy_Quaternion(from.DriveTargetOrientation);
            target.DriveTargetVelocity = (Microsoft.Robotics.PhysicalModel.Proxy.Vector3)Transform_Microsoft_Robotics_PhysicalModel_Vector3_TO_Microsoft_Robotics_PhysicalModel_Proxy_Vector3(from.DriveTargetVelocity);
            target.GearRatio = from.GearRatio;
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointLimitProperties_TO_Microsoft_Robotics_PhysicalModel_JointLimitProperties(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.JointLimitProperties target = new Microsoft.Robotics.PhysicalModel.JointLimitProperties();
            Microsoft.Robotics.PhysicalModel.Proxy.JointLimitProperties from = transformFrom as Microsoft.Robotics.PhysicalModel.Proxy.JointLimitProperties;
            target.LimitThreshold = from.LimitThreshold;
            target.Restitution = from.Restitution;
            target.Spring = (from.Spring == null) ? null : (Microsoft.Robotics.PhysicalModel.SpringProperties)Transform_Microsoft_Robotics_PhysicalModel_Proxy_SpringProperties_TO_Microsoft_Robotics_PhysicalModel_SpringProperties(from.Spring);
            return target;
        }


        public static object Transform_Microsoft_Robotics_PhysicalModel_JointLimitProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointLimitProperties(object transformFrom)
        {
            Microsoft.Robotics.PhysicalModel.Proxy.JointLimitProperties target = new Microsoft.Robotics.PhysicalModel.Proxy.JointLimitProperties();
            Microsoft.Robotics.PhysicalModel.JointLimitProperties from = transformFrom as Microsoft.Robotics.PhysicalModel.JointLimitProperties;
            target.LimitThreshold = from.LimitThreshold;
            target.Restitution = from.Restitution;
            target.Spring = (from.Spring == null) ? null : (Microsoft.Robotics.PhysicalModel.Proxy.SpringProperties)Transform_Microsoft_Robotics_PhysicalModel_SpringProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_SpringProperties(from.Spring);
            return target;
        }

        static Transforms()
        {
            Register();
        }
        public static void Register()
        {
            AddProxyTransform(typeof(Robotics.SimulationTutorial2.Proxy.State), Transform_Robotics_SimulationTutorial2_Proxy_State_TO_Robotics_SimulationTutorial2_State);
            AddSourceTransform(typeof(Robotics.SimulationTutorial2.State), Transform_Robotics_SimulationTutorial2_State_TO_Robotics_SimulationTutorial2_Proxy_State);
            AddProxyTransform(typeof(Robotics.SimulationTutorial2.Proxy.TableEntity), Transform_Robotics_SimulationTutorial2_Proxy_TableEntity_TO_Robotics_SimulationTutorial2_TableEntity);
            AddSourceTransform(typeof(Robotics.SimulationTutorial2.TableEntity), Transform_Robotics_SimulationTutorial2_TableEntity_TO_Robotics_SimulationTutorial2_Proxy_TableEntity);
            AddProxyTransform(typeof(Microsoft.Robotics.Simulation.Physics.Proxy.BoxShape), Transform_Microsoft_Robotics_Simulation_Physics_Proxy_BoxShape_TO_Microsoft_Robotics_Simulation_Physics_BoxShape);
            AddSourceTransform(typeof(Microsoft.Robotics.Simulation.Physics.BoxShape), Transform_Microsoft_Robotics_Simulation_Physics_BoxShape_TO_Microsoft_Robotics_Simulation_Physics_Proxy_BoxShape);
            AddProxyTransform(typeof(Microsoft.Robotics.Simulation.Physics.Proxy.BoxShapeProperties), Transform_Microsoft_Robotics_Simulation_Physics_Proxy_BoxShapeProperties_TO_Microsoft_Robotics_Simulation_Physics_BoxShapeProperties);
            AddSourceTransform(typeof(Microsoft.Robotics.Simulation.Physics.BoxShapeProperties), Transform_Microsoft_Robotics_Simulation_Physics_BoxShapeProperties_TO_Microsoft_Robotics_Simulation_Physics_Proxy_BoxShapeProperties);
            AddProxyTransform(typeof(Microsoft.Robotics.Simulation.Physics.Proxy.MaterialProperties), Transform_Microsoft_Robotics_Simulation_Physics_Proxy_MaterialProperties_TO_Microsoft_Robotics_Simulation_Physics_MaterialProperties);
            AddSourceTransform(typeof(Microsoft.Robotics.Simulation.Physics.MaterialProperties), Transform_Microsoft_Robotics_Simulation_Physics_MaterialProperties_TO_Microsoft_Robotics_Simulation_Physics_Proxy_MaterialProperties);
            AddProxyTransform(typeof(Microsoft.Robotics.Simulation.Physics.Proxy.MaterialAdvancedProperties), Transform_Microsoft_Robotics_Simulation_Physics_Proxy_MaterialAdvancedProperties_TO_Microsoft_Robotics_Simulation_Physics_MaterialAdvancedProperties);
            AddSourceTransform(typeof(Microsoft.Robotics.Simulation.Physics.MaterialAdvancedProperties), Transform_Microsoft_Robotics_Simulation_Physics_MaterialAdvancedProperties_TO_Microsoft_Robotics_Simulation_Physics_Proxy_MaterialAdvancedProperties);
            AddProxyTransform(typeof(Microsoft.Robotics.PhysicalModel.Proxy.SpringProperties), Transform_Microsoft_Robotics_PhysicalModel_Proxy_SpringProperties_TO_Microsoft_Robotics_PhysicalModel_SpringProperties);
            AddSourceTransform(typeof(Microsoft.Robotics.PhysicalModel.SpringProperties), Transform_Microsoft_Robotics_PhysicalModel_SpringProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_SpringProperties);
            AddProxyTransform(typeof(Microsoft.Robotics.Simulation.Physics.Proxy.SphereShape), Transform_Microsoft_Robotics_Simulation_Physics_Proxy_SphereShape_TO_Microsoft_Robotics_Simulation_Physics_SphereShape);
            AddSourceTransform(typeof(Microsoft.Robotics.Simulation.Physics.SphereShape), Transform_Microsoft_Robotics_Simulation_Physics_SphereShape_TO_Microsoft_Robotics_Simulation_Physics_Proxy_SphereShape);
            AddProxyTransform(typeof(Microsoft.Robotics.Simulation.Physics.Proxy.SphereShapeProperties), Transform_Microsoft_Robotics_Simulation_Physics_Proxy_SphereShapeProperties_TO_Microsoft_Robotics_Simulation_Physics_SphereShapeProperties);
            AddSourceTransform(typeof(Microsoft.Robotics.Simulation.Physics.SphereShapeProperties), Transform_Microsoft_Robotics_Simulation_Physics_SphereShapeProperties_TO_Microsoft_Robotics_Simulation_Physics_Proxy_SphereShapeProperties);
            AddProxyTransform(typeof(Microsoft.Robotics.PhysicalModel.Proxy.Joint), Transform_Microsoft_Robotics_PhysicalModel_Proxy_Joint_TO_Microsoft_Robotics_PhysicalModel_Joint);
            AddSourceTransform(typeof(Microsoft.Robotics.PhysicalModel.Joint), Transform_Microsoft_Robotics_PhysicalModel_Joint_TO_Microsoft_Robotics_PhysicalModel_Proxy_Joint);
            AddProxyTransform(typeof(Microsoft.Robotics.PhysicalModel.Proxy.JointProperties), Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointProperties_TO_Microsoft_Robotics_PhysicalModel_JointProperties);
            AddSourceTransform(typeof(Microsoft.Robotics.PhysicalModel.JointProperties), Transform_Microsoft_Robotics_PhysicalModel_JointProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointProperties);
            AddProxyTransform(typeof(Microsoft.Robotics.PhysicalModel.Proxy.JointLinearProperties), Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointLinearProperties_TO_Microsoft_Robotics_PhysicalModel_JointLinearProperties);
            AddSourceTransform(typeof(Microsoft.Robotics.PhysicalModel.JointLinearProperties), Transform_Microsoft_Robotics_PhysicalModel_JointLinearProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointLinearProperties);
            AddProxyTransform(typeof(Microsoft.Robotics.PhysicalModel.Proxy.JointDriveProperties), Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointDriveProperties_TO_Microsoft_Robotics_PhysicalModel_JointDriveProperties);
            AddSourceTransform(typeof(Microsoft.Robotics.PhysicalModel.JointDriveProperties), Transform_Microsoft_Robotics_PhysicalModel_JointDriveProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointDriveProperties);
            AddProxyTransform(typeof(Microsoft.Robotics.PhysicalModel.Proxy.JointProjectionProperties), Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointProjectionProperties_TO_Microsoft_Robotics_PhysicalModel_JointProjectionProperties);
            AddSourceTransform(typeof(Microsoft.Robotics.PhysicalModel.JointProjectionProperties), Transform_Microsoft_Robotics_PhysicalModel_JointProjectionProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointProjectionProperties);
            AddProxyTransform(typeof(Microsoft.Robotics.PhysicalModel.Proxy.EntityJointConnector), Transform_Microsoft_Robotics_PhysicalModel_Proxy_EntityJointConnector_TO_Microsoft_Robotics_PhysicalModel_EntityJointConnector);
            AddSourceTransform(typeof(Microsoft.Robotics.PhysicalModel.EntityJointConnector), Transform_Microsoft_Robotics_PhysicalModel_EntityJointConnector_TO_Microsoft_Robotics_PhysicalModel_Proxy_EntityJointConnector);
            AddProxyTransform(typeof(Microsoft.Robotics.PhysicalModel.Proxy.Vector3), Transform_Microsoft_Robotics_PhysicalModel_Proxy_Vector3_TO_Microsoft_Robotics_PhysicalModel_Vector3);
            AddSourceTransform(typeof(Microsoft.Robotics.PhysicalModel.Vector3), Transform_Microsoft_Robotics_PhysicalModel_Vector3_TO_Microsoft_Robotics_PhysicalModel_Proxy_Vector3);
            AddProxyTransform(typeof(Microsoft.Robotics.Simulation.Proxy.EntityState), Transform_Microsoft_Robotics_Simulation_Proxy_EntityState_TO_Microsoft_Robotics_Simulation_EntityState);
            AddSourceTransform(typeof(Microsoft.Robotics.Simulation.EntityState), Transform_Microsoft_Robotics_Simulation_EntityState_TO_Microsoft_Robotics_Simulation_Proxy_EntityState);
            AddProxyTransform(typeof(Microsoft.Robotics.Simulation.Proxy.RenderingAssets), Transform_Microsoft_Robotics_Simulation_Proxy_RenderingAssets_TO_Microsoft_Robotics_Simulation_RenderingAssets);
            AddSourceTransform(typeof(Microsoft.Robotics.Simulation.RenderingAssets), Transform_Microsoft_Robotics_Simulation_RenderingAssets_TO_Microsoft_Robotics_Simulation_Proxy_RenderingAssets);
            AddProxyTransform(typeof(Microsoft.Robotics.Simulation.Physics.Proxy.MassDensity), Transform_Microsoft_Robotics_Simulation_Physics_Proxy_MassDensity_TO_Microsoft_Robotics_Simulation_Physics_MassDensity);
            AddSourceTransform(typeof(Microsoft.Robotics.Simulation.Physics.MassDensity), Transform_Microsoft_Robotics_Simulation_Physics_MassDensity_TO_Microsoft_Robotics_Simulation_Physics_Proxy_MassDensity);
            AddProxyTransform(typeof(Microsoft.Robotics.Simulation.Physics.Proxy.ShapeAdvancedProperties), Transform_Microsoft_Robotics_Simulation_Physics_Proxy_ShapeAdvancedProperties_TO_Microsoft_Robotics_Simulation_Physics_ShapeAdvancedProperties);
            AddSourceTransform(typeof(Microsoft.Robotics.Simulation.Physics.ShapeAdvancedProperties), Transform_Microsoft_Robotics_Simulation_Physics_ShapeAdvancedProperties_TO_Microsoft_Robotics_Simulation_Physics_Proxy_ShapeAdvancedProperties);
            AddProxyTransform(typeof(Microsoft.Robotics.PhysicalModel.Proxy.Pose), Transform_Microsoft_Robotics_PhysicalModel_Proxy_Pose_TO_Microsoft_Robotics_PhysicalModel_Pose);
            AddSourceTransform(typeof(Microsoft.Robotics.PhysicalModel.Pose), Transform_Microsoft_Robotics_PhysicalModel_Pose_TO_Microsoft_Robotics_PhysicalModel_Proxy_Pose);
            AddProxyTransform(typeof(Microsoft.Robotics.PhysicalModel.Proxy.Quaternion), Transform_Microsoft_Robotics_PhysicalModel_Proxy_Quaternion_TO_Microsoft_Robotics_PhysicalModel_Quaternion);
            AddSourceTransform(typeof(Microsoft.Robotics.PhysicalModel.Quaternion), Transform_Microsoft_Robotics_PhysicalModel_Quaternion_TO_Microsoft_Robotics_PhysicalModel_Proxy_Quaternion);
            AddProxyTransform(typeof(Microsoft.Robotics.PhysicalModel.Proxy.Vector4), Transform_Microsoft_Robotics_PhysicalModel_Proxy_Vector4_TO_Microsoft_Robotics_PhysicalModel_Vector4);
            AddSourceTransform(typeof(Microsoft.Robotics.PhysicalModel.Vector4), Transform_Microsoft_Robotics_PhysicalModel_Vector4_TO_Microsoft_Robotics_PhysicalModel_Proxy_Vector4);
            AddProxyTransform(typeof(Microsoft.Robotics.Simulation.Physics.Proxy.ContactNotificationFilter), Transform_Microsoft_Robotics_Simulation_Physics_Proxy_ContactNotificationFilter_TO_Microsoft_Robotics_Simulation_Physics_ContactNotificationFilter);
            AddSourceTransform(typeof(Microsoft.Robotics.Simulation.Physics.ContactNotificationFilter), Transform_Microsoft_Robotics_Simulation_Physics_ContactNotificationFilter_TO_Microsoft_Robotics_Simulation_Physics_Proxy_ContactNotificationFilter);
            AddProxyTransform(typeof(Microsoft.Robotics.Simulation.Physics.Proxy.MaterialOpticalProperties), Transform_Microsoft_Robotics_Simulation_Physics_Proxy_MaterialOpticalProperties_TO_Microsoft_Robotics_Simulation_Physics_MaterialOpticalProperties);
            AddSourceTransform(typeof(Microsoft.Robotics.Simulation.Physics.MaterialOpticalProperties), Transform_Microsoft_Robotics_Simulation_Physics_MaterialOpticalProperties_TO_Microsoft_Robotics_Simulation_Physics_Proxy_MaterialOpticalProperties);
            AddProxyTransform(typeof(Microsoft.Robotics.PhysicalModel.Proxy.JointAngularProperties), Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointAngularProperties_TO_Microsoft_Robotics_PhysicalModel_JointAngularProperties);
            AddSourceTransform(typeof(Microsoft.Robotics.PhysicalModel.JointAngularProperties), Transform_Microsoft_Robotics_PhysicalModel_JointAngularProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointAngularProperties);
            AddProxyTransform(typeof(Microsoft.Robotics.PhysicalModel.Proxy.JointLimitProperties), Transform_Microsoft_Robotics_PhysicalModel_Proxy_JointLimitProperties_TO_Microsoft_Robotics_PhysicalModel_JointLimitProperties);
            AddSourceTransform(typeof(Microsoft.Robotics.PhysicalModel.JointLimitProperties), Transform_Microsoft_Robotics_PhysicalModel_JointLimitProperties_TO_Microsoft_Robotics_PhysicalModel_Proxy_JointLimitProperties);
        }
    }
}

