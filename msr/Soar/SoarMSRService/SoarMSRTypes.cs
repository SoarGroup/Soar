using Microsoft.Ccr.Core;
using Microsoft.Dss.Core.Attributes;
using Microsoft.Dss.ServiceModel.Dssp;
using System;
using System.Collections.Generic;
using W3C.Soap;
using soarmsr = Robotics.SoarMSR;


namespace Robotics.SoarMSR
{
    /// <summary>
    /// SoarMSR Contract class
    /// </summary>
    public sealed class Contract
    {
        
        /// <summary>
        /// The Dss Service contract
        /// </summary>
        public const String Identifier = "http://schemas.tempuri.org/2008/03/soarmsrservice.html";

        /// Prevent this class from being instantiated
        private Contract()
        {
        }

    }

    /// <summary>
    /// The SoarMSR State
    /// </summary>
    [DataContract()]
    public class SoarMSRState
    {
        [DataMember]
        public string AgentName = "SoarMSRAgent";

        [DataMember]
        public string Productions = "Apps/Soar/SoarMSRService/agents/simple-bot.soar";

        [DataMember]
        public bool SpawnDebugger = true;

        [DataMember]
        public bool HasRandomSeed = true;

        [DataMember]
        public int RandomSeed = 0;

        [DataMember]
        public double DrivePower = 0.5;

        [DataMember]
        public double ReversePower = 0.35;

        [DataMember]
        public int StopTimeout = 1500;

        [DataMember]
        public int BackUpTimeout = 1000;

        [DataMember]
        public int TurnTimeout = 500;

        [DataMember]
        public int TimeoutVariance = 400;

        [DataMember]
        public int ObstacleAngleRange = 45;

        [DataMember]
        public int MinimumObstacleRange = 750;

    }

    /// <summary>
    /// SoarMSR Main Operations Port
    /// </summary>
    [ServicePort()]
    public class SoarMSROperations : PortSet<DsspDefaultLookup, DsspDefaultDrop, Get, Replace>
    {
    }
    
    /// <summary>
    /// SoarMSR Get Operation
    /// </summary>
    public class Get : Get<GetRequestType, PortSet<SoarMSRState, Fault>>
    {

        /// <summary>
        /// SoarMSR Get Operation
        /// </summary>
        public Get()
        {
        }

        /// <summary>
        /// SoarMSR Get Operation
        /// </summary>
        public Get(Microsoft.Dss.ServiceModel.Dssp.GetRequestType body)
            :
                base(body)
        {
        }

        /// <summary>
        /// SoarMSR Get Operation
        /// </summary>
        public Get(Microsoft.Dss.ServiceModel.Dssp.GetRequestType body, Microsoft.Ccr.Core.PortSet<SoarMSRState, W3C.Soap.Fault> responsePort)
            :
                base(body, responsePort)
        {
        }
    }

    public class Replace : Replace<SoarMSRState, PortSet<DefaultReplaceResponseType, Fault>>
    {
    }
}
