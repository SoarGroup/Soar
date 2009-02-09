class KRobot extends KVehicle config(USARBot) abstract;



////////////////////////////////////////////////

//	PHYSICAL ROBOT

////////////////////////////////////////////////



// Part Definiens

struct JointPart {

    // Part

    var() name                  PartName;

    var() class<KActor>         PartClass;

    var() vector                DrawScale3D;



    // Joint

    var() class<KConstraint>    JointClass;

    var() bool                  bSteeringLocked;

    var() bool                  bSuspensionLocked;

    var() float                 BrakeTorque;

    var() name                  Parent;

    var() vector                ParentPos;

    //var() rotator             ParentRot;

    var() vector                ParentAxis;

    var() vector                ParentAxis2;

    var() vector                SelfPos;

    var() vector                SelfAxis;

    var() vector                SelfAxis2;

};

var config array<JointPart> JointParts;



// Mission Package Data Structure

struct MisPkg

{

    var() name              PkgName;

    var() vector            Location;

    var() class<MisPkgInfo> PkgClass;

};

var config array<MisPkg> MisPkgs;



var array<MisPkgInfo> allMisPkgs; // Holds information about all the mission packages

var array<MisPkgLinkInfo> allMisPkgsLinks; // Holds information about all the mission packages' links



// Joint Control

struct JointControl {

	var byte	state; // Control state:

                       // 0: no commands;

                       // 1: new command;

                       // 2: finished;

	var float	steer; // Steer angle

	var byte	order; // Specify the followed value.

	var float	value; // Control value:

                       // order =  0 , value=absolute angle;

                       // order =  1 , value=absolute speed;

                       // order =  2 , value=absolute torque;

                       // order = 10 , value=relative angle;

                       // order = 11 , value=relative speed;

                       // order = 12 , value=relative torque;



	var byte	lastCommandId; // There are new commands if lastCommandId != RS_JointsCommandId



	//Variables used only by order = 0 control:

	var float	angle; // the desired spining angle. used for order=0 control.

	var int		startAng; // the start angle. used for compare spined angle.

};

var array<JointControl>		JointsControl;



struct RobotSkin

{

    var string Name;

    var texture Skin;

    var string Comment;

};

var config array<RobotSkin> RobotSkins;



//=======================

// Networking RobotState

//=======================

//

// Replication variables used to update Robot State on clients



var KRBVec		RS_ChassisPosition;

var Quat		RS_ChassisQuaternion;

var KRBVec		RS_ChassisLinVel;

var KRBVec		RS_ChassisAngVel;



// dynamic array canot be replicated. So we must use static array here.

// Here, to make the client and server have the same state, we directly

// replicate the RigidBodyState. In the KCar class, it replicates the

// RigidBodyState related to the chassis. I don't know why they use relative

// state. I had tried to use relative state. But it involves vector and

// quaternion calculation. We need to calculate the relative pos and relative

// quat. For relative pos, it's just subtract part's (wheel's) pos from chassis

// pos. For relative quat, we need to use the third axis, that's

// Joints[i].KPriAxis1 Cross Joints[i].KSecAxis1), to calculate the relative

// quat. However, I cannot figure out the correct calculating method for general

// case, such as connect the y axis of the part to the x axis of the chassis

// through a joint. ANYWAY, the simplest, most stable and straightforward method

// is replicating all the (absolute) RigidBodyState. If the server is correct,

// then the client should also be correct. The only weakness of this method is we

// need to replicate more data.



var KRBVec		RS_PartsPos[32];

var Quat		RS_PartsQuat[32];

var vector		RS_PartsLinVel[32];

var vector		RS_PartsAngVel[32];



var float		RS_JointsSteer[32];

var byte		RS_JointsOrder[32];

var float		RS_JointsValue[32];

var byte		RS_JointsCommandId[32];



// It denotes new replicated robot state. VehicleStateReceived will

// load on client side these new params and set bNewRobotState to true.

// Should be a boolean but it's a byte that increments at each replication,

// otherwise replication mechanism would not work becasue bool var would be

// always true on server (read the code for further details).

// (using struct wuold be a possible solution to replication, but it's not necessary)

var byte        RS_RobotUpdateId;



//Used to change the robot skin

var string      RS_skinName;

var byte        RS_skinUpdateId;



// -- End Replication variables



// if CacheRobotUpdateId on client is different from RS_RobotUpdateId received from server

// then ther's a new robot state to be loaded. At this point CacheRobotUpdateId is set

// equal to RS_RobotUpdateId and bNewRobotState is set to true.

var byte		CacheRobotUpdateId;



var byte        CacheSkinUpdateId;



// New RB params were loaded from replication and we must notify

// Karma to update RB state on client side

var bool		bNewRobotState;



var bool		bNewCommand;        //only used on server in ProcessCarInput()

var float		NextNetUpdateTime;	// Next time we should force an update of vehicles state.

var float		MaxNetUpdateInterval;



// Parts that build a robot (every part has a part a parent and a joint)

var array<KActor>			Parts;

var array<Actor>			Parents;

var array<KConstraint>		Joints;



//========================

// KCarWheelJoint settings

//========================



// Steering

var float       SteerPropGap;

var float       SteerTorque;

var float       SteerSpeed;



// KCarWheelSuspension setting

var float       SuspStiffness;

var float       SuspDamping;

var float       SuspHighLimit;

var float       SuspLowLimit;

var float       SuspRef;



// The Max torque for all the joints

var config float		MaxTorque;

var config float        SafeForce;

var config float		ProtectTime;



// KDHinge Joint settings

var config float        HingePropGap;



// KCarWheelJoint defalut working torque

var config float		MotorTorque;

var config float		MotorSpeed;

var float               uuMotorSpeed;

var config float        maxSpinSpeed;



//========================

// KTire settings

//========================



var float       TireRollFriction;

var float       TireLateralFriction;

var float       TireRollSlip;

var float       TireLateralSlip;

var float       TireMinSlip;

var float       TireSlipRate;

var float       TireSoftness;

var float       TireAdhesion;

var float       TireRestitution;

//========================



// Robot parameters

var config float		Payload;

var config float		Weight;

var KRigidBodyState		ChassisState;

var KRigidBodyState		TeleportLocation; //!< Used to move the robot with MoveRobot() function.

var config float		ChassisMass;

var config InterpCurve	TorqueCurve; // Engine RPM in, Torque out.

var float		HitSoundThreshold;

var float WheelRadius;

var vector Dimensions;



// Flip

var config float	FlipTorque;

var config float	FlipTime;

var float			FlipTimeLeft;



//===============================

// Items mounted on the robot

//===============================



// Item mount structure

struct sItem {

	var class<Actor>	ItemClass;

	var name			Parent;

	var string			ItemName;

	var vector			Position;

	var vector			Direction;

	var rotator         uuDirection;

};



struct lightItem extends sItem

{

	var bool LightGlow;

};



// Sensors

var config array<sItem> Sensors;

var config float msgTimer; // Timer used for sending out sensor data

var array<Sensor> SensorList;

var array<byte>			ProcessedSensors;

// Effecters

var config array<sItem> Effecters;

var array<Effecter>     EffecterList;



// Battery

var config int batteryLife;

var int startTime;

var int myLife;



// Headlight

var config array<lightItem>	 HeadLights;

var array<USARHeadlight> HeadlightList;

var bool 		 bHeadlightOn;

var byte		 HeadlightItensity;



// Camera

var config array<sItem>	Cameras;

var array<RobotCamera>  CamList;

var RobotCamera		myCamera;

var float		PanSpeed, TiltSpeed;



// OmniCam

var config sItem		CamTexActors;

var USAREmitter			myEmitterNorth;

var USARCameraTextureClientN	myCamTexClientNorth;

var USAREmitter			myEmitterEast;

var USARCameraTextureClientE	myCamTexClientEast;

var USAREmitter			myEmitterWest;

var USARCameraTextureClientW	myCamTexClientWest;

var USAREmitter			myEmitterSouth;

var USARCameraTextureClientS	myCamTexClientSouth;

var USAREmitter			myEmitterCenter;

var USARCameraTextureClientC	myCamTexClientCenter;

var rotator 			EmitterDir;



// DrawHud Variables (to determine how many screens, which camera, etc...)

var int viewportMode;  // 0 is single view, 1 is dualviewhorizontal, 2 is dualviewvertical, and 3 is quad view

var int CamsToView[4]; // Store the camera indexes of the camera view to be displayed on the viewport

var bool bDrawHud;



//=========================

//	Others

//=========================



// Variables used for programming

var string RobotName;

var config bool bDebug;

var int			CurrentPart;

var config bool bDisplayTeamBeacon;

var float       lastTime, previousTime;

var vector      lastLocation, previousLocation;

var bool        bRobotBuilt;

var name        fpsLogger;

var config string ConverterClass;

var USARConverter converter;

var USARUtils utils;

var USARDraw Draw;

var config bool   bMountByUU;

var config float  logging_period;

var config bool   bBarCode;

var float         old_time;

var FileLog       PosLog;

var FileLog       CollLog;

var KSimParams oldKSP, newKSP;

var name          tmpName; // Used to convert string to name



//var bool bstats; //used for bump/touch events.

//var int bump_touch_cnt;



// MultiView Addon

var int			ViewNum;	//Viewport number (assigned by Register()) used by DrawHUD to render the scene

var MultiView   ViewManager; //the actual ViewManager actor (a bot)



//#############################################################################

//                              KROBOT CODE

//#############################################################################



replication

{

	// We replicate the robot state.



	reliable if(Role == ROLE_Authority)

		RS_ChassisPosition, RS_ChassisQuaternion, RS_ChassisLinVel, RS_ChassisAngVel,

		RS_PartsPos, RS_PartsQuat, RS_PartsLinVel, RS_PartsAngVel,RS_RobotUpdateId;



	reliable if(Role == ROLE_Authority)

        RS_JointsSteer, RS_JointsOrder, RS_JointsValue, RS_JointsCommandId;



	reliable if(Role == ROLE_Authority)

		FlipTimeLeft, bHeadlightOn, RobotName;



	reliable if(Role == ROLE_Authority)

        RS_skinName, RS_skinUpdateId, ViewManager, ViewNum;

}



simulated event PreBeginPlay()

{

	local class<USARConverter> cClass;

	local int i,j;



	Super.PreBeginPlay();

	utils = New class'USARUtils';

        draw = New class'USARDraw';

	bNoTeamBeacon=!bDisplayTeamBeacon;

	cClass = class<USARConverter>(DynamicLoadObject(ConverterClass, class'Class'));

	converter = new cClass;

	ConvertParam(converter);

	if (MotorTorque>MaxTorque)

		MotorTorque = MaxTorque;



        // Set up the mission package information (allocate space)

        for(i=0; i<MisPkgs.length; i++)

        {

            allMisPkgs[i] = New MisPkgs[i].PkgClass;



            for(j=0; j<allMisPkgs[i].Links.length; j++)

            {

                allMisPkgsLinks[i*allMisPkgs[i].Links.length+j] = New allMisPkgs[i].Links[j].LinkClass;

            }

        }



	//Adjust Karma params

   	KGetSimParams(oldKSP);

	newKSP = oldKSP;



	newKSP.Epsilon = 0.03;

	newKSP.GammaPerSec = 0.5;

//	newKSP.ContactSoftness = 0.001;

    newKSP.MaxPenetration = 1;

	newKSP.PenetrationScale = 5;



	KSetSimParams(newKSP);

}



simulated function ConvertParam(USARConverter converter)

{

    local int i;



    if (converter==None) {

        uuMotorSpeed = MotorSpeed;

    } else {

        uuMotorSpeed = converter.SpinSpeedToUU(MotorSpeed);

    }

    if (!bMountByUU && converter!=None) {

        for (i=0;i<JointParts.length;i++) {

		    JointParts[i].ParentPos = converter.LengthVectorToUU(JointParts[i].ParentPos);

		    JointParts[i].SelfPos = converter.LengthVectorToUU(JointParts[i].SelfPos);

        }

	for (i=0;i<Headlights.length;i++) {

		    HeadLights[i].Position = converter.LengthVectorToUU(HeadLights[i].Position);

		    HeadLights[i].uuDirection = converter.RotatorToUU(HeadLights[i].Direction);

		}

       	for (i=0;i<Cameras.length;i++) {

		    Cameras[i].Position = converter.LengthVectorToUU(Cameras[i].Position);

		    Cameras[i].uuDirection = converter.RotatorToUU(Cameras[i].Direction);

		}

       	for (i=0;i<Sensors.length;i++) {

		    Sensors[i].Position = converter.LengthVectorToUU(Sensors[i].Position);

		    Sensors[i].uuDirection = converter.RotatorToUU(Sensors[i].Direction);

		}

       	for (i=0;i<Effecters.length;i++) {

		    Effecters[i].Position = converter.LengthVectorToUU(Effecters[i].Position);

		    Effecters[i].uuDirection = converter.RotatorToUU(Effecters[i].Direction);

		}

	if (CamTexActors.ItemName!="") {

		    CamTexActors.Position = converter.LengthVectorToUU(CamTexActors.Position);

		    CamTexActors.uuDirection = converter.RotatorToUU(CamTexActors.Direction);

		}

    } else {

	for (i=0;i<Headlights.length;i++) {

		    HeadLights[i].uuDirection.Roll  = int(HeadLights[i].Direction.X);

		    HeadLights[i].uuDirection.Pitch = int(HeadLights[i].Direction.Y);

		    HeadLights[i].uuDirection.Yaw   = int(HeadLights[i].Direction.Z);

		}

       	for (i=0;i<Cameras.length;i++) {

		    Cameras[i].uuDirection.Roll  = int(Cameras[i].Direction.X);

		    Cameras[i].uuDirection.Pitch = int(Cameras[i].Direction.Y);

		    Cameras[i].uuDirection.Yaw   = int(Cameras[i].Direction.Z);

		}

       	for (i=0;i<Sensors.length;i++) {

		    Sensors[i].uuDirection.Roll  = int(Sensors[i].Direction.X);

		    Sensors[i].uuDirection.Pitch = int(Sensors[i].Direction.Y);

		    Sensors[i].uuDirection.Yaw   = int(Sensors[i].Direction.Z);

		}

       	for (i=0;i<Effecters.length;i++) {

		    Effecters[i].uuDirection.Roll  = int(Effecters[i].Direction.X);

		    Effecters[i].uuDirection.Pitch = int(Effecters[i].Direction.Y);

		    Effecters[i].uuDirection.Yaw   = int(Effecters[i].Direction.Z);

		}

        if (CamTexActors.ItemName!="") {

		    CamTexActors.uuDirection.Roll  = int(CamTexActors.Direction.X);

		    CamTexActors.uuDirection.Pitch = int(CamTexActors.Direction.Y);

		    CamTexActors.uuDirection.Yaw   = int(CamTexActors.Direction.Z);

		}

    }

}



// Register to the game

function Register()

{

	local USARDeathMatch UsarGame;

	local int Index;



 	UsarGame = USARDeathMatch(Level.Game);

	Index = UsarGame.Vehicles.length;

	UsarGame.Vehicles.Insert(Index,1);

	UsarGame.Vehicles[Index]=self;



    //bstats = UsarGame.bstats;



    old_time = startTime;



    if(myCamera != none)

    {

        foreach AllActors(class'MultiView', ViewManager) break;

    }

    if(ViewManager != None)

    	ViewNum = ViewManager.RegisterView();

    else

        ViewNum = -1;

}



// Unregister from the game

function Unregister()

{

	local USARDeathMatch UsarGame;

	local int i;



	UsarGame = USARDeathMatch(Level.Game);

	for (i=0;i<UsarGame.Vehicles.length;i++) {

		if (UsarGame.Vehicles[i]==self) {

			UsarGame.Vehicles.Remove(i,1);

			break;

		}

	}

    if(ViewManager != None)

    {

    	ViewManager.DeleteView(ViewNum);

    }

}



// When new information is received, see if its new. If so, pass bits off the the wheels.

// Each part will then update its rigid body position via the KUpdateState event.

// JTODO: This is where clever unpacking would happen.

simulated event VehicleStateReceived()

{

	local KTire Tire;

	local KDPart Part;

	local int i;

	local bool UpdateParts;



	/*

	KGetRigidBodyState(ChassisState);

	log("State<<"@ChassisState.Position.X@ChassisState.Position.Y@ChassisState.Position.Z);

	log("       "@ChassisState.Quaternion.X@ChassisState.Quaternion.Y@ChassisState.Quaternion.Z@ChassisState.Quaternion.W);

	log("       "@ChassisState.LinVel.X@ChassisState.LinVel.Y@ChassisState.LinVel.Z);

	log("       "@ChassisState.AngVel.X@ChassisState.AngVel.Y@ChassisState.AngVel.Z@">>");



	log("Receive<<"@RS_ChassisPosition.X@RS_ChassisPosition.Y@RS_ChassisPosition.Z);

	log("         "@RS_ChassisQuaternion.X@RS_ChassisQuaternion.Y@RS_ChassisQuaternion.Z);

	log("         "@RS_ChassisLinVel.X@RS_ChassisLinVel.Y@RS_ChassisLinVel.Z);

	log("         "@RS_ChassisAngVel.X@RS_ChassisAngVel.Y@RS_ChassisAngVel.Z@">>");

	*/



	// Don't do anything if vehicle isn't started up.

	if(!bRobotBuilt) {

        if(Parts.length == 0)

            return;

        for (i=0; i<Parts.length; i++)

            if (Parts[i] == None) return;

        bRobotBuilt = true;

	}



    //Serves a skin update request

    if(cacheSkinUpdateId != RS_skinUpdateId)

    {

        SetSkin(RS_skinName);

        cacheSkinUpdateId = RS_skinUpdateId;

    }



    ///////////////////////////

    // Update root chassis info

    ///////////////////////////

    if (CacheRobotUpdateId != RS_RobotUpdateId)

    {

        ChassisState.Position = RS_ChassisPosition;

    	ChassisState.Quaternion = RS_ChassisQuaternion;

    	ChassisState.LinVel = RS_ChassisLinVel;

    	ChassisState.AngVel = RS_ChassisAngVel;

    	UpdateParts = true;

        CacheRobotUpdateId = RS_RobotUpdateId;

        bNewRobotState = true;

    }



	// Figure out new state of parts

	for (i=0;i<Parts.length;i++) {



	    ///////////////////////////

	    // Update Parts

	    ///////////////////////////

	    if (UpdateParts)

	    {

        	if (Parts[i].IsA('KTire'))

            {

        		Tire = KTire(Parts[i]);

        		Tire.KGetRigidBodyState(Tire.ReceiveState);



        		Tire.ReceiveState.Position = RS_PartsPos[i];

        		Tire.ReceiveState.Quaternion = RS_PartsQuat[i];

        		Tire.ReceiveState.LinVel = KRBVecFromVector(RS_PartsLinVel[i]);

        		Tire.ReceiveState.AngVel = KRBVecFromVector(RS_PartsAngVel[i]);

        		Tire.bReceiveStateNew = true;

        	}

        	else if (Parts[i].IsA('KDPart'))

            {

        		Part = KDPart(Parts[i]);

        		Part.KGetRigidBodyState(Part.ReceiveState);



        		Part.ReceiveState.Position = RS_PartsPos[i];

        		Part.ReceiveState.Quaternion = RS_PartsQuat[i];

        		Part.ReceiveState.LinVel = KRBVecFromVector(RS_PartsLinVel[i]);

        		Part.ReceiveState.AngVel = KRBVecFromVector(RS_PartsAngVel[i]);

        		Part.bReceiveStateNew = true;

        	}

		}



	    ///////////////////////////

	    // Update Commands

	    ///////////////////////////

		if (JointsControl[i].lastCommandId!=RS_JointsCommandId[i]) {

			JointsControl[i].steer = RS_JointsSteer[i];

			JointsControl[i].order = RS_JointsOrder[i];

			JointsControl[i].value = RS_JointsValue[i];

			JointsControl[i].state = 1;

			JointsControl[i].lastCommandId = RS_JointsCommandId[i];

		}

	}

}



// This only update the chassis. The parts update themselves.

simulated event bool KUpdateState(out KRigidBodyState newState)

{

	// This should never get called on the server - but just in case!

	if(Role == ROLE_Authority)

	{

        if(!bNewRobotState)

    		return false;

    	else

    	{

    	   // bNewRobotState will be true on server only if we're trying

    	   // to teleport the robot from one location to another.

    	   newState = TeleportLocation;

    	   bNewRobotState = false;

    	   return true;

    	}

    }

	// Apply received data as new position of car chassis.

	newState = ChassisState;

	bNewRobotState = false;

	return true;

}



// Pack current state of whole car into the state struct, to be sent to the client.

// Should only get called on the server.

function PackState()

{

	local KRigidBodyState RBState;

	local int i;

	local bool UpdateParts;



	if(!KIsAwake() && !bNewCommand)

    	return; // Never send updates if physics is at rest



    if(Level.TimeSeconds > NextNetUpdateTime)

    {

        ///////////////////////////

        // Pack Chassis state

        ///////////////////////////

    	KGetRigidBodyState(RBState);

    	RS_ChassisPosition = RBState.Position;

    	RS_ChassisQuaternion = RBState.Quaternion;

    	RS_ChassisLinVel = RBState.LinVel;

    	RS_ChassisAngVel = RBState.AngVel;

        UpdateParts = true;

        RS_RobotUpdateId += 1;

        NextNetUpdateTime = Level.TimeSeconds + MaxNetUpdateInterval;

    }



	// Get each part's state.

	for (i=0;i<Parts.length;i++) {

        ///////////////////////////

        // Pack Parts State

        ///////////////////////////

        if(UpdateParts)

        {

    		Parts[i].KGetRigidBodyState(RBState);

     		RS_PartsPos[i] = RBState.Position;

       		RS_PartsQuat[i] = RBState.Quaternion;

    		RS_PartsLinVel[i] = KRBVecToVector(RBState.LinVel);

    		RS_PartsAngVel[i] = KRBVecToVector(RBState.AngVel);

        }

        ///////////////////////////

        // Pack Commands State

        ///////////////////////////



        if (JointsControl[i].state==1)

        {

            RS_JointsCommandId[i]+=1;

            RS_JointsSteer[i] = JointsControl[i].steer;

            RS_JointsOrder[i] = JointsControl[i].order;

            RS_JointsValue[i] = JointsControl[i].value;

        }

	}

}



function RobotCamera GetCamera(String name)

{

	local int i;



	for (i=0;i<CamList.length;i++)

		if (CamList[i].ItemName==name)

			return CamList[i];



	return myCamera;

}



simulated function Actor FindPart(name PartName)

{

	local int i;



	if (PartName == '' || PartName == 'None') return self;



	for (i=0;i<JointParts.length;i++) {

		if (JointParts[i].PartName == PartName) {

			return Parts[i];

		}

	}

	return None;

}



simulated function int FindLinkParent(MisPkgInfo Package, int ParentLinkNumber)

{

    local int i;



    for(i=0; i<Package.Links.length; i++)

    {

        if(Package.Links[i].LinkNumber == ParentLinkNumber)

        {

            return i;

        }

    }

    return -1;

}



simulated function MisPkgLinkInfo getMisPkgLinkInfo(int PartNumber, string MisPkgName)

{

    local int i,j;



    if(MisPkgName == "")

    {

        for(i=0; i<allMisPkgs.Length; i++)

        {

            for(j=0; j<allMisPkgs[i].Links.Length; j++)

            {

                if(allMisPkgs[i].Links[j].LinkNumber == PartNumber)

                {

                    return allMisPkgsLinks[i*allMisPkgs[i].Links.length+j];

                }

            }

        }

    }

    else

    {

        for(i=0; i<MisPkgs.Length; i++)

        {

            if(string(MisPkgs[i].PkgName) == MisPkgName)

            {

                for(j=0; j<allMisPkgs[i].Links.Length; j++)

                {

                    if(allMisPkgs[i].Links[j].LinkNumber == PartNumber)

                    {

                       return allMisPkgsLinks[i*allMisPkgs[i].Links.length+j];

                    }

                }



                break;

            }

        }

    }



    return None;

}



simulated function int FindMisPkgLinkIndex(Name LinkName)

{

    local int i;



    for(i=0; i<Joints.Length; i++)

    {

        if(JointParts[i].PartName == LinkName)

        {

            return i;

        }

    }

    return -1;

}



simulated function name getMisPkgPartName_str(string PkgName, int LinkNumber)

{

    tmpName = '';



    if(LinkNumber >= 0)

    {

        SetPropertyText("tmpName", PkgName $ "_Link" $ string(LinkNumber));

    }



    return tmpName;

}



simulated function name getMisPkgPartName(Name PkgName, int LinkNumber)

{

    tmpName = '';



    if(LinkNumber >= 0)

    {

        SetPropertyText("tmpName", string(PkgName) $ "_Link" $ string(LinkNumber));

    }



    return tmpName;

}



simulated function int getLinkNumber(string strLink)

{

    return int(Right(strLink, Len(strLink) - (InStr(strLink, "Link") + 4)));

}



simulated function string getMisPkgName(string strLink)

{

    return Left(strLink, InStr(strLink, "_Link"));

}



simulated function PostNetBeginPlay()

{

    local int i, j;

    local Actor Parent;

    local vector RotX, RotY, RotZ, offset;

    local sItem headlight;



    Super.PostNetBeginPlay();



    // Turn the information about the mission packages into joint parts

    for(i=0; i<allMisPkgs.Length; i++)

    {

        for(j=0; j<allMisPkgs[i].Links.length; j++)

        {

            JointParts.Insert(JointParts.length, 1); // Make space in the dynamic array to add a part from the mission package

            JointParts[JointParts.length - 1].PartName = getMisPkgPartName(MisPkgs[i].PkgName, allMisPkgs[i].Links[j].LinkNumber);

            JointParts[JointParts.length - 1].PartClass = allMisPkgsLinks[i*allMisPkgs[i].Links.length+j].ModelClass;

            JointParts[JointParts.length - 1].DrawScale3D = allMisPkgs[i].Links[j].DrawScale3D;

            JointParts[JointParts.length - 1].bSteeringLocked = true;

            JointParts[JointParts.length - 1].bSuspensionLocked = true;

            JointParts[JointParts.length - 1].BrakeTorque = 0;

            JointParts[JointParts.length - 1].Parent = getMisPkgPartName(MisPkgs[i].PkgName, allMisPkgs[i].Links[j].ParentLinkNumber);



            if(Caps(allMisPkgsLinks[i*allMisPkgs[i].Links.length+j].getMountPointJointType(allMisPkgs[i].Links[j].SelfMount)) == "REVOLUTE")

            {

                JointParts[JointParts.length - 1].JointClass = class'USARBot.KDHinge';



                if(j == 0)

                    JointParts[JointParts.length - 1].ParentPos = converter.LengthVectorToUU(MisPkgs[i].Location);

                else

                    JointParts[JointParts.length - 1].ParentPos = converter.LengthVectorToUU(allMisPkgsLinks[i*allMisPkgs[i].Links.length+FindLinkParent(allMisPkgs[i], allMisPkgs[i].Links[j].ParentLinkNumber)].getMountPointLocation(allMisPkgs[i].Links[j].ParentMount) * allMisPkgs[i].Links[FindLinkParent(allMisPkgs[i], allMisPkgs[i].Links[j].ParentLinkNumber)].DrawScale3D);



                JointParts[JointParts.length - 1].ParentAxis = (Vect(0,0,-1) >> converter.RotatorToUU(allMisPkgsLinks[i*allMisPkgs[i].Links.length+j].getMountPointOrientation(allMisPkgs[i].Links[j].SelfMount))) * Vect(-1,-1,-1);

                JointParts[JointParts.length - 1].SelfAxis = JointParts[JointParts.length - 1].ParentAxis;

                JointParts[JointParts.length - 1].ParentAxis2 = (Vect(0,1,0) >> converter.RotatorToUU(allMisPkgsLinks[i*allMisPkgs[i].Links.length+j].getMountPointOrientation(allMisPkgs[i].Links[j].SelfMount))) * Vect(-1,-1,-1);

                JointParts[JointParts.length - 1].SelfAxis2 = JointParts[JointParts.length - 1].ParentAxis2;



                JointParts[JointParts.length - 1].SelfPos = converter.LengthVectorToUU(allMisPkgsLinks[i*allMisPkgs[i].Links.length+j].getMountPointLocation(allMisPkgs[i].Links[j].SelfMount) * allMisPkgs[i].Links[j].DrawScale3D);

            }

            else if(Caps(allMisPkgsLinks[i*allMisPkgs[i].Links.length+j].getMountPointJointType(allMisPkgs[i].Links[j].SelfMount)) == "PRISMATIC")

            {

                JointParts[JointParts.length - 1].JointClass = class'USARBot.KSlider';



                if(j == 0)

                    JointParts[JointParts.length - 1].ParentPos = converter.LengthVectorToUU(MisPkgs[i].Location - (allMisPkgsLinks[i*allMisPkgs[i].Links.length+j].getMountPointLocation(allMisPkgs[i].Links[j].SelfMount) * allMisPkgs[i].Links[j].DrawScale3D));

                else

                    JointParts[JointParts.length - 1].ParentPos = converter.LengthVectorToUU((allMisPkgsLinks[i*allMisPkgs[i].Links.length+FindLinkParent(allMisPkgs[i], allMisPkgs[i].Links[j].ParentLinkNumber)].getMountPointLocation(allMisPkgs[i].Links[j].ParentMount) * allMisPkgs[i].Links[FindLinkParent(allMisPkgs[i], allMisPkgs[i].Links[j].ParentLinkNumber)].DrawScale3D) - (allMisPkgsLinks[i*allMisPkgs[i].Links.length+j].getMountPointLocation(allMisPkgs[i].Links[j].SelfMount) * allMisPkgs[i].Links[j].DrawScale3D));



                JointParts[JointParts.length - 1].ParentAxis = (Vect(0,1,0) >> converter.RotatorToUU(allMisPkgsLinks[i*allMisPkgs[i].Links.length+j].getMountPointOrientation(allMisPkgs[i].Links[j].SelfMount))) * Vect(-1,-1,-1);

                JointParts[JointParts.length - 1].SelfAxis = JointParts[JointParts.length - 1].ParentAxis;

                JointParts[JointParts.length - 1].ParentAxis2 = (Vect(1,0,0) >> converter.RotatorToUU(allMisPkgsLinks[i*allMisPkgs[i].Links.length+j].getMountPointOrientation(allMisPkgs[i].Links[j].SelfMount))) * Vect(-1,-1,-1);

                JointParts[JointParts.length - 1].SelfAxis2 = JointParts[JointParts.length - 1].ParentAxis2;



                JointParts[JointParts.length - 1].SelfPos = converter.LengthVectorToUU(-allMisPkgsLinks[i*allMisPkgs[i].Links.length+j].MaxRange * (Vect(0,0,-1) >> converter.RotatorToUU(allMisPkgsLinks[i*allMisPkgs[i].Links.length+j].getMountPointOrientation(allMisPkgs[i].Links[j].SelfMount))));

            }

	    else if(Caps(allMisPkgsLinks[i*allMisPkgs[i].Links.length+j].getMountPointJointType(allMisPkgs[i].Links[j].SelfMount)) == "SCISSOR")

            {

                JointParts[JointParts.length - 1].JointClass = class'USARBot.ScissorJoint';



                if(j == 0)

                    JointParts[JointParts.length - 1].ParentPos = converter.LengthVectorToUU(MisPkgs[i].Location - (allMisPkgsLinks[i*allMisPkgs[i].Links.length+j].getMountPointLocation(allMisPkgs[i].Links[j].SelfMount) * allMisPkgs[i].Links[j].DrawScale3D));

                else

                    JointParts[JointParts.length - 1].ParentPos = converter.LengthVectorToUU((allMisPkgsLinks[i*allMisPkgs[i].Links.length+FindLinkParent(allMisPkgs[i], allMisPkgs[i].Links[j].ParentLinkNumber)].getMountPointLocation(allMisPkgs[i].Links[j].ParentMount) * allMisPkgs[i].Links[FindLinkParent(allMisPkgs[i], allMisPkgs[i].Links[j].ParentLinkNumber)].DrawScale3D) - (allMisPkgsLinks[i*allMisPkgs[i].Links.length+j].getMountPointLocation(allMisPkgs[i].Links[j].SelfMount) * allMisPkgs[i].Links[j].DrawScale3D));



                JointParts[JointParts.length - 1].ParentAxis = (Vect(0,1,0) >> converter.RotatorToUU(allMisPkgsLinks[i*allMisPkgs[i].Links.length+j].getMountPointOrientation(allMisPkgs[i].Links[j].SelfMount))) * Vect(1,1,1);

                JointParts[JointParts.length - 1].SelfAxis = JointParts[JointParts.length - 1].ParentAxis;

                JointParts[JointParts.length - 1].ParentAxis2 = (Vect(1,0,0) >> converter.RotatorToUU(allMisPkgsLinks[i*allMisPkgs[i].Links.length+j].getMountPointOrientation(allMisPkgs[i].Links[j].SelfMount))) * Vect(1,1,1);

                JointParts[JointParts.length - 1].SelfAxis2 = JointParts[JointParts.length - 1].ParentAxis2;



                JointParts[JointParts.length - 1].SelfPos = converter.LengthVectorToUU(-allMisPkgsLinks[i*allMisPkgs[i].Links.length+j].MaxRange * (Vect(0,0,-1) >> converter.RotatorToUU(allMisPkgsLinks[i*allMisPkgs[i].Links.length+j].getMountPointOrientation(allMisPkgs[i].Links[j].SelfMount))));

            }

        }

    }



    ////////////////////////////////

    // Create physical robot

    ////////////////////////////////

    for (CurrentPart=0;CurrentPart<JointParts.length;CurrentPart++) {

        // Find parent

        Parent = FindPart(JointParts[CurrentPart].Parent);

        if (Parent==None) continue;

        Parents[CurrentPart] = Parent;

        if (bDebug) log("<"@Parent@Parent.Location@Parent.Rotation@">");

        GetAxes(Parent.Rotation,RotX,RotY,RotZ);



        //Spawn Joint (any KConstraint class)

        Joints[CurrentPart] = spawn(JointParts[CurrentPart].JointClass,Parent);



        if (Joints[CurrentPart].IsA('KSlider')) {

		if(Joints[CurrentPart].IsA('ScissorJoint'))

		{

			Parts[CurrentPart] = spawn(JointParts[CurrentPart].PartClass,Parent,, Parent.Location + offset.X*RotX + offset.Y*RotY + offset.Z*RotZ,Parent.Rotation);

			Parts[CurrentPart].SetDrawScale3D(JointParts[CurrentPart].DrawScale3D);

			ScissorJoint(Joints[CurrentPart]).init(Parts[CurrentPart],Parent,JointParts[CurrentPart].ParentPos,JointParts[CurrentPart].SelfPos,JointParts[CurrentPart].ParentAxis,JointParts[CurrentPart].ParentAxis2,JointParts[CurrentPart].SelfAxis,JointParts[CurrentPart].SelfAxis2);

		}

		else

		{

        	  KSlider(Joints[CurrentPart]).init1(Parent,JointParts[CurrentPart].ParentPos,JointParts[CurrentPart].ParentAxis,JointParts[CurrentPart].ParentAxis2,

        	                                     JointParts[CurrentPart].SelfPos,JointParts[CurrentPart].SelfAxis,JointParts[CurrentPart].SelfAxis2);

            offset = JointParts[CurrentPart].ParentPos - JointParts[CurrentPart].SelfPos;

            Parts[CurrentPart] = spawn(JointParts[CurrentPart].PartClass, KSlider(Joints[CurrentPart]).Part2,, Parent.Location + offset.X*RotX + offset.Y*RotY + offset.Z*RotZ,Parent.Rotation);

            Parts[CurrentPart].SetDrawScale3D(JointParts[CurrentPart].DrawScale3D);

            log("Init"@Joints[CurrentPart]);

            KSlider(Joints[CurrentPart]).init2(JointParts[CurrentPart].ParentPos,JointParts[CurrentPart].ParentAxis,JointParts[CurrentPart].ParentAxis2,

                                               Parts[CurrentPart],JointParts[CurrentPart].SelfPos,JointParts[CurrentPart].SelfAxis,JointParts[CurrentPart].SelfAxis2);

		}

        } else {

            //Spawn Part

            offset = JointParts[CurrentPart].ParentPos - JointParts[CurrentPart].SelfPos;

            Parts[CurrentPart] = spawn(JointParts[CurrentPart].PartClass, Parent,, Parent.Location + offset.X*RotX + offset.Y*RotY + offset.Z*RotZ,Parent.Rotation);

            Parts[CurrentPart].SetDrawScale3D(JointParts[CurrentPart].DrawScale3D);



            //Set primary constraint params

            Joints[CurrentPart].KConstraintActor1 = Parent;

            Joints[CurrentPart].KPos1 = JointParts[CurrentPart].ParentPos/50;

            Joints[CurrentPart].KPriAxis1 = JointParts[CurrentPart].ParentAxis;

            if (VSize(JointParts[CurrentPart].ParentAxis2)>0)

                Joints[CurrentPart].KSecAxis1 = JointParts[CurrentPart].ParentAxis2;



            //Set secondary constraint params

            Joints[CurrentPart].KConstraintActor2 = Parts[CurrentPart];

            Joints[CurrentPart].KPos2 = JointParts[CurrentPart].SelfPos/50;

            Joints[CurrentPart].KPriAxis2 = JointParts[CurrentPart].SelfAxis;

            if (VSize(JointParts[CurrentPart].SelfAxis2)>0)

                Joints[CurrentPart].KSecAxis2 = JointParts[CurrentPart].SelfAxis2;



            Joints[CurrentPart].SetPhysics(PHYS_Karma);

        }



		if (ClassIsChildOf(JointParts[CurrentPart].PartClass,class'KTire') &&

		    Joints[CurrentPart].IsA('KCarWheelJoint')) {

			(KTire(Parts[CurrentPart])).WheelJoint = KCarWheelJoint(Joints[CurrentPart]);

			(KTire(Parts[CurrentPart])).WheelJoint.KUpdateConstraintParams();

			if (Joints[CurrentPart].IsA('KSCarWheelJoint')) {

				KSCarWheelJoint(Joints[CurrentPart]).KForceThreshold = SafeForce;

				KSCarWheelJoint(Joints[CurrentPart]).StallTime = ProtectTime;

			}

		} else if (ClassIsChildOf(JointParts[CurrentPart].PartClass,class'USARBot.KDPart'))

			(KDPart(Parts[CurrentPart])).setJoint(Joints[CurrentPart]);



		if(Role != ROLE_Authority)

			KarmaParams(Parts[CurrentPart].KParams).bDestroyOnSimError = False;

	}

	if (bDebug) {

		//DumpPackages();

		DumpJoints();

	}



	// Initially make sure parameters are sync'ed with Karma

	KVehicleUpdateParams();



        // For KImpact event

        KSetImpactThreshold(HitSoundThreshold);



	// If this is not 'authority' version - don't destroy it if there is a problem.

	// The network should sort things out.

	if(Role != ROLE_Authority)

		KarmaParams(KParams).bDestroyOnSimError = False;



	// init the array size

	JointsControl.length = JointParts.length;



	/////////////////////////////////

	// Mount Items

	/////////////////////////////////



	// Mount headlights

	for (i=0;i<Headlights.length;i++)

        {

		headlight=Headlights[i]; //spawn won't take variables from lightItem, so temp sItem needs to be used



		Parent = FindPart(headlight.Parent);

		if (Parent!=None)

		{

			GetAxes(Parent.Rotation,RotX,RotY,RotZ);

			HeadlightList[i] = USARHeadlight(spawn(headlight.ItemClass,

							  Parent,,

						      Parent.Location + headlight.Position.X * RotX + headlight.Position.Y * RotY + headlight.Position.Z * RotZ,));

			HeadlightList[i].SetBase(Parent);

			HeadlightList[i].SetRelativeRotation(headlight.uuDirection);

			if(HeadLights[i].LightGlow)HeadlightList[i].spawnGlow();

		}

	}



	// Mount cameras

	for (i=0;i<Cameras.length;i++)

        {

                if(i < 4)

                    CamsToView[i] = i;



		Parent = FindPart(Cameras[i].Parent);

		if (Parent==None) continue;

		    GetAxes(Parent.Rotation,RotX,RotY,RotZ);

		CamList[i] = RobotCamera(spawn(Cameras[i].ItemClass, Parent,,

									   Parent.Location + Cameras[i].Position.X * RotX + Cameras[i].Position.Y * RotY + Cameras[i].Position.Z * RotZ,

							 ));

		CamList[i].init(Cameras[i].ItemName,Parent,Cameras[i].Position,Cameras[i].uuDirection,self,Cameras[i].Parent);

	}

	if (CamList.length>0)

	{

 	        if(CamList.Length == 1)

	            viewportMode = 0;

                else

                    viewportMode = 1;



		myCamera = CamList[0];

	}



	// Mount sensors

	for (i=0;i<Sensors.length;i++) {

		Parent = FindPart(Sensors[i].Parent);

		if (Parent==None) continue;

	    GetAxes(Parent.Rotation,RotX,RotY,RotZ);

		SensorList[i] = Sensor(spawn(Sensors[i].ItemClass,

						      Parent,,

						      Parent.Location + Sensors[i].Position.X * RotX + Sensors[i].Position.Y * RotY + Sensors[i].Position.Z * RotZ,

						      ));

		SensorList[i].init(Sensors[i].ItemName,Parent,Sensors[i].Position,Sensors[i].uuDirection,self,Sensors[i].Parent);

		ProcessedSensors[i]=0;

	}



	// Mount Effecters

	for (i=0;i<Effecters.length;i++) {

		Parent = FindPart(Effecters[i].Parent);

		if (Parent==None) continue;

	    GetAxes(Parent.Rotation,RotX,RotY,RotZ);

		EffecterList[i] = Effecter(spawn(Effecters[i].ItemClass,

						      Parent,,

						      Parent.Location + Effecters[i].Position.X * RotX + Effecters[i].Position.Y * RotY + Effecters[i].Position.Z * RotZ,

						      ));

		EffecterList[i].init(Effecters[i].ItemName,Parent,Effecters[i].Position,Effecters[i].uuDirection,self,Effecters[i].Parent);

	}



	// Mount OmniCam elements

	if (CamTexActors.ItemName!="") {

		Parent = FindPart(CamTexActors.Parent);

		if (Parent!=None) {

			GetAxes(Parent.Rotation,RotX,RotY,RotZ);



			// ***** spawn Camera Actor Emitter North

			myEmitterNorth = USAREmitter(spawn(CamTexActors.ItemClass,

					Parent,,

					Parent.Location + CamTexActors.Position.X * RotX + CamTexActors.Position.Y * RotY + CamTexActors.Position.Z * RotZ,

				));

			myEmitterNorth.bHardAttach=true;

			myEmitterNorth.SetBase(Parent);

			myEmitterNorth.SetRelativeRotation(CamTexActors.uuDirection);



			// Spawn CameraTextureClient North

			myCamTexClientNorth = spawn(class'USARBot.USARCameraTextureClientN',

					Parent,,

					Parent.Location + CamTexActors.Position.X * RotX + CamTexActors.Position.Y * RotY + CamTexActors.Position.Z * RotZ,

				);

			myCamTexClientNorth.CameraActor = myEmitterNorth;

			myCamTexClientNorth.SetBase(Parent);



			// ***** spawn Camera Actor Emitter East

			myEmitterEast = USAREmitter(spawn(CamTexActors.ItemClass,

					Parent,,

					Parent.Location + CamTexActors.Position.X * RotX + CamTexActors.Position.Y * RotY + CamTexActors.Position.Z * RotZ,

				));

			myEmitterEast.bHardAttach=true;

			myEmitterEast.SetBase(Parent);

			EmitterDir = CamTexActors.uuDirection;

			EmitterDir.Yaw = EmitterDir.Yaw + 1024*16;

			myEmitterEast.SetRelativeRotation(EmitterDir);



			// Spawn CameraTextureClient East

			myCamTexClientEast = spawn(class'USARBot.USARCameraTextureClientE',

					Parent,,

					Parent.Location + CamTexActors.Position.X * RotX + CamTexActors.Position.Y * RotY + CamTexActors.Position.Z * RotZ,

				);

			myCamTexClientEast.CameraActor = myEmitterEast;

			myCamTexClientEast.SetBase(Parent);



			// ***** spawn Camera Actor Emitter South

			myEmitterSouth = USAREmitter(spawn(CamTexActors.ItemClass,

					Parent,,

					Parent.Location + CamTexActors.Position.X * RotX + CamTexActors.Position.Y * RotY + CamTexActors.Position.Z * RotZ,

				));

			myEmitterSouth.bHardAttach=true;

			myEmitterSouth.SetBase(Parent);

			EmitterDir = CamTexActors.uuDirection;

			EmitterDir.Yaw = EmitterDir.Yaw + 1024*32;

			myEmitterSouth.SetRelativeRotation(EmitterDir);



			// Spawn CameraTextureClient  South

			myCamTexClientSouth = spawn(class'USARBot.USARCameraTextureClientS',

					Parent,,

					Parent.Location + CamTexActors.Position.X * RotX + CamTexActors.Position.Y * RotY + CamTexActors.Position.Z * RotZ,

				);

			myCamTexClientSouth.CameraActor = myEmitterSouth;

			myCamTexClientSouth.SetBase(Parent);



			// ***** spawn Camera Actor Emitter West

			myEmitterWest = USAREmitter(spawn(CamTexActors.ItemClass,

					Parent,,

					Parent.Location + CamTexActors.Position.X * RotX + CamTexActors.Position.Y * RotY + CamTexActors.Position.Z * RotZ,

				));

			myEmitterWest.bHardAttach=true;

			myEmitterWest.SetBase(Parent);

			EmitterDir = CamTexActors.uuDirection;

			EmitterDir.Yaw = EmitterDir.Yaw + 1024*48;

			myEmitterWest.SetRelativeRotation(EmitterDir);



			// Spawn CameraTextureClient West

			myCamTexClientWest = spawn(class'USARBot.USARCameraTextureClientW',

					Parent,,

					Parent.Location + CamTexActors.Position.X * RotX + CamTexActors.Position.Y * RotY + CamTexActors.Position.Z * RotZ,

				);

			myCamTexClientWest.CameraActor = myEmitterWest;

			myCamTexClientWest.SetBase(Parent);



			// ***** spawn Camera Actor Emitter Center

			myEmitterCenter = USAREmitter(spawn(CamTexActors.ItemClass,

					Parent,,

					Parent.Location + CamTexActors.Position.X * RotX + CamTexActors.Position.Y * RotY + CamTexActors.Position.Z * RotZ,

				));

			myEmitterCenter.bHardAttach=true;

			myEmitterCenter.SetBase(Parent);

			EmitterDir = CamTexActors.uuDirection;

			EmitterDir.Pitch = EmitterDir.Pitch - 1024*16;

			myEmitterCenter.SetRelativeRotation(EmitterDir);



			// Spawn CameraTextureClient Center

			myCamTexClientCenter = spawn(class'USARBot.USARCameraTextureClientC',

					Parent,,

					Parent.Location + CamTexActors.Position.X * RotX + CamTexActors.Position.Y * RotY + CamTexActors.Position.Z * RotZ,

				);

			myCamTexClientCenter.CameraActor = myEmitterCenter;

			myCamTexClientCenter.SetBase(Parent);

		}

	}



	Register();

	SetTimer(msgTimer,true);

	startTime = Level.TimeSeconds;

	lastTime = Level.TimeSeconds;

	lastLocation = Location;



	//If there's an FPSLogger triggers it

	fpsLogger = 'FPSLog';

	TriggerEvent(fpsLogger, self, None);



}



simulated function SetSkin(string skinName)

{

	local int i;

	local texture skinTexture;



    for(i = 0; i < RobotSkins.Length; i++)

        if(skinName ~= RobotSkins[i].Name)

        {

            skinTexture = RobotSkins[i].Skin;

            break;

        }

    if((i > 0) && (i == RobotSkins.Length))

         skinTexture = RobotSkins[0].Skin;    //i>0 assure [0] element existence



    if(skinTexture != none)

    {

        Skins[0] = skinTexture;

    	for (i = 0; i < Parts.Length; i++)

    	{

    		Parts[i].Skins[0] = skinTexture;

    	}

	}



	if(Role == ROLE_Authority)

	{

    	RS_skinName = skinName;

    	RS_skinUpdateId++;

    }

}



// dump joints

function DumpJoints() {

	local int i;



	for (i=0;i<Joints.length;i++)

		log(i@"Name"@JointParts[i].PartName@"Part"@Parts[i]@"Joint"@Joints[i]);

}



simulated event Destroyed()

{

	local int i;



	// Destory mounted items

	for (i=0;i<SensorList.length;i++)	SensorList[i].Destroy();

	for (i=0;i<EffecterList.length;i++)	EffecterList[i].Destroy();

	for (i=0;i<CamList.length;i++)	        CamList[i].Destroy();



	for (i=0;i<HeadlightList.length;i++) {

		if(HeadlightList[i].bGlowOn) HeadlightList[i].HeadlightGlow.Destroy();

		HeadlightList[i].Destroy(); }



	if (myEmitterNorth!=None) myEmitterNorth.Destroy();

	if (myEmitterEast!=None) myEmitterEast.Destroy();

	if (myEmitterWest!=None) myEmitterWest.Destroy();

	if (myEmitterSouth!=None) myEmitterSouth.Destroy();

	if (myEmitterCenter!=None) myEmitterCenter.Destroy();



	if (myCamTexClientNorth!=None) myCamTexClientNorth.Destroy();

	if (myCamTexClientEast!=None) myCamTexClientEast.Destroy();

	if (myCamTexClientWest!=None) myCamTexClientWest.Destroy();

	if (myCamTexClientSouth!=None) myCamTexClientSouth.Destroy();

	if (myCamTexClientCenter!=None) myCamTexClientCenter.Destroy();



	// Destory physical robot

	for (i=0;i<Parts.length;i++) {

		if (Parts[i]!=none) {

			Joints[i].Destroy();

			Parts[i].Destroy();

		}

	}



	// Destroy information about mission packages

	while(allMisPkgs.Length!=0)

	{

	    allMisPkgs[allMisPkgs.Length-1].Delete();

            allMisPkgs.Remove(allMisPkgs.Length-1, 1);

        }



	// Destroy information about mission packages' links

	while(allMisPkgsLinks.Length!=0)

	{

	    allMisPkgsLinks[allMisPkgsLinks.Length-1].Delete();

            allMisPkgsLinks.Remove(allMisPkgsLinks.Length-1, 1);

        }



	Unregister();

	converter = None;

        utils = None;

        Draw = None;

 	UntriggerEvent(fpsLogger, self, None);

	KSetSimParams(oldKSP);

	if (CollLog!=None)

		CollLog.CloseLog();

	if (PosLog!=None)

		PosLog.CloseLog();

	Super.Destroyed();

}



// For drawing the camera viewports, possible noise, and the bar code

simulated function DrawHud(Canvas C)

{

    // If we are using the camera view (bDrawHud is set in ViewTestPlayerController.uc)

    if(bDrawHud)

    {

        // If the robot does not have any more battery

        if (myLife==batteryLife)

        {

            Draw.NoPower(C, ERenderStyle.STY_Particle, 0, 0, C.SizeX, C.SizeY, C.SizeX/2-80, C.SizeY/2);

        }

        // Here, we draw the viewports

        else

        {

            Draw.KRobotCameras(C, viewportMode, CamsToView, ERenderStyle.STY_Particle, CamList, USARRemoteBot(Controller).ShowCamName, USARRemoteBot(Controller).ShowCamTime, Level.TimeSeconds-startTime);

            super.DrawHud(C);

        }

    }



    if (bBarCode)

        Draw.BarCode(C, RobotName, 0, C.SizeY-2);



}



// Call this if you change any parameters (tire, suspension etc.) and they

// will be passed down to each wheel/joint.

simulated event KVehicleUpdateParams()

{

    local KTire Part;

    local KCarWheelJoint WheelJ;

    local KDHinge HingeJ;

    local int i;

    local MisPkgLinkInfo aMisPkgLink;



    Super.KVehicleUpdateParams();



    for (i=0;i<Parts.length;i++)

    {

        if (Joints[i].IsA('KCarWheelJoint'))

        {

            WheelJ=KCarWheelJoint(Joints[i]);

            WheelJ.bKSteeringLocked = JointParts[i].bSteeringLocked;



            WheelJ.KProportionalGap = SteerPropGap;

            WheelJ.KMaxSteerTorque = SteerTorque;

            WheelJ.KMaxSteerSpeed = SteerSpeed;

            WheelJ.KBraking = JointParts[i].BrakeTorque;



            if (JointParts[i].bSuspensionLocked) {

                WheelJ.KSuspHighLimit = 0.001;

                WheelJ.KSuspLowLimit = -0.001;

                WheelJ.KSuspStiffness = 200.0;

                WheelJ.KSuspDamping = 100.0;

            }

            else {

                WheelJ.KSuspHighLimit = SuspHighLimit;

                WheelJ.KSuspLowLimit = SuspLowLimit;

                WheelJ.KSuspStiffness = SuspStiffness;

                WheelJ.KSuspDamping = SuspDamping;

            }

            // Sync params with Karma.

            WheelJ.KUpdateConstraintParams();

        }

        else if(Joints[i].IsA('KDHinge'))

        {

            HingeJ = KDHinge(Joints[i]);



            aMisPkgLink = getMisPkgLinkInfo(getLinkNumber(string(JointParts[i].PartName)), getMisPkgName(string(JointParts[i].PartName)));

            if(aMisPkgLink != None)

            {

                HingeJ.KMaxTorque = aMisPkgLink.MaxTorque;

                HingeJ.KDesiredAngVel = converter.SpinSpeedToUU(aMisPkgLink.MaxSpeed);

            }

            else

            {

                HingeJ.KMaxTorque = MotorTorque;

                HingeJ.KDesiredAngVel = uuMotorSpeed;

            }



            HingeJ.KProportionalGap = HingePropGap;

            HingeJ.KHingeType = HT_CONTROLLED;

            // Sync params with Karma.

            if (Joints[i].IsA('KSlider'))

                KSlider(Joints[i]).UpdateConstraint();

            else

                HingeJ.KUpdateConstraintParams();

        }



        if (Parts[i].IsA('KTire'))

        {

            Part=KTire(Parts[i]);

            Part.RollFriction = TireRollFriction;

            Part.LateralFriction = TireLateralFriction;

            Part.RollSlip = TireRollSlip;

	    Part.LateralSlip = TireLateralSlip;

	    Part.MinSlip = TireMinSlip;

	    Part.SlipRate = TireSlipRate;

	    Part.Softness = TireSoftness;

	    Part.Adhesion = TireAdhesion;

	    Part.Restitution = TireRestitution;

        }

    }



    KSetMass(ChassisMass);

}



// Possibly apply force to flip robot over.

simulated event KApplyForce(out vector Force, out vector Torque)

{

	local float torqueScale;

	local vector worldForward, worldUp, worldRight, torqueAxis;



	if(FlipTimeLeft == 0)

		return;



	worldForward = vect(-1, 0, 0) >> Rotation;

	worldUp = vect(0, 0, 1) >> Rotation;

	worldRight = vect(0, 1, 0) >> Rotation;



	torqueAxis = Normal(worldUp Cross vect(0, 0, 1));



	// Torque scaled by how far over we are.

	// This will be between 0 and PI - so convert to between 0 and 1.

	torqueScale = Acos(worldUp Dot vect(0, 0, 1))/3.1416;



	Torque = FlipTorque * torqueScale * torqueAxis;

}



function StartFlip(Pawn Pusher)

{

	//local vector toPusher, worldUp;



	// if we are already flipping the car - dont do it again!

	if(FlipTimeLeft > 0)

		return;



	FlipTimeLeft = FlipTime; // Start the flip on the server

	USARRemoteBot(Controller).Flip = false;

}



//given a name and return the correspond jointpart's id

function int FindJointPartId(string jname)

{

	local int i;



	for (i=0;i<JointParts.length;i++)

		if (string(JointParts[i].PartName) == jname) return i;

	return -1;

}



// Get all the children IDs of a part

function array<int> FindChildren(int idx)

{

	local array<int> res;

	local int i;

	local name myName,pName;



	//invalid idx is treated as the robot platform (hard mount).

	if (idx<0 || idx>=JointParts.length)

		myName='None';

	//iterate JointParts to find all the children

	myName = JointParts[idx].PartName;

	for (i=0;i<JointParts.length;i++) {

		pName = JointParts[i].Parent;

		if (pName=='') pName='None';

		if (i!=idx && pName==myName) {

			res.Insert(res.length,1);

			res[res.length-1] = i;

		}

	}



	return res;

}



// The relative location of the joint (mount) respect to the parent part

function vector rLocJointParent(int idx)

{

	local vector res, forward, right, upward, dif;

	local Actor parent;



	//check idx

	if (idx<0 || idx>=JointParts.length) {

		log("Invaild JointPart index in function rLocJointParent!");

		return vect(0,0,0);

	}

	//find the parent's orientation

	parent = FindPart(JointParts[idx].Parent);

	if (parent==None) {

		log("Can't find the parent in function rLocJointParent!");

		return vect(0,0,0);

	}

	if (parent.Rotation==rot(0,0,0)) {

		forward = vect(1,0,0);

		right   = vect(0,1,0);

		upward  = vect(0,0,1);

	} else

		GetAxes(parent.Rotation,forward,right,upward);

	//project to the parent's coordinate

	dif = Joints[idx].Location - parent.Location;

	res.X = dif Dot forward;

	res.Y = dif Dot right;

	res.Z = dif Dot upward;



	return res;

}



// The relative orientation of the joint (mount) respect to the parent part

// Ref: http://people.csail.mit.edu/bkph/articles/Kinematics_Vicarm_WP_69.pdf

function rotator rRotJointParent(int idx)

{

	local rotator res;

	local vector pForward, pRight, pUpward;

	local vector jForward, jRight, jUpward;

	local Actor parent, joint;

	local float m00,m01,m02,m10,m11,m12,m20,m21,m22;



	//check idx

	if (idx<0 || idx>=JointParts.length) {

		log("Invaild JointPart index in function rLocJointParent!");

		return rot(0,0,0);

	}

	//find the parent's orientation

	parent = FindPart(JointParts[idx].Parent);

	if (parent==None) {

		log("Can't find the parent in function rLocJointParent!");

		return rot(0,0,0);

	}

	if (parent.Rotation==rot(0,0,0)) {

		pForward = vect(1,0,0);

		pRight   = vect(0,1,0);

		pUpward  = vect(0,0,1);

	} else

		GetAxes(parent.Rotation,pForward,pRight,pUpward);

	//find the joint's orientation

	joint = Joints[idx];

	if (joint.Rotation==rot(0,0,0)) {

		jForward = vect(1,0,0);

		jRight   = vect(0,1,0);

		jUpward  = vect(0,0,1);

	} else

		GetAxes(joint.Rotation,jForward,jRight,jUpward);

	//calculate the rotation matrix

	m00 = jForward Dot pForward;

	m10 = jForward Dot pRight;

	m20 = jForward Dot pUpward;

	m01 = jRight   Dot pForward;

	m11 = jRight   Dot pRight;

	m21 = jRight   Dot pUpward;

	m02 = jUpward  Dot pForward;

	m12 = jUpward  Dot pRight;

	m22 = jUpward  Dot pUpward;

	//calculate the pitch, yaw and roll angles in UU which indicate we getting

	//part coordinate by rotating joint coordinate roll->pitch->yaw.

	res.Pitch = Atan(-m20,Sqrt((m00*m00+m10*m10+m21*m21+m22*m22)/2))*10430.3783505;

	if (Abs(res.Pitch-16384)<10) {

		// Can't uniquely determine yaw and roll. Here we set roll=0.

		res.Roll = 0;

		res.Yaw = -Atan(m01-m20,m11+m02)*10430.3783505;

	} else if (Abs(res.Pitch+16384)<10) {

		// Can't uniquely determine yaw and roll. Here we set roll=0.

		res.Roll = 0;

		res.Yaw = Atan(-m01-m20,m11-m02)*10430.3783505;

	} else {

		res.Yaw = Atan(m10,m00)*10430.3783505;

		res.Roll = Atan(m21,m22)*10430.3783505;

	}



	res.Yaw=-res.Yaw;

	return res;

}



// The relative location of the part respect to the joint (mount)

function vector rLocPartJoint(int idx)

{

	local vector res, forward, right, upward, dif;

	local Actor joint;



	//check idx

	if (idx<0 || idx>=JointParts.length) {

		log("Invaild JointPart index in function rLocJointParent!");

		return vect(0,0,0);

	}

	//find the joint's orientation

	joint = Joints[idx];

	if (joint.Rotation==rot(0,0,0)) {

		forward = vect(1,0,0);

		right   = vect(0,1,0);

		upward  = vect(0,0,1);

	} else

		GetAxes(joint.Rotation,forward,right,upward);

	//project to the joint's coordinate

	dif = Parts[idx].Location - joint.Location;

	res.X = dif Dot forward;

	res.Y = dif Dot right;

	res.Z = dif Dot upward;



	return res;

}



// The relative orientation of the part respect to the joint (mount)

function rotator rRotPartJoint(int idx)

{

	local rotator res;

	local vector pForward, pRight, pUpward;

	local vector jForward, jRight, jUpward;

	local Actor part, joint;

	local float m00,m01,m02,m10,m11,m12,m20,m21,m22;



	//check idx

	if (idx<0 || idx>=JointParts.length) {

		log("Invaild JointPart index in function rLocJointParent!");

		return rot(0,0,0);

	}

	//find the part's orientation

	part = Parts[idx];

	if (part.Rotation==rot(0,0,0)) {

		pForward = vect(1,0,0);

		pRight   = vect(0,1,0);

		pUpward  = vect(0,0,1);

	} else

		GetAxes(part.Rotation,pForward,pRight,pUpward);

	//find the joint's orientation which should be parent's orientation

	joint = Parents[idx];

	if (joint==None || joint.Rotation==rot(0,0,0)) {

		jForward = vect(1,0,0);

		jRight   = vect(0,1,0);

		jUpward  = vect(0,0,1);

	} else

		GetAxes(joint.Rotation,jForward,jRight,jUpward);

	//calculate the rotation matrix

	m00 = jForward Dot pForward;

	m10 = jForward Dot pRight;

	m20 = jForward Dot pUpward;

	m01 = jRight   Dot pForward;

	m11 = jRight   Dot pRight;

	m21 = jRight   Dot pUpward;

	m02 = jUpward  Dot pForward;

	m12 = jUpward  Dot pRight;

	m22 = jUpward  Dot pUpward;

	//calculate the pitch, yaw and roll angles in UU which indicate we getting

	//part coordinate by rotating joint coordinate roll->pitch->yaw.

	res.Pitch = Atan(-m20,Sqrt((m00*m00+m10*m10+m21*m21+m22*m22)/2))*10430.3783505;

	if (Abs(res.Pitch-16384)<10) {

		// Can't uniquely determine yaw and roll. Here we set roll=0.

		res.Roll = 0;

		res.Yaw = -Atan(m01-m20,m11+m02)*10430.3783505;

	} else if (Abs(res.Pitch+16384)<10) {

		// Can't uniquely determine yaw and roll. Here we set roll=0.

		res.Roll = 0;

		res.Yaw = Atan(-m01-m20,m11-m02)*10430.3783505;

	} else {

		res.Yaw = Atan(m10,m00)*10430.3783505;

		res.Roll = Atan(m21,m22)*10430.3783505;

	}

	/* Debug::

	log(JointParts[idx].PartName@pForward@pRight@pUpward@jForward@jRight@jUpward);

	log(m00@m01@m02);

	log(m10@m11@m12);

	log(m20@m21@m22);

	log(res);

	*/

	res.Yaw=-res.Yaw;

	return res;

}



simulated function int getJointAngle(KCarWheelJoint WheelJ)

{

	local Quat curQ, relQ;

	local Vector axis11, axis12, axis21, axis22, newAxis11;

	local float difCos, difSign;

	local int curAng;



	curQ = WheelJ.KConstraintActor1.KGetRBQuaternion();

	axis11 = QuatRotateVector(curQ,WheelJ.KPriAxis1);

	axis12 = QuatRotateVector(curQ,WheelJ.KSecAxis1);



	curQ = WheelJ.KConstraintActor2.KGetRBQuaternion();

	axis21 = QuatRotateVector(curQ,WheelJ.KPriAxis2);

	axis22 = QuatRotateVector(curQ,WheelJ.KSecAxis2);



	relQ = QuatFindBetween(axis12,axis22);

	newAxis11 = QuatRotateVector(relQ,axis11);



	difCos = newAxis11 Dot axis21;

	if (difCos>1.0) difCos = 1.0;

	if (difCos<-1.0) difCos = -1.0;



	difSign = (newAxis11 Cross axis21) Dot axis22;

	if (difSign<0) difSign=-1.0;

	else difSign=1.0;



	curAng = difSign * ACos(difCos)*32768/PI;



	return curAng;

}



// Get relative orientation between two rotators

function rotator getRelativeOrientation(rotator firstRotator, rotator secondRotator)

{

    local vector result;



    local vector firstForward, firstRight, firstUpward;

    local vector secondForward, secondRight, secondUpward;



    local float r00,r01,r02,r10,r11,r12,r20,r21,r22;



    // Find the second part's orientation in terms of vectors

    if (secondRotator == Rot(0,0,0))

    {

        secondForward = vect(1,0,0);

        secondRight   = vect(0,1,0);

        secondUpward  = vect(0,0,-1);

    }

    else

    {

        secondForward = (Vect(1,0,0) >> secondRotator);

        secondRight   = (Vect(0,1,0) >> secondRotator);

        secondUpward  = (Vect(0,0,-1) >> secondRotator);

    }



    // Find the first part's orientation in terms of vectors

    if (firstRotator == Rot(0,0,0))

    {

        firstForward = vect(1,0,0);

        firstRight   = vect(0,1,0);

        firstUpward  = vect(0,0,-1);

    }

    else

    {

        firstForward = (Vect(1,0,0) >> firstRotator);

        firstRight   = (Vect(0,1,0) >> firstRotator);

        firstUpward  = (Vect(0,0,-1) >> firstRotator);

    }



    // Calculate the Rotation Matrix

    r00 = secondForward Dot firstForward;

    r01 = secondForward Dot firstRight;

    r02 = secondForward Dot firstUpward;

    r10 = secondRight   Dot firstForward;

    r11 = secondRight   Dot firstRight;

    r12 = secondRight   Dot firstUpward;

    r20 = secondUpward  Dot firstForward;

    r21 = secondUpward  Dot firstRight;

    r22 = secondUpward  Dot firstUpward;



    // Calculate the X,Y,Z rotation angles, in radians

    result.Y = ASin(r02);



    if(result.Y < 1.5707963267948966192313216916398)

    {

        if(result.Y > -1.5707963267948966192313216916398)

        {

            result.X = ATan(-r12,r22);

            result.Z = ATan(-r01,r00);

        }

        else

        {

            // Not a unique solution

            result.X = -ATan(r10,r11);

            result.Z = 0;

        }

    }

    else

    {

        // Not a unique solution

        result.X = ATan(r10,r11);

        result.Z = 0;

    }



    result.X = -result.X;

    result.Y = -result.Y;

    result.Z = -result.Z;



    return Converter.RotatorToUU(result);

}



function ProcessCarInput()

{

    local int i, j, selfJointIndex, parentJointIndex, LinkCount, ParentLink;

    local int JIdx, JScount, JVcount, JOcount;

    local string type, name, outstring, tmpstring;

    local vector tmpvect;

    local rotator tmprot;



    bNewCommand = false;

    // Set sensor/Effecters/cameras

    JIdx=-1;

    type = USARRemoteBot(Controller).SetType;

    name = USARRemoteBot(Controller).SetName;



    if (type!="" && Caps(type)!="JOINT" && !USARRemoteBot(Controller).emptyBattery)

    {

        outstring = "RES {Time "$Level.TimeSeconds$"} {Type "$type$"} {Name "$USARRemoteBot(Controller).SetName$"}";

        for (i=0;i<EffecterList.length;i++) {

            if (EffecterList[i].isType(type) && EffecterList[i].isName(name)) {

                tmpstring = "{Status "$EffecterList[i].Set(USARRemoteBot(Controller).Opcode,USARRemoteBot(Controller).Params)$"}";

                JIdx=i;

                break;

            }

        }

        if (i<EffecterList.length)

            USARRemoteBot(Controller).myConnection.SendLine(outstring@tmpstring);



        for (i=0;i<CamList.length;i++) {

            if (CamList[i].isType(type) && CamList[i].isName(name)) {

                tmpstring = "{Status "$CamList[i].Set(USARRemoteBot(Controller).Opcode,USARRemoteBot(Controller).Params)$"}";

                //if (i==0)

                USARRemoteBot(Controller).myCameraZoom[i] = CamList[i].getFov();

                JIdx=i;

                break;

            }

        }

        if (i<CamList.length)

            USARRemoteBot(Controller).myConnection.SendLine(outstring@tmpstring);



        for (i=0;i<SensorList.length;i++) {

            if (SensorList[i].isType(type) && SensorList[i].isName(name)) {

                tmpstring = "{Status "$SensorList[i].Set(USARRemoteBot(Controller).Opcode,USARRemoteBot(Controller).Params)$"}";

                JIdx=i;

                break;

            }

        }

        if (i<SensorList.length)

            USARRemoteBot(Controller).myConnection.SendLine(outstring@tmpstring);



        if (JIdx==-1 && Caps(type) != "VIEWPORTS")

            USARRemoteBot(Controller).myConnection.SendLine(outstring@"{Status Failed}");

    }



    // Return geo info

    JIdx=-1;

    type = USARRemoteBot(Controller).GeoType;

    name = USARRemoteBot(Controller).GeoName;

    if (type!="" && !USARRemoteBot(Controller).emptyBattery)

    {

        // Sensor GEO Message

        outstring="";

        for (i=0;i<Sensors.length;i++) {

            if (SensorList[i].isType(type) && (name=="" || SensorList[i].isName(name))) {

                outstring = outstring@SensorList[i].GetGeoData();

                JIdx = i;

            }

        }

        if (outstring!="")

            USARRemoteBot(Controller).myConnection.SendLine(SensorList[JIdx].GetGeoHead()$outstring);



        // Effecter GEO Message

        outstring="";

        for (i=0;i<Effecters.length;i++) {

            if (EffecterList[i].isType(type) && (name=="" || EffecterList[i].isName(name))) {

                outstring = outstring@EffecterList[i].GetGeoData();

                JIdx = i;

            }

        }

        if (outstring!="")

            USARRemoteBot(Controller).myConnection.SendLine(EffecterList[JIdx].GetGeoHead()$outstring);



        // Camera GEO Message

        outstring="";

        for (i=0;i<Cameras.length;i++) {

            if (CamList[i].isType(type) && (name=="" || CamList[i].isName(name))) {

                outstring = outstring@CamList[i].GetGeoData();

                JIdx = i;

            }

        }

        if (outstring!="")

            USARRemoteBot(Controller).myConnection.SendLine(CamList[JIdx].GetGeoHead()$outstring);



        // Mission Package GEO Message

        outstring="";

        for (i=0;i<allMisPkgs.length;i++)

        {

            if ((Caps(type) == "MISPKG") && ((name == "") || (Caps(name) == Caps(string(MisPkgs[i].PkgName)))))

            {

                outstring = outstring @ "{Name " $ MisPkgs[i].PkgName $ "}";



                LinkCount = 0;



                for(j=0; j<allMisPkgs[i].Links.Length; j++)

                {

                    if(allMisPkgs[i].Links[j].LinkNumber > 0)

                    {

                        selfJointIndex = FindMisPkgLinkIndex(getMisPkgPartName(MisPkgs[i].PkgName, allMisPkgs[i].Links[j].LinkNumber));

                        parentJointIndex = FindMisPkgLinkIndex(getMisPkgPartName(MisPkgs[i].PkgName, allMisPkgs[i].Links[j].ParentLinkNumber));



                        if(LinkCount == 0)

                        {

                            ParentLink = -1; // Since this is the first link, it is attached to the vehicle (note: the vehicle is defined with a "Link Number" of -1)

                            tmpvect = utils.getRelativePosition(Joints[selfJointIndex].Location, Location, Rotation);

                            tmprot = getRelativeOrientation(Rot(0,0,0), Converter.RotatorToUU(allMisPkgsLinks[i*allMisPkgs[i].Links.length+j].getMountPointOrientation(allMisPkgs[i].Links[j].SelfMount)));

                        }

                        else

                        {

                            ParentLink = allMisPkgs[i].Links[j].ParentLinkNumber;

                            tmpvect = utils.getRelativePosition(Joints[selfJointIndex].Location, Joints[parentJointIndex].Location, Converter.RotatorToUU(allMisPkgsLinks[i*allMisPkgs[i].Links.length+FindLinkParent(allMisPkgs[i], allMisPkgs[i].Links[j].ParentLinkNumber)].getMountPointOrientation(allMisPkgs[i].Links[FindLinkParent(allMisPkgs[i], allMisPkgs[i].Links[j].ParentLinkNumber)].SelfMount)));

                            tmprot = getRelativeOrientation(Converter.RotatorToUU(allMisPkgsLinks[i*allMisPkgs[i].Links.length+FindLinkParent(allMisPkgs[i], allMisPkgs[i].Links[j].ParentLinkNumber)].getMountPointOrientation(allMisPkgs[i].Links[FindLinkParent(allMisPkgs[i], allMisPkgs[i].Links[j].ParentLinkNumber)].SelfMount)), Converter.RotatorToUU(allMisPkgsLinks[i*allMisPkgs[i].Links.length+j].getMountPointOrientation(allMisPkgs[i].Links[j].SelfMount)));

                        }



                        outstring = outstring @ allMisPkgsLinks[i*allMisPkgs[i].Links.length+j].GetGeoData(getMisPkgPartName(MisPkgs[i].PkgName, allMisPkgs[i].Links[j].LinkNumber), Converter.VectorString(tmpvect, 4), Converter.Str_RotatorFromUU(tmprot, 4), allMisPkgs[i].Links[j].LinkNumber, ParentLink);



                        LinkCount++;

                    }

                }



                JIdx = i;

            }

        }

        if (outstring!="")

            USARRemoteBot(Controller).myConnection.SendLine("GEO {Type MisPkg}"$outstring);





        if (JIdx==-1)

        {

            if(type != "Robot")

            {

                if (name!="")

                    USARRemoteBot(Controller).myConnection.SendLine("GEO {Type "$type$"} {Name "$name$"}");

                else

                    USARRemoteBot(Controller).myConnection.SendLine("GEO {Type "$type$"}");

            }

        }

    }



    // Return CONF Information

    JIdx = -1;

    type = USARRemoteBot(Controller).ConfType;

    name = USARRemoteBot(Controller).ConfName;

    if (type!="" && !USARRemoteBot(Controller).emptyBattery)

    {

        // Sensor CONF Message

        outstring="";

        for (i=0;i<Sensors.length;i++) {

            if (SensorList[i].isType(type) && (name=="" || SensorList[i].isName(name))) {

                outstring = outstring@SensorList[i].GetConfData();

                JIdx = i;

            }

        }

        if (outstring!="")

            USARRemoteBot(Controller).myConnection.SendLine(SensorList[JIdx].GetConfHead()$outstring);



        // Camera CONF Message

        outstring="";

        for (i=0;i<Cameras.length;i++) {

            if (CamList[i].isType(type) && (name=="" || CamList[i].isName(name))) {

                outstring = outstring@CamList[i].GetConfData();

                JIdx = i;

            }

        }

        if (outstring!="")

            USARRemoteBot(Controller).myConnection.SendLine(CamList[JIdx].GetConfHead()$outstring);



        // Effecter CONF Message

        outstring="";

        for (i=0;i<Effecters.length;i++) {

            if (EffecterList[i].isType(type) && (name=="" || EffecterList[i].isName(name))) {

                outstring = outstring@EffecterList[i].GetConfData();

                JIdx = i;

            }

        }

        if (outstring!="")

            USARRemoteBot(Controller).myConnection.SendLine(EffecterList[JIdx].GetConfHead()$outstring);



        // Mission Package CONF Message

        outstring="";

        for (i=0;i<allMisPkgs.length;i++)

        {

            if ((Caps(type) == "MISPKG") && ((name == "") || (Caps(name) == Caps(string(MisPkgs[i].PkgName)))))

            {

                outstring = outstring @ "{Name " $ MisPkgs[i].PkgName $ "}";



                for(j=0; j<allMisPkgs[i].Links.Length; j++)

                {

                    if(allMisPkgs[i].Links[j].LinkNumber > 0)

                    {

                        outstring = outstring @ allMisPkgsLinks[i*allMisPkgs[i].Links.length+j].GetConfData(getMisPkgPartName(MisPkgs[i].PkgName, allMisPkgs[i].Links[j].LinkNumber), allMisPkgs[i].Links[j].SelfMount, allMisPkgs[i].Links[j].LinkNumber);

                    }

                }



                JIdx = i;

            }

        }

        if (outstring!="")

            USARRemoteBot(Controller).myConnection.SendLine("CONF {Type MisPkg}"$outstring);





        if (JIdx==-1)

        {

            if(type != "Robot")

            {

                if (name!="")

                    USARRemoteBot(Controller).myConnection.SendLine("CONF {Type "$type$"} {Name "$name$"}");

                else

                    USARRemoteBot(Controller).myConnection.SendLine("CONF {Type "$type$"}");

            }

        }

    }



    // No command

    if (USARRemoteBot(Controller).JointControlIdx==0 &&

        Caps(USARRemoteBot(Controller).SetType)!="JOINT")

        return;

    else

        bNewCommand = True;



    JScount = USARRemoteBot(Controller).JSteer.Length;

    JOcount = USARRemoteBot(Controller).JOrder.Length;

    JVcount = USARRemoteBot(Controller).JValue.Length;



	// Low level DRIVE commands

	for (i=0;i<USARRemoteBot(Controller).JointControlIdx;i++) {

		JIdx = FindJointPartId(USARRemoteBot(Controller).JName[i]);   //Always valid access

		if (JIdx>=0)

        {

			JointsControl[JIdx].state = 1; // new command



			//We are accessing dynamic arrays so we must check their length

			if(i < JScount) JointsControl[JIdx].steer = USARRemoteBot(Controller).JSteer[i];

                else JointsControl[JIdx].steer = 0;



            if(i < JOcount) JointsControl[JIdx].order = USARRemoteBot(Controller).JOrder[i];

                else JointsControl[JIdx].order = 0;



            if(i < JVcount) JointsControl[JIdx].value = USARRemoteBot(Controller).JValue[i];

                else JointsControl[JIdx].value = 0;



			if (JointsControl[JIdx].order==2 && JointsControl[JIdx].value>MaxTorque)

				JointsControl[JIdx].value = MaxTorque;

        }

		if (bDebug)

			log("Input <<"@USARRemoteBot(Controller).JointControlIdx

			        @USARRemoteBot(Controller).JName[i]

			        @USARRemoteBot(Controller).JSteer[i]

			        @USARRemoteBot(Controller).JOrder[i]

			        @USARRemoteBot(Controller).JValue[i]

			        @JIdx@">>");

	}

	if (Caps(USARRemoteBot(Controller).SetType)=="JOINT") {

		JIdx = FindJointPartId(USARRemoteBot(Controller).SetName);

		if (JIdx>=0) {

			JointsControl[JIdx].state = 1;

			if (USARRemoteBot(Controller).Opcode=="0" ||

			    caps(USARRemoteBot(Controller).Opcode)=="ANGLE")

			    JointsControl[JIdx].order = 0;

			else if (USARRemoteBot(Controller).Opcode=="1" ||

			         caps(USARRemoteBot(Controller).Opcode)=="VELOCITY")

			    JointsControl[JIdx].order = 1;

			else if (USARRemoteBot(Controller).Opcode=="2" ||

			         caps(USARRemoteBot(Controller).Opcode)=="TORQUE")

			    JointsControl[JIdx].order = 2;

			else if (USARRemoteBot(Controller).Opcode=="10" ||

			    caps(USARRemoteBot(Controller).Opcode)=="RELANGLE")

			    JointsControl[JIdx].order = 10;

			else if (USARRemoteBot(Controller).Opcode=="11" ||

			         caps(USARRemoteBot(Controller).Opcode)=="RELVELOCITY")

			    JointsControl[JIdx].order = 11;

			else if (USARRemoteBot(Controller).Opcode=="12" ||

			         caps(USARRemoteBot(Controller).Opcode)=="RELTORQUE")

			    JointsControl[JIdx].order = 12;



			i = InStr(USARRemoteBot(Controller).Params,",");

			if( i == -1 ) {

				JointsControl[JIdx].steer = 0;

				if (JointsControl[JIdx].order<2)

					JointsControl[JIdx].value = converter.AngleToUU(float(USARRemoteBot(Controller).Params));

				else

					JointsControl[JIdx].value = float(USARRemoteBot(Controller).Params);

			} else {

				JointsControl[JIdx].steer = converter.AngleToUU(float(mid(USARRemoteBot(Controller).Params,i+1)));

				if (JointsControl[JIdx].order<2)

					JointsControl[JIdx].value = converter.AngleToUU(float(left(USARRemoteBot(Controller).Params,i)));

				else

					JointsControl[JIdx].value = float(left(USARRemoteBot(Controller).Params,i));

			}

			if (JointsControl[JIdx].order==2 && JointsControl[JIdx].value>MaxTorque)

				JointsControl[JIdx].value = MaxTorque;

		}

	}

}



function ResetCarInput()

{

	USARRemoteBot(Controller).SetType="";

	USARRemoteBot(Controller).GeoType="";

	USARRemoteBot(Controller).ConfType="";

	USARRemoteBot(Controller).MPName="";

	USARRemoteBot(Controller).JointControlIdx=0;

}



simulated event SetInitialState()

{

	Super.SetInitialState();

	Enable('Tick');

}



//

// I assume newPos was defined as "var vector newPos;"

//

function MoveRobot(vector newPos)

{

	local vector diff, tmpVec;

	local int i;

	local KTire Tire;

	local KDPart Part;



	KGetRigidBodyState(TeleportLocation);

	diff = newPos - Location;

	TeleportLocation.Position = KRBVecFromVector(newPos);

	TeleportLocation.AngVel = KRBVecFromVector(vect(0,0,0));

	TeleportLocation.LinVel = KRBVecFromVector(vect(0,0,0));

	bNewRobotState = true;



	for (i=0; i<Parts.length; i++)

	{

		if (Parts[i].IsA('KTire'))

		{

			Tire = KTire(Parts[i]);

			Tire.KGetRigidBodyState(Tire.ReceiveState);

			tmpVec = Tire.Location + diff;

			Tire.ReceiveState.Position  =  KRBVecFromVector(tmpVec);



			Tire.ReceiveState.LinVel = KRBVecFromVector(vect(0,0,0));

			Tire.ReceiveState.AngVel = KRBVecFromVector(vect(0,0,0));

			Tire.bReceiveStateNew = true;

		}

		else if (Parts[i].IsA('KDPart'))

		{

			Part = KDPart(Parts[i]);

			Part.KGetRigidBodyState(Part.ReceiveState);

			tmpVec = Part.Location + diff;

			Part.ReceiveState.Position = KRBVecFromVector(tmpVec);

			Part.ReceiveState.LinVel = KRBVecFromVector(vect(0,0,0));

			Part.ReceiveState.AngVel = KRBVecFromVector(vect(0,0,0));

			Part.bReceiveStateNew = true;

		}

	}

}



simulated function Tick(float Delta)

{

	local int i;

	local float spinAng, difAng, slowAng;

	local KCarWheelJoint WheelJ;

	local KDHinge HingeJ;

	local Tracing T;

	local vector loc;

	local float time,time_now;

	local MisPkgLinkInfo aMisPkgLink;

	local string outstring;

	local array<string> CamStatus;



	// for demo play

	if (Controller==None) return;



	// Initialize the CamStatus array

	while(CamStatus.Length != Cameras.Length)

	{

		CamStatus.Insert(CamStatus.Length,1);

		CamStatus[CamStatus.Length-1] = "OK";

	}





	Super.Tick(Delta);



	time = Level.TimeSeconds;

	loc = Location;



        //If we are flipping than wake up Karma

	if( FlipTimeLeft > 0  )

		KWake();



        // Set the viewports and send the viewport response message when appropriate

        if(!USARRemoteBot(Controller).emptyBattery) // If the battery is not empty

        {

            if((USARRemoteBot(Controller).ViewportUpdate & 0x01) == 0x01) // If we need to set a new viewport configuration

                viewportMode = USARRemoteBot(Controller).ViewportMode;



            if((USARRemoteBot(Controller).ViewportUpdate & 0x02) == 0x02) // If we need to set a new camera for viewport 1

                CamsToView[0] = USARRemoteBot(Controller).CamerasToView[0];



            if((USARRemoteBot(Controller).ViewportUpdate & 0x04) == 0x04) // If we need to set a new camera for viewport 2

                CamsToView[1] = USARRemoteBot(Controller).CamerasToView[1];



            if((USARRemoteBot(Controller).ViewportUpdate & 0x08) == 0x08) // If we need to set a new camera for viewport 3

                CamsToView[2] = USARRemoteBot(Controller).CamerasToView[2];



            if((USARRemoteBot(Controller).ViewportUpdate & 0x10) == 0x10) // If we need to set a new camera for viewport 4

                CamsToView[3] = USARRemoteBot(Controller).CamerasToView[3];



            // Send a response message about the viewports

            if(Caps(USARRemoteBot(Controller).SetType)=="VIEWPORTS")

            {

                outstring = "RES {Time "$Level.TimeSeconds$"} {Type Viewports}";



                // Get the viewport configuration

                if(viewportMode == 0)

                    outstring = outstring @ "{Config SingleView}";

                else if(viewportMode == 1)

                    outstring = outstring @ "{Config QuadView}";

                outstring = outstring @ "{Status " $ USARRemoteBot(Controller).ViewportStatus[0] $ "}";



                // Get the camera name assigned to viewport 1

                if(CamsToView[0] == -2)

                    outstring = outstring @ "{Viewport1 Disabled}";

                else if(CamsToView[0] == -1)

                    outstring = outstring @ "{Viewport1 None}";

                else

                    outstring = outstring @ "{Viewport1 " $ Cameras[CamsToView[0]].ItemName $ "}";

                outstring = outstring @ "{Status " $ USARRemoteBot(Controller).ViewportStatus[1] $ "}";



                // Get the camera name assigned to viewport 2

                if(CamsToView[1] == -2)

                    outstring = outstring @ "{Viewport2 Disabled}";

                else if(CamsToView[1] == -1)

                    outstring = outstring @ "{Viewport2 None}";

                else

                    outstring = outstring @ "{Viewport2 " $ Cameras[CamsToView[1]].ItemName $ "}";

                outstring = outstring @ "{Status " $ USARRemoteBot(Controller).ViewportStatus[2] $ "}";



                // Get the camera name assigned to viewport 3

                if(CamsToView[2] == -2)

                    outstring = outstring @ "{Viewport3 Disabled}";

                else if(CamsToView[2] == -1)

                    outstring = outstring @ "{Viewport3 None}";

                else

                    outstring = outstring @ "{Viewport3 " $ Cameras[CamsToView[2]].ItemName $ "}";

                outstring = outstring @ "{Status " $ USARRemoteBot(Controller).ViewportStatus[3] $ "}";



                // Get the camera name assigned to viewport 4

                if(CamsToView[3] == -2)

                    outstring = outstring @ "{Viewport4 Disabled}";

                else if(CamsToView[3] == -1)

                    outstring = outstring @ "{Viewport4 None}";

                else

                    outstring = outstring @ "{Viewport4 " $ Cameras[CamsToView[3]].ItemName $ "}";

                outstring = outstring @ "{Status " $ USARRemoteBot(Controller).ViewportStatus[4] $ "}";



                USARRemoteBot(Controller).myConnection.SendLine(outstring);

            }

        }



        // SERVER Only processing.

        if(Role == ROLE_Authority)

	{

		ProcessCarInput();    // Process input.

		ResetCarInput();

		PackState();          // Pack updated car info into replication structure.



        // Create a trace, if needed.

        // (spawn will be replicated on the clients by the engine)

        if((USARRemoteBot(Controller).LeaveTrace) && (VSize(loc-previousLocation) > 7) && ((time - previousTime) >= USARRemoteBot(Controller).TraceInterval))

        {

            previousTime = time;

            T = spawn(class'USARBot.Tracing');

            T.Texture = USARRemoteBot(Controller).TraceTexture;



            // Set the tag for this point (so that we can have different traces)

            tmpName = '';

            SetPropertyText("tmpName", "Trace_" $ USARRemoteBot(Controller).TraceNumber);

            T.Tag = tmpName;



            previousLocation = T.Location;

        }



        if (RobotName=="")

        {

            RobotName = USARRemoteBot(Controller).RobotName;

            log("MyName"@RobotName);

        }





	}



        // Battery: If battery low then set velocity to 0 (this will brake every joint, see later...)

	if (myLife>=batteryLife)

            KillBattery();

	else

	    myLife = Level.TimeSeconds - startTime;



    //Check every joint for new commands or for still executing ones.

    //Every command requires only one tick for execution at exception of

    //KDCarWheelJoint order = 0 that requires many ticks.

	for (i=0;i<JointsControl.length;i++)

    {

		if (JointsControl[i].state==0) continue;



		if (Joints[i].IsA('KCarWheelJoint'))

        {

            WheelJ = KCarWheelJoint(Joints[i]);

			//log(">>"@i@JointsControl[i].state@JointsControl[i].order@JointsControl[i].value@JointsControl[i].steer);

			//log("    "@WheelJ.KMotorTorque@WheelJ.KMaxSpeed@WheelJ.KBraking);



            // Drive

            ///////////

            // Brake

            if (JointsControl[i].order==3) {

                WheelJ.KBraking = JointsControl[i].value;

                WheelJ.KMotorTorque = 0.0;

            }

            // Torque

            else if (JointsControl[i].order==2) {

                WheelJ.KMotorTorque = JointsControl[i].value;

                WheelJ.KMaxSpeed = 1310700;

                if (JointsControl[i].value!=0)

                    WheelJ.KBraking = 0.0;

                else

                    WheelJ.KBraking = JointParts[i].BrakeTorque + 0.5;

            }

            // Speed

            else if (JointsControl[i].order==1) {

                WheelJ.KMaxSpeed = JointsControl[i].value;

                if (JointsControl[i].value!=0) {

                    WheelJ.KBraking = 0.0;

                    WheelJ.KMotorTorque = MotorTorque;

                } else {

                    WheelJ.KBraking = JointParts[i].BrakeTorque + 500; // 0.5

                    WheelJ.KMotorTorque = 0.0;

                }

            }

            // Angle

            else if (JointsControl[i].order==0) {

                JointsControl[i].order=10;

                JointsControl[i].value=JointsControl[i].value-getJointAngle(WheelJ);

            }

            if (JointsControl[i].order==10 && JointsControl[i].value!=0.0) {

                spinAng = getJointAngle(WheelJ);

                slowAng = Delta*uuMotorSpeed*1.5;



                if (JointsControl[i].state == 1) {

                    JointsControl[i].startAng = spinAng;

                    JointsControl[i].angle = 0.0;

                    difAng = 0.0;

                    WheelJ.KBraking = 0.0;

                    WheelJ.KMotorTorque = MotorTorque;

                    if (JointsControl[i].value>0) {

                        if (JointsControl[i].value<=slowAng)

                            WheelJ.KMaxSpeed = 500;

                        else

                            WheelJ.KMaxSpeed = uuMotorSpeed;

                    }

                    else {

                        if (JointsControl[i].value>=-slowAng)

                            WheelJ.KMaxSpeed = -500;

                        else

                            WheelJ.KMaxSpeed = -uuMotorSpeed;

                    }

                    JointsControl[i].state = 254; // -2

                }

                else {

                    difAng = spinAng - JointsControl[i].startAng;

                    if (difAng>32768) difAng -= 65535;

                    else if (difAng<-32768) difAng += 65535;

                    JointsControl[i].angle += difAng;

                    JointsControl[i].startAng = spinAng;



                    if (JointsControl[i].value>0 && JointsControl[i].angle>=JointsControl[i].value-slowAng)

                        if ((JointsControl[i].value-JointsControl[i].angle)<3000*Delta)

                            WheelJ.KMaxSpeed = 500;

                        else

                            WheelJ.KMaxSpeed = 1500;

                    if (JointsControl[i].value<0 && JointsControl[i].angle<=JointsControl[i].value+slowAng)

                        if ((JointsControl[i].value-JointsControl[i].angle)>-3000*Delta)

                            WheelJ.KMaxSpeed = -500;

                        else

                            WheelJ.KMaxSpeed = -1500;

                    if ( (JointsControl[i].value>0 && JointsControl[i].angle>=JointsControl[i].value) ||

                         (JointsControl[i].value<0 && JointsControl[i].angle<=JointsControl[i].value) ) {

                        WheelJ.KMotorTorque = 0.0;

                        WheelJ.KBraking = 1.0;

                        WheelJ.KMaxSpeed = 0.0;

                        JointsControl[i].value = 0.0;

                        JointsControl[i].state = 0;

                    }

                }

            //log("<<"@i@JointsControl[i].state@JointsControl[i].order@JointsControl[i].value);

            //log("    "@KCarWheelJoint(Joints[i]).KMotorTorque@KCarWheelJoint(Joints[i]).KMaxSpeed@KCarWheelJoint(Joints[i]).KBraking);

            //log("Order_0"@spinAng@JointsControl[i].startAng@"<-->"@difAng@JointsControl[i].angle@JointsControl[i].value@JointsControl[i].state@Parts[i].Rotation);

                if (bDebug)

                    log("Order_0"@i@spinAng@JointsControl[i].startAng@"<-->"@difAng@JointsControl[i].angle@JointsControl[i].value@JointsControl[i].state);

            }



            // Steer

            if (JointsControl[i].steer!=65535)

                WheelJ.KSteerAngle = JointsControl[i].steer;



            if (JointsControl[i].order%10>0) JointsControl[i].state = 0;



            WheelJ.KUpdateConstraintParams();



            if (bDebug)

                log("Tick <<"@i@WheelJ.KSteerAngle@WheelJ.KMotorTorque@WheelJ.KMaxSpeed@WheelJ.KBraking@">>");

        } // End KCarWheelJoint

        else if(Joints[i].IsA('KDHinge'))

        {

            HingeJ = KDHinge(Joints[i]);

            aMisPkgLink = getMisPkgLinkInfo(getLinkNumber(string(JointParts[i].PartName)), getMisPkgName(string(JointParts[i].PartName)));

            HingeJ.KDesiredAngle = 0; // This is set to 0 to simplify the next relative/absolute ANGLE switch().



            if(Joints[i].IsA('KSlider'))

                KSlider(Joints[i]).setDistance(0);



            switch(JointsControl[i].order)

            {

               case 10: // Drive: relative ANGLE

                   if(Joints[i].IsA('KSlider'))

                       KSlider(Joints[i]).setDistance(KSlider(Joints[i]).getDistance());

                   else

                       HingeJ.KDesiredAngle = HingeJ.KCurrentAngle; //This will transform the absolute angle in relative.



               case  0: // Drive: absolute ANGLE

                    HingeJ.KHingeType = HT_Controlled;



                    if(aMisPkgLink != None) // If the joint we are trying to rotate is part of a mission package

                    {

                        HingeJ.KMaxTorque = aMisPkgLink.MaxTorque;

                        HingeJ.KDesiredAngVel = Converter.SpinSpeedToUU(aMisPkgLink.MaxSpeed);



                        if(Joints[i].IsA('KSlider'))

                        {

                            // by default, the value is transfered to angle in UU. Now we need to transfer it to length in UU.

                            JointsControl[i].value = converter.LengthToUU(JointsControl[i].value*PI/32768);

                            KSlider(Joints[i]).setDistance(KSlider(Joints[i]).DesiredDistance + JointsControl[i].value);

                        }

                        else

                        {

                            //-- We make sure that the angle is valid

                            if(aMisPkgLink.MinRange > aMisPkgLink.MaxRange) // If the minimum angle possible is bigger than the maximum angle possible, there is no constraint on the angle

                            {

                                HingeJ.KDesiredAngle -= JointsControl[i].value;

                            }

                            else                                            // Otherwise, there might be a constraint on the angle, so we have to check for one

                            {

                                // If the desired angle is smaller than the minimum angle possible, we use the minimum angle possible

                                if(JointsControl[i].value < Converter.AngleToUU(aMisPkgLink.MinRange))      HingeJ.KDesiredAngle -= Converter.AngleToUU(aMisPkgLink.MinRange);



                                // If the desired angle is larger than the maximum angle possible, we use the maximum angle possible

                                else if(JointsControl[i].value > Converter.AngleToUU(aMisPkgLink.MaxRange)) HingeJ.KDesiredAngle -= Converter.AngleToUU(aMisPkgLink.MaxRange);



                                // Otherwise, we use the value received from the controller

                                else                                                                        HingeJ.KDesiredAngle -= JointsControl[i].value;

                            }

                            //--

                        }

                    }

                    else if(Joints[i].IsA('KSlider'))

                    {

                        // by default, the value is transfered to angle in UU. Now we need to transfer it to length in UU.

                        JointsControl[i].value = converter.LengthToUU(JointsControl[i].value*PI/32768);

                        KSlider(Joints[i]).DesiredDistance += JointsControl[i].value;

                        HingeJ.KMaxTorque = MotorTorque;

                        HingeJ.KDesiredAngVel = uuMotorSpeed;

                    }

                    else

                    {

                        HingeJ.KDesiredAngle -= JointsControl[i].value;

                        HingeJ.KMaxTorque = MotorTorque;

                        HingeJ.KDesiredAngVel = uuMotorSpeed;

                    }

                    HingeJ.Update();

                    break;



                case 1: // Drive: ANGLE SPEED



                    if(aMisPkgLink != None) // If the joint we are trying to rotate is part of a mission package

                    {

                        if(Joints[i].IsA('KSlider'))

                        {

                            // by default, the value is transfered to angle in UU. Now we need to transfer it to length in UU.

                            JointsControl[i].value = Converter.LengthToUU(JointsControl[i].value*PI/32768);

                            if(JointsControl[i].value < -Converter.LengthToUU(aMisPkgLink.MaxSpeed))     KSlider(Joints[i]).DesiredSpeed = -Converter.LengthToUU(aMisPkgLink.MaxSpeed);

                            else if(JointsControl[i].value > Converter.LengthToUU(aMisPkgLink.MaxSpeed)) KSlider(Joints[i]).DesiredSpeed = Converter.LengthToUU(aMisPkgLink.MaxSpeed);

                            else                                                                         KSlider(Joints[i]).setSpeed(JointsControl[i].value);

                        }

                        else if(JointsControl[i].value==0) {

                        	HingeJ.KHingeType = HT_Controlled;

                        	HingeJ.KDesiredAngle = HingeJ.KCurrentAngle;

                        	HingeJ.KDesiredAngVel = Converter.SpinSpeedToUU(aMisPkgLink.MaxSpeed);

                        }

                        else if(aMisPkgLink.MinRange > aMisPkgLink.MaxRange)

                        {

                            HingeJ.KHingeType = HT_Motor;



                            // Here, we make sure that the speed does go above (or below) the maximum (or minimum) speed

                            if(JointsControl[i].value < -Converter.SpinSpeedToUU(aMisPkgLink.MaxSpeed))     HingeJ.KDesiredAngVel = Converter.SpinSpeedToUU(aMisPkgLink.MaxSpeed);

                            else if(JointsControl[i].value > Converter.SpinSpeedToUU(aMisPkgLink.MaxSpeed)) HingeJ.KDesiredAngVel = -Converter.SpinSpeedToUU(aMisPkgLink.MaxSpeed);

                            else                                                                            HingeJ.KDesiredAngVel = -JointsControl[i].value;

                        }

                        else

                        {

                            // Since this link has an angle constraint, we have to control this joint (HT_Controlled) and limit its rotation

                            HingeJ.KHingeType = HT_Controlled;



                            // Here, we set the constraint, based on the mission package link's range

                            if(JointsControl[i].value < 0) HingeJ.KDesiredAngle -= Converter.AngleToUU(aMisPkgLink.MinRange);

                            else                           HingeJ.KDesiredAngle -= Converter.AngleToUU(aMisPkgLink.MaxRange);



                            // Here, we make sure that the speed does go above (or below) the maximum (or minimum) speed

                            if((JointsControl[i].value < -Converter.SpinSpeedToUU(aMisPkgLink.MaxSpeed)) ||

                               (JointsControl[i].value > Converter.SpinSpeedToUU(aMisPkgLink.MaxSpeed)))    HingeJ.KDesiredAngVel = Converter.SpinSpeedToUU(aMisPkgLink.MaxSpeed);

                            else                                                                            HingeJ.KDesiredAngVel = Abs(JointsControl[i].value);

                        }



                        HingeJ.KMaxTorque = aMisPkgLink.MaxTorque;

                    }

                    else if(Joints[i].IsA('KSlider'))

                    {

                        // by default, the value is transfered to angle in UU. Now we need to transfer it to length in UU.

                        JointsControl[i].value = converter.LengthToUU(JointsControl[i].value*PI/32768);

                        KSlider(Joints[i]).setSpeed(JointsControl[i].value);

                    }

                    else if (JointsControl[i].value==0) {

                        HingeJ.KHingeType = HT_Controlled;

                        HingeJ.KMaxTorque = MotorTorque;

                        HingeJ.KDesiredAngVel = uuMotorSpeed;

                    }

                    else

                    {

                        HingeJ.KHingeType = HT_Motor;

                        HingeJ.KMaxTorque = MotorTorque;

                        HingeJ.KDesiredAngVel = -JointsControl[i].value;

                    }



                    HingeJ.Update();

                    break;



                case 2: // Drive: TORQUE



                    if(aMisPkgLink != None) // If the joint we are trying to rotate is part of a mission package

                    {

                        if(aMisPkgLink.MinRange > aMisPkgLink.MaxRange)

                        {

                            HingeJ.KHingeType = HT_Motor;



                            // Here, we make sure that the torque does go above (or below) the maximum (or minimum) torque

                            if(JointsControl[i].value < -aMisPkgLink.MaxTorque)     HingeJ.KMaxTorque = aMisPkgLink.MaxTorque;

                            else if(JointsControl[i].value > aMisPkgLink.MaxTorque) HingeJ.KMaxTorque = -aMisPkgLink.MaxTorque;

                            else                                                    HingeJ.KMaxTorque = -JointsControl[i].value;



                            if(JointsControl[i].value > 0)      HingeJ.KDesiredAngVel = converter.SpinSpeedToUU(aMisPkgLink.MaxSpeed);

                            else if(JointsControl[i].value < 0) HingeJ.KDesiredAngVel = -converter.SpinSpeedToUU(aMisPkgLink.MaxSpeed);

                            else                                HingeJ.KDesiredAngVel = 0;

                        }

                        else

                        {

                            // Since this link has an angle constraint, we have to control this joint (HT_Controlled) and limit its rotation

                            HingeJ.KHingeType = HT_Controlled;



                            // Here, we set the constraint, based on the mission package link's range

                            if(JointsControl[i].value < 0) HingeJ.KDesiredAngle -= converter.AngleToUU(aMisPkgLink.MaxRange);

                            else                           HingeJ.KDesiredAngle -= converter.AngleToUU(aMisPkgLink.MinRange);



                            // Here, we make sure that the torque does go above (or below) the maximum (or minimum) torque

                            if(JointsControl[i].value < -aMisPkgLink.MaxTorque)     HingeJ.KMaxTorque = aMisPkgLink.MaxTorque;

                            else if(JointsControl[i].value > aMisPkgLink.MaxTorque) HingeJ.KMaxTorque = -aMisPkgLink.MaxTorque;

                            else                                                    HingeJ.KMaxTorque = -JointsControl[i].value;



                            HingeJ.KDesiredAngVel = converter.SpinSpeedToUU(aMisPkgLink.MaxSpeed);

                        }

                    }

                    else

                    {

                        HingeJ.KHingeType = HT_Motor;

                        HingeJ.KMaxTorque = -JointsControl[i].value;

                        if(JointsControl[i].value > 0)

                            HingeJ.KDesiredAngVel = 327680;  //5 turns/sec

                        else

                            HingeJ.KDesiredAngVel = -327680; //5 turns/sec

                    }



                    HingeJ.Update();

                    break;

			}

			JointsControl[i].state = 0;

		} // End KDHinge

	} // End for



    // if there are new commands but Karma is on rest awake it

	if (bNewCommand && !KIsAwake())

        KWake();



	// other control commands from GameBot

	if (Role == ROLE_Authority)

        {

	    bHeadlightOn=USARRemoteBot(Controller).Light;

	    if (USARRemoteBot(Controller).Flip) StartFlip(None);



            // Set the camera FOVs

            if(!USARRemoteBot(Controller).emptyBattery) // If the battery is not empty, we can send the message

            {

                for(i=0; i<USARRemoteBot(Controller).myCameraZoom.Length; i++)

                {

                    if (USARRemoteBot(Controller).myCameraZoom[i] >=0 && myCamera!=NONE)

                    {

                        // Get the status for the response message

                        if ((USARRemoteBot(Controller).myCameraZoom[i]==0) || (USARRemoteBot(Controller).myCameraZoom[i]<=CamList[i].degCameraMinFov) || (USARRemoteBot(Controller).myCameraZoom[i]>CamList[i].degCameraMaxFov)) CamStatus[i] = "Failed";



                        if (USARRemoteBot(Controller).myCameraZoom[i]==0)

                            USARRemoteBot(Controller).myCameraZoom[i] = CamList[i].degCameraDefFov; // reset

                        else if (USARRemoteBot(Controller).myCameraZoom[i]<=CamList[i].degCameraMinFov)

                            USARRemoteBot(Controller).myCameraZoom[i] = CamList[i].degCameraMinFov; // low bound

                        else if (USARRemoteBot(Controller).myCameraZoom[i]>CamList[i].degCameraMaxFov)

                            USARRemoteBot(Controller).myCameraZoom[i] = CamList[i].degCameraMaxFov; // upper bound



                        if(i==0)

                            myCamera.setFov(USARRemoteBot(Controller).myCameraZoom[i]);



                        CamList[i].setFov(USARRemoteBot(Controller).myCameraZoom[i]);

                        USARRemoteBot(Controller).myCameraZoom[i] = -1;

                    }

                }



                // Send response message, if applicable

                if(USARRemoteBot(Controller).CamResponse)

                {

                    outstring = "RES {Time "$Level.TimeSeconds$"} {Type Camera}";



                    for(i=0; i<USARRemoteBot(Controller).myCameraZoom.Length; i++)

                    {

                        outstring = outstring @ "{Name " $ Cameras[i].ItemName $ "}";

                        outstring = outstring @ "{FOV " $ Converter.Str_AngleFromDeg(CamList[i].getFov()) $ "}";

                        outstring = outstring @ "{Status " $ CamStatus[i] $ "}";

                    }



                    USARRemoteBot(Controller).myConnection.SendLine(outstring);

                    USARRemoteBot(Controller).CamResponse = false;

                }

            }

	}



	// Projecting effect

	for(i=0; i<HeadlightList.length; i++)

	{

		if(Level.NetMode != NM_DedicatedServer && HeadlightList[i]!=None) {

			HeadlightList[i].DetachProjector();

			if(bHeadlightOn) {HeadlightList[i].AttachProjector(); HeadlightList[i].bLightActive= true;}

			else HeadlightList[i].bLightActive= false;

		}

	}



	// Flipping

	if(FlipTimeLeft > 0) {

		FlipTimeLeft -= Delta;

		FlipTimeLeft = FMax(FlipTimeLeft, 0.0); // Make sure it doesn't go negative

	}



    // Logging of the position and pos

    // NOTE (marco.zaratti@gmail.com):

    //   If you active these lines the AIBO replication will be very bad

    //   in client-server simulation (at least on my PC). I don't know why, perhaps

    //   it happens only on my PC.



   	time_now = Level.TimeSeconds;

   	if(time_now - old_time > logging_period) {

       	//logging happens here

       	// Time + Name + Position + Orientation

       	time = Level.TimeSeconds;

       	loc = Location;

       //fposlog(time@USARRemoteBot(Controller).RobotName@Converter.Str_LengthVectorFromUU(loc)@Converter.Str_RotatorFromUU(Rotation));

	fposlog(time@USARRemoteBot(Controller).RobotName@Converter.LengthFromUU(loc.X)@Converter.LengthFromUU(loc.Y)@Converter.LengthFromUU(loc.Z)@Converter.AngleFromUU(Rotation.Roll)@Converter.AngleFromUU(Rotation.Pitch)@Converter.AngleFromUU(Rotation.Yaw));

       	old_time = time_now;

   	}



	//Synchronizes MultiView camera coordinates

	SyncMultiView();



	return;

}



simulated function SyncMultiView()

{

	if(ViewManager != none)

	{

	   ViewManager.UpdateView(ViewNum, myCamera.Location, myCamera.Rotation, CamList[0].CameraFov);

	}

}



function timer()

{

    local string outstring;

    local int i,j;

    local float time;

    local MisPkgLinkInfo aMisPkgLink;



    time = Level.TimeSeconds;



    // STA message moved to the KRobot subcalsses



    outstring = "";



    // Sensor State Message

    if(!USARRemoteBot(Controller).emptyBattery) // If the battery is not empty, we can send the message

    {

        for (i=0;i<SensorList.length;i++) {

            if (ProcessedSensors[i]==0) {

                if (SensorList[i].bUseGroup)

                    outstring = getGroupData(SensorList[i].ItemType);

                else

                    outstring = SensorList[i].GetData();

                if (outstring != "")

                    USARRemoteBot(Controller).myConnection.SendLine(SensorList[i].GetHead()@outstring);

            }

        }

    }



    for (i=0;i<SensorList.length;i++)

        ProcessedSensors[i] = 0;



    // Mission Package State Message

    if(!USARRemoteBot(Controller).emptyBattery) // If the battery is not empty, we can send the message

    {

        for (i=0;i<MisPkgs.length;i++)

        {

            outstring = "{Time " $ Level.TimeSeconds $ "} {Name " $ MisPkgs[i].PkgName $ "}";



            for(j=0; j<JointParts.Length; j++)

            {

                aMisPkgLink = getMisPkgLinkInfo(getLinkNumber(string(JointParts[j].PartName)), string(MisPkgs[i].PkgName));

                if((getLinkNumber(string(JointParts[j].PartName)) > 0) && (getMisPkgName(string(JointParts[j].PartName)) == string(MisPkgs[i].PkgName)) && (aMisPkgLink != None))

                {

                    outstring = outstring @ "{Link " $ getLinkNumber(string(JointParts[j].PartName)) $ "}";



                    if (Joints[j].IsA('KSlider'))

                        outstring = outstring @ "{Value " $ Converter.Str_LengthFromUU(KSlider(Joints[j]).getDistance()) $ "} {Torque " $ -KSlider(Joints[j]).KMaxTorque $ "}";

                    else if(Joints[j].IsA('KDHinge'))

                        outstring = outstring @ "{Value " $ Converter.Str_AngleFromUU(-KHinge(Joints[j]).KCurrentAngle) $ "} {Torque " $ -KHinge(Joints[j]).KMaxTorque $ "}";

                }

            }



            USARRemoteBot(Controller).myConnection.SendLine("MISSTA"@outstring);

        }

    }

}



function String getGroupData(String type) {

	local String outstring;

	local int i;



	outstring = "";

	for (i=0;i<SensorList.length;i++) {

		if (SensorList[i].ItemType == type) {

			if (outstring=="")

				outstring = SensorList[i].GetData();

			else

			outstring = outstring@SensorList[i].GetData();

			ProcessedSensors[i] = 1;

		}

	}

	return outstring;

}



/*

function flog(string x) {

    local string filename;

    filename = USARRemoteBot(Controller).RobotName$"-collisions";



    if(CollLog == None) {

        CollLog = spawn(class'FileLog');

        if(CollLog != None) {

            CollLog.OpenLog(filename);

            CollLog.Logf(x);

        } else {

            log(x);

        }

    } else {

        CollLog.Logf(x);

    }

}

*/



function KillBattery()

{

    local int i;



    myLife=batteryLife;



    for (i=0;i<JointsControl.length;i++)

    {

        USARRemoteBot(Controller).deadBattery();

        JointsControl[i].order=1;

        JointsControl[i].value=0;

    }

}



function fposlog(string x) {

    local string filename;

    filename = USARRemoteBot(Controller).RobotName$"-positions";



    //UsarGame = USARDeathMatch(Level.Game);



    if(PosLog == None) {

        PosLog = spawn(class'FileLog');

        if(PosLog != None) {

            PosLog.OpenLog(filename);

            PosLog.Logf(x);

        } else {

            log(x);

        }

    } else {

        PosLog.Logf(x);

    }

}



//*********************************************************************************************************************

// PhysicsVolumeChange Event

// -------------------------

//     Event called whenever the robot's Center of Gravity touches a new physics volume.

//     If the robot's Center of Gravity touches a water volume (i.e. the robot is in the water), its battery is killed.

//*********************************************************************************************************************

simulated event PhysicsVolumeChange( PhysicsVolume NewVolume )

{

    if(!self.IsA('UnderwaterRobot'))
    {
        Super.PhysicsVolumeChange(NewVolume);

        if(NewVolume.bWaterVolume)

            KillBattery();
    }

}



/*

function Touch( actor Other )

{

    bump_touch_cnt++;

    if(bstats) {

        flog("was touched by"@Other@"at"@Level.TimeSeconds@bump_touch_cnt@"times");

        Log("KRobot: was touched by"@Other@"at"@Level.TimeSeconds@bump_touch_cnt@"times");

    }

}





function Bump( actor Other )

{

    bump_touch_cnt++;

    if(bstats) {

        flog(bump_touch_cnt@"I was bumped by"@Other@"at"@Level.TimeSeconds);

        Log(bump_touch_cnt@"Krobot: I was bumped by"@Other@"at"@Level.TimeSeconds);

    }

}*/





//*********************************************************************************************************************

// Function that updates the spin speed of the wheel number given by the first parameter.

// Please note that the spinSpeed is in Unreal Units.

//*********************************************************************************************************************

function setSpinSpeed(int wheelNumber, float spinSpeed)

{

    JointsControl[wheelNumber].state = 1;

    JointsControl[wheelNumber].order = 1;

    JointsControl[wheelNumber].value = spinSpeed;

}





//*********************************************************************************************************************

// Function that updates the steer angle of the wheel number given by the first parameter.

// Please note that the steerAngle is in Unreal Units.

//*********************************************************************************************************************

function setAngle(int jointIndex, float angle)

{

    JointsControl[jointIndex].state = 1;



    if(Joints[jointIndex].IsA('KCarWheelJoint'))

    {

        JointsControl[jointIndex].order = 1;

        JointsControl[jointIndex].steer = angle;

    }

    else if(Joints[jointIndex].IsA('KDHinge'))

    {

        JointsControl[jointIndex].order = 0;

        JointsControl[jointIndex].value = angle;

    }

}



defaultproperties

{

//Scaled with 4.762 at Mon Sep 25 14:21:50 EDT 2006

	bDebug=false

	ConverterClass="USARBot.USARConverter"

	bMountByUU=false

	AmbientGlow=48

	bSpecialHUD=True

	bBarCode=False



	//Set the following 2 variables to true if you want shadow

    //from the chassis of the robot:

	bVehicleShadows=False

	bDrawVehicleShadow=False



	MaxTorque=60.0

	SafeForce=1000

	ProtectTime=1.0



   	MotorTorque=20.0

	MotorSpeed=0.1745 // rad per second

	HingePropGap=364.0 //uu



	SteerPropGap=1000.000000

	SteerTorque=1000.000000

	SteerSpeed=15000.000000 // in UU

	SuspStiffness=150.000000

	SuspDamping=15.000000

	SuspHighLimit=1.000000

	SuspLowLimit=-1.000000



	TireRollFriction=15.000000

	TireLateralFriction=15.00000

	TireRollSlip=0.060000

	TireLateralSlip=0.060000

	TireMinSlip=0.001000

	TireSlipRate=0.000500

	TireSoftness=0.000000

	TireAdhesion=0.000000

	TireRestitution=0.000000



    bBlockActors=true

	bCollideActors=True

	CollisionRadius=25

	CollisionHeight=10



	ChassisMass=1.000000



	FlipTorque=350.000000

	FlipTime=3.000000

	MaxNetUpdateInterval=0.05



	HitSoundThreshold=30.000000



	batteryLife=1200 // 20 minutes of battery life

	msgTimer=0.200000



    logging_period=1



    CamsToView[0] = -2;

    CamsToView[1] = -2;

    CamsToView[2] = -2;

    CamsToView[3] = -2;

    bDrawHud = true;



    DrawType=DT_StaticMesh

	StaticMesh=StaticMesh'USARSim_VehicleParts_Meshes.Test.TestBody'

	DrawScale=4.762

    Begin Object Class=KarmaParamsRBFull Name=KParams0

        KStartEnabled=True

        bHighDetailOnly=False

        bClientOnly=False

        bKDoubleTickRate=True

        bKNonSphericalInertia=True

    	KInertiaTensor(0)=0.4`

        KInertiaTensor(3)=0.4

        KInertiaTensor(5)=0.4

        KMaxAngularSpeed=100

        KMaxSpeed=25000

        KLinearDamping=0.0

        KAngularDamping=0.0

        KFriction=0.5

        Name="KParams0"

    End Object

    KParams=KarmaParamsRBFull'USARBot.KRobot.KParams0'

}

