//-----------------------------------------------------------------------
//  This file is part of the Microsoft Robotics Studio Code Samples.
// 
//  Copyright (C) Microsoft Corporation.  All rights reserved.
//
//  $File: DriveControl.cs $ $Revision: 1 $
//-----------------------------------------------------------------------

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;
using Microsoft.Ccr.Core;
using game = Microsoft.Robotics.Services.GameController.Proxy;
using drive = Microsoft.Robotics.Services.Drive.Proxy;
using arm = Microsoft.Robotics.Services.ArticulatedArm.Proxy;
using sicklrf = Microsoft.Robotics.Services.Sensors.SickLRF.Proxy;
using cs = Microsoft.Dss.Services.Constructor;
using Microsoft.Dss.ServiceModel.Dssp;
using System.Drawing.Drawing2D;
using System.IO;
using Microsoft.Dss.Core;
using Microsoft.Robotics.Simulation.Physics.Proxy;
using Microsoft.Robotics.PhysicalModel.Proxy;

namespace Microsoft.Robotics.Services.SimpleDashboard
{
    partial class DriveControl : Form
    {
        DriveControlEvents _eventsPort;

        public DriveControl(DriveControlEvents EventsPort)
        {
            _eventsPort = EventsPort;

            InitializeComponent();
            txtPort.ValidatingType = typeof(ushort);
        }

        private void cbJoystick_SelectedIndexChanged(object sender, EventArgs e)
        {
            IEnumerable<game.Controller> list = cbJoystick.Tag as IEnumerable<game.Controller>;

            if (list != null)
            {
                if (cbJoystick.SelectedIndex >= 0)
                {
                    int index = 0;
                    foreach (game.Controller controller in list)
                    {
                        if (index == cbJoystick.SelectedIndex)
                        {
                            OnChangeJoystick change = new OnChangeJoystick(this);
                            change.Joystick = controller;

                            _eventsPort.Post(change);

                            return;
                        }
                        index++;
                    }
                }
            }
        }

        private void DriveControl_Load(object sender, EventArgs e)
        {
            _eventsPort.Post(new OnLoad(this));
        }

        private void DriveControl_FormClosed(object sender, FormClosedEventArgs e)
        {
            _eventsPort.Post(new OnClosed(this));
        }

        public void ReplaceJoystickList(IEnumerable<game.Controller> controllers)
        {
            cbJoystick.BeginUpdate();
            try
            {
                cbJoystick.Items.Clear();
                foreach (game.Controller controller in controllers)
                {
                    int index = cbJoystick.Items.Add(controller.InstanceName);
                    if (controller.Current == true)
                    {
                        cbJoystick.SelectedIndex = index;
                    }
                }
                cbJoystick.Tag = controllers;
            }
            finally
            {
                cbJoystick.EndUpdate();
            }
        }

        public void UpdateJoystickAxes(game.Axes axes)
        {
            int x = axes.X;
            int y = -axes.Y;

            lblX.Text = x.ToString();
            lblY.Text = y.ToString();
            lblZ.Text = axes.Z.ToString();


            DrawJoystick(x, y);

            if (!chkStop.Checked)
            {
                int left;
                int right;

                if (chkDrive.Checked == true)
                {
                    if (y > -100)
                    {
                        left = y + axes.X / 4;
                        right = y - axes.X / 4;
                    }
                    else
                    {
                        left = y - axes.X / 4;
                        right = y + axes.X / 4;
                    }
                }
                else
                {
                    left = right = 0;
                }
                _eventsPort.Post(new OnMove(this, left, right));
            }
        }

        public void UpdateJoystickButtons(game.Buttons buttons)
        {
            if (buttons.Pressed != null && buttons.Pressed.Count > 0)
            {
                string[] buttonString = buttons.Pressed.ConvertAll<string>(
                    delegate(bool button)
                    {
                        return button ? "X" : "O";
                    }
                ).ToArray();

                lblButtons.Text = string.Join(" ", buttonString);

                if (chkStop.Checked && buttons.Pressed.Count > 2)
                {
                    if (buttons.Pressed[2] == true)
                    {
                        chkStop.Checked = false;
                    }
                }
                else if (buttons.Pressed[1] == true && buttons.Pressed.Count > 1)
                {
                    chkStop.Checked = true;
                }

                if (buttons.Pressed[0] != chkDrive.Checked)
                {
                    chkDrive.Checked = buttons.Pressed[0];
                }
            }

        }

        private void DrawJoystick(int x, int y)
        {
            Bitmap bmp = new Bitmap(picJoystick.Width, picJoystick.Height);
            Graphics g = Graphics.FromImage(bmp);

            int width = bmp.Width - 1;
            int height = bmp.Height - 1;

            g.Clear(Color.Transparent);
            g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.HighQuality;
            GraphicsPath path = new GraphicsPath();
            path.AddEllipse(0, 0, width, height);

            PathGradientBrush pathBrush = new PathGradientBrush(path);
            pathBrush.CenterPoint = new PointF(width / 3f, height / 3f);
            pathBrush.CenterColor = Color.White;
            pathBrush.SurroundColors = new Color[] { Color.LightGray };
            
            g.FillPath(pathBrush, path);
            g.DrawPath(Pens.Black, path);

            int partial = y * height / 2200;
            if (partial > 0)
            {
                g.DrawArc(Pens.Black,
                    0,
                    height / 2 - partial,
                    width,
                    2 * partial,
                    180,
                    180);
            }
            else if (partial == 0)
            {
                g.DrawLine(Pens.Black, 0, height / 2, width, height / 2);
            }
            else
            {
                g.DrawArc(Pens.Black,
                    0,
                    height / 2 + partial,
                    width,
                    -2 * partial,
                    0,
                    180);
            }

            partial = x * width / 2200;
            if (partial > 0)
            {
                g.DrawArc(Pens.Black,
                    width / 2 - partial,
                    0,
                    2 * partial,
                    height,
                    270,
                    180);
            }
            else if (partial == 0)
            {
                g.DrawLine(Pens.Black, width / 2, 0, width / 2, height);
            }
            else
            {
                g.DrawArc(Pens.Black,
                    width / 2 + partial,
                    0,
                    -2 * partial,
                    height,
                    90,
                    180);
            }

            picJoystick.Image = bmp;
        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            string machine = txtMachine.Text;
            
            if (machine.Length == 0)
            {
                txtMachine.Focus();
                return;
            }

            object obj = txtPort.ValidateText();

            if (obj == null)
            {
                obj = (ushort)0;
            }

            ushort port = (ushort)obj;

            UriBuilder builder = new UriBuilder(Schemes.DsspTcp, machine, port, ServicePaths.InstanceDirectory);

            _eventsPort.Post(new OnConnect(this, builder.ToString()));
        }

        public void ReplaceDirectoryList(ServiceInfoType[] list)
        {
            listDirectory.BeginUpdate();
            try
            {
                listDirectory.Tag = list;
                listDirectory.Items.Clear();

                if (list.Length > 0)
                {
                    UriBuilder node = new UriBuilder(list[0].Service);
                    node.Path = null;
                    lblNode.Text = node.Host + ":" + node.Port;
                    txtMachine.Text = node.Host;
                    txtPort.Text = node.Port.ToString();

                    linkDirectory.Enabled = true;
                }
                else
                {
                    lblNode.Text = string.Empty;
                    linkDirectory.Enabled = false;
                }

                foreach (ServiceInfoType info in list)
                {
                    if (info.Contract == sicklrf.Contract.Identifier ||
                        info.Contract ==  drive.Contract.Identifier ||
                        info.Contract == arm.Contract.Identifier)
                    {
                        string prefix = string.Empty;
                        foreach(PartnerType partner in info.PartnerList)
                        {
                            if(partner.Name.ToString().EndsWith(":Entity"))
                            {
                                prefix = partner.Service;
                                int last = prefix.LastIndexOf('/');
                                prefix = "(" + prefix.Substring(last + 1) + ")";
                                break;
                            }
                        }
                        Uri serviceUri = new Uri(info.Service);
                        listDirectory.Items.Add(prefix + " " + serviceUri.AbsolutePath);
                    }
                }

                if (ServiceByContract(sicklrf.Contract.Identifier) == null)
                {
                    btnStartLRF.Enabled = true;
                    btnConnectLRF.Enabled = false;
                }
                else
                {
                    btnStartLRF.Enabled = false;
                    btnConnectLRF.Enabled = true;
                }

                btnConnect_ArticulatedArm.Enabled = ServiceByContract(arm.Contract.Identifier) != null;
                btnJointParamsApply.Enabled = btnConnect_ArticulatedArm.Enabled;
            }
            finally
            {
                listDirectory.EndUpdate();
            }
        }

        private void linkDirectory_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
        {
            if (listDirectory.Tag is ServiceInfoType[])
            {
                ServiceInfoType[] list = (ServiceInfoType[])listDirectory.Tag;

                if (list.Length > 0)
                {
                    UriBuilder node;

                    if (list[0].AliasList != null &&
                        list[0].AliasList.Count > 0)
                    {
                        node = new UriBuilder(list[0].AliasList[0]);
                    }
                    else
                    {
                        node = new UriBuilder(list[0].Service);
                    }

                    node.Path = "directory";
                    System.Diagnostics.Process.Start(node.ToString());
                }
            }
        }

        string ServiceByContract(string contract)
        {
            if (listDirectory.Tag is ServiceInfoType[])
            {
                ServiceInfoType[] list = (ServiceInfoType[])listDirectory.Tag;

                foreach (ServiceInfoType service in list)
                {
                    if (service.Contract == contract)
                    {
                        return service.Service;
                    }
                }
            }
            return null;
        }

        private void btnStartLRF_Click(object sender, EventArgs e)
        {
            OnStartService start = new OnStartService(this, sicklrf.Contract.Identifier);
            start.Constructor = ServiceByContract(cs.Contract.Identifier);

            if (start.Constructor != null)
            {
                _eventsPort.Post(start);
            }
        }

        private void btnConnectLRF_Click(object sender, EventArgs e)
        {
            string lrf = ServiceByContract(sicklrf.Contract.Identifier);
            if (lrf != null)
            {
                _eventsPort.Post(new OnConnectSickLRF(this, lrf));
            }
        }

        public void StartedSickLRF()
        {
            btnConnect_Click(this, EventArgs.Empty);
        }

        private void chkDrive_CheckedChanged(object sender, EventArgs e)
        {

        }

        private void chkStop_CheckedChanged(object sender, EventArgs e)
        {
            if (chkStop.Checked)
            {
                _eventsPort.Post(new OnEStop(this));
            }
        }

        DateTime _lastLaser = DateTime.Now;

        public void ReplaceLaserData(sicklrf.State stateType)
        {
            if (stateType.TimeStamp < _lastLaser)
            {
                return;
            }
            _lastLaser = stateType.TimeStamp;
            TimeSpan delay = DateTime.Now - stateType.TimeStamp;
            lblDelay.Text = delay.ToString();

            Bitmap bmp = new Bitmap(stateType.DistanceMeasurements.Length, 100);
            Graphics g = Graphics.FromImage(bmp);
            g.Clear(Color.LightGray);

            int half = bmp.Height / 2;

            for (int x = 0; x < stateType.DistanceMeasurements.Length; x++)
            {
                int range = stateType.DistanceMeasurements[x];
                if (range > 0 && range < 8192)
                {
                    int h = bmp.Height * 500 / stateType.DistanceMeasurements[x];
                    if (h < 0)
                    {
                        h = 0;
                    }
                    Color col = LinearColor(Color.DarkBlue, Color.LightGray, 0, 8192, range);
                    g.DrawLine(new Pen(col), bmp.Width - x, half - h, bmp.Width - x, half + h);
                }
            }
            picLRF.Image = bmp;

            if (btnConnectLRF.Enabled)
            {
                btnConnectLRF.Enabled = false;
            }
            if (!btnDisconnect.Enabled)
            {
                btnDisconnect.Enabled = true;
            }
        }

        private Color LinearColor(Color nearColor, Color farColor, int nearLimit, int farLimit, int value)
        {
            if (value <= nearLimit)
            {
                return nearColor;
            }
            else if (value >= farLimit)
            {
                return farColor;
            }

            int span = farLimit - nearLimit;
            int pos = value - nearLimit;

            int r = (nearColor.R * (span - pos) + farColor.R * pos) / span;
            int g = (nearColor.G * (span - pos) + farColor.G * pos) / span;
            int b = (nearColor.B * (span - pos) + farColor.B * pos) / span;

            return Color.FromArgb(r, g, b);
        }

        private void btnDisconnect_Click(object sender, EventArgs e)
        {
            _eventsPort.Post(new OnDisconnectSickLRF(this));
            btnConnectLRF.Enabled = true;
            btnDisconnect.Enabled = false;
        }

        DateTime _lastMotor = DateTime.Now;

        public void UpdateMotorData(drive.DriveDifferentialTwoWheelState data)
        {
            if (data.TimeStamp > _lastMotor)
            {
                _lastMotor = data.TimeStamp;
                TimeSpan lag = DateTime.Now - data.TimeStamp;
                lblMotor.Text = data.IsEnabled ? "On" : "Off";
                lblLag.Text = string.Format("{0}.{1:D000}",
                    lag.TotalSeconds, lag.Milliseconds);

            }
        }

        private void chkLog_CheckedChanged(object sender, EventArgs e)
        {
            txtLogFile.Enabled = !chkLog.Checked;
            btnBrowse.Enabled = !chkLog.Checked;
            _eventsPort.Post(new OnLogSetting(this, chkLog.Checked, txtLogFile.Text));
        }

        private void btnBrowse_Click(object sender, EventArgs e)
        {
            saveFileDialog.InitialDirectory = LayoutPaths.RootDir + LayoutPaths.StoreDir;
            saveFileDialog.FileName = txtLogFile.Text;
            if (DialogResult.OK == saveFileDialog.ShowDialog(this))
            {
                txtLogFile.Text = saveFileDialog.FileName;
            }
        }

        public void ErrorLogging(Exception ex)
        {
            MessageBox.Show(this, 
                "Exception thrown by logging:\n" + ex.Message, 
                Text, 
                MessageBoxButtons.OK, 
                MessageBoxIcon.Exclamation
            );

            chkLog.Checked = false;
            txtLogFile.Enabled = true;
            btnBrowse.Enabled = true;
        }

        private void picJoystick_MouseLeave(object sender, EventArgs e)
        {
            UpdateJoystickButtons(new game.Buttons());
            UpdateJoystickAxes(new game.Axes());
        }

        private void picJoystick_MouseMove(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Left)
            {
                int x, y;
                x = Math.Min(picJoystick.Width, Math.Max(e.X, 0));
                y = Math.Min(picJoystick.Height, Math.Max(e.Y, 0));

                x = x * 2000 / picJoystick.Width - 1000;
                y = y * 2000 / picJoystick.Height - 1000;

                game.Axes axes = new game.Axes();
                axes.X = x;
                axes.Y = y;

                UpdateJoystickAxes(axes);
            }
        }

        private void picJoystick_MouseUp(object sender, MouseEventArgs e)
        {
            picJoystick_MouseLeave(sender, e);
        }

        public void PerformedRoundTrip(bool roundTrip)
        {
            string title = roundTrip ? "Remote Drive Control" : "Remote Drive Control - Connection Down";

            if (Text != title)
            {
                Text = title;
            }
        }

        private void listDirectory_DoubleClick(object sender, EventArgs e)
        {
            ServiceInfoType[] list = listDirectory.Tag as ServiceInfoType[];

            if (list != null &&
                listDirectory.SelectedIndex >= 0 &&
                listDirectory.SelectedIndex < list.Length)
            {
                // get the service path from the selected item
                string[] tokens = ((string)listDirectory.SelectedItem).Split(new char[] { ' ' });
                if (tokens.Length == 0)
                    return;

                ServiceInfoType info = FindServiceInfoFromServicePath(tokens[tokens.Length - 1]);
                if (info == null)
                    return;
                
                if (info.Contract == drive.Contract.Identifier)
                {
                    _eventsPort.Post(new OnConnectMotor(this, info.Service));
                }
                else if (info.Contract == sicklrf.Contract.Identifier)
                {
                    _eventsPort.Post(new OnConnectSickLRF(this, info.Service));
                }
                else if (info.Contract == arm.Contract.Identifier)
                {
                    _eventsPort.Post(new OnConnectArticulatedArm(this, info.Service));
                }
            }
        }

        ServiceInfoType FindServiceInfoFromServicePath(string path)
        {
            ServiceInfoType[] list = (ServiceInfoType[])listDirectory.Tag;

            UriBuilder builder = new UriBuilder(list[0].Service);
            builder.Path = path;

            string uri = builder.ToString();

            return FindServiceInfoFromServiceUri(uri);
        }

        ServiceInfoType FindServiceInfoFromServiceUri(string uri)
        {
            ServiceInfoType[] list = (ServiceInfoType[])listDirectory.Tag;

            foreach (ServiceInfoType si in list)
            {
                if (si.Service == uri)
                    return si;
            }
            return null;
        }

        private void saveFileDialog_FileOk(object sender, CancelEventArgs e)
        {
            string path = Path.GetFullPath(saveFileDialog.FileName);
            if (!path.StartsWith(saveFileDialog.InitialDirectory))
            {
                MessageBox.Show("Log file must be in a subdirectory of the store", Text, MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                e.Cancel = true;
            }
        }

        private void btnJointParamsApply_Click(object sender, EventArgs e)
        {
            try
            {
                short angle = short.Parse(textBoxJointAngle.Text);
                OnApplyJointParameters j = new OnApplyJointParameters(this,
                    angle,
                    lblActiveJointValue.Text);
                _eventsPort.Post(j);
            }
            catch
            {
                MessageBox.Show("Invalid angle", Text, MessageBoxButtons.OK, MessageBoxIcon.Exclamation);                
            }
        }

        private void btnConnect_ArticulatedArm_Click(object sender, EventArgs e)
        {
            string armService = ServiceByContract(arm.Contract.Identifier);
            if (armService != null)
            {
                _eventsPort.Post(new OnConnectArticulatedArm(this, armService));
            }
        }

        public void ReplaceArticulatedArmJointList(arm.ArticulatedArmState state)
        {
            btnConnect_ArticulatedArm.Enabled = false;
            btnJointParamsApply.Enabled = true;
            listArticulatedArmJoints.BeginUpdate();
            try
            {
                listArticulatedArmJoints.Tag = state;
                listArticulatedArmJoints.Items.Clear();

                if (state.Joints.Count > 0)
                {
                    UriBuilder node = new UriBuilder(state.Joints[0].State.Name);
                    node.Path = null;
                    lblActiveJointValue.Text = state.Joints[0].State.Name;

                    foreach (Joint j in state.Joints)
                    {
                        listArticulatedArmJoints.Items.Add(j.State.Name);
                    }
                }
                else
                {
                    lblActiveJointValue.Text = string.Empty;
                }
            }
            finally
            {
                listArticulatedArmJoints.EndUpdate();
            }
        }

        private void listArticulatedArmJointList_DoubleClick(object sender, EventArgs e)
        {
            arm.ArticulatedArmState state = listArticulatedArmJoints.Tag as arm.ArticulatedArmState;

            if (state != null &&
                listArticulatedArmJoints.SelectedIndex >= 0 &&
                listArticulatedArmJoints.SelectedIndex < state.Joints.Count)
            {
                lblActiveJointValue.Text = (string)listArticulatedArmJoints.SelectedItem;
            }
        }
    }

    class WebCamView : Form
    {
        PictureBox picture;

        public WebCamView()
        {
            SuspendLayout();
            Text = "WebCam";
            FormBorderStyle = FormBorderStyle.FixedToolWindow;
            ControlBox = false;
            ClientSize = new Size(320, 240);
            ShowIcon = false;
            ShowInTaskbar = false;

            picture = new PictureBox();
            picture.Name = "Picture";
            picture.Dock = DockStyle.Fill;

            Controls.Add(picture);

            ResumeLayout();
        }

        public Image Image
        {
            get { return picture.Image; }
            set
            {
                picture.Image = value;
                ClientSize = value.Size;
            }
        }

        private DateTime _timestamp;

        public DateTime TimeStamp
        {
            get { return _timestamp; }
            set
            {
                _timestamp = value;
                Text = "WebCam - " + _timestamp.ToString("hh:mm:ss.ff");
            }
        }
    }

    class DriveControlEvents 
        : PortSet<
            OnLoad,
            OnClosed,
            OnChangeJoystick,
            OnConnect,
            OnConnectMotor,
            OnConnectSickLRF,
            OnConnectArticulatedArm,
            OnConnectWebCam,
            OnStartService,
            OnMove,
            OnEStop,
            OnApplyJointParameters,
            OnDisconnectSickLRF,
            OnDisconnectWebCam,
            OnLogSetting,
            OnQueryFrame
        >
    {
    }
    
    class DriveControlEvent
    {
        private DriveControl _driveControl;

        public DriveControl DriveControl
        {
            get { return _driveControl; }
            set { _driveControl = value; }
        }

        public DriveControlEvent(DriveControl driveControl)
        {
            _driveControl = driveControl;
        }
    }

    class OnLoad : DriveControlEvent
    {
        public OnLoad(DriveControl form)
            : base(form)
        {
        }
    }

    class OnConnect : DriveControlEvent
    {
        string _service;

        public string Service
        {
            get { return _service; }
            set { _service = value; }
        }

        public OnConnect(DriveControl form, string service)
            : base(form)
        {
            _service = service;
        }
    }

    class OnConnectMotor : OnConnect
    {
        public OnConnectMotor(DriveControl form, string service)
            : base(form, service)
        {
        }
    }

    class OnConnectSickLRF : OnConnect
    {
        public OnConnectSickLRF(DriveControl form, string service)
            : base(form, service)
        {
        }
    }

    class OnConnectArticulatedArm : OnConnect
    {
        public OnConnectArticulatedArm(DriveControl form, string service)
            : base(form, service)
        {
        }
    }

    class OnConnectSimulatedArm : OnConnect
    {
        public OnConnectSimulatedArm(DriveControl form, string service)
            : base(form, service)
        {
        }
    }

    class OnStartService : DriveControlEvent
    {
        string _contract;
        string _constructor;

        public string Contract
        {
            get { return _contract; }
            set { _contract = value; }
        }

        public string Constructor
        {
            get { return _constructor; }
            set { _constructor = value; }
        }


        public OnStartService(DriveControl form, string contract)
            : base(form)
        {
            _contract = contract;
        }
    }

    class OnClosed : DriveControlEvent
    {
        public OnClosed(DriveControl form)
            : base(form)
        {
        }
    }

    class OnChangeJoystick : DriveControlEvent
    {
        game.Controller _joystick;

        public game.Controller Joystick
        {
            get { return _joystick; }
            set { _joystick = value; }
        }

        public OnChangeJoystick(DriveControl form)
            : base(form)
        {
        }
    }

    class OnMove : DriveControlEvent
    {
        int _left;

        public int Left
        {
            get { return _left; }
            set { _left = value; }
        }

        int _right;

        public int Right
        {
            get { return _right; }
            set { _right = value; }
        }

        public OnMove(DriveControl form, int left, int right)
            : base(form)
        {
            _left = left * 750 / 1250;
            _right = right * 750 / 1250;
        }
    }

    class OnEStop : DriveControlEvent
    {
        public OnEStop(DriveControl form)
            : base(form)
        {
        }
    }

    class OnSynchronizeArms : DriveControlEvent
    {
        public OnSynchronizeArms(DriveControl form)
            : base(form)
        {
        }
    }

    class OnApplyJointParameters : DriveControlEvent
    {
        int _angle;

        public int Angle
        {
            get { return _angle; }
            set { _angle = value; }
        }

        string _jointName;

        public string JointName
        {
            get { return _jointName; }
            set { _jointName = value; }
        }

        public OnApplyJointParameters(DriveControl form, int angle, string name)
            : base(form)
        {
            _angle = angle;
            _jointName = name;
        }
    }

    class OnDisconnectSickLRF : DriveControlEvent
    {
        public OnDisconnectSickLRF(DriveControl form)
            : base(form)
        {
        }
    }

    class OnLogSetting : DriveControlEvent
    {
        bool _log;
        string _file;

        public bool Log
        {
            get { return _log; }
            set { _log = value; }
        }

        public string File
        {
            get { return _file; }
            set { _file = value; }
        }

        public OnLogSetting(DriveControl form, bool log, string file)
            : base(form)
        {
            _log = log;
            _file = file;
        }
    }

    class OnConnectWebCam : OnConnect
    {
        public OnConnectWebCam(DriveControl form, string service)
            : base(form, service)
        {
        }
    }

    class OnDisconnectWebCam : DriveControlEvent
    {
        public OnDisconnectWebCam(DriveControl form)
            : base(form)
        {
        }
    }

    class OnQueryFrame : DriveControlEvent
    {
        public OnQueryFrame(DriveControl form)
            : base(form)
        {
        }
    }
}