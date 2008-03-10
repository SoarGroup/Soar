using System;
using System.Collections.Generic;
using System.Text;

namespace Robotics.SoarMSR
{
    public class BumperState
    {
        public BumperState() { }
        public BumperState(BumperState bumper)
        {
            this._frontBumperPressed = bumper._frontBumperPressed;
            this._frontBumperWasPressed = bumper._frontBumperWasPressed;
            this._rearBumperPressed = bumper._rearBumperPressed;
            this._rearBumperWasPressed = bumper._rearBumperWasPressed;
        }

        private bool _frontBumperWasPressed = false;
        public bool FrontBumperWasPressed
        {
            get { return _frontBumperWasPressed; }
        }

        private bool _frontBumperPressed = false;
        public bool FrontBumperPressed
        {
            get { return _frontBumperPressed; }
            set
            {
                _frontBumperPressed = value;
                if (_frontBumperPressed)
                    _frontBumperWasPressed = true;
            }
        }

        private bool _rearBumperWasPressed = false;
        public bool RearBumperWasPressed
        {
            get { return _rearBumperWasPressed; }
        }

        private bool _rearBumperPressed = false;
        public bool RearBumperPressed
        {
            get { return _rearBumperPressed; }
            set
            {
                _rearBumperPressed = value;
                if (_rearBumperPressed)
                    _rearBumperWasPressed = true;
            }
        }

        public void Reset()
        {
            _frontBumperWasPressed = false;
            _rearBumperWasPressed = false;
        }
    }

    public class OverrideState
    {
        public OverrideState() { }
        public OverrideState(OverrideState overrideState)
        {
            this.OverrideActive = overrideState.OverrideActive;
            this.OverrideLeft = overrideState.OverrideLeft;
            this.OverrideRight = overrideState.OverrideRight;
        }

        public bool OverrideActive = false;
        public float OverrideLeft = 0;
        public float OverrideRight = 0;

    }
}
