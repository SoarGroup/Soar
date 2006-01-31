package edu.umich.tanksoar;

import sml.*;

import java.util.Random;
import java.util.Iterator;
import java.util.ArrayList;
//import java.lang.reflect.*;

/**
 * 
 * This class wraps up all of the input link elements needed for JTankSoar.
 * The object's members that are indented the least, that is, the closest to the
 * left margin, are higher in the input link tree.  The objects indented more are leaf
 * elements, and "hang" off of higher elements.  The input link is at the top, naturally.
 * 
 * Notes on the updates.  The accuracy of the TS input link spec at the time this class
 * was written is dubious at best, dangerously vague at worst.  That being the case, items
 * on the input link behave as closely as possible to how they do in the Tcl implementation.
 * You may notice, for example, that there is no case for the updates of tanks that appear on 
 * the radar when you have not moved but the other tank has.  In this situation, one might 
 * expect that tank to appear with the same Id but with a different value for the location WME.
 * This is not the case.  The whole tank structure "blinks" and it is as if a new tank was spotted
 * a square away from where the old one was. * 
 * 
 * @author stokesd
 *
 */
public class TankSoarInputLink
{
  private Agent agent;
  private Tank tank;

  private Identifier inputLinkRoot = null;
    private Identifier blocked = null;
      private StringElement blockedBackward = null;
      private StringElement blockedForward = null;
      private StringElement blockedLeft = null;
      private StringElement blockedRight = null;

    private Identifier incoming = null;
	    private StringElement incomingBackward = null;
	    private StringElement incomingForward = null;
	    private StringElement incomingLeft = null;
	    private StringElement incomingRight = null;

    private Identifier radar = null;

    //There can be multiple energy squares on the radar
    private ArrayList storedRadarEnergySquares = new ArrayList();

    //There can be multiple heath squares on the radar
    private ArrayList storedRadarHealthSquares = new ArrayList();    

    //There can be multiple missiler powerups on the radar
    private ArrayList storedRadarMissileBuckets = new ArrayList();

    //There can be multiple obstacles on the radar
    private ArrayList storedRadarObstacles = new ArrayList();

    //There can be multiple open spaces on the radar
    private ArrayList storedRadarOpenSpaces = new ArrayList();

    //There can be multiple tanks on the radar
    private ArrayList storedRadarTanks = new ArrayList();

    private Identifier rwaves = null;
      private StringElement rwavesBackward = null;
      private StringElement rwavesForward = null;
      private StringElement rwavesLeft = null;
      private StringElement rwavesRight = null;

    private Identifier smell = null;
      private StringElement smellColor = null;
      // smell distance will either be an integer or the string null, so we need both of these,
      // but will only be using one at a time
      private IntElement smellDistanceInt = null;
      private StringElement smellDistanceString = null;

    private StringElement sound = null;

    private IntElement clock = null;
    private StringElement direction = null;
    private IntElement energy = null;
    private StringElement energyRecharger = null;
    private IntElement health = null;
    private StringElement healthRecharger = null;
    private IntElement missiles = null;
    private StringElement myColor = null;
    private IntElement radarDistances = null;
    private IntElement radarSetting = null;
    private StringElement radarStatus = null;
    private FloatElement random = null;
    private StringElement resurrected = null;
    private StringElement shieldStatus = null;
    private IntElement x = null;
    private IntElement y = null;
    private Identifier constants = null;

    private Random randomNumGen = null;

  TankSoarInputLink(Agent inAgent, Tank myTank)
  {
    agent = inAgent;
    inputLinkRoot = agent.GetInputLink();
    tank = myTank;

    blocked = agent.CreateIdWME(inputLinkRoot, BLOCKED);
    incoming = agent.CreateIdWME(inputLinkRoot, INCOMING);
    radar = agent.CreateIdWME(inputLinkRoot, RADAR);
    rwaves = agent.CreateIdWME(inputLinkRoot, RWAVES);
    //There is only going to be one smell at a time- whichever is closest (if any smell)
    smell = agent.CreateIdWME(inputLinkRoot, SMELL);

    constants = agent.CreateIdWME(inputLinkRoot, CONSTANTS);
      agent.CreateIntWME(constants, ABSORBDAMAGE, ABSORBDAMAGE_INT);
      agent.CreateIntWME(constants, ENERGYINCREASE, ENERGYINCREASE_INT);
      agent.CreateIntWME(constants, HEALTHINCREASE, HEALTHINCREASE_INT);
      agent.CreateIntWME(constants, MAPDIM, MAPDIM_INT);
      agent.CreateIntWME(constants, MAXENERGY, MAXENERGY_INT);
      agent.CreateIntWME(constants, MAXHEALTH, MAXHEALTH_INT);
      agent.CreateIntWME(constants, MAXRADAR, MAXRADAR_INT);
      agent.CreateIntWME(constants, MISSILEINCREASE, MISSILEINCREASE_INT);
      agent.CreateIntWME(constants, PROJECTILEDAMAGE, PROJECTILEDAMAGE_INT);
      agent.CreateIntWME(constants, SHIELDCOST, SHIELDCOST_INT);
      agent.CreateIntWME(constants, SOUNDDIST, SOUNDDIST_INT);
      agent.CreateIntWME(constants, TERRAINDAMAGE, TERRAINDAMAGE_INT);
      agent.CreateIntWME(constants, WORLDCOUNTLIMIT, WORLDCOUNTLIMIT_INT);
    //no need to Commit() here, that will be done right before the agent is run
  }


  /**
   * 
   * @brief Called by TankInputInfo after that class has gotten all of the updated world information
   *        TankInputInfo passes itself it
   */
  public void doAllUpdates(TankInputInfo info)
  {
    updateBlocked(info);
    updateIncoming(info);
    updateRadar(info);
    updateRWaves(info);
    updateSmell(info);

    if(sound == null)
      sound = agent.CreateStringWME(inputLinkRoot, SOUND, info.sound);
    else if(!sound.GetValue().equals(info.sound))
      agent.Update(sound, info.sound);

    if(clock == null)
      clock = agent.CreateIntWME(inputLinkRoot, CLOCK, info.clock);
    else
      agent.Update(clock, info.clock);

    if(direction == null)
      direction = agent.CreateStringWME(inputLinkRoot, DIRECTION, directionIntToString(info.direction));
    else if(!direction.GetValue().equals(directionIntToString(info.direction)))
      agent.Update(direction, directionIntToString(info.direction));

    if(energy == null)
      energy = agent.CreateIntWME(inputLinkRoot, ENERGY, info.energy);
    else if(energy.GetValue() != info.energy)
      agent.Update(energy, info.energy);

    if(energyRecharger == null)
      energyRecharger = agent.CreateStringWME(inputLinkRoot, ENERGYRECHARGER, info.energyrecharger ? YES : NO);
    else if(!energyRecharger.GetValue().equals(info.energyrecharger ? YES : NO))
      agent.Update(energyRecharger, info.energyrecharger ? YES : NO);

    if(health == null)
      health = agent.CreateIntWME(inputLinkRoot, HEALTH, info.health);
    else if(health.GetValue() != info.health)
      agent.Update(health, info.health);

    if(healthRecharger == null)
      healthRecharger = agent.CreateStringWME(inputLinkRoot, HEALTHRECHARGER, info.healthrecharger ? YES : NO);
    else if(!healthRecharger.GetValue().equals(info.healthrecharger ? YES : NO))
      agent.Update(healthRecharger, info.healthrecharger ? YES : NO);

    if(missiles == null)
      missiles = agent.CreateIntWME(inputLinkRoot, MISSILES, info.missiles);
    else if(missiles.GetValue() != info.missiles)
      agent.Update(missiles, info.missiles);

    if(myColor == null)
      myColor = agent.CreateStringWME(inputLinkRoot, MY_COLOR, tank.getColorName());

    if(radarDistances == null)
      radarDistances = agent.CreateIntWME(inputLinkRoot, RADAR_DISTANCES, info.radarDistance);
    else if(radarDistances.GetValue() != info.radarDistance)
      agent.Update(radarDistances, info.radarDistance);

    if(radarSetting == null)
    {
      //TankSoarLogger.log("radarSetting for tank " + tank.myColorName + " was null, needing to be created.");
      radarSetting = agent.CreateIntWME(inputLinkRoot, RADAR_SETTING, info.radarSetting);
    }
    else if(radarSetting.GetValue() != info.radarSetting)
      agent.Update(radarSetting, info.radarSetting);
    
    if(radarStatus == null)
      radarStatus = agent.CreateStringWME(inputLinkRoot, RADAR_STATUS, info.radarStatus ? ON : OFF);
    else if(!radarStatus.GetValue().equals(info.radarStatus ? ON : OFF))
      agent.Update(radarStatus, info.radarStatus ? ON : OFF);

    if(random == null)
      random = agent.CreateFloatWME(inputLinkRoot, RANDOM_STR, info.random);
    else//don't check, just update
      agent.Update(random, info.random);

    if(resurrected == null)
      resurrected = agent.CreateStringWME(inputLinkRoot, RESURRECT, info.resurrected ? YES : NO);
    else if(!resurrected.GetValue().equals(info.resurrected ? YES : NO))
      agent.Update(resurrected, info.resurrected ? YES : NO);

    if(shieldStatus == null)
      shieldStatus = agent.CreateStringWME(inputLinkRoot, SHIELD_STATUS, info.shieldStatus ? ON : OFF);
    else if(!shieldStatus.GetValue().equals(info.shieldStatus ? ON : OFF))
      agent.Update(shieldStatus, info.shieldStatus ? ON : OFF);

    if(x == null)
      x = agent.CreateIntWME(inputLinkRoot, X_COORD, info.X);
    else if(x.GetValue() != info.X)
      agent.Update(x, info.X);

    if(y == null)
      y = agent.CreateIntWME(inputLinkRoot, Y_COORD, info.Y);
    else if(y.GetValue() != info.Y)
      agent.Update(y, info.Y);

    agent.Commit();

    //TankSoarLogger.log("Printing ILink after update and commit");
    //tank.debugPrintInputLink();    
  }

  //If the radar is off, kill everything except parent radar id
  //If the radar is on and the tank has moved or rotated, kill everything and recreate from scratch
  protected void updateRadar(TankInputInfo info)
  {
    //radar parent id is permanent, no update necessary

    //The "kill everything" case  
    if((info.radarStatus == false) || tank.movedLastTurn() || tank.rotatedLastTurn())
    { 
      cleanUpRadarElements(storedRadarEnergySquares);
      cleanUpRadarElements(storedRadarHealthSquares);
      cleanUpRadarElements(storedRadarMissileBuckets);
      cleanUpRadarElements(storedRadarObstacles);
      cleanUpRadarElements(storedRadarOpenSpaces);
      cleanUpRadarElements(storedRadarTanks);
    }//radar is OFF, or tank has move/rotated
    //The else case here is actually handled by the update functions called below
    //======================
    if(info.radarStatus == true)//Radar is ON
    //======================    
    {
      updateRadarEnergySquares(info);
      updateRadarHealthSquares(info);
      updateRadarMissiles(info);
      updateRadarOpenSpaces(info);
      updateRadarObstacles(info);
      updateRadarTanks(info);
    }//else radar is on
  }


  //Updates the sensors for blocked back/front/left/right
  protected void updateBlocked(TankInputInfo info)
  {
    //If the tank has moved or rotated - destroy leaf WMEs, recreate later
    if(tank.movedLastTurn() || tank.rotatedLastTurn())
    {
      //blocked parent id is permanent, no update necessary
      if(blockedBackward != null)
      {
        agent.DestroyWME(blockedBackward);
        blockedBackward = null;
      }

      if(blockedForward != null)
      {
//TankSoarLogger.log("\t\tDESTROYING BLOCKED FORWARD for agent " + tank.getColorName());
        agent.DestroyWME(blockedForward);
        blockedForward = null;
      }

      if(blockedLeft != null)
      {
        agent.DestroyWME(blockedLeft);
        blockedLeft = null;
      }

      if(blockedRight != null)
      {
        agent.DestroyWME(blockedRight);
        blockedRight = null;
      }
    }

    blockedBackward = doDirectionalUpdate(blocked, blockedBackward, BACKWARD, info.blocked.backward ? YES : NO);
    blockedForward = doDirectionalUpdate(blocked, blockedForward, FORWARD, info.blocked.forward ? YES : NO);
    blockedLeft = doDirectionalUpdate(blocked, blockedLeft, LEFT, info.blocked.left ? YES : NO);
    blockedRight = doDirectionalUpdate(blocked, blockedRight, RIGHT, info.blocked.right ? YES : NO);
  }


  //=======================================
  //============== MISSILES ===============
  //=======================================
  protected void updateRadarMissiles(TankInputInfo info)
  {
    //----get currently visible missile powerups
    if(info == null)
    {
      TankSoarLogger.log("\texiting updateRadarMissiles early because 'info' was null");
      return;
    }

    TankInputInfo.RadarSightsQueryResult currentMissiles = info.getMissilePowerupsInSight();

    //----iterate through list of old missiles and remove any that no longer match
    Iterator storedMItr = storedRadarMissileBuckets.listIterator();
    while(storedMItr.hasNext())
    {
      RadarMissile oldMissile = (RadarMissile) storedMItr.next();//cast should never fail
      RadarSightsIndex oldMissPos = oldMissile.getRelativePosition();
      //there was no match, remove this from actual input link and proxy
      if(currentMissiles.positionAndDistanceMatch(oldMissPos) == false)
      {
        oldMissile.cleanUp();
        try{storedMItr.remove();}
        catch(IllegalStateException e){}//shouldn't ever throw
      }
    }

    //----iterate through all current powerups that don't have corresponding
    //----wmes for this update, create wmes
    for(int curMissileIndex = 0; curMissileIndex < currentMissiles.getNumResults() ; ++curMissileIndex)
    {
      RadarMissile tempMissile = new RadarMissile(currentMissiles.getRelativePositionForElementAt(curMissileIndex));
      if(!storedRadarMissileBuckets.contains(tempMissile))
        storedRadarMissileBuckets.add(tempMissile);
      else
      {
        tempMissile.cleanUp();
        continue;
      }
    }//for each current missile
  }// end function

  //=======================================
  //============== ENERGY =================
  //=======================================    
  protected void updateRadarEnergySquares(TankInputInfo info)
  {
    //----get currently visible energy squares
    if(info == null)
    {
      TankSoarLogger.log("\texiting updateRadarEnergySquares early because 'info' was null");
      return;
    }

    TankInputInfo.RadarSightsQueryResult currentESquares = info.getEnergySquaresInSight();

    //----iterate through list of old energy rechargers and remove any that no longer match
    Iterator storedESItr = storedRadarEnergySquares.listIterator();

    while(storedESItr.hasNext())
    {
      RadarEnergySquare oldES = (RadarEnergySquare) storedESItr.next();//cast should never fail
      RadarSightsIndex oldESPos = oldES.getRelativePosition();
      //there was no match, remove this from actual input link and proxy
      if(currentESquares.positionAndDistanceMatch(oldESPos) == false)
      {
        oldES.cleanUp();
        try{storedESItr.remove();}
        catch(IllegalStateException e){}//shouldn't ever throw
      }
    }

    //----iterate through all current energy squares that don't have corresponding
    //----wmes for this update, create wmes
    for(int curESIndex = 0; curESIndex < currentESquares.getNumResults() ; ++curESIndex)
    {
      RadarEnergySquare tempES = new RadarEnergySquare(currentESquares.getRelativePositionForElementAt(curESIndex));
      if(!storedRadarEnergySquares.contains(tempES))
        storedRadarEnergySquares.add(tempES);
      else
      {
        tempES.cleanUp();
        continue;
      }
    }//for each current energy squares
  }

  //=======================================
  //============== HEALTH =================
  //=======================================
  protected void updateRadarHealthSquares(TankInputInfo info)
  {
    //----get currently visible health squares
    if(info == null)
    {
      TankSoarLogger.log("\texiting updateRadarHealthSquares early because 'info' was null");
      return;
    }

    TankInputInfo.RadarSightsQueryResult currentHSquares = info.getHealthSquaresInSight();

    //----iterate through list of old health rechargers and remove any that no longer match
    Iterator storedHSItr = storedRadarHealthSquares.listIterator();

    while(storedHSItr.hasNext())
    {
      RadarHealthSquare oldHS = (RadarHealthSquare) storedHSItr.next();//cast should never fail
      RadarSightsIndex oldHSPos = oldHS.getRelativePosition();

      //there was no match, remove this from actual input link and proxy
      if(currentHSquares.positionAndDistanceMatch(oldHSPos) == true)
      {
        oldHS.cleanUp();
        try{storedHSItr.remove();}
        catch(IllegalStateException e){}//shouldn't ever throw
      }
    }

    //----iterate through all current health squares that don't have corresponding
    //----wmes for this update, create wmes
    for(int curHSIndex = 0; curHSIndex < currentHSquares.getNumResults() ; ++curHSIndex)
    {
      RadarHealthSquare tempHS = new RadarHealthSquare(currentHSquares.getRelativePositionForElementAt(curHSIndex));
      if(!storedRadarHealthSquares.contains(tempHS))
      {
        storedRadarHealthSquares.add(tempHS);
      }
      else
      {
        tempHS.cleanUp();
        continue;
      }
    }//for each current energy squares
  }

  /* =======================================
   * ============ OPEN SPACES ==============
   * =======================================
   * check existing wmes to see if they match current situation, remove excess
   * add wmes for all new items, mark as inspected
   **/
  protected void updateRadarOpenSpaces(TankInputInfo info)
  {
    //TankSoarLogger.log("UpdateRadarOpenSpaces beginning...");
    if(info == null)
    {
      TankSoarLogger.log("\t<><><>exiting updateRadarOpenSpaces early because 'info' was null");
      return;
    }

    //----get currently visible open spaces powerups
    TankInputInfo.RadarSightsQueryResult currentOpenSpaces = info.getOpenSpacesInSight();

    /*Look through stored openspaces. If there isn't a current space that matches up
     *with a stored one, remove it.
     */
    Iterator storedOSItr = storedRadarOpenSpaces.listIterator();
    while(storedOSItr.hasNext())
    {
      RadarOpenSpace oldOS = (RadarOpenSpace) storedOSItr.next();//cast should never fail
      RadarSightsIndex oldOSPos = oldOS.getRelativePosition();
      //there was no match, remove this from actual input link and proxy 
      if(currentOpenSpaces.positionAndDistanceMatch(oldOSPos) == false)
      {
        oldOS.cleanUp();
        try{storedOSItr.remove();}
        catch(IllegalStateException e){}//shouldn't ever throw
      }//else
    }//while there are old OSs to look through

    /* ----iterate through all current open spaces that don't have corresponding 
       ----wmes for this update, create wmes */
    for(int curOpenIndex = 0; curOpenIndex < currentOpenSpaces.getNumResults(); ++curOpenIndex)
    {   
      RadarOpenSpace tempOS = new 
            RadarOpenSpace(currentOpenSpaces.getRelativePositionForElementAt(curOpenIndex));
      if(!storedRadarOpenSpaces.contains(tempOS))
        storedRadarOpenSpaces.add(tempOS);
      else
      {
        tempOS.cleanUp();
        continue;
      }
    }//for each current open space
  }

  /** =======================================
   *  ===========  OBSTACLES   ==============
   *  =======================================
   *  check existing wmes to see if they match current situation, remove excess
   *  add wmes for all new items, mark as inspected
   */
  protected void updateRadarObstacles(TankInputInfo info)
  {
    if(info == null)
    {
      TankSoarLogger.log("\t<><><>exiting UpdateRadarObstacles early because 'info' was null");
      return;
    }
    //----get currently visible obstacles
    TankInputInfo.RadarSightsQueryResult currentObstacles = info.getObstaclesInSight();

    /*Look through stored obstacles. If there isn't a current space that matches up
     *with a stored one, remove it
     */
    Iterator storedObsItr = storedRadarObstacles.listIterator();
    while(storedObsItr.hasNext())
    {
      RadarObstacle oldObstacle = (RadarObstacle) storedObsItr.next();//cast should never fail
      RadarSightsIndex oldObsLoc = oldObstacle.getRelativePosition();
      //there was no match, remove this from actual input link and proxy
      if(currentObstacles.positionAndDistanceMatch(oldObsLoc) == false)
      {
        oldObstacle.cleanUp();
        try{storedObsItr.remove();}
        catch(IllegalStateException e){}//shouldn't ever throw
      }//else
    }//while there are old obstacles to look through

    /* ----iterate through all current obstacles that don't have corresponding 
       ----wmes for this update, create wmes */
    for(int curObsIndex = 0; curObsIndex < currentObstacles.getNumResults(); ++curObsIndex)
    {
      RadarObstacle tempObs = new RadarObstacle(currentObstacles.getRelativePositionForElementAt(curObsIndex));
      if(!storedRadarObstacles.contains(tempObs))
        storedRadarObstacles.add(tempObs);
      else
      {
        tempObs.cleanUp();
        continue;
      }
    }//for each current obstacle
  }

  /** =======================================
   *  ==============  TANKS  ================
   *  =======================================
   * check existing wmes to see if they match current situation, remove excess
   * add wmes for all new items, mark as inspected
   */
  protected void updateRadarTanks(TankInputInfo info)
  {
    if(info == null)
    {
      TankSoarLogger.log("\t<><><>exiting UpdateRadarTanks early because 'info' was null");
      return;
    }

    //----get currently visible tanks
    TankInputInfo.RadarSightsQueryResult currentTanks = info.getTanksInSight(tank);

    /*Look through stored obstacles. If there isn't a current space that matches up
     *with a stored one, remove it
     */
    Iterator storedTItr = storedRadarTanks.listIterator();
    while(storedTItr.hasNext())
    {
      RadarTank oldTank = (RadarTank) storedTItr.next();//cast should never fail
      RadarSightsIndex oldTankLoc = oldTank.getRelativePosition();

      //there was no match, remove this from actual input link and proxy
      if(currentTanks.positionDistanceAndColorMatch(oldTankLoc, oldTank.color) == false)
      {
        oldTank.cleanUp();
        try{storedTItr.remove();}
        catch(IllegalStateException e){}//shouldn't ever throw
      }//else
    }//while there are old tanks to look through

    /* ----iterate through all current tanks that don't have corresponding 
       ----wmes for this update, create wmes */
    for(int curTankIndex = 0; curTankIndex < currentTanks.getNumResults(); ++curTankIndex)
    {
      RadarTank tempTank = new RadarTank(currentTanks.getRelativePositionForElementAt(curTankIndex),
                                                          currentTanks.getColorForElementAt(curTankIndex));
      if(!storedRadarTanks.contains(tempTank))
        storedRadarTanks.add(tempTank);
      else
      {
        tempTank.cleanUp();
        continue;
      }
    }//for each current tanks
  }

  /**
   *  Updates the smell sensor when the game state requires, or because tank moved/turned
   * 
   */
  protected void updateSmell(TankInputInfo info)
  {
    //smell parent id is permanent, no update necessary
    boolean smellDistanceAsInteger = false;
    if(info.smell.distance == -1)
      smellDistanceAsInteger = false;
    else
      smellDistanceAsInteger = true;

    //if the tank moved or roated last turn, entire structure off of "smell" must
    //be destroyed and recreated.  Also handles initialization case
    if((tank.movedLastTurn() || tank.rotatedLastTurn()))
    {
      if(smellColor != null)
      { 
        agent.DestroyWME(smellColor);
        smellColor = null;      
      }
      if(smellDistanceInt != null)
      {
        agent.DestroyWME(smellDistanceInt);
        smellDistanceInt = null;
      }
      if(smellDistanceString != null)
      {
        agent.DestroyWME(smellDistanceString);
        smellDistanceString = null;
      }
    }

    //If leaf WMEs are null, for whatever reason, create new WMEs for them
    //otherwise, update the string or int value for distance, and the string value for color
    //if they don't match
    //COLOR
    if(smellColor == null)
      smellColor = agent.CreateStringWME(smell, COLOR_STR, info.smell.color);
    else if(!info.smell.color.equals(smellColor.GetValueAsString()))
      agent.Update(smellColor, info.smell.color);

    //DISTANCE
    if(smellDistanceAsInteger)
    {
      if(smellDistanceInt == null)
        smellDistanceInt = agent.CreateIntWME(smell, DISTANCE, info.smell.distance);
      else if(smellDistanceInt.GetValue() != info.smell.distance)
        agent.Update(smellDistanceInt, info.smell.distance);
    }
    else//smell currently should be string "none"
    {
      if(smellDistanceString == null)
        smellDistanceString = agent.CreateStringWME(smell, DISTANCE, NONE);
      //really is no else case.  If smellDistanceString isn't null, then it must be "none"
      //which means it doesn't need to be updated.
    }    
  }

  protected void updateIncoming(TankInputInfo info)
  { //incoming parent id is permanent, no update necessary
    //if the the tank has moved, or rotated, destroy all leaf WMEs
    if(tank.movedLastTurn() || tank.rotatedLastTurn())
    {
      if(incomingBackward != null)
      {
        agent.DestroyWME(incomingBackward);
        incomingBackward = null;
      }
      if(incomingForward != null)
      {
        agent.DestroyWME(incomingForward);
        incomingForward = null;
      }
      if(incomingLeft != null)
      {
        agent.DestroyWME(incomingLeft);
        incomingLeft = null;
      }
      if(incomingRight != null)
      {
        agent.DestroyWME(incomingRight);
        incomingRight = null;
      }
    }

    incomingBackward = doDirectionalUpdate(incoming, incomingBackward, BACKWARD, info.incoming.backward ? YES : NO);
    incomingForward = doDirectionalUpdate(incoming, incomingForward, FORWARD, info.incoming.forward ? YES : NO);
    incomingLeft = doDirectionalUpdate(incoming, incomingLeft, LEFT, info.incoming.left ? YES : NO);
    incomingRight = doDirectionalUpdate(incoming, incomingRight, RIGHT, info.incoming.right ? YES : NO);
  }

  //if the child attributes are null, for whatever reason, create WMEs for them.
  //Otherwise update it if it doesn't match the current state
  //Because Java is retarded, we have to return the element, otherwise it's unchanged outside of fxn
  private StringElement doDirectionalUpdate(Identifier parentId, StringElement childAttribute, String direction, String currentValue)
  {
    if(childAttribute == null)
      childAttribute = agent.CreateStringWME(parentId, direction, currentValue);

    else if(!childAttribute.GetValue().equals(currentValue))
      agent.Update(childAttribute, currentValue);

    return childAttribute;
  }

  //Updates the RWaves sensor whenever the game state changes require it,
  //and also when the tank has moved or turned
  protected void updateRWaves(TankInputInfo info)
  {
    //rwaves parent id is pernament, no update necessary
    //if the tannk had moved or rotated, destroy all leaf WMES
    if(tank.movedLastTurn() || tank.rotatedLastTurn())
    {
      if(rwavesBackward != null)
      {
        agent.DestroyWME(rwavesBackward);
        rwavesBackward = null;
      } 
      if(rwavesForward != null)
      {
        agent.DestroyWME(rwavesForward);
        rwavesForward = null;
      }
      if(rwavesLeft != null)
      {
        agent.DestroyWME(rwavesLeft);
        rwavesLeft = null;
      }      
      if(rwavesRight != null)
      {
        agent.DestroyWME(rwavesRight);
        rwavesRight = null;
      }
    }

    rwavesBackward = doDirectionalUpdate(rwaves, rwavesBackward, BACKWARD, info.rwaves.backward ? YES : NO);
    rwavesForward = doDirectionalUpdate(rwaves, rwavesForward, FORWARD, info.rwaves.forward ? YES : NO);
    rwavesLeft = doDirectionalUpdate(rwaves, rwavesLeft, LEFT, info.rwaves.left ? YES : NO);
    rwavesRight = doDirectionalUpdate(rwaves, rwavesRight, RIGHT, info.rwaves.right ? YES : NO);
  }

  //Makeshift enums for I/O links
  public static final int MAX_HEALTH    = 1000;
//%%%  public static final int MAX_ENERGY    = 1000;
    //EPISODIC_MEMORY:  removed energy as consideration
  public static final int MAX_ENERGY    = 1000000;
  public static final int MIN_HEALTH    = 0;
  public static final int MIN_DISTANCE  = 0;
  public static final int MAX_COORD     = 14;
  //public static final int MAX_MISSILES;

  //Constant values for "constant" identifier
  public final int ABSORBDAMAGE_INT      = 250;
  public final int ENERGYINCREASE_INT    = 200;
  public final int HEALTHINCREASE_INT    = 150;
  public static final int MAPDIM_INT     = 16;
//%%%  public final int MAXENERGY_INT         = 1000;
    //EPISODIC_MEMORY:  removed energy as consideration
  public final int MAXENERGY_INT         = 1000000;
  public final int MAXHEALTH_INT         = 1000;
  public final int MAXRADAR_INT          = 14;
  public final int MISSILEINCREASE_INT   = 7;
  public final int PROJECTILEDAMAGE_INT  = 400;
  public final int SHIELDCOST_INT        = 20;
  public final int SOUNDDIST_INT         = 7;
  public final int TERRAINDAMAGE_INT     = 100;
  public final int WORLDCOUNTLIMIT_INT   = 4000;

  //InputLink strings
  public final String ABSORBDAMAGE       = "absorbdamage";
  public final String BACKWARD           = "backward";
  public final String BLOCKED            = "blocked";
  public final String CENTER             = "center";
  public final String CLOCK              = "clock";
  public final String COLOR_STR          = "color";
  public final String CONSTANTS          = "constants";
  public final String DIRECTION          = "direction";
  public final String DISTANCE           = "distance";
  public final String EAST               = "east";
  public final String ENERGY             = "energy";
  public final String ENERGYINCREASE     = "energyincrease";
  public final String ENERGYRECHARGER    = "energyrecharger";
  public final String FORWARD            = "forward";
  public final String HEALTH             = "health";
  public final String HEALTHRECHARGER    = "healthrecharger";
  public final String HEALTHINCREASE     = "healthincrease";
  public final String INCOMING           = "incoming";
  public final String LEFT               = "left";
  public final String MAPDIM             = "mapdim";
  public final String MAXENERGY          = "maxenergy";
  public final String MAXHEALTH          = "maxhealth";
  public final String MAXRADAR           = "maxradar";
  public final String MISSILES           = "missiles";
  public final String MISSILEINCREASE    = "missileincrease";
  public final String MY_COLOR           = "my-color";
  public final String NO                 = "no";  
  public final String NONE               = "none";
  public final String NORTH              = "north";
  public final String OBSTACLE           = "obstacle";
  public final String OFF                = "off";
  public final String ON                 = "on";
  public final String OPEN               = "open";
  public final String POSITION           = "position";
  public final String PROJECTILEDAMAGE   = "projectiledamage";
  public final String RADAR              = "radar";
  public final String RADAR_DISTANCES    = "radar-distance";
  public final String RADAR_SETTING      = "radar-setting";
  public final String RADAR_STATUS       = "radar-status";
  public final String RANDOM_STR         = "random";
  public final String RESURRECT          = "resurrect"; //this should TOTALLY be resurrected, but we're 
                                                        //making this Java version conform to the Tcl one
  public final String RIGHT              = "right";
  public final String RWAVES             = "rwaves";
  public final String SHIELDCOST         = "shieldcost";
  public final String SHIELD_STATUS      = "shield-status";
  public final String SILENT             = "silent";
  public final String SMELL              = "smell";
  public final String SOUND              = "sound";
  public final String SOUNDDIST          = "sounddist";
  public final String SOUTH              = "south";
  public final String TANK               = "tank";
  public final String TERRAINDAMAGE      = "terraindamage";
  public final String WEST               = "west";
  public final String WORLDCOUNTLIMIT    = "worldcountlimit";
  public final String X_COORD            = "x";
  public final String Y_COORD            = "y";
  public final String YES			           = "yes";


  //thin wrapper around the WMEs corresponding to a missle powerup on the radar
  protected class RadarMissile extends TankSoarRadarElement
  {
    RadarMissile(RadarSightsIndex index)
    {
      relativePosition = index;
      radarMissiles = agent.CreateIdWME(radar, MISSILES);
        radarMissileDistance = agent.CreateIntWME(radarMissiles, DISTANCE, index.distance);
        radarMissilePosition = agent.CreateStringWME(radarMissiles, POSITION, positionIntToString(index.position));          
    }
      
    public void cleanUp()
    {
      agent.DestroyWME(radarMissiles);
      radarMissiles = null;
      radarMissileDistance = null;
      radarMissilePosition = null;
    }
    public Identifier radarMissiles = null;
    public IntElement radarMissileDistance = null;
    public StringElement radarMissilePosition = null;
  }

  //thin wrapper around the WMEs corresponding to a obstacle on the radar
	protected class RadarObstacle extends TankSoarRadarElement
	{
		RadarObstacle(RadarSightsIndex relativePos)
		{
			radarObstacle = agent.CreateIdWME(radar, OBSTACLE);
			  radarObstacleDistance = agent.CreateIntWME(radarObstacle, DISTANCE, relativePos.distance);
			  radarObstaclePosition = agent.CreateStringWME(radarObstacle, POSITION,
					positionIntToString(relativePos.position));
		}

		public void cleanUp()
		{
			agent.DestroyWME(radarObstacle);
			radarObstacle = null;
			radarObstacleDistance = null;
			radarObstaclePosition = null;
		}

		public Identifier radarObstacle = null;

		public IntElement radarObstacleDistance = null;

		public StringElement radarObstaclePosition = null;
	}

  //thin wrapper around the WMEs corresponding to a open space on the radar
	protected class RadarOpenSpace extends TankSoarRadarElement
	{
		RadarOpenSpace(RadarSightsIndex relativePos)
		{
			radarOpen = agent.CreateIdWME(radar, OPEN);
			  radarOpenDistance = agent.CreateIntWME(radarOpen, DISTANCE,
					relativePos.distance);
			  radarOpenPosition = agent.CreateStringWME(radarOpen, POSITION,
					positionIntToString(relativePos.position));
		}

		public void cleanUp()
		{
			agent.DestroyWME(radarOpen);
			radarOpen = null;
			radarOpenDistance = null;
			radarOpenPosition = null;
		}

		public Identifier radarOpen = null;

		public IntElement radarOpenDistance = null;

		public StringElement radarOpenPosition = null;
	}

  //thin wrapper around the WMEs corresponding to an energy recharger on the radar
	protected class RadarEnergySquare extends TankSoarRadarElement
	{
		RadarEnergySquare(RadarSightsIndex relativePos)
		{
			radarEnergy = agent.CreateIdWME(radar, ENERGY);
			  radarEnergyDistance = agent.CreateIntWME(radarEnergy, DISTANCE,
					relativePos.distance);
			  radarEnergyPosition = agent.CreateStringWME(radarEnergy, POSITION,
					positionIntToString(relativePos.position));
		}

		public void cleanUp()
		{
			agent.DestroyWME(radarEnergy);
			radarEnergy = null;
			radarEnergyDistance = null;
			radarEnergyPosition = null;
		}

		private Identifier radarEnergy = null;

		private IntElement radarEnergyDistance = null;

		private StringElement radarEnergyPosition = null;
	}

  //thin wrapper around the WMEs that correspond to a health recharger on the radar
	protected class RadarHealthSquare extends TankSoarRadarElement
	{
		RadarHealthSquare(RadarSightsIndex relativePos)
		{
			radarHealth = agent.CreateIdWME(radar, HEALTH);
			  radarHealthDistance = agent.CreateIntWME(radarHealth, DISTANCE,
					relativePos.distance);
			  radarHealthPosition = agent.CreateStringWME(radarHealth, POSITION,
					positionIntToString(relativePos.position));
		}

		public void cleanUp()
		{
			agent.DestroyWME(radarHealth);
			radarHealth = null;
			radarHealthDistance = null;
			radarHealthPosition = null;
		}

		private Identifier radarHealth = null;

		private IntElement radarHealthDistance = null;

		private StringElement radarHealthPosition = null;
	}
  
  //thin wrapper around the WMEs corresponding to a tank on the radar
  public class RadarTank extends TankSoarRadarElement
	{
		RadarTank(RadarSightsIndex inPos, String inColor)
		{
			radarTank = agent.CreateIdWME(radar, TANK);
			  radarTankDistance = agent.CreateIntWME(radarTank, DISTANCE, inPos.distance);
			  radarTankPosition = agent.CreateStringWME(radarTank, POSITION,
					positionIntToString(inPos.position));
			  radarTankColor = agent.CreateStringWME(radarTank, COLOR_STR, inColor);
		}

		public void cleanUp()
		{
			agent.DestroyWME(radarTank);
			radarTank = null;
			radarTankDistance = null;
			radarTankPosition = null;
			radarTankColor = null;
		}

		public Identifier radarTank = null;

		public IntElement radarTankDistance = null;

		public StringElement radarTankPosition = null;

		public StringElement radarTankColor = null;
	}
  
//	public static class RadarElementFactory
//  { 
//    protected final String RO = "edu.umich.tanksoar.TankSoarInputLink$RadarObstacle";
//    protected final String ROS = "edu.umich.tanksoar.TankSoarInputLink$RadarOpenSpace";
//		protected final String RM = "edu.umich.tanksoar.TankSoarInputLink$RadarMissile";
//    protected final String RT = "edu.umich.tanksoar.TankSoarInputLink$RadarTank";
		  
//    TankSoarRadarElement createInstance(String className, RelativePosition post, String color)
//    {
//      TankSoarLogger.log("class name string is >" + className + "<");
//		  if(className.equals(RT)) return new RadarTank(loc, color);
//      else if(className.equals(RO)) return new RadarObstacle(loc);
//      else if(className.equals(ROS)) return new RadarOpenSpace(loc);
//      else if(className.equals(RM)) return new RadarMissile(loc);
//      else 
//      {
//		    TankSoarLogger.log("RadarElementFactory got unexpected type: " + * className);
//        return null;
//      }
//    }
//  }
		 
  //=======================================
	//========= Utility functions ===========
	//=======================================
	//objects on the radar have a integer that corresponds to a position relative
	//to the direction that the Tank is facing.
	public String positionIntToString(int pos)
	{
		switch(pos)
		{
			case 0:
				return LEFT;
			case 1:
				return CENTER;
			case 2:
				return RIGHT;
			default:
				break;
		}//switch
		return null;
	}

	public String directionIntToString(int dir)
	{
		String retVal = "";
		switch(dir)
		{
			case Tank.NORTH:
				retVal = NORTH;
				break;
			case Tank.SOUTH:
				retVal = SOUTH;
				break;
			case Tank.EAST:
				retVal = EAST;
				break;
			case Tank.WEST:
				retVal = WEST;
				break;
			default:
				TankSoarLogger.log("TankSoarInputLink::directionIntToString got unexpected vale "	+ dir);
				break;
		}
		return retVal;
	}

  /**
   * 
   * @param elementList - the list of TankSoarRadarElements to clean
   */
  protected void cleanUpRadarElements(ArrayList elementList)
  {
    Iterator storedElementItr = elementList.listIterator();

    while(storedElementItr.hasNext())
    {
      try
      {
        ((TankSoarRadarElement) storedElementItr.next()).cleanUp();
        storedElementItr.remove();
      }
      catch(IllegalStateException e)
      {
        TankSoarLogger.log("TSInputLink::Error in attempt to delete a radar element....");
      }
      catch(ClassCastException e)
      {
        TankSoarLogger.log("TSInputLink::Error casting in attmept to delete a radar element...") ;
      }
    }//while
  }

  private void log(Object o)
  {
    if(o != null)
      TankSoarLogger.log(o.toString());
    else
      TankSoarLogger.log("The object can't be logged because it's null....");
  }

}

