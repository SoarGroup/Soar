//-----------------------------------------------------------------------
//  This file is part of the Microsoft Robotics Studio Code Samples.
// 
//  Copyright (C) Microsoft Corporation.  All rights reserved.
//
//  $File: SimpleDashboardTypes.cs $ $Revision: 1 $
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.ComponentModel;


using Microsoft.Ccr.Core;
using Microsoft.Dss.ServiceModel.Dssp;
using Microsoft.Dss.Core.Attributes;
using dssp = Microsoft.Dss.ServiceModel.Dssp;

namespace Microsoft.Robotics.Services.SimpleDashboard
{

    /// <summary>
    /// DSS Contract for SimpleDashboard
    /// </summary>
    static class Contract
    {
        /// <summary>
        /// The DSS Namespace for SimpleDashboard
        /// </summary>
        public const string Identifier = "http://schemas.microsoft.com/robotics/2006/01/simpledashboard.html";
    }

    /// <summary>
    /// The SimpleDashboard Operations Port
    /// </summary>
    class SimpleDashboardOperations : PortSet<DsspDefaultLookup, DsspDefaultDrop, Get, Replace>
    {
    }

    /// <summary>
    /// DSS Get Definition for SimpleDashboard 
    /// </summary>
    [Description("Gets the current state of the service.")]
    class Get : Get<dssp.GetRequestType, PortSet<SimpleDashboardState, W3C.Soap.Fault>>
    {
        /// <summary>
        /// Default DSS Get Constructor
        /// </summary>
        public Get()
        {
        }

        /// <summary>
        /// DSS GetRequestType Constructor
        /// </summary>
        /// <param name="body"></param>
        public Get(dssp.GetRequestType body)
            : base(body)
        {
        }

    }

    /// <summary>
    /// DSS Replace Definition for SimpleDashboard 
    /// </summary>
    [Description("Changes (or indicates a change to) the entire state of the service.")]
    class Replace : Replace<SimpleDashboardState, PortSet<dssp.DefaultReplaceResponseType, W3C.Soap.Fault>>
    {
        /// <summary>
        /// Default DSS Get Constructor
        /// </summary>
        public Replace()
        {
        }

        /// <summary>
        /// DSS SimpleDashboard StateType Constructor
        /// </summary>
        /// <param name="body"></param>
        public Replace(SimpleDashboardState body)
            : base(body)
        {
        }       
    }

}
