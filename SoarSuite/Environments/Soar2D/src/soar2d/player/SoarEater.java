package soar2d.player;

import java.util.ArrayList;
import java.util.logging.*;

import sml.*;
import soar2d.Direction;
import soar2d.Names;
import soar2d.Simulation;
import soar2d.Soar2D;
import soar2d.World;
import soar2d.world.Cell;
import soar2d.world.CellObject;

class SoarCell {
	Identifier me;
	StringElement content;

	Identifier north;
	Identifier south;
	Identifier east;
	Identifier west;
	
	boolean iterated = false;
}

public class SoarEater extends Eater {
	Logger logger = Soar2D.logger;
	
	private Agent agent;
	private float random;
	java.awt.Point previousLocation;
	
	private StringElement directionWME;
	private IntElement scoreWME;
	private IntElement xWME;
	private IntElement yWME;
	private FloatElement randomWME;
	private SoarCell[][] cells = new SoarCell[(Soar2D.config.kEaterVision * 2 ) + 1][(Soar2D.config.kEaterVision * 2 ) + 1];

	public SoarEater(Agent agent, PlayerConfig playerConfig) {
		super(agent.GetAgentName(), playerConfig);
		this.agent = agent;
		
		previousLocation = new java.awt.Point(-1, -1);
		
		Identifier eater = agent.CreateIdWME(agent.GetInputLink(), Names.kEaterID);
		
		directionWME = agent.CreateStringWME(eater, Names.kDirectionID, Names.kNorth);
		agent.CreateStringWME(eater, Names.kNameID, getName());
		scoreWME = agent.CreateIntWME(eater, Names.kScoreID, getPoints());
		xWME = agent.CreateIntWME(eater, Names.kxID, 0);
		yWME = agent.CreateIntWME(eater, Names.kyID, 0);
		
		for (int i = 0; i < cells.length; ++i) {
			for (int j = 0; j < cells.length; ++j) {
				cells[i][j] = new SoarCell();
			}
		}
		
		cells[Soar2D.config.kEaterVision][Soar2D.config.kEaterVision].me = agent.CreateIdWME(agent.GetInputLink(), Names.kMyLocationID);
		createView(Soar2D.config.kEaterVision, Soar2D.config.kEaterVision);
		
		random = 0;
		generateNewRandom();
		randomWME = agent.CreateFloatWME(agent.GetInputLink(), Names.kRandomID, random);
		
		agent.Commit();
	}
	
	private void createView(int x, int y) {
		if (x >= 0 && x <= (Soar2D.config.kEaterVision * 2) && y >=0 && y <= (Soar2D.config.kEaterVision * 2) && !cells[x][y].iterated) {
			cells[x][y].iterated = true;
			cells[x][y].content = agent.CreateStringWME(cells[x][y].me, Names.kContentID, Names.kEmpty);

			if (x > 0) {
				if (cells[x - 1][y].me == null)
					cells[x - 1][y].me = agent.CreateIdWME(cells[x][y].me, Names.kWest);
				else
					cells[x][y].west = agent.CreateSharedIdWME(cells[x][y].me, Names.kWest, cells[x - 1][y].me);
			}
			
			if (x < (Soar2D.config.kEaterVision * 2)) {
				if (cells[x + 1][y].me == null)
					cells[x + 1][y].me = agent.CreateIdWME(cells[x][y].me, Names.kEast);
				else
					cells[x][y].east = agent.CreateSharedIdWME(cells[x][y].me, Names.kEast, cells[x + 1][y].me);
			}
			
			if (y > 0) {
				if (cells[x][y - 1].me == null)
					cells[x][y - 1].me = agent.CreateIdWME(cells[x][y].me, Names.kNorth);
				else
					cells[x][y].north = agent.CreateSharedIdWME(cells[x][y].me, Names.kNorth, cells[x][y - 1].me);
			}
			
			if (y < (Soar2D.config.kEaterVision * 2)) {
				if (cells[x][y + 1].me == null)
					cells[x][y + 1].me = agent.CreateIdWME(cells[x][y].me, Names.kSouth);
				else
					cells[x][y].south = agent.CreateSharedIdWME(cells[x][y].me, Names.kSouth, cells[x][y + 1].me);
			}
			
			createView(x - 1,y);
			createView(x + 1,y);
			createView(x,y - 1);
			createView(x,y + 1);
		}	
	}
	
	private void generateNewRandom() {
		float newRandom;
		do {
			newRandom = Simulation.random.nextFloat();
		} while (this.random == newRandom);
		this.random = newRandom;
	}
	
	public void update(World world, java.awt.Point location) {
		boolean moved = (location.x != this.previousLocation.x) || (location.y != this.previousLocation.y);
		
		java.awt.Point viewLocation = new java.awt.Point();
		for (int x = 0; x < cells.length; ++x) {
			viewLocation.x = x - Soar2D.config.kEaterVision + location.x;
			for (int y = 0; y < cells[x].length; ++y) {
				viewLocation.y = y - Soar2D.config.kEaterVision + location.y;
				if (!world.isInBounds(viewLocation)) {
					agent.Update(cells[x][y].content, Names.kWall);
					continue;
				}
				Cell cell = world.getCell(viewLocation.x, viewLocation.y);
				String content = null;
				if (cell.getPlayer() != null) {
					content = Names.kEater;
					
				} else {
					if (cell.enterable()) {
						ArrayList<CellObject> list = cell.getAllWithProperty(Names.kPropertyEdible);
						if (list.size() > 0) {
							// FIXME: deal with multiple foods
							content = list.get(0).getName();
						} else {
							content = Names.kEmpty;
						}
					} else {
						content = Names.kWall;
					}
				}
				
				if (moved || !cells[x][y].content.GetValue().equalsIgnoreCase(content)) {
					agent.Update(cells[x][y].content, content);
				}
			}
		}

		if (scoreWME.GetValue() != getPoints()) {
			agent.Update(scoreWME, getPoints());
		}
		
		if (!directionWME.GetValue().equalsIgnoreCase(Direction.stringOf[getFacingInt()])) {
			agent.Update(directionWME, Direction.stringOf[getFacingInt()]);
		}

		if (moved) {
			agent.Update(xWME, location.x);
			agent.Update(yWME, location.y);
		}
		
		// Random
		float oldrandom = random;
		do {
			random = Simulation.random.nextFloat();
		} while (random == oldrandom);
		agent.Update(randomWME, random);
		
		agent.Commit();
	}
	
	public MoveInfo getMove() {
		if (agent.GetNumberCommands() == 0) {
			if (logger.isLoggable(Level.FINE)) logger.fine(getName() + " issued no command.");
			return new MoveInfo();
		}
		
		if (agent.GetNumberCommands() > 1) {
			if (logger.isLoggable(Level.FINE)) logger.fine(getName() + " issued more than one command, using first.");
		}

		Identifier commandId = agent.GetCommand(0);
		String commandName = commandId.GetAttribute();

		MoveInfo move = new MoveInfo();
		if (commandName.equalsIgnoreCase(Names.kMoveID)) {
			move.move = true;
			move.jump = false;
		} else if (commandName.equalsIgnoreCase(Names.kJumpID)) {
			move.move = true;
			move.jump = true;
		} else if (commandName.equalsIgnoreCase(Names.kStopID)) {
			move.stop = true;
		} else {
			logger.warning("Unknown command: " + commandName);
			return new MoveInfo();
		}
		
		String donteat = commandId.GetParameterValue(Names.kDontEatID);
		if (donteat == null) {
			move.eat = true;
		} else {
			move.eat = donteat.equalsIgnoreCase(Names.kTrue) ? false : true;
		}
		
		String direction = commandId.GetParameterValue(Names.kDirectionID);
		if (direction != null) {
			move.moveDirection = Direction.getInt(direction); 
			this.setFacingInt(move.moveDirection);
			commandId.AddStatusComplete();
			agent.ClearOutputLinkChanges();
			agent.Commit();
			return move;
		}
		
		logger.warning("Improperly formatted command: " + commandName);
		return new MoveInfo();
	}
	
	public void reset() {
		agent.InitSoar();
	}
}
