using System;
using System.Collections.Generic;
using System.Text;

using sml;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace Robotics.SimulationTutorial2
{
    public class Soar
    {
        private sml.Kernel _kernel;
        private sml.Agent _agent;
        private sml.Kernel.UpdateEventCallback _updateCall;

        private FloatElement _suggestedLeftWME;
        private FloatElement _suggestedRightWME;

        // this state needs to be reentrant
        private double _commandedLeft;
        private double _commandedRight;
        private double _suggestedLeft;
        private double _suggestedRight;

        void UpdateEventCallback(sml.smlUpdateEventId eventID, IntPtr callbackData, IntPtr kernelPtr, smlRunFlags runFlags)
        {
            // read output link, cache commands
            int numberOfCommands = _agent.GetNumberCommands();
            Identifier command;
            double commandedLeft = 0;
            bool commandedLeftChange = false;
            double commandedRight = 0;
            bool commandedRightChange = false;

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
                            commandedLeftChange = true;
                            commandedLeft = Double.Parse(leftPowerString);
                        }

                        String rightPowerString = command.GetParameterValue("right");
                        if (rightPowerString != null)
                        {
                            commandedRightChange = true;
                            commandedRight = Double.Parse(rightPowerString);
                        }

                        break;

                    default:
                        break;
                }
            }

            double suggestedLeft;
            double suggestedRight;

            // lock state
            lock (this)
            {
                // write commands to state
                if (commandedLeftChange)
                {
                    _commandedLeft = commandedLeft;
                }

                if (commandedRightChange)
                {
                    _commandedRight = commandedRight;
                }

                // cache state for input link
                suggestedLeft = _suggestedLeft;
                suggestedRight = _suggestedRight;

                // unlock state
            }

            // write input link from cache
            _agent.Update(_suggestedLeftWME, suggestedLeft);
            _agent.Update(_suggestedRightWME, suggestedRight);
        }

        public void Init()
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

        public void Run()
        {
            _kernel.RunAllAgentsForever();
        }
    }
}
