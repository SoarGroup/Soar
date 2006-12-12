package soar2d.player;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
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
	HashMap<String, StringElement> content = new HashMap<String, StringElement>();
	
	Identifier box;
	HashMap<String, StringElement> boxProperties = new HashMap<String, StringElement>();

	Identifier north;
	Identifier south;
	Identifier east;
	Identifier west;
	
	boolean iterated = false;
}

public class SoarEater extends Eater {
	private Agent agent;
	private float random;
	
	private StringElement directionWME;
	private IntElement scoreWME;
	private IntElement xWME;
	private IntElement yWME;
	private FloatElement randomWME;
	private SoarCell[][] cells = new SoarCell[(Soar2D.config.kEaterVision * 2 ) + 1][(Soar2D.config.kEaterVision * 2 ) + 1];
	private ArrayList<String> shutdownCommands;

	public SoarEater(Agent agent, PlayerConfig playerConfig) {
		super(playerConfig);
		this.agent = agent;
		this.shutdownCommands = playerConfig.getShutdownCommands();
		
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
		moved = (location.x != this.previousLocation.x) || (location.y != this.previousLocation.y);
		
		java.awt.Point viewLocation = new java.awt.Point();
		for (int x = 0; x < cells.length; ++x) {
			viewLocation.x = x - Soar2D.config.kEaterVision + location.x;
			for (int y = 0; y < cells[x].length; ++y) {
				viewLocation.y = y - Soar2D.config.kEaterVision + location.y;

				SoarCell soarCell = cells[x][y];
				
				// Get cell if in bounds.
				Cell cell = null;
				if (world.isInBounds(viewLocation)) {
					cell = world.map.getCell(viewLocation.x, viewLocation.y);
				}
				
				// Clear the content if we moved.
				if (moved) {
					destroyWMEsAndClear(soarCell.content);
					if (soarCell.box != null) {
						agent.DestroyWME(soarCell.box);
						soarCell.box = null;
						soarCell.boxProperties.clear();
					}
					
					// If we're out of bounds create a wall
					if (cell == null) {
						createContent(soarCell.content, soarCell, Names.kWallID);
						continue;
					}
					
					// If cell is wall, do the wall
					if (!cell.enterable()) {
						ArrayList<CellObject> walls = cell.getAllWithProperty(Names.kPropertyBlock);
						assert walls.size() == 1;
						CellObject wall = walls.get(0);
						createContent(soarCell.content, soarCell, wall.getStringProperty(Names.kPropertyID));
						continue;
					}
					
					// player test
					Player player = cell.getPlayer();
					if (player != null) {
						createContent(soarCell.content, soarCell, Names.kEaterID);
					}
					
					// food test
					ArrayList<CellObject> foodList = Soar2D.simulation.world.map.getObjectManager().getTemplatesWithProperty(Names.kPropertyEdible);
					Iterator<CellObject> objectIter = foodList.iterator();
					boolean hadFood = false;
					while (objectIter.hasNext()) {
						CellObject food = objectIter.next();
						if (cell.hasObject(food.getName())) {
							hadFood = true;
							createContent(soarCell.content, soarCell, food.getStringProperty(Names.kPropertyID));
						}
					}
					
					// box test
					ArrayList<CellObject> boxes = cell.getAllWithProperty(Names.kPropertyBox);
					assert boxes.size() <= 1;
					if (boxes.size() > 0) {
						createBox(soarCell, boxes.get(0));
					
					}
					// empty test
					// a cell is empty if it doesn't have food, a player, or a box
					// wall is implied since we can't get here if there is a wall
					if(!hadFood && (player == null) && (boxes.size() == 0)) {
						createContent(soarCell.content, soarCell, Names.kEmpty);
					}
				} else {
				
					// We didn't move
					// Create a new name -> wme map
					// for each item (that we care about) in the cell
					//   if it doesn't exist in the soarcell map
					//     create it and store in new map
					//   else
					//     take existing and store in new map
					//   remove from soarcell map
					// destroy remaining elements in soarcell map
					// assign new map to soarcell map

					HashMap<String, StringElement> newContent = new HashMap<String, StringElement>();
	
					Player player = cell.getPlayer();
					if (player != null) {
						StringElement element = soarCell.content.remove(Names.kEaterID);
						if (element == null) {
							createContent(newContent, soarCell, Names.kEaterID);
						} else {
							newContent.put(Names.kEaterID, element);
						}
					}
					
					// food test
					ArrayList<CellObject> foodList = Soar2D.simulation.world.map.getObjectManager().getTemplatesWithProperty(Names.kPropertyEdible);
					Iterator<CellObject> objectIter = foodList.iterator();
					boolean hadFood = false;
					while (objectIter.hasNext()) {
						CellObject food = objectIter.next();
						if (cell.hasObject(food.getName())) {
							hadFood = true;
							StringElement element = soarCell.content.remove(food.getName());
							if (element == null) {
								createContent(newContent, soarCell, food.getStringProperty(Names.kPropertyID));
							} else {
								newContent.put(food.getStringProperty(Names.kPropertyID), element);
							}
						}
					}
					
					// box test is a special case
					ArrayList<CellObject> boxes = cell.getAllWithProperty(Names.kPropertyBox);
					assert boxes.size() <= 1;
					if (boxes.size() > 0) {
						CellObject box = boxes.get(0);
						if (soarCell.box == null) {
							assert soarCell.boxProperties.size() == 0;
							createBox(soarCell, box);
						} else {
							
							// Check the box properties using the same algorithm
							HashMap<String, StringElement> newBoxProperties = new HashMap<String, StringElement>();
							Iterator<String> iter = box.getPropertyNames().iterator();
							while (iter.hasNext()) {
								String name = iter.next();
								StringElement element = soarCell.boxProperties.remove(name);
								if (element == null) {
									String value = box.getStringProperty(name);
									element = agent.CreateStringWME(soarCell.box, name, value);
								}
								newBoxProperties.put(name, element);
							}
							
							destroyWMEsAndClear(soarCell.boxProperties);
							soarCell.boxProperties = newBoxProperties;
						}
					}
					
					// empty test
					// a cell is empty if it doesn't have food, a player, or a box
					// wall is implied since we can't get here if there is a wall
					if(!hadFood && (player == null) && (boxes.size() == 0)) {
						StringElement element = soarCell.content.remove(Names.kEmpty);
						if (element == null) {
							createContent(newContent, soarCell, Names.kEmpty);
						} else {
							newContent.put(Names.kEmpty, element);
						}
					}
					
					destroyWMEsAndClear(soarCell.content);
					soarCell.content = newContent;
				} // if moved/didn't move
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
	
	private void createBox(SoarCell soarCell, CellObject box) {
		soarCell.box = agent.CreateIdWME(soarCell.me, box.getStringProperty(Names.kPropertyID));
		assert soarCell.box != null;
		Iterator<String> iter = box.getPropertyNames().iterator();
		while (iter.hasNext()) {
			String name = iter.next();
			String value = box.getStringProperty(name);
			StringElement element = agent.CreateStringWME(soarCell.box, name, value);
			soarCell.boxProperties.put(name, element);
		}
	}
	
	private void destroyWMEsAndClear(HashMap<String, StringElement> map) {
		Iterator<StringElement> contentIter = map.values().iterator();
		while (contentIter.hasNext()) {
			StringElement element = contentIter.next();
			agent.DestroyWME(element);
		}
		map.clear();
	}
	
	private void createContent(HashMap<String, StringElement> map, SoarCell soarCell, String name) {
		StringElement element = agent.CreateStringWME(soarCell.me, Names.kContentID, name);
		assert element != null;
		map.put(name, element);
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
		} else if (commandName.equalsIgnoreCase(Names.kOpenID)) {
			move.open = true;
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
		
		Iterator<StringElement> iter;
		for (int x = 0; x < cells.length; ++x) {
			for (int y = 0; y < cells[x].length; ++y) {
				if (cells[x][y].boxProperties.size() > 0) {
					iter = cells[x][y].boxProperties.values().iterator();
					while (iter.hasNext()) {
						StringElement name = iter.next();
						agent.DestroyWME(name);
					}
					cells[x][y].boxProperties.clear();
				}
			}
		}
	}
	public void shutdown() {
		assert agent != null;
		if (shutdownCommands == null) { 
			return;
		}
		Iterator<String> iter = shutdownCommands.iterator();
		while(iter.hasNext()) {
			String command = iter.next();
			String result = getName() + ": result: " + agent.ExecuteCommandLine(command, true);
			Soar2D.logger.info(getName() + ": shutdown command: " + command);
			if (agent.HadError()) {
				Soar2D.control.severeError(result);
			} else {
				Soar2D.logger.info(getName() + ": result: " + result);
			}
		}
	}
}
