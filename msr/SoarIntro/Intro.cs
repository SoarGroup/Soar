//------------------------------------------------------------------------------
// Intro.cs
//
// ORIGINAL COMMENTS, abridged:
// Sample MSRS program that illustrates a simple wandering behaviour
// using a simulated robot. The robot wanders at random until a bumper
// notification is received. Then it stops, reverses direction for a
// short period, turns a random amount and then starts driving again.
//
// Written by Trevor Taylor, Queensland University of Technology
// This code is freely available
//
//------------------------------------------------------------------------------
//
// This version is hacked to use Soar as the controller.
//
//------------------------------------------------------------------------------
using Microsoft.Ccr.Core;
using Microsoft.Dss.Core;
using Microsoft.Dss.Core.Attributes;
using Microsoft.Dss.ServiceModel.Dssp;
using Microsoft.Dss.ServiceModel.DsspServiceBase;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Security.Permissions;
using xml = System.Xml;

// Additional namespaces after creation with dssnewservice
//
// Robotics Tutorial 1 Step 1 -- Add reference
// Add a reference to RoboticsCommon.proxy which provides
// generic interfaces. Don't add references to the actual
// hardware here! The partnership will be set up later in
// the manifest.
using bumper = Microsoft.Robotics.Services.ContactSensor.Proxy;

// Robotics Tutorial 2 Step 1 -- Add reference
// The reference already exists, just insert this using statement.
// Does not work though. Should use a drive. See below.
using motor = Microsoft.Robotics.Services.Motor.Proxy;

// Robotics Tutorial 3 Step 1 -- Add reference
// This is a generic drive service. It can therefore work with
// and service that implements a DifferentialDrive.
using drive = Microsoft.Robotics.Services.Drive.Proxy;

using sicklrf = Microsoft.Robotics.Services.Sensors.SickLRF.Proxy;

using sml;
using System.Threading;

namespace Robotics.Intro
{

    [DisplayName("Intro")]
    [Description("The Intro Service - Wander using bumpers")]
    [Contract(Contract.Identifier)]

    public class IntroService : DsspServiceBase
    {
        // TT Dec-2006 - Add an initial state partner to get the config
        // TT Jul-2007 - Change in behaviour for V1.5
        public const string InitialStateUri = ServicePaths.MountPoint + @"/Apps/QUT/Config/Intro.Config.xml";

        // Add an InitialStatePartner so that the config file will be read
        // NOTE: Creating a new instance of the state here will NOT
        // work if there is no config file because InitialStatePartner
        // will replace it with null!!! See the code in Start().
        [InitialStatePartner(Optional = true, ServiceUri = InitialStateUri)]

        private IntroState _state;

        [ServicePort("/intro", AllowMultipleInstances = false)]
        private IntroOperations _mainPort = new IntroOperations();

        // Robotics Tutorial 1 Step 2 -- Add a partner
        // See also the details in the manifest.
        // Note that the policy says UseExisting.
        // We will be creating the Simulation first, so this service
        // should already exist.
        [Partner("bumper", Contract = bumper.Contract.Identifier,
             CreationPolicy = PartnerCreationPolicy.UseExisting)]
        private bumper.ContactSensorArrayOperations _bumperPort = new bumper.ContactSensorArrayOperations();

        // Robotics Tutorial 2 Step 2 -- Add a partner
        //[Partner("motor", Contract = motor.Contract.Identifier,
        //CreationPolicy = PartnerCreationPolicy.UseExisting)] 
        //private motor.MotorOperations _motorPort = new motor.MotorOperations();

        // Robotics Tutorial 3 Step 1 -- Add partners
        // A bumper partner already exists above
        [Partner("Drive",
           Contract = drive.Contract.Identifier,
            CreationPolicy = PartnerCreationPolicy.UseExisting)]
        drive.DriveOperations _drivePort = new drive.DriveOperations();

        [Partner("Laser", Contract = sicklrf.Contract.Identifier, CreationPolicy = PartnerCreationPolicy.UseExisting)]
        sicklrf.SickLRFOperations _laserPort = new sicklrf.SickLRFOperations();
        sicklrf.SickLRFOperations _laserNotify = new sicklrf.SickLRFOperations();

        /// <summary>
        /// Default Service Constructor
        /// </summary>
        public IntroService(DsspServiceCreationPort creationPort)
            :
                base(creationPort)
        {

        }

        private DateTime _simulationStart;

        /// <summary>
        /// Service Start
        /// </summary>
        protected override void Start()
        {
            // TT Dec-2006 - Setup for initial state
            // The state might already have been created using
            // the Initial State Partner above. If so, then we
            // don't want to create a new one!
            if (_state == null)
            {
                _state = new IntroState();
                // Do any other initialization here for the default
                // settings that you might want ...
            }

            _simulationStart = DateTime.Now;

            // There should be some code in here to validate the settings
            // from the config file just in case the user entered some
            // invalid values ...

            // Now save the State
            // This creates a new file the first time it is run.
            // Later, it re-reads the existing file, but by then
            // the file has been populated with the default values.
            SaveState(_state);

            // Initialize Soar
            InitializeSoar();

            // Listen on the main port for requests and call the appropriate handler.
            ActivateDsspOperationHandlers();

            // Robotics Tutorial Step 3 -- Service Startup Initialization
            // Start() is a required method that was created by
            // dssnewservice. We need to modify it.
            // Start listening for bumpers
            // See the method code below
            SubscribeToBumpers();

            // Publish the service to the local Node Directory
            DirectoryInsert();

            // display HTTP service Uri
            LogInfo(LogGroups.Console, "Service uri: ");

            // Start Soar after everything is loaded
            Thread soarThread = new Thread(new ThreadStart(RunSoar));
            soarThread.Start();

        }

        protected override void Shutdown()
        {
            ShutdownSoar();

            base.Shutdown();
        }


        // ---------------- Begin Soar Code

        private sml.Kernel _kernel;
        private sml.Agent _agent;
        private sml.Kernel.UpdateEventCallback _updateCall;

        private FloatElement _overrideLeftWME;
        private FloatElement _overrideRightWME;
        private StringElement _overrideActiveWME;
        private StringElement _frontBumperWasPressedWME;
        private StringElement _frontBumperPressedWME;
        private FloatElement _frontBumperTimeWME;
        private StringElement _rearBumperWasPressedWME;
        private StringElement _rearBumperPressedWME;
        private FloatElement _rearBumperTimeWME;
        private FloatElement _timeWME;
        private FloatElement _randomWME;

        private Random _random;

        private bool _running = false;
        private bool _stop = false;

        // this state needs to be reentrant
        private bool _overrideChanged = false; // not protected on read
        private bool _overrideActive = false; // not protected on read
        private double _overrideLeft = 0;
        private double _overrideRight = 0;
        private bool _frontBumperWasPressed = false;
        private bool _rearBumperWasPressed = false;
        private bool _frontBumperPressed = false;
        private bool _rearBumperPressed = false;

        private void InitializeSoar()
        {
            _kernel = sml.Kernel.CreateKernelInNewThread("SoarKernelSML");
            if (_kernel.HadError())
                throw new Exception("Error initializing kernel: " + _kernel.GetLastErrorDescription());

            _agent = _kernel.CreateAgent("hal");

            // We test the kernel for an error after creating an agent as the agent
            // object may not be properly constructed if the create call failed so
            // we store errors in the kernel in this case.  Once this create is done we can work directly with the agent.
            if (_kernel.HadError())
                throw new Exception("Error creating agent: " + _kernel.GetLastErrorDescription());

            _kernel.SetAutoCommit(false);

            //Trace.WriteLine(_agent.ExecuteCommandLine("pwd"));
            _agent.LoadProductions("Apps/QUT/SoarIntro/agents/simple-bot.soar");

            // Prepare communication channel
            Identifier inputLink = _agent.GetInputLink();

            if (inputLink == null)
                throw new Exception("Error getting the input link");

            Identifier overrideWME = _agent.CreateIdWME(inputLink, "override");
            _overrideLeftWME = _agent.CreateFloatWME(overrideWME, "left", 0);
            _overrideRightWME = _agent.CreateFloatWME(overrideWME, "right", 0);
            _overrideActiveWME = _agent.CreateStringWME(overrideWME, "active", "false");

            Identifier configWME = _agent.CreateIdWME(inputLink, "config");
            
            Identifier powerWME = _agent.CreateIdWME(configWME, "power");
            _agent.CreateFloatWME(powerWME, "drive", _state.MaximumPower);
            _agent.CreateFloatWME(powerWME, "reverse", _state.BackUpPower);

            Identifier delayWME = _agent.CreateIdWME(configWME, "delay");
            _agent.CreateFloatWME(delayWME, "stop", _state.StopTimeout);
            _agent.CreateFloatWME(delayWME, "reverse", _state.BackUpTimeout);
            _agent.CreateFloatWME(delayWME, "turn", _state.TurnTimeout);
            _agent.CreateFloatWME(delayWME, "variance", _state.TimeoutVariance);

            _timeWME = _agent.CreateFloatWME(inputLink, "time", 0);
            _randomWME = _agent.CreateFloatWME(inputLink, "random", 0);

            Identifier sensorsWME = _agent.CreateIdWME(inputLink, "sensors");
            Identifier bumperWME = _agent.CreateIdWME(sensorsWME, "bumper");
            Identifier frontWME = _agent.CreateIdWME(bumperWME, "front");
            _frontBumperPressedWME = _agent.CreateStringWME(frontWME, "pressed", "false");
            _frontBumperWasPressedWME = _agent.CreateStringWME(frontWME, "was-pressed", "false");
            Identifier rearWME = _agent.CreateIdWME(bumperWME, "rear");
            _rearBumperPressedWME = _agent.CreateStringWME(rearWME, "pressed", "false");
            _rearBumperWasPressedWME = _agent.CreateStringWME(rearWME, "was-pressed", "false");

            // commit input link structure
            _agent.Commit();

            _updateCall = new sml.Kernel.UpdateEventCallback(UpdateEventCallback);
            _kernel.RegisterForUpdateEvent(sml.smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, _updateCall, null);

            // spawn debugger
            if (_state.SpawnDebugger)
            {
                SpawnDebugger();
            }

            LogInfo(LogGroups.Console, "Soar initialized.");
        }

        private void SpawnDebugger()
        {
            LogInfo(LogGroups.Console, "Spawning debugger...");
            System.Diagnostics.Process debuggerProc = new System.Diagnostics.Process();
            debuggerProc.EnableRaisingEvents = false;
            debuggerProc.StartInfo.WorkingDirectory = "bin";
            debuggerProc.StartInfo.FileName = "java";
            debuggerProc.StartInfo.Arguments = "-jar SoarJavaDebugger.jar -remote";
            debuggerProc.Start();

            // wait for it
            LogInfo(LogGroups.Console, "Starting debugger...");
            bool ready = false;
            // do this loop if timeout seconds is 0 (code for wait indefinitely) or if we have tries left
            for (int tries = 0; tries < 15; ++tries)
            {
                _kernel.GetAllConnectionInfo();
                if (_kernel.HasConnectionInfoChanged())
                {
                    for (int i = 0; i < _kernel.GetNumberConnections(); ++i)
                    {
                        ConnectionInfo info = _kernel.GetConnectionInfo(i);
                        if (info.GetName() == "java-debugger")
                        {
                            if (info.GetAgentStatus() == sml_Names.kStatusReady)
                            {
                                ready = true;
                                break;
                            }
                        }
                    }
                    if (ready)
                    {
                        break;
                    }
                }
                LogInfo(LogGroups.Console, "Waiting for java-debugger...");
                Thread.Sleep(1000);
            }

            if (!ready)
                LogInfo(LogGroups.Console, "Debugger spawn failed!");

        }

        private void UpdateEventCallback(sml.smlUpdateEventId eventID, IntPtr callbackData, IntPtr kernelPtr, smlRunFlags runFlags)
        {
            // check for stop
            if (_stop)
            {
                _stop = false;
                LogInfo(LogGroups.Console, "Soar: Update: Stopping all agents.");
                _kernel.StopAllAgents();
                return;
            }

            // read output link, cache commands
            int numberOfCommands = _agent.GetNumberCommands();
            Identifier command;
            bool receivedCommand = false;
            drive.SetDrivePowerRequest request = new drive.SetDrivePowerRequest();

            for (int i = 0; i < numberOfCommands; ++i)
            {
                command = _agent.GetCommand(i);
                String commandName = command.GetAttribute();

                switch (commandName)
                {
                    case "drive-power":
                        String leftPowerString = command.GetParameterValue("left");
                        if (leftPowerString != null)
                        {
                            receivedCommand = true;
                            request.LeftWheelPower = double.Parse(leftPowerString);
                        }

                        String rightPowerString = command.GetParameterValue("right");
                        if (rightPowerString != null)
                        {
                            receivedCommand = true;
                            request.RightWheelPower = double.Parse(rightPowerString);
                        }

                        String stopString = command.GetParameterValue("stop");
                        if (stopString != null)
                        {
                            if (bool.Parse(stopString))
                            {
                                receivedCommand = true;
                                request.LeftWheelPower = 0;
                                request.RightWheelPower = 0;
                                StopNow();
                            }
                        }

                        if (receivedCommand)
                        {
                            command.AddStatusComplete();
                        }
                        else
                        {
                            LogInfo(LogGroups.Console, "Soar: Unknown drive-power command.");
                            command.AddStatusError();
                        }

                        break;

                    default:
                        LogInfo(LogGroups.Console, "Soar: Unknown command.");
                        command.AddStatusError();
                        break;
                }
            }
            if (receivedCommand && _drivePort != null)
            {
                _drivePort.SetDrivePower(request);
            }

            bool overrideActive = bool.Parse(_overrideActiveWME.GetValue());
            if (_overrideActive != overrideActive)
                _agent.Update(_overrideActiveWME, _overrideActive.ToString().ToLowerInvariant());

            if (_overrideChanged)
            {
                double overrideLeft;
                double overrideRight;

                // lock state
                lock (this)
                {
                    // cache state for input link
                    overrideLeft = _overrideLeft;
                    overrideRight = _overrideRight;

                    // reset flag
                    _overrideChanged = false;

                    // unlock state
                }

                // write input link from cache
                LogInfo(LogGroups.Console, "Override: (" + overrideLeft + "," + overrideRight + ")");
                _agent.Update(_overrideLeftWME, overrideLeft);
                _agent.Update(_overrideRightWME, overrideRight);
            }

            bool frontBumperWasPressed = false;
            bool rearBumperWasPressed = false;
            bool frontBumperPressed = false;
            bool rearBumperPressed = false;

            // lock state
            lock (this)
            {
                // cache state
                frontBumperWasPressed = _frontBumperWasPressed;
                rearBumperWasPressed = _rearBumperWasPressed;
                frontBumperPressed = _frontBumperPressed;
                rearBumperPressed = _rearBumperPressed;

                // reset flag
                _frontBumperWasPressed = false;
                _rearBumperWasPressed = false;

                // unlock state
            }


            if (frontBumperWasPressed != bool.Parse(_frontBumperWasPressedWME.GetValue()))
            {
                _agent.Update(_frontBumperWasPressedWME, frontBumperWasPressed.ToString().ToLowerInvariant());
            }
            if (rearBumperWasPressed != bool.Parse(_rearBumperWasPressedWME.GetValue()))
            {
                _agent.Update(_rearBumperWasPressedWME, rearBumperWasPressed.ToString().ToLowerInvariant());
            }
            if (frontBumperPressed != bool.Parse(_frontBumperPressedWME.GetValue()))
            {
                _agent.Update(_frontBumperPressedWME, frontBumperPressed.ToString().ToLowerInvariant());
            }
            if (rearBumperPressed != bool.Parse(_rearBumperPressedWME.GetValue()))
            {
                _agent.Update(_rearBumperPressedWME, rearBumperPressed.ToString().ToLowerInvariant());
            }

            DateTimeConverter dtc = new DateTimeConverter();

            TimeSpan elapsed = System.DateTime.Now - _simulationStart;
            _agent.Update(_timeWME, elapsed.TotalMilliseconds);
            _agent.Update(_randomWME, _randomGen.NextDouble());

            // commit input link changes
            _agent.Commit();
        }

        private void RunSoar()
        {
            _running = true;
            LogInfo(LogGroups.Console, "Soar: Started.");
            _kernel.RunAllAgentsForever();
            _running = false;
            LogInfo(LogGroups.Console, "Soar: Stopped.");
        }

        private void ShutdownSoar()
        {
            if (_kernel == null)
                return;

            while (_running)
            {
                LogInfo(LogGroups.Console, "Soar: Stop requested.");
                _stop = true;
                System.Threading.Thread.Sleep(100);
            }

            _kernel.Shutdown();
            _kernel = null;
        }

        // ---------------- End Soar Code

        // Robotics Tutorial 1 Step 4 -- Subscribing to services
        // Robotics Tutorial 2 Step 2 -- Same code
        // Robotics Tutorial 3 Step 2 -- Similar
        /// <summary>
        /// Subscribe to the Bumpers service
        /// </summary>
        // Create the bumper notification port
        bumper.ContactSensorArrayOperations bumperNotificationPort = new bumper.ContactSensorArrayOperations();
        void SubscribeToBumpers()
        {

            // Subscribe to the bumper service, receive notifications on the bumperNotificationPort
            _bumperPort.Subscribe(bumperNotificationPort);

            // Start listening for updates from the bumper service
            //            Activate(
            //                Arbiter.Receive<bumper.Update>
            //                    (true, bumperNotificationPort, BumperHandler));
            Activate(
                Arbiter.Interleave(
                    new TeardownReceiverGroup
                    (
                        Arbiter.Receive<DsspDefaultDrop>
                            (false, bumperNotificationPort, DropHandler)
                    ),
                    new ExclusiveReceiverGroup(
                        Arbiter.Receive<bumper.Update>
                            (true, bumperNotificationPort, BumperHandler)
                    ),
                    new ConcurrentReceiverGroup()
                )
            );
        }

        void DropHandler(DsspDefaultDrop drop)
        {
            ShutdownSoar();

            base.DefaultDropHandler(drop);
        }

        /// <summary>
        /// Get Handler
        /// </summary>
        /// <param name="get"></param>
        /// <returns></returns>
        [ServiceHandler(ServiceHandlerBehavior.Concurrent)]
        public virtual IEnumerator<ITask> GetHandler(Get get)
        {
            get.ResponsePort.Post(_state);
            yield break;
        }
        /// <summary>
        /// Replace Handler
        /// </summary>
        /// <param name="replace"></param>
        /// <returns></returns>
        [ServiceHandler(ServiceHandlerBehavior.Exclusive)]
        public virtual IEnumerator<ITask> ReplaceHandler(Replace replace)
        {
            _state = replace.Body;
            replace.ResponsePort.Post(DefaultReplaceResponseType.Instance);
            yield break;
        }

        /// <summary>
        /// Handle Bumper Notifications
        /// </summary>
        /// <param name="notification"></param>
        void BumperHandler(bumper.Update notification)
        {
            string message;
            string bumperName;
            DateTime thisTimestamp;
            TimeSpan timediff;

            // Find out which bumper this is
            int num = notification.Body.HardwareIdentifier;

            if (string.IsNullOrEmpty(notification.Body.Name))
            {
                LogInfo(LogGroups.Console, "Bumper name is null or empty.");
                return;
            }
            else
                bumperName = notification.Body.Name.ToLowerInvariant();

            if (bumperName.Contains("front"))
            {
                lock (this)
                {
                    //LogInfo(LogGroups.Console, "Front: " + (frontBumperPressed ? "Pressed" : "Released") + " @ " + frontBumperTime);
                    _frontBumperPressed = notification.Body.Pressed;
                    if (_frontBumperPressed)
                        _frontBumperWasPressed = true;
                }
            }
            else if (bumperName.Contains("rear"))
            {
                lock (this)
                {
                    _rearBumperPressed = notification.Body.Pressed;
                    if (_rearBumperPressed)
                        _rearBumperWasPressed = true;
                }
            }
        }

        Random _randomGen = new Random();

        // StopNow() is a quick stop that is called as soon as a
        // bumper press is detected. Unfortunately, inertia makes
        // the robot keep bumping up against the wall. This results
        // in a flood of messages. If there are to many to process,
        // there might still be some in the queue after the behavior
        // has finished, and the robot will stop even though it is
        // no longer in contact with the wall ... This leaves it
        // stranded and you have to start it again :-(
        void StopNow()
        {
            drive.AllStopRequest stopReq = new drive.AllStopRequest();

            // TT Jul-2007 - Oops! This Arbiter.Choice was not activated
            Arbiter.Activate(TaskQueue,
                Arbiter.Choice(
                   _drivePort.AllStop(stopReq),
                   delegate(DefaultUpdateResponseType success)
                   {
                   },
                   delegate(W3C.Soap.Fault failure)
                   {
                       LogError("Failed to Stop!");
                   }
                )
            );
        }
    }
}
