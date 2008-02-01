//-----------------------------------------------------------------------
//  This file is part of the Microsoft Robotics Studio Code Samples.
// 
//  Copyright (C) Microsoft Corporation.  All rights reserved.
//
//  $File: SimpleDashboardState.cs $ $Revision: 1 $
//-----------------------------------------------------------------------

using Microsoft.Dss.Core.Attributes;
using System;
using System.Collections.Generic;
using System.ComponentModel;

using drive = Microsoft.Robotics.Services.Drive.Proxy;
using sicklrf = Microsoft.Robotics.Services.Sensors.SickLRF.Proxy;

namespace Microsoft.Robotics.Services.SimpleDashboard
{
    /// <summary>
    /// SimpleDashboard StateType
    /// </summary>
    [DataContract]
    public class SimpleDashboardState
    {
        [DataMember]
        [Description ("Specifies whether to log messages.")]
        public bool Log;
        [DataMember]
        [Description("Specifies the filename to log the data to.")]
        public string LogFile;
    }
}
