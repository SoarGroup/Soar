//------------------------------------------------------------------------------
// IntroTypes.cs
//
// TT Dec-2006 - Updated for version 1.0 release of MSRS
// TT Jul-2007 - Updated for version 1.5 release of MSRS
//
//------------------------------------------------------------------------------
using Microsoft.Ccr.Core;
using Microsoft.Dss.ServiceModel.Dssp;
using System;
using System.Collections.Generic;
using Microsoft.Dss.Core.Attributes;
using W3C.Soap;
using intro = Robotics.Intro;

//[assembly: ContractNamespace(intro.Contract.Identifier, ClrNamespace="Robotics.Intro")]

namespace Robotics.Intro
{
    // The Contract    
    public sealed class Contract
    {
        public const string Identifier = "http://schemas.tempuri.org/2006/08/intro.html";
        /// Prevent this class from being instantiated
        private Contract()
        {
        }
    }

    // The possible states of the robot whilst wandering
    // NOTE: The DataContract attribute is necessary because this enum
    // is used in the State
    [DataContract]
    public enum MoveStates
    {
        Stop,
        Turn,
        MoveStraight
    }

    // The main State for the Intro service
    [DataContract()]
    public class IntroState
    {
        private MoveStates _moveState = MoveStates.Stop;

        // Robotics Tutorial 2 Step 3 -- Modify the Bumper Handler
        // We don't use this (yet)
        [DataMember]
        public bool MotorOn = true;

        // TT Dec-2006 - Added this flag so that the Lego NXT could be
        // used which only has a front bumper. 
        // Set this to true for the Lego, false for the Pioneer.
        // The better way to do it might be to determine the number
        // of bumpers and see if there is a rear one, but I don't have
        // time to do this right now ...
        // Of course, it will still work with the Pioneer if it is
        // true but the robot will never explore backwards.
        // The Lego robot tends to fall over if it hits the wall too
        // hard, or get wedged if it bumps into the wall at a small
        // angle. Therefore this is not a very reliable way to explore.
        [DataMember]
        public bool ForwardMovesOnly = true;

        // TT Dec-2006 - Moved the various timeouts here to make them
        // visible so that they can be changed externally
        [DataMember]
        public int StopTimeout = 1500;
        [DataMember]
        public int BackUpTimeout = 1000;
        [DataMember]
        public int TurnTimeout = 500;
        [DataMember]
        public int TimeoutVariance = 400;
        [DataMember]
        public int MinimumDriveTimeout = 500;

        // TT Dec-2006 - Moved the power settings here also
        [DataMember]
        public double MinimumPower = 0.2;
        // TT Jul-2007 - Reduced these values
        [DataMember]
        public double MaximumPower = 0.5;
        [DataMember]
        public double BackUpPower = 0.35;

        // Info used to control bumper notification handling
        [DataMember]
        public DateTime nextTimestamp;
        [DataMember]
        public int lastBumperNum = -1;
        [DataMember]
        public int insideBehavior = 0;

    }

    public class IntroOperations : PortSet<DsspDefaultLookup, DsspDefaultDrop, Get, Replace>
    {
    }
    public class Get : Get<GetRequestType, PortSet<IntroState, Fault>>
    {
    }
    public class Replace : Replace<IntroState, PortSet<DefaultReplaceResponseType, Fault>>
    {
    }
}
