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
import soar2d.world.CellObject;

/**
 * @author voigtjr
 *
 * represents a cell in the 5x5 agent view
 */
class SoarCell {
	/**
	 * current cell id
	 */
	Identifier me;
	/**
	 * the list of stuff in the cell
	 */
	HashMap<String, StringElement> content = new HashMap<String, StringElement>();
	
	/**
	 * box in current cell, null if none
	 */
	Identifier box;
	/**
	 * properties on the box if there is one
	 */
	HashMap<String, StringElement> boxProperties = new HashMap<String, StringElement>();

	/**
	 * id (likely shared) to the cell/wme to the north
	 */
	Identifier north;
	/**
	 * id (likely shared) to the cell/wme to the south
	 */
	Identifier south;
	/**
	 * id (likely shared) to the cell/wme to the east
	 */
	Identifier east;
	/**
	 * id (likely shared) to the cell/wme to the west
	 */
	Identifier west;
	
	/**
	 * used during initialization
	 */
	boolean iterated = false;
}

/**
 * @author voigtjr
 *
 * the soar eater class is a human eater except controlled by soar instead of
 * human input.  someday may share code with eater so we're extending that for
 * now
 */
public class SoarEater extends Eater {
	/**
	 * the soar agent
	 */
	private Agent agent;
	/**
	 * a random number, guaranteed to change every frame
	 */
	private float random;
	
	/**
	 * which way we're facing
	 */
	private StringElement directionWME;
	/**
	 * our current score
	 */
	private IntElement scoreWME;
	/**
	 * our current x location
	 */
	private IntElement xWME;
	/**
	 * our current y location
	 */
	private IntElement yWME;
	/**
	 * the wme for the random number
	 */
	private FloatElement randomWME;
	/**
	 * the 5x5 vision grid
	 */
	private SoarCell[][] cells = new SoarCell[(Soar2D.config.kEaterVision * 2 ) + 1][(Soar2D.config.kEaterVision * 2 ) + 1];
	/**
	 * soar commands to run before this agent is destroyed
	 */
	private ArrayList<String> shutdownCommands;

	/**
	 * @param agent a valid soar agent
	 * @param playerConfig the rest of the player config
	 */
	public SoarEater(Agent agent, PlayerConfig playerConfig) {
		super(playerConfig, false);
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

		// bootstrap the 5x5 grid
		cells[Soar2D.config.kEaterVision][Soar2D.config.kEaterVision].me = agent.CreateIdWME(agent.GetInputLink(), Names.kMyLocationID);
		createView(Soar2D.config.kEaterVision, Soar2D.config.kEaterVision);
		
		random = 0;
		generateNewRandom();
		randomWME = agent.CreateFloatWME(agent.GetInputLink(), Names.kRandomID, random);
		
		agent.Commit();
	}
	
	/**
	 * @param x current cell x location to init
	 * @param y current cell y location to init
	 * 
	 * recursive function to initialize the 5x5 grid. kind of confusing but works great.
	 * each cell's iterated bool gets set true if it is initialized.
	 */
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
	
	/**
	 * create a new random number
	 * make sure it is different from current
	 */
	private void generateNewRandom() {
		float newRandom;
		do {
			newRandom = Simulation.random.nextFloat();
		} while (this.random == newRandom);
		this.random = newRandom;
	}
	
	/* (non-Javadoc)
	 * @see soar2d.player.Eater#update(soar2d.World, java.awt.Point)
	 */
	public void update(World world, java.awt.Point location) {
		// check to see if we've moved
		moved = (location.x != this.previousLocation.x) || (location.y != this.previousLocation.y);
		if (moved) {
			this.previousLocation = new java.awt.Point(location);
		}
		
		// update the 5x5
		java.awt.Point viewLocation = new java.awt.Point();
		for (int x = 0; x < cells.length; ++x) {
			viewLocation.x = x - Soar2D.config.kEaterVision + location.x;
			for (int y = 0; y < cells[x].length; ++y) {
				viewLocation.y = y - Soar2D.config.kEaterVision + location.y;

				// get the current soarcell to update
				SoarCell soarCell = cells[x][y];
				
				// Clear the content if we moved.
				if (moved) {
					
					// we've moved, clear anything there
					destroyWMEsAndClear(soarCell.content);
					if (soarCell.box != null) {
						agent.DestroyWME(soarCell.box);
						soarCell.box = null;
						soarCell.boxProperties.clear();
					}
					
					// If we're out of bounds create a wall
					if (!world.isInBounds(viewLocation)) {
						createContent(soarCell.content, soarCell, Names.kWallID);
						continue;
					}
					
					// If cell is wall, do the wall
					if (!world.map.enterable(viewLocation)) {
						// get all things that block
						ArrayList<CellObject> walls = world.map.getAllWithProperty(viewLocation, Names.kPropertyBlock);
						
						// we must have at least one
						assert walls.size() == 1;
						
						// get the object
						CellObject wall = walls.get(0);
						
						// update the soarcell with the new content
						// use the id property as its id on the input link
						assert wall.hasProperty(Names.kPropertyID);
						createContent(soarCell.content, soarCell, wall.getProperty(Names.kPropertyID));
						continue;
					}
					
					// if we get to this point, the cell is not a wall
					
					// player test
					Player player = world.map.getPlayer(viewLocation);
					if (player != null) {
						// ther is a player in the cell, use the eaterid
						createContent(soarCell.content, soarCell, Names.kEaterID);
					}
					
					// food test
					// get all foods
					ArrayList<CellObject> foodList = world.map.getObjectManager().getTemplatesWithProperty(Names.kPropertyEdible);
					Iterator<CellObject> objectIter = foodList.iterator();
					boolean hadFood = false;
					while (objectIter.hasNext()) {
						// for each food
						CellObject food = objectIter.next();
						// see if it is in the cell
						if (world.map.hasObject(viewLocation, food.getName())) {
							// yes
							hadFood = true;
							// create it using its id property on the link
							assert food.hasProperty(Names.kPropertyID);
							createContent(soarCell.content, soarCell, food.getProperty(Names.kPropertyID));
						}
					}
					
					// box test
					// get all boxes
					ArrayList<CellObject> boxes = world.map.getAllWithProperty(viewLocation, Names.kPropertyBox);
					assert boxes.size() <= 1;
					if (boxes.size() > 0) {
						// create them if necessary
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

					// create the new map
					HashMap<String, StringElement> newContent = new HashMap<String, StringElement>();

					// Anything out of bounds will not change
					if (!world.isInBounds(viewLocation)) {
						continue;
					}
	
					// check for a player
					Player player = world.map.getPlayer(viewLocation);
					if (player != null) {
						
						// there is a player, see if there was one there before
						StringElement element = soarCell.content.remove(Names.kEaterID);
						if (element == null) {
							// no, create it
							createContent(newContent, soarCell, Names.kEaterID);
						} else {
							// yes, keep it
							newContent.put(Names.kEaterID, element);
						}
						
						// TODO: we don't tell if it is a different player, it is possible if it is.
						// perhaps we should blink if it is a different player.
					}
					
					// food test
					// get all foods
					ArrayList<CellObject> foodList = Soar2D.simulation.world.map.getObjectManager().getTemplatesWithProperty(Names.kPropertyEdible);
					Iterator<CellObject> objectIter = foodList.iterator();
					boolean hadFood = false;
					while (objectIter.hasNext()) {
						// for each food
						CellObject food = objectIter.next();
						// does the cell have one
						if (world.map.hasObject(viewLocation, food.getName())) {
							// yes
							hadFood = true;
							
							// did it have one before
							StringElement element = soarCell.content.remove(food.getName());
							if (element == null) {
								// no, create it
								assert food.hasProperty(Names.kPropertyID);
								createContent(newContent, soarCell, food.getProperty(Names.kPropertyID));
							} else {
								// yes, save it
								assert food.hasProperty(Names.kPropertyID);
								newContent.put(food.getProperty(Names.kPropertyID), element);
							}
						}
					}
					
					// box test is a special case
					// get all boxes
					ArrayList<CellObject> boxes = world.map.getAllWithProperty(viewLocation, Names.kPropertyBox);
					
					// TODO: there can only be one (as of right now)
					assert boxes.size() <= 1;
					
					// if there are boxes
					if (boxes.size() > 0) {
						CellObject box = boxes.get(0);
						// was there a box
						if (soarCell.box == null) {
							// no, so make sure there are no properties
							assert soarCell.boxProperties.size() == 0;
							
							// and create one
							createBox(soarCell, box);
						} else {
							
							// there was a box
							
							// Check the box properties using the same algorithm
							HashMap<String, StringElement> newBoxProperties = new HashMap<String, StringElement>();
							Iterator<String> iter = box.getPropertyNames().iterator();
							while (iter.hasNext()) {
								// for each property on the box
								String name = iter.next();
								// was the property on it before?
								StringElement element = soarCell.boxProperties.remove(name);
								if (element == null) {
									// no, add it
									String value = box.getProperty(name);
									element = agent.CreateStringWME(soarCell.box, name, value);
								}
								// save it regardless
								newBoxProperties.put(name, element);
							}
							
							// get rid of the old properties
							destroyWMEsAndClear(soarCell.boxProperties);
							// set the new ones
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
					
					// get rid of the old content
					destroyWMEsAndClear(soarCell.content);
					
					// set the new content
					soarCell.content = newContent;
				} // if moved/didn't move
			}
		}

		// update the score if it changed
		if (scoreWME.GetValue() != getPoints()) {
			agent.Update(scoreWME, getPoints());
		}
		
		// update the facing if it changed
		if (!directionWME.GetValue().equalsIgnoreCase(Direction.stringOf[getFacingInt()])) {
			agent.Update(directionWME, Direction.stringOf[getFacingInt()]);
		}

		// if we moved, update the location
		if (moved) {
			agent.Update(xWME, location.x);
			agent.Update(yWME, location.y);
		}
		
		// update the random no matter what
		float oldrandom = random;
		do {
			random = Simulation.random.nextFloat();
		} while (random == oldrandom);
		agent.Update(randomWME, random);
		
		// commit everything
		agent.Commit();
	}
	
	/**
	 * @param soarCell the cell that contains the box
	 * @param box the box object
	 */
	private void createBox(SoarCell soarCell, CellObject box) {
		// create the wme
		assert box.hasProperty(Names.kPropertyID);
		soarCell.box = agent.CreateIdWME(soarCell.me, box.getProperty(Names.kPropertyID));
		// make sure that happened
		assert soarCell.box != null;
		
		// go through the properties and add them
		Iterator<String> iter = box.getPropertyNames().iterator();
		while (iter.hasNext()) {
			String name = iter.next();
			String value = box.getProperty(name);
			StringElement element = agent.CreateStringWME(soarCell.box, name, value);
			soarCell.boxProperties.put(name, element);
		}
	}
	
	/**
	 * @param map the map to clear
	 * 
	 * take all the wmes in the map and destroy them, then clear the map
	 */
	private void destroyWMEsAndClear(HashMap<String, StringElement> map) {
		Iterator<StringElement> contentIter = map.values().iterator();
		while (contentIter.hasNext()) {
			StringElement element = contentIter.next();
			agent.DestroyWME(element);
		}
		map.clear();
	}
	
	/**
	 * @param map the name to element mapping for the cell
	 * @param soarCell the cell
	 * @param name the name of the content to create
	 */
	private void createContent(HashMap<String, StringElement> map, SoarCell soarCell, String name) {
		// create the wme
		StringElement element = agent.CreateStringWME(soarCell.me, Names.kContentID, name);
		// make sure it happened
		assert element != null;
		// store the element
		map.put(name, element);
	}
	
	/* (non-Javadoc)
	 * @see soar2d.player.Eater#getMove()
	 */
	public MoveInfo getMove() {
		// if there was no command issued, that is kind of strange
		if (agent.GetNumberCommands() == 0) {
			if (logger.isLoggable(Level.FINE)) logger.fine(getName() + " issued no command.");
			return new MoveInfo();
		}

		// go through the commands
		// see move info for details
		MoveInfo move = new MoveInfo();
		for (int i = 0; i < agent.GetNumberCommands(); ++i) {
			Identifier commandId = agent.GetCommand(i);
			String commandName = commandId.GetAttribute();
			
			if (commandName.equalsIgnoreCase(Names.kMoveID)) {
				if (move.move) {
					logger.warning(getName() + "multiple move/jump commands detected (move)");
					continue;
				}
				move.move = true;
				move.jump = false;
				
				String direction = commandId.GetParameterValue(Names.kDirectionID);
				if (direction != null) {
					move.moveDirection = Direction.getInt(direction); 
					this.setFacingInt(move.moveDirection);
					commandId.AddStatusComplete();
					continue;
				}
				
			} else if (commandName.equalsIgnoreCase(Names.kJumpID)) {
				if (move.move) {
					logger.warning(getName() + "multiple move/jump commands detected, ignoring (jump)");
					continue;
				}
				move.move = true;
				move.jump = true;
				String direction = commandId.GetParameterValue(Names.kDirectionID);
				if (direction != null) {
					move.moveDirection = Direction.getInt(direction); 
					this.setFacingInt(move.moveDirection);
					commandId.AddStatusComplete();
					continue;
				}

			} else if (commandName.equalsIgnoreCase(Names.kStopSimID)) {
				if (move.stopSim) {
					logger.warning(getName() + "multiple stop commands detected, ignoring");
					continue;
				}
				move.stopSim = true;
				commandId.AddStatusComplete();
				continue;
				
			} else if (commandName.equalsIgnoreCase(Names.kOpenID)) {
				if (move.open) {
					logger.warning(getName() + "multiple open commands detected, ignoring");
					continue;
				}
				move.open = true;
				commandId.AddStatusComplete();
				continue;
				
			} else if (commandName.equalsIgnoreCase(Names.kDontEatID)) {
				if (move.dontEat) {
					logger.warning(getName() + "multiple dont eat commands detected, ignoring");
					continue;
				}
				move.dontEat = true;
				commandId.AddStatusComplete();
				continue;
				
			} else {
				logger.warning("Unknown command: " + commandName);
				continue;
			}
			
			logger.warning("Improperly formatted command: " + commandName);
		}
		agent.ClearOutputLinkChanges();
		agent.Commit();
		return move;
	}
	
	/* (non-Javadoc)
	 * @see soar2d.player.Player#reset()
	 */
	public void reset() {
		
		previousLocation = new java.awt.Point(-1, -1);
		
		// clear the 5x5
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

		agent.InitSoar();
	}
	
	/* (non-Javadoc)
	 * @see soar2d.player.Eater#shutdown()
	 */
	public void shutdown() {
		assert agent != null;
		if (shutdownCommands != null) { 
			// execute the pre-shutdown commands
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
}
