package edu.umich.soar.gridmap2d.soar;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

import org.apache.log4j.Logger;

import edu.umich.soar.gridmap2d.Direction;
import edu.umich.soar.gridmap2d.Gridmap2D;
import edu.umich.soar.gridmap2d.Names;
import edu.umich.soar.gridmap2d.Simulation;
import edu.umich.soar.gridmap2d.map.TankSoarMap;
import edu.umich.soar.gridmap2d.players.CommandInfo;
import edu.umich.soar.gridmap2d.players.Player;
import edu.umich.soar.gridmap2d.players.RadarCell;
import edu.umich.soar.gridmap2d.players.Tank;
import edu.umich.soar.gridmap2d.players.TankCommander;
import edu.umich.soar.gridmap2d.players.TankState;

import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import sml.StringElement;
import sml.WMElement;
import sml.smlRunEventId;

public class SoarTank implements Agent.RunEventInterface, TankCommander {
	private static Logger logger = Logger.getLogger(SoarTank.class);

	private Tank player;
	private Agent agent;
	private String [] shutdownCommands;
	private boolean attemptedMove = false;

	public SoarTank(Tank player, Agent agent, String[] shutdown_commands) {
		this.player = player;
		this.agent = agent;
		this.shutdownCommands = shutdown_commands;
		// TODO: need blink if no change set to true
		
		radarCellIDs = new Identifier[TankState.RADAR_WIDTH][TankState.RADAR_HEIGHT];
		radarColors = new StringElement[TankState.RADAR_WIDTH][TankState.RADAR_HEIGHT];

		agent.RegisterForRunEvent(smlRunEventId.smlEVENT_AFTER_INTERRUPT, this, null);
		agent.RegisterForRunEvent(smlRunEventId.smlEVENT_MAX_MEMORY_USAGE_EXCEEDED, this, null);
		m_InputLink = agent.GetInputLink();
		
		if (!agent.Commit()) {
			Gridmap2D.control.errorPopUp(Names.Errors.commitFail + player.getName());
		}
	}
	
	public void commit() {
		TankState state = player.getState();
		Direction facing = player.getFacing();
		String facingString = facing.id();

		String shieldStatus = state.getShieldsUp() ? Names.kOn : Names.kOff;
		String blockedForward = ((state.getBlocked() & facing.indicator()) > 0) ? Names.kYes : Names.kNo;
		String blockedBackward = ((state.getBlocked() & facing.backward().indicator()) > 0) ? Names.kYes : Names.kNo;
		String blockedLeft = ((state.getBlocked() & facing.left().indicator()) > 0) ? Names.kYes : Names.kNo;
		String blockedRight = ((state.getBlocked() & facing.right().indicator()) > 0) ? Names.kYes : Names.kNo;
		String incomingForward = ((state.getIncoming() & facing.indicator()) > 0) ? Names.kYes : Names.kNo;
		String incomingBackward = ((state.getIncoming() & facing.backward().indicator()) > 0) ? Names.kYes : Names.kNo;
		String incomingLeft = ((state.getIncoming() & facing.left().indicator()) > 0) ? Names.kYes : Names.kNo;
		String incomingRight = ((state.getIncoming() & facing.right().indicator()) > 0) ? Names.kYes : Names.kNo;
		String smellColorString = (state.getSmellColor() == null) ? Names.kNone : state.getSmellColor();
		String soundString;
		if (state.getSound() == facing) {
			soundString = Names.kForwardID;
		} else if (state.getSound() == facing.backward()) {
			soundString = Names.kBackwardID;
		} else if (state.getSound() == facing.left()) {
			soundString = Names.kLeftID;
		} else if (state.getSound() == facing.right()) {
			soundString = Names.kRightID;
		} else {
			soundString = Names.kSilentID;
		}
		int worldCount = Gridmap2D.simulation.getWorldCount();
		String radarStatus = state.getRadarSwitch() ? Names.kOn : Names.kOff;
		float oldrandom = random;
		do {
			random = Simulation.random.nextFloat();
		} while (random == oldrandom);
		String rwavesForward = (state.getRwaves() & facing.indicator()) > 0 ? Names.kYes : Names.kNo;
		String rwavesBackward = (state.getRwaves() & facing.backward().indicator()) > 0 ? Names.kYes : Names.kNo;;
		String rwavesLeft = (state.getRwaves() & facing.left().indicator()) > 0 ? Names.kYes : Names.kNo;
		String rwavesRight = (state.getRwaves() & facing.right().indicator()) > 0 ? Names.kYes : Names.kNo;

		if (logger.isTraceEnabled()) {
			logger.trace(player.getName() + " input dump: ");
			logger.trace(player.getName() + ": x,y: " + player.getLocation()[0] + "," + player.getLocation()[1]);
			logger.trace(player.getName() + ": " + Names.kEnergyRechargerID + ": " + (state.getOnEnergyCharger() ? Names.kYes : Names.kNo));
			logger.trace(player.getName() + ": " + Names.kHealthRechargerID + ": " + (state.getOnHealthCharger() ? Names.kYes : Names.kNo));
			logger.trace(player.getName() + ": " + Names.kDirectionID + ": " + facingString);
			logger.trace(player.getName() + ": " + Names.kEnergyID + ": " + state.getEnergy());
			logger.trace(player.getName() + ": " + Names.kHealthID + ": " + state.getHealth());
			logger.trace(player.getName() + ": " + Names.kShieldStatusID + ": " + shieldStatus);
			logger.trace(player.getName() + ": blocked (forward): " + blockedForward);
			logger.trace(player.getName() + ": blocked (backward): " + blockedBackward);
			logger.trace(player.getName() + ": blocked (left): " + blockedLeft);
			logger.trace(player.getName() + ": blocked (right): " + blockedRight);
			logger.trace(player.getName() + ": " + Names.kCurrentScoreID + ": " + player.getPoints());
			logger.trace(player.getName() + ": incoming (forward): " + incomingForward);
			logger.trace(player.getName() + ": incoming (backward): " + incomingBackward);
			logger.trace(player.getName() + ": incoming (left): " + incomingLeft);
			logger.trace(player.getName() + ": incoming (right): " + incomingRight);
			logger.trace(player.getName() + ": smell (color): " + smellColorString);
			logger.trace(player.getName() + ": smell (distance): " + state.getSmellDistance());
			logger.trace(player.getName() + ": " + Names.kSoundID + ": " + soundString);
			logger.trace(player.getName() + ": " + Names.kMissilesID + ": " + state.getMissiles());
			logger.trace(player.getName() + ": " + Names.kMyColorID + ": " + player.getColor());
			logger.trace(player.getName() + ": " + Names.kClockID + ": " + worldCount);
			logger.trace(player.getName() + ": " + Names.kRadarStatusID + ": " + radarStatus);
			logger.trace(player.getName() + ": " + Names.kRadarDistanceID + ": " + state.getObservedPower());
			logger.trace(player.getName() + ": " + Names.kRadarSettingID + ": " + state.getRadarPower());
			logger.trace(player.getName() + ": " + Names.kRandomID + "random: " + random);
			logger.trace(player.getName() + ": rwaves (forward): " + rwavesForward);
			logger.trace(player.getName() + ": rwaves (backward): " + rwavesBackward);
			logger.trace(player.getName() + ": rwaves (left): " + rwavesLeft);
			logger.trace(player.getName() + ": rwaves (right): " + rwavesRight);
		}

		if (m_Reset) {
			// location
			m_xWME = CreateIntWME(m_InputLink, Names.kXID, player.getLocation()[0]);
			m_yWME = CreateIntWME(m_InputLink, Names.kYID, player.getLocation()[1]);
			
			// charger detection
			String energyRecharger = state.getOnEnergyCharger() ? Names.kYes : Names.kNo;
			m_EnergyRechargerWME = CreateStringWME(m_InputLink, Names.kEnergyRechargerID, energyRecharger);

			String healthRecharger = state.getOnHealthCharger() ? Names.kYes : Names.kNo;
			m_HealthRechargerWME = CreateStringWME(m_InputLink, Names.kHealthRechargerID, healthRecharger);

			// facing
			m_DirectionWME = CreateStringWME(m_InputLink, Names.kDirectionID, facingString);

			// energy and health status
			m_EnergyWME = CreateIntWME(m_InputLink, Names.kEnergyID, state.getEnergy());
			m_HealthWME = CreateIntWME(m_InputLink, Names.kHealthID, state.getHealth());

			// shield status
			m_ShieldStatusWME = CreateStringWME(m_InputLink, Names.kShieldStatusID, shieldStatus);

			// blocked sensor
			m_BlockedWME = agent.CreateIdWME(m_InputLink, Names.kBlockedID);
			m_BlockedForwardWME = CreateStringWME(m_BlockedWME, Names.kForwardID, blockedForward);
			m_BlockedBackwardWME = CreateStringWME(m_BlockedWME, Names.kBackwardID, blockedBackward);
			m_BlockedLeftWME = CreateStringWME(m_BlockedWME, Names.kLeftID, blockedLeft);
			m_BlockedRightWME = CreateStringWME(m_BlockedWME, Names.kRightID, blockedRight);				

			// score status
			m_CurrentScoreWME = agent.CreateIdWME(m_InputLink, Names.kCurrentScoreID);
			initScoreWMEs();

			// incoming sensor
			m_IncomingWME = agent.CreateIdWME(m_InputLink, Names.kIncomingID);
			m_IncomingBackwardWME = CreateStringWME(m_IncomingWME, Names.kBackwardID, incomingForward);
			m_IncomingForwardWME = CreateStringWME(m_IncomingWME, Names.kForwardID, incomingBackward);
			m_IncomingLeftWME = CreateStringWME(m_IncomingWME, Names.kLeftID, incomingLeft);
			m_IncomingRightWME = CreateStringWME(m_IncomingWME, Names.kRightID, incomingRight);
			
			// smell sensor
			m_SmellWME = agent.CreateIdWME(m_InputLink, Names.kSmellID);
			m_SmellColorWME = CreateStringWME(m_SmellWME, Names.kColorID, smellColorString);
			if (state.getSmellColor() == null) {
				m_SmellDistanceWME = null;
				m_SmellDistanceStringWME = CreateStringWME(m_SmellWME, Names.kDistanceID, Names.kNone);
			} else {
				m_SmellDistanceWME = CreateIntWME(m_SmellWME, Names.kDistanceID, state.getSmellDistance());
				m_SmellDistanceStringWME = null;
			}

			// sound sensor
			m_SoundWME = CreateStringWME(m_InputLink, Names.kSoundID, soundString);			

			// missile quantity indicator
			m_MissilesWME = CreateIntWME(m_InputLink, Names.kMissilesID, state.getMissiles());

			// my color
			m_MyColorWME = CreateStringWME(m_InputLink, Names.kMyColorID, player.getColor());

			// clock (world count)
			m_ClockWME = CreateIntWME(m_InputLink, Names.kClockID, worldCount);

			// resurrect sensor
			m_ResurrectWME = CreateStringWME(m_InputLink, Names.kResurrectID, Names.kYes);

			// radar sensors
			m_RadarStatusWME = CreateStringWME(m_InputLink, Names.kRadarStatusID, radarStatus);
			if (state.getRadarSwitch()) {
				m_RadarWME = agent.CreateIdWME(m_InputLink, Names.kRadarID);
				generateNewRadar(state);
			} else {
				m_RadarWME = null;
			}
			m_RadarDistanceWME = CreateIntWME(m_InputLink, Names.kRadarDistanceID, state.getObservedPower());
			m_RadarSettingWME = CreateIntWME(m_InputLink, Names.kRadarSettingID, state.getRadarPower());

			// random indicator
			m_RandomWME = CreateFloatWME(m_InputLink, Names.kRandomID, random);

			// rwaves sensor
			m_RWavesWME = agent.CreateIdWME(m_InputLink, Names.kRWavesID);
			m_RWavesForwardWME = CreateStringWME(m_RWavesWME, Names.kForwardID, rwavesBackward);
			m_RWavesBackwardWME = CreateStringWME(m_RWavesWME, Names.kBackwardID, rwavesForward);
			m_RWavesLeftWME = CreateStringWME(m_RWavesWME, Names.kLeftID, rwavesLeft);
			m_RWavesRightWME = CreateStringWME(m_RWavesWME, Names.kRightID, rwavesRight);

		} else {
			if (player.getMoved()) {
				// location
				if (player.getLocation()[0] != m_xWME.GetValue()) {
					Update(m_xWME, player.getLocation()[0]);
				}
				
				if (player.getLocation()[1] != m_yWME.GetValue()) {
					Update(m_yWME, player.getLocation()[1]);
				}
				
				// charger detection
				// TODO: consider SetBlinkIfNoChange(false) in constructor
				String energyRecharger = state.getOnEnergyCharger() ? Names.kYes : Names.kNo;
				if ( !m_EnergyRechargerWME.GetValue().equals(energyRecharger) ) {
					Update(m_EnergyRechargerWME, energyRecharger);
				}

				String healthRecharger = state.getOnHealthCharger() ? Names.kYes : Names.kNo;
				if ( !m_HealthRechargerWME.GetValue().equals(healthRecharger) ) {
					Update(m_HealthRechargerWME, healthRecharger);
				}
			}
			
			boolean rotated = !m_DirectionWME.GetValue().equalsIgnoreCase(facingString);
			if (rotated) {
				// facing
				Update(m_DirectionWME, facingString);
			}

			// stats detection
			if (m_EnergyWME.GetValue() != state.getEnergy()) {
				Update(m_EnergyWME, state.getEnergy());
			}
			if (m_HealthWME.GetValue() != state.getHealth()) {
				Update(m_HealthWME, state.getHealth());
			}			

			// shield status
			if (!m_ShieldStatusWME.GetValue().equalsIgnoreCase(shieldStatus)) {
				Update(m_ShieldStatusWME, shieldStatus);
			}
			
			// blocked sensor
			if (attemptedMove || rotated || !m_BlockedForwardWME.GetValue().equalsIgnoreCase(blockedForward)) {
				Update(m_BlockedForwardWME, blockedForward);
			}
			if (attemptedMove || rotated || !m_BlockedBackwardWME.GetValue().equalsIgnoreCase(blockedBackward)) {
				Update(m_BlockedBackwardWME, blockedBackward);
			}
			if (attemptedMove || rotated || !m_BlockedLeftWME.GetValue().equalsIgnoreCase(blockedLeft)) {
				Update(m_BlockedLeftWME, blockedLeft);
			}
			if (attemptedMove || rotated || !m_BlockedRightWME.GetValue().equalsIgnoreCase(blockedRight)) {
				Update(m_BlockedRightWME, blockedRight);
			}

			// scores
			if (playersChanged) {
				initScoreWMEs();
			}
			for (Player p : players) {
				if (p.pointsChanged()) {
					Update(m_Scores.get(p.getColor()), p.getPoints());
				}
			}

			// incoming sensor
			if (!m_IncomingForwardWME.GetValue().equalsIgnoreCase(incomingForward)) {
				Update(m_IncomingForwardWME, incomingForward);
			}
			if (!m_IncomingBackwardWME.GetValue().equalsIgnoreCase(incomingBackward)) {
				Update(m_IncomingBackwardWME, incomingBackward);
			}
			if (!m_IncomingLeftWME.GetValue().equalsIgnoreCase(incomingLeft)) {
				Update(m_IncomingLeftWME, incomingLeft);
			}
			if (!m_IncomingRightWME.GetValue().equalsIgnoreCase(incomingRight)) {
				Update(m_IncomingRightWME, incomingRight);
			}

			// smell sensor
			if (!m_SmellColorWME.GetValue().equalsIgnoreCase(smellColorString)) {
				Update(m_SmellColorWME, smellColorString);
			}
			if (state.getSmellColor() == null) {
				if (m_SmellDistanceWME != null) {
					DestroyWME(m_SmellDistanceWME);
					m_SmellDistanceWME = null;
				}
				if (m_SmellDistanceStringWME == null) {
					m_SmellDistanceStringWME = CreateStringWME(m_SmellWME, Names.kDistanceID, Names.kNone);
				}
			} else {
				if (m_SmellDistanceWME == null) {
					m_SmellDistanceWME = CreateIntWME(m_SmellWME, Names.kDistanceID, state.getSmellDistance());
				} else {
					if (m_SmellDistanceWME.GetValue() != state.getSmellDistance()) {
						Update(m_SmellDistanceWME, state.getSmellDistance());
					}
				}
				if (m_SmellDistanceStringWME != null) {
					DestroyWME(m_SmellDistanceStringWME);
					m_SmellDistanceStringWME = null;
				}
			}

			// sound sensor
			if (!m_SoundWME.GetValue().equalsIgnoreCase(soundString)) {
				Update(m_SoundWME, soundString);
			}

			// missile quantity indicator
			if (m_MissilesWME.GetValue() != state.getMissiles()) {
				Update(m_MissilesWME, state.getMissiles());
			}

			// clock (world count)
			Update(m_ClockWME, worldCount);

			// resurrect sensor
			if (state.getResurrectFrame() != Gridmap2D.simulation.getWorldCount()) {
				if (!m_ResurrectWME.GetValue().equalsIgnoreCase(Names.kNo)) {
					Update(m_ResurrectWME, Names.kNo);
				}
			}

			// radar sensors
			if (!m_RadarStatusWME.GetValue().equalsIgnoreCase(radarStatus)) {
				Update(m_RadarStatusWME, radarStatus);
			}
			if (state.getRadarSwitch()) {
				if (m_RadarWME == null) {
					m_RadarWME = agent.CreateIdWME(m_InputLink, Names.kRadarID);
					generateNewRadar(state);
				} else {
					updateRadar(player.getMoved() || rotated, state);
				}
			} else {
				if (m_RadarWME != null) {
					clearRadar(state);
					DestroyWME(m_RadarWME);
					m_RadarWME = null;
				}
			}
			if (m_RadarDistanceWME.GetValue() != state.getObservedPower()) {
				Update(m_RadarDistanceWME, state.getObservedPower());
			}
			if (m_RadarSettingWME.GetValue() != state.getRadarPower()) {
				Update(m_RadarSettingWME, state.getRadarPower());
			}

			// random indicator
			Update(m_RandomWME, random);

			// rwaves sensor
			if (!m_RWavesForwardWME.GetValue().equalsIgnoreCase(rwavesForward)) {
				Update(m_RWavesForwardWME, rwavesForward);
			}
			if (!m_RWavesBackwardWME.GetValue().equalsIgnoreCase(rwavesBackward)) {
				Update(m_RWavesBackwardWME, rwavesBackward);
			}
			if (!m_RWavesLeftWME.GetValue().equalsIgnoreCase(rwavesLeft)) {
				Update(m_RWavesLeftWME, rwavesLeft);
			}
			if (!m_RWavesRightWME.GetValue().equalsIgnoreCase(rwavesRight)) {
				Update(m_RWavesRightWME, rwavesRight);
			}
		}
		
		m_Reset = false;
		
		if (!agent.Commit()) {
			Gridmap2D.control.errorPopUp(Names.Errors.commitFail + player.getName());
			Gridmap2D.control.stopSimulation();
		}
	}

	public CommandInfo nextCommand() {
		attemptedMove = false;

		if (agent.GetNumberCommands() == 0) {
			logger.debug(player.getName() + " issued no command.");
			return new CommandInfo();
		}
		
		Identifier moveId = null;
		CommandInfo move = new CommandInfo();
		boolean moveWait = false;
		for (int i = 0; i < agent.GetNumberCommands(); ++i) {
			Identifier commandId = agent.GetCommand(i);
			String commandName = commandId.GetAttribute();

			if (commandName.equalsIgnoreCase(Names.kMoveID)) {
				if (move.move || moveWait) {
					logger.debug(player.getName() + ": extra move commands");
					commandId.AddStatusError();
					continue;
				}

				String moveDirection = commandId.GetParameterValue(Names.kDirectionID);
				if (moveDirection == null) {
					logger.warn(player.getName() + ": null move direction");
					commandId.AddStatusError();
					continue;
				}
				
				if (moveDirection.equalsIgnoreCase(Names.kForwardID)) {
					move.moveDirection = player.getFacing();
				} else if (moveDirection.equalsIgnoreCase(Names.kBackwardID)) {
					move.moveDirection = player.getFacing().backward();
				} else if (moveDirection.equalsIgnoreCase(Names.kLeftID)) {
					move.moveDirection = player.getFacing().left();
				} else if (moveDirection.equalsIgnoreCase(Names.kRightID)) {
					move.moveDirection = player.getFacing().right();
				} else if (moveDirection.equalsIgnoreCase(Names.kNone)) {
					// legal wait
					moveWait = true;
					commandId.AddStatusComplete();
					continue;
				} else {
					logger.warn(player.getName() + ": illegal move direction: " + moveDirection);
					commandId.AddStatusError();
					continue;
				}
				moveId = commandId;
				move.move = true;
				attemptedMove = true;
				
			} else if (commandName.equalsIgnoreCase(Names.kFireID)) {
				if (move.fire == true) {
					logger.debug(player.getName() + ": extra fire commands");
					commandId.AddStatusError();
					continue;
				}
	 			move.fire = true;

	 			// Weapon ignored
				
			} else if (commandName.equalsIgnoreCase(Names.kRadarID)) {
				if (move.radar == true) {
					logger.debug(player.getName() + ": extra radar commands");
					commandId.AddStatusError();
					continue;
				}
				
				String radarSwitch = commandId.GetParameterValue(Names.kSwitchID);
				if (radarSwitch == null) {
					logger.warn(player.getName() + ": null radar switch");
					commandId.AddStatusError();
					continue;
				}
				move.radar = true;
				move.radarSwitch = radarSwitch.equalsIgnoreCase(Names.kOn) ? true : false;  
				
			} else if (commandName.equalsIgnoreCase(Names.kRadarPowerID)) {
				if (move.radarPower == true) {
					logger.debug(player.getName() + ": extra radar power commands");
					commandId.AddStatusError();
					continue;
				}
				
				String powerValue = commandId.GetParameterValue(Names.kSettingID);
				if (powerValue == null) {
					logger.warn(player.getName() + ": null radar power");
					commandId.AddStatusError();
					continue;
				}
				
				try {
					move.radarPowerSetting = Integer.decode(powerValue).intValue();
				} catch (NumberFormatException e) {
					e.printStackTrace();
					logger.warn(player.getName() + ": unable to parse radar power setting " + powerValue + ": " + e.getMessage());
					commandId.AddStatusError();
					continue;
				}
				move.radarPower = true;
				
			} else if (commandName.equalsIgnoreCase(Names.kShieldsID)) {
				if (move.shields == true) {
					logger.debug(player.getName() + ": extra shield commands");
					commandId.AddStatusError();
					continue;
				}
				
				String shieldsSetting = commandId.GetParameterValue(Names.kSwitchID);
				if (shieldsSetting == null) {
					logger.warn(player.getName() + ": null shields setting");
					commandId.AddStatusError();
					continue;
				}
				move.shields = true;
				move.shieldsSetting = shieldsSetting.equalsIgnoreCase(Names.kOn) ? true : false; 
				
			} else if (commandName.equalsIgnoreCase(Names.kRotateID)) {
				if (move.rotate == true) {
					logger.debug(player.getName() + ": extra rotate commands");
					commandId.AddStatusError();
					continue;
				}
				
				move.rotateDirection = commandId.GetParameterValue(Names.kDirectionID);
				if (move.rotateDirection == null) {
					logger.warn(player.getName() + ": null rotation direction");
					commandId.AddStatusError();
					continue;
				}
				
				move.rotate = true;
				
			} else {
				logger.warn(player.getName() + ": unknown command: " + commandName);
				commandId.AddStatusError();
				continue;
			}
			commandId.AddStatusComplete();
		}
		
    	agent.ClearOutputLinkChanges();
    	
		if (!agent.Commit()) {
			Gridmap2D.control.errorPopUp(Names.Errors.commitFail + this.player.getName());
			Gridmap2D.control.stopSimulation();
		}
		
		// Do not allow a move if we rotated.
		if (move.rotate) {
			if (move.move) {
				logger.debug(": move ignored (rotating)");
				assert moveId != null;
				moveId.AddStatusError();
				moveId = null;
				move.move = false;
			}
		}

		return move;
	}

	public void update(TankSoarMap tankSoarMap) {
		// update happens in "commit", after all tanks' states have been updated.
	}

	public void playersChanged(Player[] players) {
		this.players = players;
		playersChanged = true;
	}

	public void reset() {
		mem_exceeded = false;
		
		if (agent == null) {
			return;
		}
		
		// TODO: clear wmes!
//		clearWMEs();
//
//		if (!agent.Commit()) {
//			Gridmap2D.control.errorPopUp(Names.Errors.commitFail + player.getName());
//			Gridmap2D.control.stopSimulation();
//		}
		
		agent.InitSoar();
		
//		if (!agent.Commit()) {
//			Gridmap2D.control.errorPopUp(Names.Errors.commitFail + player.getName());
//			Gridmap2D.control.stopSimulation();
//		}
	}
	
	public void shutdown() {
		assert agent != null;
		if (shutdownCommands != null) { 
			// execute the pre-shutdown commands
			for (String command : shutdownCommands) {
				String result = player.getName() + ": result: " + agent.ExecuteCommandLine(command, true);
				logger.info(player.getName() + ": shutdown command: " + command);
				if (agent.HadError()) {
					Gridmap2D.control.errorPopUp(result);
				} else {
					logger.info(player.getName() + ": result: " + result);
				}
			}
		}

		clearWMEs();
	}
	
//////////////////////
	

	private Identifier m_InputLink;
	private Identifier m_BlockedWME;
	private StringElement m_BlockedBackwardWME;
	private StringElement m_BlockedForwardWME;
	private StringElement m_BlockedLeftWME;
	private StringElement m_BlockedRightWME;
	private IntElement m_ClockWME;
	private Identifier m_CurrentScoreWME;
	
	private Map<String, IntElement> m_Scores = new HashMap<String, IntElement>(7);
	
	private StringElement m_DirectionWME;
	private IntElement m_EnergyWME;
	private StringElement m_EnergyRechargerWME;
	private IntElement m_HealthWME;
	private StringElement m_HealthRechargerWME;
	private Identifier m_IncomingWME;
	private StringElement m_IncomingBackwardWME;
	private StringElement m_IncomingForwardWME;
	private StringElement m_IncomingLeftWME;
	private StringElement m_IncomingRightWME;
	private IntElement m_MissilesWME;
	private StringElement m_MyColorWME;
	private StringElement m_RadarStatusWME;
	private IntElement m_RadarDistanceWME;
	private IntElement m_RadarSettingWME;
	private Identifier m_RadarWME;
	private FloatElement m_RandomWME;
	private StringElement m_ResurrectWME;
	private Identifier m_RWavesWME;
	private StringElement m_RWavesBackwardWME;
	private StringElement m_RWavesForwardWME;
	private StringElement m_RWavesLeftWME;
	private StringElement m_RWavesRightWME;
	private StringElement m_ShieldStatusWME;
	private Identifier m_SmellWME;
	private StringElement m_SmellColorWME;
	private IntElement m_SmellDistanceWME;
	private StringElement m_SmellDistanceStringWME;
	private StringElement m_SoundWME;
	private IntElement m_xWME;
	private IntElement m_yWME;			

	private Identifier[][] radarCellIDs;
	private StringElement[][] radarColors;

	private float random = 0;
	private boolean m_Reset = true;
	
	private boolean playersChanged = false;
	private Player[] players = null;
	private boolean mem_exceeded = false;
	

	public void runEventHandler(int eventID, Object data, Agent agent, int phase) {
		if (eventID == smlRunEventId.smlEVENT_AFTER_INTERRUPT.swigValue()) {
			if (!Gridmap2D.control.isStopped()) {
				logger.warn(player.getName() + ": agent interrupted");
				// only penalize interruptions when running headless
				if (!Gridmap2D.wm.using()) {
					Gridmap2D.simulation.interrupted(agent.GetAgentName());
				}
			}
		} else if (!mem_exceeded && eventID == smlRunEventId.smlEVENT_MAX_MEMORY_USAGE_EXCEEDED.swigValue()) {
			logger.warn(player.getName() + ": agent exceeded maximum memory usage");
			Gridmap2D.simulation.interrupted(agent.GetAgentName());
			Gridmap2D.control.stopSimulation();
			mem_exceeded = true;
		} else {
			assert false;
		}
	}
	
	private void DestroyWME(WMElement wme) {
		assert wme != null;
		agent.DestroyWME(wme);
	}

	private void Update(StringElement wme, String value) {
		assert wme != null;
		assert value != null;
		agent.Update(wme, value);
	}

	private void Update(IntElement wme, int value) {
		assert wme != null;
		agent.Update(wme, value);
	}

	private void Update(FloatElement wme, float value) {
		assert wme != null;
		agent.Update(wme, value);
	}
	
	private IntElement CreateIntWME(Identifier id, String attribute, int value) {
		assert id != null;
		assert attribute != null;
		return agent.CreateIntWME(id, attribute, value);
	}

	private StringElement CreateStringWME(Identifier id, String attribute, String value) {
		assert id != null;
		assert attribute != null;
		assert value != null;
		return agent.CreateStringWME(id, attribute, value);
	}

	private FloatElement CreateFloatWME(Identifier id, String attribute, float value) {
		assert id != null;
		assert attribute != null;
		return agent.CreateFloatWME(id, attribute, value);
	}

	public void fragged() {
		if (m_Reset == true) {
			return;
		}
		
		clearWMEs();
		
		m_Reset = true;
	}
	
	private void clearWMEs() {
		DestroyWME(m_BlockedWME);
		m_BlockedWME = null;
		
		DestroyWME(m_ClockWME);
		m_ClockWME = null;
		
		DestroyWME(m_CurrentScoreWME);
		m_CurrentScoreWME = null;
		m_Scores = new HashMap<String, IntElement>(7);

		DestroyWME(m_DirectionWME);
		m_DirectionWME = null;
		
		DestroyWME(m_EnergyWME);
		m_EnergyWME = null;
		
		DestroyWME(m_EnergyRechargerWME);
		m_EnergyRechargerWME = null;
		
		DestroyWME(m_HealthWME);
		m_HealthWME = null;
		
		DestroyWME(m_HealthRechargerWME);
		m_HealthRechargerWME = null;
		
		DestroyWME(m_IncomingWME);
		m_IncomingWME = null;
		
		DestroyWME(m_MissilesWME);
		m_MissilesWME = null;
		
		DestroyWME(m_MyColorWME);
		m_MyColorWME = null;
		
		DestroyWME(m_RadarStatusWME);
		m_RadarStatusWME = null;
		
		DestroyWME(m_RadarDistanceWME);
		m_RadarDistanceWME = null;
		
		DestroyWME(m_RadarSettingWME);
		m_RadarSettingWME = null;
		
		if (m_RadarWME != null) {
			DestroyWME(m_RadarWME);
			m_RadarWME = null;
		}
		DestroyWME(m_RandomWME);
		m_RandomWME = null;
		
		DestroyWME(m_ResurrectWME);
		m_ResurrectWME = null;
		
		DestroyWME(m_RWavesWME);
		m_RWavesWME = null;
		
		DestroyWME(m_ShieldStatusWME);
		m_ShieldStatusWME = null;
		
		DestroyWME(m_SmellWME);
		m_SmellWME = null;
		
		DestroyWME(m_SoundWME);
		m_SoundWME = null;
		
		DestroyWME(m_xWME);
		m_xWME = null;
		
		DestroyWME(m_yWME);
		m_yWME = null;
		
		TankState state = player.getState();
		clearRadar(state);
	}
	
	private void initScoreWMEs() {
		if (m_CurrentScoreWME == null) {
			return;
		}

		Set<String> unseen = new HashSet<String>();
		unseen.add("blue");
		unseen.add("red");
		unseen.add("yellow");
		unseen.add("green");
		unseen.add("purple");
		unseen.add("orange");
		unseen.add("black");
		
		for (Player p : players) {
			IntElement scoreElement = m_Scores.get(p.getColor());
			unseen.remove(p.getColor());
			if (scoreElement == null) {
				scoreElement = agent.CreateIntWME(m_CurrentScoreWME, p.getColor(), p.getPoints());
				m_Scores.put(p.getColor(), scoreElement);
			}
		}
		
		Iterator<String> unseenIter = unseen.iterator();
		while (unseenIter.hasNext()) {
			String color = unseenIter.next();
			IntElement unseenElement = m_Scores.remove(color);
			if (unseenElement != null) {
				DestroyWME(unseenElement);
			}
		}
		
		playersChanged = false;
	}

	private void generateNewRadar(TankState state) {
		int height;
		if (logger.isTraceEnabled()) {
			logger.trace(player.getName() + ": radar data: generating new"); 
		}
		for (height = 0; height < TankState.RADAR_HEIGHT; ++height) {
			boolean done = false;
			for (int width = 0; width < TankState.RADAR_WIDTH; ++width) {
				// Always skip self, this screws up the tanks.
				if (width == 1 && height == 0) {
					if (logger.isTraceEnabled()) {
						logger.trace(player.getName() + ": " + height + "," + width + ": skip self"); 
					}
					continue;
				}
				if (state.getRadar()[width][height] == null) {
					// if center is null, we're done
					if (width == 1) {
						done = true;
						if (logger.isTraceEnabled()) {
							logger.trace(player.getName() + ": " + height + "," + width + ": done (center null)"); 
						}
						break;
					}
				} else {
					// Create a new WME
					radarCellIDs[width][height] = agent.CreateIdWME(m_RadarWME, getCellID(state.getRadar()[width][height]));
					CreateIntWME(radarCellIDs[width][height], Names.kDistanceID, height);
					CreateStringWME(radarCellIDs[width][height], Names.kPositionID, getPositionID(width));
					if (state.getRadar()[width][height].player != null) {
						radarColors[width][height] = CreateStringWME(radarCellIDs[width][height], Names.kColorID, state.getRadar()[width][height].player.getColor());
						if (logger.isTraceEnabled()) {
							logger.trace(player.getName() + ": " + height + "," + width + ": " + getCellID(state.getRadar()[width][height]) + " " + state.getRadar()[width][height].player.getColor()); 
						}
					} else {
						if (logger.isTraceEnabled()) {
							logger.trace(player.getName() + ": " + height + "," + width + ": " + getCellID(state.getRadar()[width][height])); 
						}
					}
				}
			}
			if (done == true) {
				break;
			}
		}
		assert (height - 1) == state.getObservedPower();
	}
	
	private void updateRadar(boolean movedOrRotated, TankState state) {
		if (logger.isTraceEnabled()) {
			logger.trace(player.getName() + ": radar data: updating"); 
		}
		for (int width = 0; width < TankState.RADAR_WIDTH; ++width) {
			for (int height = 0; height < TankState.RADAR_HEIGHT; ++height) {
				// Always skip self, this screws up the tanks.
				if (width == 1 && height == 0) {
					if (logger.isTraceEnabled()) {
						logger.trace(player.getName() + ": " + height + "," + width + ": skip self"); 
					}
					continue;
				}
				if (state.getRadar()[width][height] == null || (height > state.getObservedPower()) || ((height == state.getObservedPower()) && (width != 1))) {
					// Unconditionally delete the WME
					if (radarCellIDs[width][height] != null) {
						DestroyWME(radarCellIDs[width][height]);
						radarCellIDs[width][height] = null;
						radarColors[width][height] = null;
						if (logger.isTraceEnabled()) {
							logger.trace(player.getName() + ": " + height + "," + width + ": (deleted)"); 
						}
					} else {
						if (logger.isTraceEnabled()) {
							logger.trace(player.getName() + ": " + height + "," + width + ": (null)"); 
						}
					}
					
				} else {
					
					if (radarCellIDs[width][height] == null) {
						radarCellIDs[width][height] = agent.CreateIdWME(m_RadarWME, getCellID(state.getRadar()[width][height]));
						CreateIntWME(radarCellIDs[width][height], Names.kDistanceID, height);
						CreateStringWME(radarCellIDs[width][height], Names.kPositionID, getPositionID(width));
						if (state.getRadar()[width][height].player != null) {
							radarColors[width][height] = CreateStringWME(radarCellIDs[width][height], Names.kColorID, state.getRadar()[width][height].player.getColor());
							if (logger.isTraceEnabled()) {
								logger.trace(player.getName() + ": " + height + "," + width + ": " + getCellID(state.getRadar()[width][height]) + " " + state.getRadar()[width][height].player.getColor() + " (created)"); 
							}
						} else {
							if (logger.isTraceEnabled()) {
								logger.trace(player.getName() + ": " + height + "," + width + ": " + getCellID(state.getRadar()[width][height]) + " (created)"); 
							}
						}
					} else {
						boolean changed = !radarCellIDs[width][height].GetAttribute().equals(getCellID(state.getRadar()[width][height]));

						// Update if relevant change
						if (movedOrRotated || changed) {
							DestroyWME(radarCellIDs[width][height]);
							radarCellIDs[width][height] = agent.CreateIdWME(m_RadarWME, getCellID(state.getRadar()[width][height]));
							CreateIntWME(radarCellIDs[width][height], Names.kDistanceID, height);
							CreateStringWME(radarCellIDs[width][height], Names.kPositionID, getPositionID(width));
							if (state.getRadar()[width][height].player != null) {
								radarColors[width][height] = CreateStringWME(radarCellIDs[width][height], Names.kColorID, state.getRadar()[width][height].player.getColor());
								if (logger.isTraceEnabled()) {
									logger.trace(player.getName() + ": " + height + "," + width + ": " + getCellID(state.getRadar()[width][height]) + " " + state.getRadar()[width][height].player.getColor()); 
								}
							} else {
								if (logger.isTraceEnabled()) {
									logger.trace(player.getName() + ": " + height + "," + width + ": " + getCellID(state.getRadar()[width][height])); 
								}
							}
						}
					}
				}
			}
		}
	}

	private void clearRadar(TankState state) {
		for (int width = 0; width < TankState.RADAR_WIDTH; ++width) {
			for (int height = 0; height < TankState.RADAR_HEIGHT; ++height) {
				radarCellIDs[width][height] = null;
				radarColors[width][height] = null;
			}
		}
	}
	
	private String getCellID(RadarCell cell) {
		if (cell.player != null) {
			return Names.kTankID;
		}
		if (cell.obstacle) {
			return Names.kObstacleID;
		}
		if (cell.energy) {
			return Names.kEnergyID;
		}
		if (cell.health) {
			return Names.kHealthID;
		}
		if (cell.missiles) {
			return Names.kMissilesID;
		}
		return Names.kOpenID;
	}
	
	private String getPositionID(int i) {
		switch (i) {
		case 0:
			return Names.kLeftID;
		default:
		case 1:
			return Names.kCenterID;
		case 2:
			return Names.kRightID;
		}
	}


}
