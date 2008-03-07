using System;
using System.Collections.Generic;
using System.Text;

namespace Robotics.SoarMSR
{
    class Soar
    {
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

    }
}
