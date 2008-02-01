using drive = Microsoft.Robotics.Services.Drive.Proxy;
using System;
using System.Collections.Generic;
using System.Text;

using sml;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace Microsoft.Robotics.Services.SimpleDashboard
{
    public class Soar
    {
        private sml.Kernel _kernel;
        private sml.Agent _agent;
        private sml.Kernel.UpdateEventCallback _updateCall;

        private FloatElement _suggestedLeftWME;
        private FloatElement _suggestedRightWME;
        private bool _running = false;
        private bool _stop = false;
        drive.DriveOperations _drivePort = null;

        // this state needs to be reentrant
        private bool _suggestedChanged; // not protected on read
        private double _suggestedLeft;
        private double _suggestedRight;

        void UpdateEventCallback(sml.smlUpdateEventId eventID, IntPtr callbackData, IntPtr kernelPtr, smlRunFlags runFlags)
        {
            // check for stop
            if (_stop)
            {
                _stop = false;
                Trace.WriteLine("Soar: Update: Stopping all agents.");
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

                        if (receivedCommand)
                        {
                            command.AddStatusComplete();
                        }
                        else
                        {
                            Trace.WriteLine("Soar: Unknown drive-power command.");
                            command.AddStatusError();
                        }

                        break;

                    default:
                        Trace.WriteLine("Soar: Unknown command.");
                        command.AddStatusError();
                        break;
                }
            }
            if (receivedCommand && _drivePort != null)
            {
                Trace.WriteLine("LeftWheelPower: " + request.LeftWheelPower + ", RightWheelPower: " + request.RightWheelPower);

                _drivePort.SetDrivePower(request);
            }

            if (_suggestedChanged)
            {
                double suggestedLeft;
                double suggestedRight;

                // lock state
                lock (this)
                {
                    // cache state for input link
                    suggestedLeft = _suggestedLeft;
                    suggestedRight = _suggestedRight;

                    // reset flag
                    _suggestedChanged = false;

                    // unlock state
                }

                Trace.WriteLine("suggested (" + suggestedLeft + "," + suggestedRight + ")");

                // write input link from cache
                _agent.Update(_suggestedLeftWME, suggestedLeft);
                _agent.Update(_suggestedRightWME, suggestedRight);
            }
        }

        internal void Init()
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

            //Trace.WriteLine(_agent.ExecuteCommandLine("pwd"));
            _agent.LoadProductions("samples/SimulationTutorials/SoarTest/agents/suggestion-agent.soar");

            // Prepare communication channel
            Identifier inputLink = _agent.GetInputLink();

            if (inputLink == null)
                throw new Exception("Error getting the input link");

            Identifier suggestedPower = _agent.CreateIdWME(inputLink, "suggested-power");
            _suggestedLeftWME = _agent.CreateFloatWME(suggestedPower, "left", 0);
            _suggestedRightWME = _agent.CreateFloatWME(suggestedPower, "right", 0);
            _suggestedLeft = 0;
            _suggestedRight = 0;

            // Register for update event
            _updateCall = new sml.Kernel.UpdateEventCallback(UpdateEventCallback);
            int callbackId = _kernel.RegisterForUpdateEvent(sml.smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, _updateCall, null);
        }

        internal void Run()
        {
            _running = true;
            Trace.WriteLine("Soar: Started.");
            _kernel.RunAllAgentsForever();
            _running = false;
            Trace.WriteLine("Soar: Stopped.");
        }

        internal void Shutdown()
        {
            while (_running)
            {
                Trace.WriteLine("Soar: Stop requested.");
                _stop = true;
                System.Threading.Thread.Sleep(100);
            }

            _kernel.Shutdown();
        }

        internal void SetDrivePort(Microsoft.Robotics.Services.Drive.Proxy.DriveOperations drivePort)
        {
            _drivePort = drivePort;
        }

        internal void Suggest(double left, double right)
        {
            // lock state
            lock (this)
            {
                _suggestedLeft = left;
                _suggestedRight = right;
                _suggestedChanged = true;
            }            
        }
    }
}
