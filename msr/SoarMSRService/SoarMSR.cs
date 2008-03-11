using Microsoft.Ccr.Core;
using Microsoft.Dss.Core;
using Microsoft.Dss.Core.Attributes;
using Microsoft.Dss.ServiceModel.Dssp;
using Microsoft.Dss.ServiceModel.DsspServiceBase;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Xml;
using W3C.Soap;
using System.Security.Permissions;
using soarmsr = Robotics.SoarMSR;
using System.Threading;

using bumper = Microsoft.Robotics.Services.ContactSensor.Proxy;
using drive = Microsoft.Robotics.Services.Drive.Proxy;
using sicklrf = Microsoft.Robotics.Services.Sensors.SickLRF.Proxy;


namespace Robotics.SoarMSR
{
    /// <summary>
    /// Implementation class for SoarMSR
    /// </summary>
    [DisplayName("SoarMSR")]
    [Description("The SoarMSR Service - Control using Soar")]
    [Contract(Contract.Identifier)]
    public class SoarMSRService : DsspServiceBase
    {
        // For saving and restoring state
        public const string InitialStateUri = ServicePaths.MountPoint + @"/Apps/QUT/Config/SoarMSR.Config.xml";
        [InitialStatePartner(Optional = true, ServiceUri = InitialStateUri)]
        protected SoarMSRState _state;
        
        /// <summary>
        /// _main Port
        /// </summary>
        [ServicePort("/soarmsr", AllowMultipleInstances = false)]
        protected SoarMSROperations _mainPort = new SoarMSROperations();

        [Partner("bumper", Contract = bumper.Contract.Identifier, CreationPolicy = PartnerCreationPolicy.UseExisting)]
        protected bumper.ContactSensorArrayOperations _bumperPort = new bumper.ContactSensorArrayOperations();
        protected bumper.ContactSensorArrayOperations _bumperNotify = new bumper.ContactSensorArrayOperations();

        [Partner("Drive", Contract = drive.Contract.Identifier, CreationPolicy = PartnerCreationPolicy.UseExisting)]
        protected drive.DriveOperations _drivePort = new drive.DriveOperations();

        [Partner("Laser", Contract = sicklrf.Contract.Identifier, CreationPolicy = PartnerCreationPolicy.UseExisting)]
        protected sicklrf.SickLRFOperations _laserPort = new sicklrf.SickLRFOperations();
        protected sicklrf.SickLRFOperations _laserNotify = new sicklrf.SickLRFOperations();

        Soar _soar = new Soar();

        /// <summary>
        /// Default Service Constructor
        /// </summary>
        public SoarMSRService(DsspServiceCreationPort creationPort) : 
                base(creationPort)
        {
        }
        
        /// <summary>
        /// Service Start
        /// </summary>
        protected override void Start()
        {
			base.Start();
			// Add service specific initialization here.

            if (_state == null)
            {
                _state = new SoarMSRState();
            }

            // There should be some code in here to validate the settings
            // from the config file just in case the user entered some
            // invalid values ...

            // Now save the State
            // This creates a new file the first time it is run.
            // Later, it re-reads the existing file, but by then
            // the file has been populated with the default values.
            SaveState(_state);

            _soar.Log += LogHandler;
            _soar.DriveOutput += DriveOutputHandler;
            _soar.InitializeSoar(_state);

            SubscribeToBumpers();

            // Start Soar after everything is loaded
            Thread soarThread = new Thread(new ThreadStart(_soar.RunSoar));
            soarThread.Start();
        }

        protected void SubscribeToBumpers()
        {
            _bumperPort.Subscribe(_bumperNotify);
            Activate(
                Arbiter.Interleave(
                    new TeardownReceiverGroup(),
                    new ExclusiveReceiverGroup(
                        Arbiter.Receive<bumper.Update>(true, _bumperNotify, BumperHandler)
                    ),
                    new ConcurrentReceiverGroup()
                )
            );
        }

        protected void BumperHandler(bumper.Update notification)
        {
            if (string.IsNullOrEmpty(notification.Body.Name))
            {
                LogInfo(LogGroups.Console, "Bumper name is null or empty.");
                return;
            }

            string bumperName = notification.Body.Name.ToLowerInvariant();

            if (bumperName.Contains("front"))
            {
                lock (_soar.Bumper)
                {
                    _soar.Bumper.FrontBumperPressed = notification.Body.Pressed;
                }
            }
            else if (bumperName.Contains("rear"))
            {
                lock (_soar.Bumper)
                {
                    _soar.Bumper.RearBumperPressed = notification.Body.Pressed;
                }
            }
        }

        protected void LogHandler(string message)
        {
            LogInfo(LogGroups.Console, message);
        }

        [ServiceHandler(ServiceHandlerBehavior.Concurrent)]
        public virtual IEnumerator<ITask> GetHandler(Get get)
        {
            get.ResponsePort.Post(_state);
            yield break;
        }

        protected void DriveOutputHandler(DriveOutputState output)
        {
            if (output.AllStop)
            {
                Arbiter.Activate(TaskQueue,
                    Arbiter.Choice(
                       _drivePort.AllStop(new drive.AllStopRequest()),
                       delegate(DefaultUpdateResponseType success) { },
                       delegate(W3C.Soap.Fault failure)
                       {
                           LogError("Failed to Stop!");
                       }
                    )
                );
            }
            else
            {
                _drivePort.SetDrivePower(output);
            }
        }

        protected override void Shutdown()
        {
            _soar.ShutdownSoar();
            _soar.Log -= LogHandler;
            _soar.DriveOutput -= DriveOutputHandler;

            base.Shutdown();
        }
    }
}
