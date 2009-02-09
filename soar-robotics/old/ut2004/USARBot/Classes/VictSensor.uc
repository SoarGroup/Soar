#exec OBJ LOAD FILE=..\Textures\USARSim_Objects_Textures.utx

class VictSensor extends sensor config(USARBot);

// Configuration Variables
var config float Distance;
var config float HorizontalFOV;
var config float HorizontalStep;
var config float VerticalFOV;
var config float VerticalStep;
var config bool bShowResults;

// Programming Variables
var array<Tracing> Points;
var Texture HitVictim;
var Texture HitFalseAlarm;
var Texture HitNonVictim;
var Texture Miss;
var int timeStep;

// Structures
struct VictPartInfo
{
    var string segName;          // Segment Name (same as bone name)
    var string partName;         // Part Name to be displayed in the sensor message
    var vector hitLocation;      // Part Centroid
    var bool centroidCalculated; // The hitLocation centroid will be of AT MOST two parts. This tells us if we have already calculated the centroid.
};

struct VictInfo
{
    // Unique name assigned by UnrealEd (this is how we differentiate between multiple victims)
    var name utName;

    // Information about the victim
    var array<VictPartInfo> VictPartsInfo;
};

var array<VictInfo> VictsInfo;

/**************************************************************************************************************
    Function called by KRobot to get the information, from the victim's sensor, out through the interface.
**************************************************************************************************************/
function String GetConfData()
{
	local string outstring;
	outstring = Super.GetConfData();
	outstring = outstring@"{MaxRange "$converter.FloatString(Distance)$"} {HorizontalFOV "$converter.FloatString(HorizontalFOV)$"} {VerticalFOV "$converter.FloatString(VerticalFOV)$"}";
	return outstring;
}

/**************************************************************************************************************
    This is the core function of the victim sensor. It sends traces following a different pattern every
  time step and stores various information about the hit into the data structures. The function goes through
  4 trace patterns and sends out the information collected during all the steps at the end of step #4. 
  Since each time step is separated by 0.2s, the victim sensor sends out data once every 0.8s (rate of 1.25Hz).
**************************************************************************************************************/
function String GetData()
{
    local Actor ActorHit;
    local Texture PointTexture;
    local vector TraceEnd, HitLocation, relHitLocation, HitNormal;
    local rotator Turn;
    local float Horizontal, iniHorizontal, Vertical, iniVertical, hitDistance, tmp;
    local int VictIndex, i, j;
    local string Outstring;
    local array<name> UnrecordedFalseAlarms;
    local array<name> RecordedSedans;

    Outstring = "";
    timeStep++;

    if(bShowResults)
        RemovePoints();    // Remove the points from the world, if applicable

    // Initialize the trace pattern
    if(timeStep==1)
    {
        iniHorizontal = (HorizontalFOV/2);
        iniVertical = (VerticalFOV/2);
    }
    else if(timeStep==2)
    {
        iniHorizontal = (HorizontalFOV/2) - (HorizontalStep/2);
        iniVertical = (VerticalFOV/2);
    }
    else if(timeStep==3)
    {
        iniHorizontal = (HorizontalFOV/2) - (HorizontalStep/2);
        iniVertical = (VerticalFOV/2) - (VerticalStep/2);
    }
    else if(timeStep==4)
    {
        iniHorizontal = (HorizontalFOV/2);
        iniVertical = (VerticalFOV/2) - (VerticalStep/2);
    }

    // Here, we generate the traces based on the configuration variables
    for(Horizontal = iniHorizontal; Horizontal >= -(HorizontalFOV/2); Horizontal -= HorizontalStep)
    {
        for(Vertical = iniVertical; Vertical >= -(VerticalFOV/2); Vertical -= VerticalStep)
        {
            // Calculate the end point of the trace and send it out
            Turn.Yaw = Converter.AngleToUU(Horizontal);
            Turn.Pitch = Converter.AngleToUU(Vertical);
            TraceEnd = Location + Converter.LengthToUU(Distance)*vector(Utils.rTurn(Rotation,turn));

            // Get the actor hit and the location of the hit (location of the hit is TraceEnd if nothing was hit)
	    ActorHit = Trace(HitLocation, HitNormal, TraceEnd, Location, true);
            if(ActorHit == None)
                HitLocation = TraceEnd;

            // Here, we populate the Victim's array, which will be used to calculate the report level
            if(ActorHit != None)
            {
                // If we have hit a victim
                if(ClassIsChildOf(ActorHit.Class,class'VicColActor'))
	        {
	            PointTexture = HitVictim;

                    VictIndex = FindVictim(VictsInfo, VictimPawn(VicColActor(ActorHit).Owner).Name);

                    // If this hit landed on a victim that is not in our array of victims
                    if(VictIndex == -1)
                    {
                        VictsInfo.Insert(VictsInfo.Length, 1); // Add space for a new entry
                        VictsInfo[VictsInfo.Length-1].utName = VictimPawn(VicColActor(ActorHit).Owner).Name; // Save Victim Name (unique for each victims)
                        VictIndex = VictsInfo.Length-1;
                    }

                    PopulateVictPartInfo(ActorHit, VictIndex); // Save Victim Hit Information
                }
                else if(ClassIsChildOf(ActorHit.Class,class'Sedan'))
	        {
	            PointTexture = HitNonVictim;
	            
	            if ( ! AlarmInArray( RecordedSedans, ActorHit.Name ) )
	            {
		        //Log("recording sedan detection");

                        RecordedSedans.Insert( RecordedSedans.Length, 1 );
                        RecordedSedans[ RecordedSedans.Length - 1 ] = ActorHit.Name;
                        
	            	VictIndex = FindVictim( VictsInfo, ActorHit.Name );

	            	// If this hit is not in our array of victims
	            	if ( VictIndex == -1 )
	            	{
	            	    VictsInfo.Insert( VictsInfo.Length, 1 ); // Add space for a new entry
	            	    VictsInfo[ VictsInfo.Length - 1 ].utName = ActorHit.Name; // Save Victim Name (unique for each victims)
	            	    VictIndex = VictsInfo.Length - 1;
	            	    PopulateVictPartInfo(ActorHit, VictIndex); // Save Victim Hit Information
	            	}

	            }
                }
                // If we have hit a false alarm volume (we treat it as if it were a real victim hit)
                else if(ClassIsChildOf(ActorHit.Class,class'FalseAlarmVolume'))
                {
                    relHitLocation = utils.getRelativePosition(ActorHit.Location, Location, Rotation);
                    hitDistance = Sqrt((relHitLocation.x * relHitLocation.x) + (relHitLocation.y * relHitLocation.y) + (relHitLocation.z * relHitLocation.z));
                    tmp = (utils.gaussRand(Mean,Sigma)-Mean);
                    
                    //Log("hitDistance * tmp=" $ hitDistance * tmp);

                    if (((hitDistance * tmp) <= (Sigma*Distance/3)) && ((hitDistance * tmp) >= (-(Sigma*Distance/3))))
                    {
                        PointTexture = HitNonVictim;

                        if(!AlarmInArray(UnrecordedFalseAlarms, ActorHit.Name))
                        {
                            UnrecordedFalseAlarms.Insert(UnrecordedFalseAlarms.Length, 1);
                            UnrecordedFalseAlarms[UnrecordedFalseAlarms.Length-1] = ActorHit.Name;
                        }
                    }
                    else
                    {
                        // We only record this false alarm if it has not been flagged as "unrecorded"
                        if(!AlarmInArray(UnrecordedFalseAlarms, ActorHit.Name))
                        {
                            VictIndex = FindVictim(VictsInfo, ActorHit.Name);

                            // If this hit landed on a false alarm that is not in our array of victims
                            if(VictIndex == -1)
                            {
                                VictsInfo.Insert(VictsInfo.Length, 1); // Add space for a new entry
                                VictsInfo[VictsInfo.Length-1].utName = ActorHit.Name; // Save False Alarm Name (unique for each victims)
                                PopulateVictPartInfo(ActorHit, VictsInfo.Length-1); // Save Hit Information
                            }
                        }
                        else
                        {
                            PointTexture = HitNonVictim;
                        }
                    }
                }
                // If we have hit something that is neither a victim nor a false alarm
                else
                {
                    PointTexture = HitNonVictim;
                }
            }
            // Here when we have hit nothing
            else
                PointTexture = Miss;

            // Display where the traces hit if bShowResults is set to true
            if(bShowResults)
                AddPoint(HitLocation, PointTexture);
        }
    }

    // When we reach the last time step, we need to generate the Outstring
    if(timeStep == 4)
    {
        // Get the appropriate message string, depending on the VictsInfoArray
        for(i=0; i<VictsInfo.Length; i++)
        {
            for(j=0; j<VictsInfo[i].VictPartsInfo.Length; j++)
            {
                if(Outstring == "")
                    Outstring = "{Name "$ItemName$"} " $
                                "{PartName " $ VictsInfo[i].VictPartsInfo[j].partName $ "} " $
                                "{Location " $ VictsInfo[i].VictPartsInfo[j].hitLocation $ "}";
                else
                    Outstring = Outstring $ " {PartName " $ VictsInfo[i].VictPartsInfo[j].partName $ "}" $
                                            " {Location " $ VictsInfo[i].VictPartsInfo[j].hitLocation $ "}";
            }
        }

        // Get everything ready for the next time step
        timeStep = 0;          // Reset the time step
        EmptyVictsInfoArray(); // Empty the victim's info array

        if(Outstring == "")
            Outstring = "{Status NoVictims}";
        
	//Log("Vict outstring: " $ Outstring);
    }
    
    // Clean up the array of unrecorded alarms
    while(UnrecordedFalseAlarms.Length != 0)
        UnrecordedFalseAlarms.Remove(UnrecordedFalseAlarms.Length-1, 1);

    while ( RecordedSedans.Length != 0 )
        RecordedSedans.Remove( RecordedSedans.Length - 1, 1 );

    return Outstring;
}

function bool AlarmInArray(array<Name>UnrecordedFalseAlarms, Name ActorName)
{
    local int i;
    for(i=0; i<UnrecordedFalseAlarms.Length; i++)
        if(UnrecordedFalseAlarms[i]==ActorName)
            return true;
            
    return false;
}

/**************************************************************************************************************
    This simple function looks through the array of victims to find a victim with the same name as the one
  passed as a parameter. If the victim cannot be found in the array, -1 is returned. Please note that the
  victim's name used are the ones assigned automatically by UnrealEd and are ALWAYS unique. This means that
  two different victims will NEVER have the same victim name.
**************************************************************************************************************/
function int FindVictim(array<VictInfo> VictsInfo, Name ActorName)
{
    local int i;

    for(i=0; i<VictsInfo.Length; i++)
    {
        if(VictsInfo[i].utName == ActorName)
            return i;
    }

    // We get here if we have not found the victim
    return -1;
}

/**************************************************************************************************************
    This function populates the victim array with the various hit information. The generic part name
  and the hit location are of great importance since they will be sent through the interface.
**************************************************************************************************************/
function PopulateVictPartInfo(Actor ActorHit, int VictIndex)
{
    local string segName;     // Segment Name (same as bone name)
    local string partName;    // Part Name to be displayed in the sensor message
    local int i;

    // If we are dealing with a victim
    if(ClassIsChildOf(ActorHit.Class,class'VicColActor'))
    {
        segName = VicColActor(ActorHit).boneName;
        partName = getGenericPartName(segName);

        // Go through the array of parts to see if it has already been hit by a trace
        for(i = 0; i<VictsInfo[VictIndex].VictPartsInfo.Length; i++)
        {
            if(HitAlreadyIndexed(VictIndex, i, segName, ActorHit))
                return;
        }

        // Here when this is the first trace that hits this part
        VictsInfo[VictIndex].VictPartsInfo.Insert(VictsInfo[VictIndex].VictPartsInfo.Length, 1); // Add an entry to the dynamic array
        VictsInfo[VictIndex].VictPartsInfo[VictsInfo[VictIndex].VictPartsInfo.Length-1].segName = segName;
        VictsInfo[VictIndex].VictPartsInfo[VictsInfo[VictIndex].VictPartsInfo.Length-1].partName = partName;
        VictsInfo[VictIndex].VictPartsInfo[VictsInfo[VictIndex].VictPartsInfo.Length-1].hitLocation = AddError(utils.getRelativePosition(ActorHit.Location, Location, Rotation));
    }
    // If we are dealing with a false alarm
    else if(ClassIsChildOf(ActorHit.Class,class'FalseAlarmVolume'))
    {
        // Here when this is the first trace that hits this part
        VictsInfo[VictIndex].VictPartsInfo.Insert(VictsInfo[VictIndex].VictPartsInfo.Length, 1); // Add an entry to the dynamic array
        VictsInfo[VictIndex].VictPartsInfo[VictsInfo[VictIndex].VictPartsInfo.Length-1].partName = getGenericPartName(string(FalseAlarmVolume(ActorHit).FalseAlarmType));
        VictsInfo[VictIndex].VictPartsInfo[VictsInfo[VictIndex].VictPartsInfo.Length-1].hitLocation = AddError(utils.getRelativePosition(ActorHit.Location, Location, Rotation));
    }
    else if(ClassIsChildOf(ActorHit.Class,class'Sedan'))
    {
        // Here when this is the first trace that hits this part
        VictsInfo[VictIndex].VictPartsInfo.Insert(VictsInfo[VictIndex].VictPartsInfo.Length, 1); // Add an entry to the dynamic array
        VictsInfo[VictIndex].VictPartsInfo[VictsInfo[VictIndex].VictPartsInfo.Length-1].partName = "Head";
        VictsInfo[VictIndex].VictPartsInfo[VictsInfo[VictIndex].VictPartsInfo.Length-1].hitLocation = utils.getRelativePosition(ActorHit.Location, Location, Rotation);
    }

    return;
}

/**************************************************************************************************************
    This simple function adds gaussian noise to the hit location.
**************************************************************************************************************/
function vector AddError(vector hitLocation)
{
    local vector result;
    local float hitDistance;

    hitDistance = Sqrt((hitLocation.x * hitLocation.x) + (hitLocation.y * hitLocation.y) + (hitLocation.z * hitLocation.z));
    result.x = hitLocation.x + (hitDistance * utils.gaussRand(Mean,Sigma));
    result.y = hitLocation.y + (hitDistance * utils.gaussRand(Mean,Sigma));
    result.z = hitLocation.z + (hitDistance * utils.gaussRand(Mean,Sigma));
    
    return result;
}

/**************************************************************************************************************
    The HitAlreadyIndexed function returns true when a trace has alread hit the 'segName' and false otherwise.
  Please note that there is no distinction between forearms/upperarms (treated as "arms") and thighs/calves
  (treated as "legs"). In addition, the function calculates the parts' centroid in the case that a
  forearm/upperarm or thigh/calf pair was hit, if not already calculated.
**************************************************************************************************************/
function bool HitAlreadyIndexed(int VictIndex, int i, string segName, Actor ActorHit)
{
    local bool result, needToCalculateCentroid;

    result = false;

    // If the trace hits the exact part that is in the array's entry
    if(VictsInfo[VictIndex].VictPartsInfo[i].segName == segName)
    {
        result = true;
    }
    // If the trace hits a right forearm, and the array's entry is a right upperarm (we do not differentiate between forearms and upperarms)
    else if((segName == "Bip02 R Forearm") && (VictsInfo[VictIndex].VictPartsInfo[i].segName == "Bip02 R UpperArm"))
    {
        needToCalculateCentroid = !VictsInfo[VictIndex].VictPartsInfo[i].centroidCalculated;
        result = true;
    }
    // If the trace hits a left forearm, and the array's entry is a left upperarm (we do not differentiate between forearms and upperarms)
    else if((segName == "Bip02 L Forearm") && (VictsInfo[VictIndex].VictPartsInfo[i].segName == "Bip02 L UpperArm"))
    {
        needToCalculateCentroid = !VictsInfo[VictIndex].VictPartsInfo[i].centroidCalculated;
        result = true;
    }
    // If the trace hits a right upperarm, and the array's entry is a right forearm (we do not differentiate between forearms and upperarms)
    else if((segName == "Bip02 R UpperArm") && (VictsInfo[VictIndex].VictPartsInfo[i].segName == "Bip02 R Forearm"))
    {
        needToCalculateCentroid = !VictsInfo[VictIndex].VictPartsInfo[i].centroidCalculated;
        result = true;
    }
    // If the trace hits a left upperarm, and the array's entry is a left forearm (we do not differentiate between forearms and upperarms)
    else if((segName == "Bip02 L UpperArm") && (VictsInfo[VictIndex].VictPartsInfo[i].segName == "Bip02 L Forearm"))
    {
        needToCalculateCentroid = !VictsInfo[VictIndex].VictPartsInfo[i].centroidCalculated;
        result = true;
    }
    // If the trace hits a right calf, and the array's entry is a right thigh (we do not differentiate between calves and thighs)
    else if((segName == "Bip02 R Calf") && (VictsInfo[VictIndex].VictPartsInfo[i].segName == "Bip02 R Thigh"))
    {
        needToCalculateCentroid = !VictsInfo[VictIndex].VictPartsInfo[i].centroidCalculated;
        result = true;
    }
    // If the trace hits a left calf, and the array's entry is a right thigh (we do not differentiate between calves and thighs)
    else if((segName == "Bip02 L Calf") && (VictsInfo[VictIndex].VictPartsInfo[i].segName == "Bip02 L Thigh"))
    {
        needToCalculateCentroid = !VictsInfo[VictIndex].VictPartsInfo[i].centroidCalculated;
        result = true;
    }
    // If the trace hits a right thigh, and the array's entry is a right calf (we do not differentiate between calves and thighs)
    else if((segName == "Bip02 R Thigh") && (VictsInfo[VictIndex].VictPartsInfo[i].segName == "Bip02 R Calf"))
    {
        needToCalculateCentroid = !VictsInfo[VictIndex].VictPartsInfo[i].centroidCalculated;
        result = true;
    }
    // If the trace hits a left thigh, and the array's entry is a left calf (we do not differentiate between calves and thighs)
    else if((segName == "Bip02 L Thigh") && (VictsInfo[VictIndex].VictPartsInfo[i].segName == "Bip02 L Calf"))
    {
        needToCalculateCentroid = !VictsInfo[VictIndex].VictPartsInfo[i].centroidCalculated;
        result = true;
    }
    
    // Calculate the centroid, if needed
    if(needToCalculateCentroid)
    {
        VictsInfo[VictIndex].VictPartsInfo[i].hitLocation = (VictsInfo[VictIndex].VictPartsInfo[i].hitLocation + AddError(utils.getRelativePosition(ActorHit.Location, Location, Rotation))) / 2;
        VictsInfo[VictIndex].VictPartsInfo[i].centroidCalculated = true;
    }

    return result;
}

/**************************************************************************************************************
    Function that takes the segment name (a.k.a bone name) from a victim's collision box and returns the
  appropriate "generic name."  Generic names are used so that there is no distinction between upper and lower
  arms, thighs and calves, as well as left and right.
**************************************************************************************************************/
function string getGenericPartName(string segName)
{
    if(segName == "Bip02 Head" || segName == "1")
        return "Head";
    else if(segName == "Bip02 L UpperArm" || segName == "Bip02 R UpperArm" || segName == "Bip02 L Forearm" || segName == "Bip02 R Forearm" || segName == "2")
        return "Arm";
    else if(segName == "Bip02 L Hand" || segName == "Bip02 R Hand" || segName == "5")
        return "Hand";
    else if(segName == "Bip02 Spine" || segName == "0")
        return "Chest";
    else if(segName == "Bip02 Pelvis" || segName == "6")
        return "Pelvis";
    else if(segName == "Bip02 L Thigh" || segName == "Bip02 R Thigh" || segName == "Bip02 L Calf" || segName == "Bip02 R Calf" || segName == "3")
        return "Leg";
    else if(segName == "Bip02 L Foot" || segName == "Bip02 R Foot" || segName == "4")
        return "Foot";
        
    // Should NEVER get here!!
    return "Unknown";
}

/**************************************************************************************************************
    Event called when the Victim Sensor is destroyed (i.e. removed from the world). The even simply cleans up
  any points that might have been displayed by the victim sensor (depending on the bShowResults variable).
**************************************************************************************************************/
simulated event Destroyed()
{
    // Clear any points that might be in the world
    if(bShowResults)
        RemovePoints();
    Super.Destroyed();
}

/**************************************************************************************************************
    The AddPoint function takes two parameters: a location vector and a texture.  The function places a point
  in the world at the location given as a first parameter and uses the texture given in the second parameter.
  The actor created is saved in a dynamic array called "Points" so that we can delete them later on.
  Please note that this function is only called when the variable bShowResults is true.
**************************************************************************************************************/
function AddPoint(vector Location, Texture TextureType)
{
    Points.Insert(Points.length, 1);                                    // Make space in the dynamic array
    Points[Points.length-1] = spawn(class'USARBot.Tracing',,,Location); // Store the point in the dynamic array
    Points[Points.length-1].Texture = TextureType;                      // Set the point's texture
}

/**************************************************************************************************************
    Function that cleans up the array of victims and any information that might be stored in it.
**************************************************************************************************************/
function EmptyVictsInfoArray()
{
    while(VictsInfo.Length != 0)
    {
        VictsInfo.Remove(VictsInfo.Length-1, 1); // Remove the point from the dynamic array
    }
}

/**************************************************************************************************************
    The RemovePoints function goes through the dynamic array of points and removes them from both the world
  and the dynamic array.
  Please note that this function is only called when the variable bShowResults is true.
**************************************************************************************************************/
function RemovePoints()
{
    while(Points.Length != 0)
    {
        Points[Points.Length-1].Destroy(); // Remove the point from the world
        Points.Remove(Points.Length-1, 1); // Remove the point from the dynamic array
    }
}

defaultproperties
{
    Distance=4
    HorizontalFOV=0.6981317  //40 degrees
    HorizontalStep=0.0698131 //4 degrees
    VerticalFOV=0.6981317    //40 degrees
    VerticalStep=0.0698131   //4 degrees
    bShowResults=true
    bWithTimeStamp=true
    timeStep = 0;
    
    // Textures used when bShowResult is true
    HitVictim = Texture'USARSim_Objects_Textures.Trace.Green';     // Texture shown when the trace hits a victim
    HitFalseAlarm = Texture'USARSim_Objects_Textures.Trace.Cyan'; // Texture shown when the trace hits a false alarm tag
    HitNonVictim = Texture'USARSim_Objects_Textures.Trace.Yellow'; // Texture shown when the trace hits something that is not a victim
    Miss = Texture'USARSim_Objects_Textures.Trace.Red';            // Texture shown when the trace hits nothing

    Mean=0.0
    Sigma=0.05

    ItemType="VictSensor"
    Drawscale=0.4762
}