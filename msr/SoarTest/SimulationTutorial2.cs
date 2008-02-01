//-----------------------------------------------------------------------
//  This file is part of the Microsoft Robotics Studio Code Samples.
//
//  Copyright (C) Microsoft Corporation.  All rights reserved.
//
//  $File: SimulationTutorial2.cs $ $Revision: 1 $
//-----------------------------------------------------------------------

using Microsoft.Ccr.Core;
using Microsoft.Ccr.Adapters.WinForms;
using Microsoft.Dss.Core;
using Microsoft.Dss.Core.Attributes;
using Microsoft.Dss.ServiceModel.Dssp;
using Microsoft.Dss.ServiceModel.DsspServiceBase;

using System;
using System.Collections.Generic;
using System.Security.Permissions;

#region Simulation namespaces
using Microsoft.Robotics.Simulation;
using Microsoft.Robotics.Simulation.Engine;
using engineproxy = Microsoft.Robotics.Simulation.Engine.Proxy;
using Microsoft.Robotics.Simulation.Physics;

#region CODECLIP 01-1
using drive = Microsoft.Robotics.Services.Simulation.Drive.Proxy;
using lrf = Microsoft.Robotics.Services.Simulation.Sensors.LaserRangeFinder.Proxy;
using bumper = Microsoft.Robotics.Services.Simulation.Sensors.Bumper.Proxy;
using simwebcam = Microsoft.Robotics.Services.Simulation.Sensors.SimulatedWebcam.Proxy;
#endregion

using Microsoft.Robotics.PhysicalModel;
using System.ComponentModel;
using Microsoft.Dss.Core.DsspHttp;
using System.Net;
using System.Threading;
#endregion

namespace Robotics.SimulationTutorial2
{
    [DisplayName("Simulation Tutorial 2")]
    [Description("Simulation Tutorial 2 Service")]
    [Contract(Contract.Identifier)]
    public class SimulationTutorial2 : DsspServiceBase
    {
        State _state = new State();

        // partner attribute will cause simulation engine service to start
        [Partner("Engine", 
            Contract = engineproxy.Contract.Identifier, 
            CreationPolicy = PartnerCreationPolicy.UseExistingOrCreate)]
        private engineproxy.SimulationEnginePort _engineServicePort = 
            new engineproxy.SimulationEnginePort();

        // Main service port
        [ServicePort("/SimulationTutorial2", AllowMultipleInstances=false)]
        private SimulationTutorial2Operations _mainPort = 
            new SimulationTutorial2Operations();

        public SimulationTutorial2(DsspServiceCreationPort creationPort) : 
                base(creationPort)
        {

        }

        protected override void Start()
        {
            base.Start();

            // Orient sim camera view point
            SetupCamera();
            // Add objects (entities) in our simulated world
            PopulateWorld();

        }

        #region CODECLIP 01-2
        private void SetupCamera()
        {
            // Set up initial view
            CameraView view = new CameraView();
            view.EyePosition = new Vector3(2.491269f, 0.598689f, 1.046625f);
            view.LookAtPoint = new Vector3(1.873792f, 0.40983f, 0.2830455f);
            SimulationEngine.GlobalInstancePort.Update(view);
        }

        private void PopulateWorld()
        {
            AddSky();
            AddGround();
            AddCameras();
            AddTable(new Vector3(1, 0.5f, -2));
            AddPioneer3DXRobot(new Vector3(1, 0.1f, 0));
            AddLegoNxtRobot(new Vector3(2, 0.1f, 0));
            //AddIRobotCreateRobot(new Vector3(2, 0.1f, 0));  // uncomment this to add an iRobot Create robot
        }
        #endregion

        private void AddCameras()
        {            
            CameraEntity cam1 = new CameraEntity(640, 480);
            cam1.State.Name = "newcam1";
            cam1.State.Pose.Position = new Vector3(20f, 1f, -10f);
            cam1.State.Pose.Orientation = Quaternion.FromAxisAngle(0, 1, 0, (float)(-Math.PI / 2.0f));
            cam1.IsRealTimeCamera = false;

            // set cam1.IsRealTimeCamera to true and uncomment the following lines 
            // to start another simulated webcam service on camera "newcam1". 
            // Careful: more realtime cameras mean more concurrent renderings,
            // resulting in a degradation of the rendering frame rate
            //CreateService(
            //    simwebcam.Contract.Identifier,
            //    Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
            //        "http://localhost" + cam2.State.Name)
            //);

            SimulationEngine.GlobalInstancePort.Insert(cam1);
        }

        #region Environment Entities

        void AddSky()
        {
            // Add a sky using a static texture. We will use the sky texture
            // to do per pixel lighting on each simulation visual entity
            SkyEntity sky = new SkyEntity("sky.dds", "sky_diff.dds");
            SimulationEngine.GlobalInstancePort.Insert(sky);

            // Add a directional light to simulate the sun.
            LightSourceEntity sun = new LightSourceEntity();
            sun.State.Name = "Sun";
            sun.Type = LightSourceEntityType.Directional;
            sun.Color = new Vector4(0.8f, 0.8f, 0.8f, 1);
            sun.Direction = new Vector3(-1.0f, -1.0f, 0.5f);
            SimulationEngine.GlobalInstancePort.Insert(sun);
        }

        void AddGround()
        {
            HeightFieldShapeProperties hf = new HeightFieldShapeProperties("height field",
                64, // number of rows 
                100, // distance in meters, between rows
                64, // number of columns
                100, // distance in meters, between columns
                1, // scale factor to multiple height values 
                -1000); // vertical extent of the height field. Should be set to large negative values

            // create array with height samples
            hf.HeightSamples = new HeightFieldSample[hf.RowCount * hf.ColumnCount];
            for (int i = 0; i < hf.RowCount * hf.ColumnCount; i++)
            {
                hf.HeightSamples[i] = new HeightFieldSample();
                hf.HeightSamples[i].Height = (short)(Math.Sin(i * 0.01));
            }

            // create a material for the entire field. We could also specify material per sample.
            hf.Material = new MaterialProperties("ground", 0.8f, 0.5f, 0.8f);

            // insert ground entity in simulation and specify a texture
            SimulationEngine.GlobalInstancePort.Insert(new HeightFieldEntity(hf, "03RamieSc.dds"));
        }

        #region CODECLIP 02-1
        void AddTable(Vector3 position)
        {
            // create an instance of our custom entity
            TableEntity entity = new TableEntity(position);

            // Name the entity
            entity.State.Name = "table:"+Guid.NewGuid().ToString();

            // Insert entity in simulation. 
            SimulationEngine.GlobalInstancePort.Insert(entity);
        }
        #endregion
        #endregion

        #region Robot Entities

        #region Pioneer
        #region CODECLIP 03-1
        void AddPioneer3DXRobot(Vector3 position)
        {
            Pioneer3DX robotBaseEntity = CreateMotorBase(ref position);

            // Create Laser entity and start simulated laser service
            LaserRangeFinderEntity laser = CreateLaserRangeFinder();
            // insert laser as child to motor base
            robotBaseEntity.InsertEntity(laser);

            // Create bumper array entity and start simulated bumper service
            BumperArrayEntity bumperArray = CreateBumperArray();
            // insert as child of motor base
            robotBaseEntity.InsertEntity(bumperArray);

            // create Camera Entity ans start SimulatedWebcam service
            CameraEntity camera = CreateCamera();
            // insert as child of motor base
            robotBaseEntity.InsertEntity(camera);

            // Finally insert the motor base and its two children 
            // to the simulation
            SimulationEngine.GlobalInstancePort.Insert(robotBaseEntity);
        }
        #endregion

        #region CODECLIP 03-3
        private Pioneer3DX CreateMotorBase(ref Vector3 position)
        {
            // use supplied entity that creates a motor base 
            // with 2 active wheels and one caster
            Pioneer3DX robotBaseEntity = new Pioneer3DX(position);

            // specify mesh. 
            robotBaseEntity.State.Assets.Mesh = "Pioneer3dx.bos";            
            // specify color if no mesh is specified. 
            robotBaseEntity.ChassisShape.State.DiffuseColor = new Vector4(0.8f, 0.25f, 0.25f, 1.0f);

            // the name below must match manifest
            robotBaseEntity.State.Name = "P3DXMotorBase";

            #region CODECLIP 04-1
            // Start simulated arcos motor service
            drive.Contract.CreateService(ConstructorPort,
                Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
                    "http://localhost/" + robotBaseEntity.State.Name)
            );
            #endregion
            return robotBaseEntity;
        }
        #endregion

        #region CODECLIP 03-4
        private LaserRangeFinderEntity CreateLaserRangeFinder()
        {
            // Create a Laser Range Finder Entity .
            // Place it 30cm above base CenterofMass. 
            LaserRangeFinderEntity laser = new LaserRangeFinderEntity(
                new Pose(new Vector3(0, 0.30f, 0)));

            laser.State.Name = "P3DXLaserRangeFinder";
            laser.LaserBox.State.DiffuseColor = new Vector4(0.25f, 0.25f, 0.8f, 1.0f);

            // Create LaserRangeFinder simulation service and specify
            // which entity it talks to
            lrf.Contract.CreateService(
                ConstructorPort,
                Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
                "http://localhost/" + laser.State.Name));
            return laser;
        }
        #endregion

        #region CODECLIP 03-5
        private BumperArrayEntity CreateBumperArray()
        {
            // Create a bumper array entity with two bumpers
            BoxShape frontBumper = new BoxShape(
                new BoxShapeProperties("front",
                    0.001f,
                    new Pose(new Vector3(0, 0.05f, -0.25f)),
                    new Vector3(0.40f, 0.03f, 0.03f)
                )
            );
            frontBumper.State.DiffuseColor = new Vector4(0.1f, 0.1f, 0.1f, 1.0f);

            BoxShape rearBumper = new BoxShape(
                new BoxShapeProperties("rear",
                    0.001f,
                    new Pose(new Vector3(0, 0.05f, 0.25f)),
                    new Vector3(0.40f, 0.03f, 0.03f)
                )
            );
            rearBumper.State.DiffuseColor = new Vector4(0.1f, 0.1f, 0.1f, 1.0f);

            // The physics engine will issue contact notifications only
            // if we enable them per shape
            frontBumper.State.EnableContactNotifications = true;
            rearBumper.State.EnableContactNotifications = true;

            // put some force filtering so we only get notified for significant bumps
            //frontBumper.State.ContactFilter = new ContactNotificationFilter(1,1);
            //rearBumper.State.ContactFilter = new ContactNotificationFilter(1, 1);

            BumperArrayEntity
                bumperArray = new BumperArrayEntity(frontBumper, rearBumper);
            // entity name, must match manifest partner name
            bumperArray.State.Name = "P3DXBumpers";

            // start simulated bumper service
            
            bumper.Contract.CreateService(
                ConstructorPort,
                Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
                "http://localhost/" + bumperArray.State.Name));
            return bumperArray;
        }
        #endregion

        #region CODECLIP 03-6
        private CameraEntity CreateCamera()
        {
            // low resolution, wide Field of View
            CameraEntity cam = new CameraEntity(320, 240);
            cam.State.Name = "robocam";
            // just on top of the bot
            cam.State.Pose.Position = new Vector3(0.0f, 0.5f, 0.0f);
            // camera renders in an offline buffer at each frame
            // required for service
            cam.IsRealTimeCamera = true;

            // Start simulated webcam service
            simwebcam.Contract.CreateService(
                ConstructorPort,
                Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
                    "http://localhost/" + cam.State.Name)
            );

            return cam;
        }
        #endregion
        #endregion

        #region Lego NXT
        void AddLegoNxtRobot(Vector3 position)
        {
            LegoNXTTribot robotBaseEntity = CreateLegoNxtMotorBase(ref position);

            // Create bumper array entity and start simulated bumper service
            BumperArrayEntity bumperArray = CreateLegoNxtBumper();

            // insert as child of motor base
            robotBaseEntity.InsertEntity(bumperArray);

            // Finaly insert the motor base and its two children 
            // to the simulation
            SimulationEngine.GlobalInstancePort.Insert(robotBaseEntity);
        }


        private LegoNXTTribot CreateLegoNxtMotorBase(ref Vector3 position)
        {
            // use supplied entity that creates a motor base 
            // with 2 active wheels and one caster
            LegoNXTTribot robotBaseEntity = new LegoNXTTribot(position);

            // specify mesh. 
            robotBaseEntity.State.Assets.Mesh = "LegoNXTTribot.bos";

            // the name below must match manifest
            robotBaseEntity.State.Name = "LegoNXTMotorBase";

            // Start simulated arcos motor service
            CreateService(
                drive.Contract.Identifier,
                Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
                    "http://localhost/" + robotBaseEntity.State.Name)
            );
            return robotBaseEntity;
        }

        private BumperArrayEntity CreateLegoNxtBumper()
        {
            // create a little bumper shape that models the NXT bumper
            BoxShape frontBumper = new BoxShape(
                new BoxShapeProperties(
                    "front", 0.001f, //mass
                    new Pose(new Vector3(0, 0.063f, -0.09f)), //position
                    new Vector3(0.023f, 0.023f, 0.045f)));   

            // The physics engine will issue contact notifications only
            // if we enable them per shape
            frontBumper.State.EnableContactNotifications = true;

            BumperArrayEntity
                bumperArray = new BumperArrayEntity(frontBumper);
            // entity name, must match manifest partner name
            bumperArray.State.Name = "LegoNXTBumpers";

            // start simulated bumper service
            CreateService(
                bumper.Contract.Identifier,
                Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
                "http://localhost/" + bumperArray.State.Name));
            return bumperArray;
        }

        #endregion

        #region IRobot Create
        void AddIRobotCreateRobot(Vector3 position)
        {
            IRobotCreate robotBaseEntity = CreateIRobotCreateMotorBase(ref position);

            // Create bumper array entity and start simulated bumper service
            BumperArrayEntity bumperArray = CreateIRobotCreateBumper(true);
            // insert as child of motor base
            robotBaseEntity.InsertEntity(bumperArray);

            bumperArray = CreateIRobotCreateBumper(false);
            // insert as child of motor base
            robotBaseEntity.InsertEntity(bumperArray);
            
            // Finally insert the motor base and its children to the simulation
            SimulationEngine.GlobalInstancePort.Insert(robotBaseEntity);
        }


        private IRobotCreate CreateIRobotCreateMotorBase(ref Vector3 position)
        {
            // use supplied entity that creates a motor base 
            // with 2 active wheels and one caster
            IRobotCreate robotBaseEntity = new IRobotCreate(position);

            // specify mesh. 
            robotBaseEntity.State.Assets.Mesh = "IRobot-Create.bos";

            // the name below must match manifest
            robotBaseEntity.State.Name = "IRobotCreateMotorBase";

            // Start simulated arcos motor service
            CreateService(
                drive.Contract.Identifier,
                Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
                    "http://localhost/" + robotBaseEntity.State.Name)
            );
            return robotBaseEntity;
        }

        private BumperArrayEntity CreateIRobotCreateBumper(bool isLeft)
        {
            string Name;
            Vector3 Offset;
            float Rotation;
            if (isLeft)
            {
                Name = "Left";
                Offset = new Vector3(-0.07f, 0.055f, -0.14f);
                Rotation = 1.75f;
            }
            else
            {
                Name = "Right";
                Offset = new Vector3(0.07f, 0.055f, -0.14f);
                Rotation = -1.75f;
            }

            // create a little bumper shape that models the iRobot Create bumper
            BoxShape Bumper = new BoxShape(
                new BoxShapeProperties(
                    Name, 0.001f, //mass
                    new Pose(Offset, // position
                        Quaternion.FromAxisAngle(0, 1, 0, (float)(Rotation * Math.PI / 2.0f))), // rotation
                new Vector3(0.15f, 0.06f, 0.01f))); // dimensions

            // The physics engine will issue contact notifications only
            // if we enable them per shape
            Bumper.State.EnableContactNotifications = true;

            BumperArrayEntity
                bumperArray = new BumperArrayEntity(Bumper);
            // entity name, must match manifest partner name
            bumperArray.State.Name = "IRobotCreateBumper" + Name;

            // start simulated bumper service
            CreateService(
                bumper.Contract.Identifier,
                Microsoft.Robotics.Simulation.Partners.CreateEntityPartner(
                "http://localhost/" + bumperArray.State.Name));
            
            return bumperArray;
        }

        #endregion
        #endregion

        #region DSSP Handlers
        [ServiceHandler(ServiceHandlerBehavior.Concurrent)]
        public IEnumerator<ITask> HttpGetHandler(HttpGet get)
        {
            get.ResponsePort.Post(new HttpResponseType(HttpStatusCode.OK, _state));
            // we use the http get handler for something unusual, but fun:
            // Whenever a web browser does a GET, we will throw some objects in the simulation
            Replace replace = new Replace();
            replace.Body = _state;
            replace.Body.TableCount++;
            _mainPort.Post(replace);
            yield break;
        }

        [ServiceHandler(ServiceHandlerBehavior.Exclusive)]
        public IEnumerator<ITask> ReplaceHandler(Replace replace)
        {
            AddTable(new Vector3(1, 5, 1));
            _state = replace.Body;
            replace.ResponsePort.Post(new DefaultReplaceResponseType());
            yield break;
        }

        #endregion
    }

    #region CODECLIP 02-2
    /// <summary>
    /// An entity for approximating a table. 
    /// </summary>
    [DataContract]
    public class TableEntity : MultiShapeEntity
    {
        /// <summary>
        /// Default constructor.
        /// </summary>
        public TableEntity() { }

        /// <summary>
        /// Custom constructor, programmatically builds physics primitive shapes to describe
        /// a particular table. 
        /// </summary>
        /// <param name="position"></param>
        public TableEntity(Vector3 position)
        {
            State.Pose.Position = position;
            State.Assets.Mesh = "table_01.obj";
            float tableHeight = 0.65f;
            float tableWidth = 1.05f;
            float tableDepth = 0.7f;
            float tableThinkness = 0.03f;
            float legThickness = 0.03f;
            float legOffset = 0.05f;

            // add a shape for the table surface
            BoxShape tableTop = new BoxShape(
                new BoxShapeProperties(30, 
                new Pose(new Vector3(0, tableHeight, 0)), 
                new Vector3(tableWidth, tableThinkness, tableDepth))
            );

            // add a shape for the left leg
            BoxShape tableLeftLeg = new BoxShape(
                new BoxShapeProperties(10, // mass in kg
                new Pose(
                    new Vector3(-tableWidth/2 + legOffset, tableHeight/2, 0)), 
                new Vector3(legThickness, tableHeight + tableThinkness, tableDepth))
            );

            BoxShape tableRightLeg = new BoxShape(
                new BoxShapeProperties(10, // mass in kg
                new Pose(
                    new Vector3(tableWidth / 2 - legOffset, tableHeight / 2, 0)), 
                new Vector3(legThickness, tableHeight + tableThinkness, tableDepth)) 
            );

            BoxShapes = new List<BoxShape>();
            BoxShapes.Add(tableTop);
            BoxShapes.Add(tableLeftLeg);
            BoxShapes.Add(tableRightLeg);
        }

        public override void Update(FrameUpdate update)
        {
            base.Update(update);
        }
    }
    #endregion

    public static class Contract
    {
        public const string Identifier = "http://schemas.tempuri.org/2006/06/simulationtutorial2.html";
    }

    [DataContract]
    public class State
    {
        [DataMember]
        public string Message;
        public State(string message)
        {
            Message = message;
        }
        [DataMember]
        public int TableCount;
        public State()
        {
        }
    }

    public class Replace : Replace<State, DsspResponsePort<DefaultReplaceResponseType>>
    {
    }

    [ServicePort]
    public class SimulationTutorial2Operations : PortSet<DsspDefaultLookup, DsspDefaultDrop, HttpGet, Replace>
    {
    }
}
