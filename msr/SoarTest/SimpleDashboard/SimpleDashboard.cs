//-----------------------------------------------------------------------
//  This file is part of the Microsoft Robotics Studio Code Samples.
// 
//  Copyright (C) Microsoft Corporation.  All rights reserved.
//
//  $File: SimpleDashboard.cs $ $Revision: 1 $
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;


using W3C.Soap;
using Microsoft.Ccr.Core;
using Microsoft.Ccr.Adapters.WinForms;
using Microsoft.Dss.Core;
using Microsoft.Dss.Core.Attributes;
using Microsoft.Dss.ServiceModel.Dssp;
using Microsoft.Dss.ServiceModel.DsspServiceBase;
using Microsoft.Dss.Services.Serializer;
using Microsoft.Robotics.Simulation.Physics.Proxy;
using Microsoft.Robotics.Simulation.Proxy;
using System.IO;
using System.Xml;
using System.Xml.Serialization;

using arm = Microsoft.Robotics.Services.ArticulatedArm.Proxy;
using cs = Microsoft.Dss.Services.Constructor;
using drive = Microsoft.Robotics.Services.Drive.Proxy;
using ds = Microsoft.Dss.Services.Directory;
using fs = Microsoft.Dss.Services.FileStore;
using game = Microsoft.Robotics.Services.GameController.Proxy;
using sicklrf = Microsoft.Robotics.Services.Sensors.SickLRF.Proxy;
using submgr = Microsoft.Dss.Services.SubscriptionManager;
using dssp = Microsoft.Dss.ServiceModel.Dssp;

using Microsoft.Robotics.PhysicalModel.Proxy;
using System.ComponentModel;
using System.Diagnostics;
using System.Threading;

namespace Microsoft.Robotics.Services.SimpleDashboard
{

    /// <summary>
    /// Simple Dashboard Service
    /// </summary>
    [DisplayName("Simple Dashboard")]
    [Description("Provides access to a simple Windows UI for interacting with sensor and actuator services.\n(Partner with the 'Game Controller' service.)")]
    [Contract(Contract.Identifier)]
    class SimpleDashboardService : DsspServiceBase
    {
        // shared access to state is protected by the interleave pattern
        // when we activate the handlers
        SimpleDashboardState _state = new SimpleDashboardState();

        [ServicePort("/simpledashboard", AllowMultipleInstances = true)]
        SimpleDashboardOperations _mainPort = new SimpleDashboardOperations();

        [Partner("GameController", Contract = game.Contract.Identifier, CreationPolicy = PartnerCreationPolicy.UseExistingOrCreate)]
        game.GameControllerOperations _gameControllerPort = new game.GameControllerOperations();
        game.GameControllerOperations _gameControllerNotify = new game.GameControllerOperations();
        Port<Shutdown> _gameControllerShutdown = new Port<Shutdown>();

        DriveControl _driveControl;
        DriveControlEvents _eventsPort = new DriveControlEvents();

        Soar _soar = new Soar();

        #region Startup
        /// <summary>
        /// SimpleDashboardService Default DSS Constuctor
        /// </summary>
        /// <param name="pCreate"></param>
        public SimpleDashboardService(DsspServiceCreationPort pCreate) : base(pCreate)
        {
            
        }

        /// <summary>
        /// Entry Point for the SimpleDashboard Service
        /// </summary>
        protected override void Start()
        {
            // Handlers that need write or exclusive access to state go under
            // the exclusive group. Handlers that need read or shared access, and can be
            // concurrent to other readers, go to the concurrent group.
            // Other internal ports can be included in interleave so you can coordinate
            // intermediate computation with top level handlers.
            Activate(Arbiter.Interleave(
                new TeardownReceiverGroup
                (
                    Arbiter.Receive<DsspDefaultDrop>(false, _mainPort, DropHandler)
                ),
                new ExclusiveReceiverGroup
                (
                    Arbiter.ReceiveWithIterator<Replace>(true, _mainPort, ReplaceHandler),
                    Arbiter.ReceiveWithIterator<OnLoad>(true, _eventsPort, OnLoadHandler),
                    Arbiter.Receive<OnClosed>(true, _eventsPort, OnClosedHandler),
                    Arbiter.ReceiveWithIterator<OnChangeJoystick>(true, _eventsPort, OnChangeJoystickHandler),
                    Arbiter.Receive<OnLogSetting>(true, _eventsPort, OnLogSettingHandler)
                ),
                new ConcurrentReceiverGroup
                (
                    Arbiter.Receive<DsspDefaultLookup>(true,_mainPort,DefaultLookupHandler),
                    Arbiter.ReceiveWithIterator<Get>(true, _mainPort, GetHandler),
                    
                    Arbiter.ReceiveWithIterator<game.Replace>(true, _gameControllerNotify, JoystickReplaceHandler),
                    Arbiter.ReceiveWithIterator<game.UpdateAxes>(true, _gameControllerNotify, JoystickUpdateAxesHandler),
                    Arbiter.ReceiveWithIterator<game.UpdateButtons>(true, _gameControllerNotify, JoystickUpdateButtonsHandler),
                    Arbiter.ReceiveWithIterator<sicklrf.Replace>(true, _laserNotify, OnLaserReplaceHandler),
                    Arbiter.ReceiveWithIterator<OnConnect>(true, _eventsPort, OnConnectHandler),
                    
                    Arbiter.ReceiveWithIterator<drive.Update>(true, _driveNotify, OnDriveUpdateNotificationHandler),
                    Arbiter.ReceiveWithIterator<OnConnectMotor>(true, _eventsPort, OnConnectMotorHandler),
                    Arbiter.ReceiveWithIterator<OnMove>(true, _eventsPort, OnMoveHandler),
                    Arbiter.ReceiveWithIterator<OnEStop>(true, _eventsPort, OnEStopHandler),
                    
                    Arbiter.ReceiveWithIterator<OnStartService>(true, _eventsPort, OnStartServiceHandler),
                    Arbiter.ReceiveWithIterator<OnConnectSickLRF>(true, _eventsPort, OnConnectSickLRFHandler),
                    Arbiter.Receive<OnDisconnectSickLRF>(true, _eventsPort, OnDisconnectSickLRFHandler),

                    Arbiter.ReceiveWithIterator<OnConnectArticulatedArm>(true, _eventsPort, OnConnectArticulatedArmHandler),
                    Arbiter.ReceiveWithIterator<OnApplyJointParameters>(true, _eventsPort, OnApplyJointParametersHandler)
                )
            ));

            DirectoryInsert();

            // Initialize Soar
            _soar.Init();

            WinFormsServicePort.Post(new RunForm(CreateForm));

            // Start Soar after everything is loaded
            Thread soarThread = new Thread(new ThreadStart(_soar.Run));
            soarThread.Start();

        }

        #endregion

        #region WinForms interaction

        System.Windows.Forms.Form CreateForm()
        {
            return new DriveControl(_eventsPort);
        }       

        #endregion

        #region DSS Handlers

        /// <summary>
        /// Get Handler returns SimpleDashboard State.
        /// </summary>
        /// <remarks>
        /// We declare this handler as an iterator so we can easily do
        /// sequential, logically blocking receives, without the need
        /// of nested Activate calls
        /// </remarks>
        /// <param name="get"></param>
        IEnumerator<ITask> GetHandler(Get get)
        {
            get.ResponsePort.Post(_state);
            yield break;
        }

        /// <summary>
        /// Replace Handler sets SimpleDashboard State
        /// </summary>
        /// <param name="replace"></param>
        IEnumerator<ITask> ReplaceHandler(Replace replace)
        {
            _state = replace.Body;
            replace.ResponsePort.Post(dssp.DefaultReplaceResponseType.Instance);
            yield break;
        }

        /// <summary>
        /// Drop Handler shuts down SimpleDashboard
        /// </summary>
        /// <param name="drop"></param>
        void DropHandler(DsspDefaultDrop drop)
        {
            _soar.Shutdown();
            _soar = null;

            PerformShutdown(ref _laserShutdown);
            PerformShutdown(ref _motorShutdown);
            if (_driveControl != null)
            {
                DriveControl drive = _driveControl;
                _driveControl = null;

                WinFormsServicePort.FormInvoke(
                    delegate()
                    {
                        if (!drive.IsDisposed)
                        {
                            drive.Dispose();
                        }
                    }
                );
            }

            base.DefaultDropHandler(drop);
        }

        void PerformShutdown(ref Port<Shutdown> port)
        {
            if (port == null)
                return;
            Shutdown shutdown = new Shutdown();
            port.Post(shutdown);
            port = null;
        }

        #endregion

        #region Joystick Operations

        Choice EnumerateJoysticks()
        {
            return Arbiter.Choice(
                _gameControllerPort.GetControllers(new game.GetControllersRequest()),
                delegate(game.GetControllersResponse response)
                {
                    WinFormsServicePort.FormInvoke(
                        delegate()
                        {
                            _driveControl.ReplaceJoystickList(response.Controllers);
                        }
                    );
                },
                delegate(Fault fault)
                {
                    LogError(fault);
                }
            );
        }

        Choice SubscribeToJoystick()
        {
            return Arbiter.Choice(
                _gameControllerPort.Subscribe(_gameControllerNotify),
                delegate(SubscribeResponseType response) { },
                delegate(Fault fault) { LogError(fault); }
            );

        }

        IEnumerator<ITask> JoystickReplaceHandler(game.Replace replace)
        {
            if (_driveControl != null)
            {
                WinFormsServicePort.FormInvoke(
                    delegate()
                    {
                        _driveControl.UpdateJoystickButtons(replace.Body.Buttons);
                        _driveControl.UpdateJoystickAxes(replace.Body.Axes);
                    }
                );
            }

            yield break;
        }

        IEnumerator<ITask> JoystickUpdateAxesHandler(game.UpdateAxes update)
        {
            if (_driveControl != null)
            {
                WinFormsServicePort.FormInvoke(
                    delegate()
                    {
                        _driveControl.UpdateJoystickAxes(update.Body);
                    }
                );
            }
            yield break;
        }

        IEnumerator<ITask> JoystickUpdateButtonsHandler(game.UpdateButtons update)
        {
            if (_driveControl != null)
            {
                WinFormsServicePort.FormInvoke(
                    delegate()
                    {
                        _driveControl.UpdateJoystickButtons(update.Body);
                    }
                );
            }
            yield break;
        }

        IEnumerator<ITask> OnChangeJoystickHandler(OnChangeJoystick onChangeJoystick)
        {
            if (onChangeJoystick.DriveControl == _driveControl)
            {
                Activate(Arbiter.Choice(
                    _gameControllerPort.ChangeController(onChangeJoystick.Joystick),
                    delegate(DefaultUpdateResponseType response)
                    {
                        LogInfo("Changed Joystick");
                    },
                    delegate(Fault f)
                    {
                        LogError(null, "Unable to change Joystick", f);
                    })
                );
            }
            yield break;
        }

        #endregion

        #region Drive Control Event Handlers

        IEnumerator<ITask> OnLoadHandler(OnLoad onLoad)
        {
            _driveControl = onLoad.DriveControl;

            LogInfo("Loaded Form");

            yield return EnumerateJoysticks();

            yield return SubscribeToJoystick();
        }


        void OnClosedHandler(OnClosed onClosed)
        {
            if (onClosed.DriveControl == _driveControl)
            {
                LogInfo("Form Closed");

                _mainPort.Post(new DsspDefaultDrop(DropRequestType.Instance));
            }
        }


        IEnumerator<ITask> OnConnectHandler(OnConnect onConnect)
        {
            if (onConnect.DriveControl == _driveControl)
            {
                UriBuilder builder = new UriBuilder(onConnect.Service);
                builder.Scheme = new Uri(ServiceInfo.Service).Scheme;

                ds.DirectoryPort port = ServiceForwarder<ds.DirectoryPort>(builder.Uri);
                ds.Get get = new ds.Get();

                port.Post(get);
                ServiceInfoType[] list = null;

                yield return Arbiter.Choice(get.ResponsePort,
                    delegate(ds.GetResponseType response)
                    {
                        list = response.RecordList;
                    },
                    delegate(Fault fault)
                    {
                        list = new ServiceInfoType[0];
                        LogError(fault);
                    }
                );

                WinFormsServicePort.FormInvoke(
                    delegate()
                    {
                        _driveControl.ReplaceDirectoryList(list);
                    }
                );
            }
        }

        IEnumerator<ITask> OnStartServiceHandler(OnStartService onStartService)
        {
            if (onStartService.DriveControl == _driveControl &&
                onStartService.Constructor != null)
            {
                cs.ConstructorPort port = ServiceForwarder<cs.ConstructorPort>(onStartService.Constructor);

                ServiceInfoType request = new ServiceInfoType(onStartService.Contract);
                cs.Create create = new cs.Create(request);

                port.Post(create);

                string service = null;

                yield return Arbiter.Choice(
                    create.ResponsePort,
                    delegate(CreateResponse response)
                    {
                        service = response.Service;
                    },
                    delegate(Fault fault)
                    {
                        LogError(fault);
                    }
                );


                if (service == null)
                {
                    yield break;
                }

                WinFormsServicePort.FormInvoke(
                    delegate()
                    {
                        _driveControl.StartedSickLRF();
                    }
                );
            }
        }

        #endregion

        #region Motor operations

        drive.DriveOperations _drivePort = null;
        drive.DriveOperations _driveNotify = new drive.DriveOperations();
        Port<Shutdown> _motorShutdown = null;

        IEnumerator<ITask> OnConnectMotorHandler(OnConnectMotor onConnectMotor)
        {
            if (onConnectMotor.DriveControl == _driveControl)
            {
                drive.EnableDriveRequest request = new drive.EnableDriveRequest(false);
                if (_drivePort != null)
                {
                    yield return Arbiter.Choice(
                        _drivePort.EnableDrive(request),
                        delegate(DefaultUpdateResponseType response) { },
                        delegate(Fault f)
                        {
                            LogError(f);
                        }
                    );

                    if (_motorShutdown != null)
                    {
                        PerformShutdown(ref _motorShutdown);
                    }
                }

                _drivePort = ServiceForwarder<drive.DriveOperations>(onConnectMotor.Service);
                _soar.SetDrivePort(_drivePort);
                _motorShutdown = new Port<Shutdown>();

                drive.ReliableSubscribe subscribe = new drive.ReliableSubscribe(
                    new ReliableSubscribeRequestType(10)
                );
                subscribe.NotificationPort = _driveNotify;
                subscribe.NotificationShutdownPort = _motorShutdown;

                _drivePort.Post(subscribe);

                yield return Arbiter.Choice(
                    subscribe.ResponsePort,
                    delegate(SubscribeResponseType response)
                    {
                        LogInfo("Subscribed to " + onConnectMotor.Service);
                    },
                    delegate(Fault fault)
                    {
                        _motorShutdown = null;
                        LogError(fault);
                    }
                );

                request = new drive.EnableDriveRequest(true);

                yield return Arbiter.Choice(
                    _drivePort.EnableDrive(request),
                    delegate(DefaultUpdateResponseType response) { },
                    delegate(Fault f)
                    {
                        LogError(f);
                    }
                );
            }
        }

        IEnumerator<ITask> OnDriveUpdateNotificationHandler(drive.Update notification)
        {
            if (_driveControl != null)
            {
                WinFormsServicePort.FormInvoke(
                    delegate()
                    {
                        _driveControl.UpdateMotorData(notification.Body);
                    }
                );
            }

            LogObject(notification.Body);
            yield break;
        }

        double MOTOR_POWER_SCALE_FACTOR = 0.001;
        IEnumerator<ITask> OnMoveHandler(OnMove onMove)
        {
            //if (onMove.DriveControl == _driveControl && _drivePort != null)
            //{
            //    drive.SetDrivePowerRequest request = new drive.SetDrivePowerRequest();
            //    request.LeftWheelPower = (double)onMove.Left * MOTOR_POWER_SCALE_FACTOR;
            //    request.RightWheelPower = (double)onMove.Right * MOTOR_POWER_SCALE_FACTOR;

            //    Trace.WriteLine("LeftWheelPower: " + request.LeftWheelPower + ", RightWheelPower: " + request.RightWheelPower);

            //    _drivePort.SetDrivePower(request);
            //}
            //yield break;
            _soar.Suggest((double)onMove.Left * MOTOR_POWER_SCALE_FACTOR, (double)onMove.Right * MOTOR_POWER_SCALE_FACTOR);
            yield break;
        }

        IEnumerator<ITask> OnEStopHandler(OnEStop onEStop)
        {
            if (onEStop.DriveControl == _driveControl && _drivePort != null)
            {
                LogInfo("Requesting EStop");
                drive.AllStopRequest request = new drive.AllStopRequest();

                _drivePort.AllStop(request);
            }
            yield break;
        }

        #endregion

        #region Articulated Arm operations
        arm.ArticulatedArmOperations _articulatedArmPort;
        IEnumerator<ITask> OnConnectArticulatedArmHandler(OnConnectArticulatedArm onConnect)
        {
            arm.ArticulatedArmState armState = null;

            if (onConnect.DriveControl != _driveControl)
                yield break;

            _articulatedArmPort = ServiceForwarder<arm.ArticulatedArmOperations>(onConnect.Service);
            yield return Arbiter.Choice(
                _articulatedArmPort.Get(new GetRequestType()),
                delegate(arm.ArticulatedArmState state) { armState = state; },
                delegate(Fault f) { LogError(f); }
            );

            if (armState == null)
                yield break;

            WinFormsServicePort.FormInvoke(delegate()
            {
                _driveControl.ReplaceArticulatedArmJointList(armState);
            });

            yield break;
        }

        IEnumerator<ITask> OnApplyJointParametersHandler(OnApplyJointParameters onApply)
        {
            arm.SetJointTargetPoseRequest req = new arm.SetJointTargetPoseRequest();
            req.JointName = onApply.JointName;
            AxisAngle aa = new AxisAngle(
                new Vector3(1, 0, 0),
                (float)(Math.PI * 2 * ((double)onApply.Angle / 360)));

            req.TargetOrientation = aa;
                
            _articulatedArmPort.SetJointTargetPose(req);
            yield break;
        }

        #endregion

        #region Laser Range Finder operations

        sicklrf.SickLRFOperations _laserPort;
        sicklrf.SickLRFOperations _laserNotify = new sicklrf.SickLRFOperations();
        Port<Shutdown> _laserShutdown = null;

        IEnumerator<ITask> OnConnectSickLRFHandler(OnConnectSickLRF onConnectSickLRF)
        {
            if (onConnectSickLRF.DriveControl != _driveControl)
                yield break;
            _laserPort = ServiceForwarder<sicklrf.SickLRFOperations>(onConnectSickLRF.Service);
            _laserShutdown = new Port<Shutdown>();

            sicklrf.ReliableSubscribe subscribe = new sicklrf.ReliableSubscribe(
                new ReliableSubscribeRequestType(5)
            );
            subscribe.NotificationPort = _laserNotify;
            subscribe.NotificationShutdownPort = _laserShutdown;

            _laserPort.Post(subscribe);

            yield return Arbiter.Choice(
                subscribe.ResponsePort,
                delegate(SubscribeResponseType response)
                {
                    LogInfo("Subscribed to " + onConnectSickLRF.Service);
                },
                delegate(Fault fault)
                {
                    _laserShutdown = null;
                    LogError(fault);
                }
            );
        }

        IEnumerator<ITask> OnLaserReplaceHandler(sicklrf.Replace replace)
        {
            if (_driveControl != null)
            {
                WinFormsServicePort.FormInvoke(
                    delegate()
                    {
                        _driveControl.ReplaceLaserData(replace.Body);
                    }
                );
            }

            LogObject(replace.Body);
            yield break;
        }

        void OnDisconnectSickLRFHandler(OnDisconnectSickLRF onDisconnectSickLRF)
        {
            PerformShutdown(ref _laserShutdown);
        }

        #endregion

        #region Logging operations

        fs.FileStorePort _fileStorePort = null;
        object _fspLock = new object();

        void OnLogSettingHandler(OnLogSetting onLogSetting)
        {
            _state.Log = onLogSetting.Log;
            _state.LogFile = onLogSetting.File;

            if (_state.Log)
            {
                try
                {
                    Uri file = new Uri(_state.LogFile);


                    fs.FileStoreCreate fsCreate = new fs.FileStoreCreate(file, new fs.FileStorePort());
                    FileStoreConstructorPort.Post(fsCreate);
                    Activate(
                        Arbiter.Choice(
                            fsCreate.ResultPort,
                            delegate(fs.FileStorePort fsp)
                            {
                                LogInfo("Started Logging");
                                lock (_fspLock)
                                {
                                    _fileStorePort = fsp;
                                }
                            },
                            delegate(Exception ex)
                            {
                                WinFormsServicePort.FormInvoke(delegate()
                                    {
                                        _driveControl.ErrorLogging(ex);
                                    }
                                );

                            }
                        )
                    );
                }
                catch (Exception e)
                {
                    WinFormsServicePort.FormInvoke(delegate()
                        {
                            _driveControl.ErrorLogging(e);
                        }
                    );
                }
            }
            else if (_fileStorePort != null)
            {
                LogInfo("Stop Logging");
                lock(_fspLock)
                {
                    fs.FileStorePort fsp = _fileStorePort;

                    LogInfo("Flush Log");
                    fsp.Post(new fs.Flush());

                    Activate(
                        Arbiter.Receive(false, TimeoutPort(1000),
                            delegate(DateTime signal)
                            {
                                LogInfo("Stop Log");
                                fsp.Post(new Shutdown());
                            }
                        )
                    );
                    
                    _fileStorePort = null;
                }
            }
        }

        void LogObject(object data)
        {
            lock (_fspLock)
            {
                if (_state.Log &&
                    _fileStorePort != null)
                {
                    _fileStorePort.Post(new fs.WriteObject(data));
                }
            }
        }

        #endregion
    }

}
