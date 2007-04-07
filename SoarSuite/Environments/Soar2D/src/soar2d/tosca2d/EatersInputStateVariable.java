/**
 * 
 */
package soar2d.tosca2d;

/**
 * @author Doug
 *
 */

import java.awt.Point;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

import soar2d.*;
import soar2d.player.ToscaEater;
import soar2d.world.CellObject;
import tosca.Double;
import tosca.Group;
import tosca.Integer;
import tosca.RefValue;
import tosca.Value;
import tosca.Vector;

public class EatersInputStateVariable extends JavaStateVariable {
	protected Value m_Value = new Value() ;
	
	private static final int kInfoBoxID = 0;

	public EatersInputStateVariable(String name) {
		super(name) ;
	}
	
	public void Initialize() {
		// For the moment, initialize as an empty group
		Group init = new Group() ;
		GetCurrentValue().TakeGroup(init) ;
	}

	private RefValue createMapCell(ToscaEater eater, World world, Point viewLocation, Point location)
	{
		int maxProperties = 20 ;
		Vector mapCell = new Vector(maxProperties) ;
		
		mapCell.Set(0,0.0) ;	// Property 0 is whether this location is valid or not
		
		if (world.getMap().isInBounds(viewLocation)) {
			mapCell.Set(0, 1.0) ;	// Property 0 is whether this location is valid or not
			
			if (!world.getMap().enterable(viewLocation)) {
				mapCell.Set(1, 1.0) ;	// Property 1 is whether it's a wall
			} else {
				// Property 3: is there a box in the cell
				// box test
				// get all boxes
				ArrayList<CellObject> boxes = world.getMap().getAllWithProperty(viewLocation, Names.kPropertyBox);
				// max one box per cell, we're not prepared to handle more
				assert boxes.size() <= 1;
				if (boxes.size() > 0) {
					// there is a box
					mapCell.Set(3, 1.0);

					// Property 4: box id
					// box id 0 is the info box (kInfoBoxID)
					// box id 1..<number of boxes> are reward boxes

					// id the box
					CellObject box = boxes.get(0);
					assert box.hasProperty(Names.kPropertyBoxID);
					int boxID = box.getIntProperty(Names.kPropertyBoxID);
					
					// box should never be negative
					assert boxID >= 0;
					mapCell.Set(4, boxID);

					// Property 5 is the symbol of the box (the target box) from the info box
					// Property 6 is the action (open-code)
					// Both of these only set if they exist

					// if we're on the info box
					if (boxID == kInfoBoxID) {
						// if the target box property exists
						if (box.hasProperty(Names.kPropertyPositiveBoxID)) {
							// set property 5 to the target
							mapCell.Set(5, box.getIntProperty(Names.kPropertyPositiveBoxID));
						} else {
							mapCell.Set(5, 0);
						}
						
						// if the action (open) code exists
						if (box.hasProperty(Names.kPropertyOpenCode)) {
							// set property 6 to the code
							mapCell.Set(6, box.getIntProperty(Names.kPropertyOpenCode));
						} else {
							mapCell.Set(6, 0);
						}
					}

				} else {
					// there is no box
					mapCell.Set(3, 0);
					mapCell.Set(4, 0);
					mapCell.Set(5, 0);
					mapCell.Set(6, 0);
				}
			}
		}
		
		return mapCell ;
	}
	
	public void update(int time, soar2d.player.ToscaEater eater, World world, java.awt.Point location) {
		// The value is stored as a group containing some named values and
		// then a map group which contains all of the cells around this eater
		Group main = new Group() ;
		main.AddNamedValue("x", new tosca.Integer(location.x)) ;
		main.AddNamedValue("y", new tosca.Integer(location.y)) ;
		main.AddNamedValue("facing", new tosca.Integer(eater.getEater().getFacingInt())) ;
		
		double reward = -1.0;
		if (eater.getEater().pointsChanged()) {
			reward = eater.getEater().getPointsDelta();
		}
		main.AddNamedValue("reward", new tosca.Double(reward));
		
		main.AddNamedValue("world-count", new tosca.Integer(world.getWorldCount()));
		
		Group map = new Group() ;
		
		java.awt.Point viewLocation = new java.awt.Point();
		int visionRange = Soar2D.config.getEaterVision() ;
		for (int x = location.x - visionRange; x <= location.x + visionRange; ++x) {
			for (int y = location.y - visionRange; y <= location.y + visionRange; ++y) {
				viewLocation.x = x ;
				viewLocation.y = y ;

				RefValue mapCell = createMapCell(eater, world, viewLocation, location) ;				
				
				// We'll name the map entries "cell00" to "cell55" so we can pull them out by name if we wish.
				int indexX = x - (location.x - visionRange) ;
				int indexY = y - (location.y - visionRange) ;
				
				String name = "cell" + indexX + indexY ;
				map.AddNamedValue(name, mapCell) ;
			}
		}
		main.AddNamedValue("map", map) ;
		
		/*
		// update the 5x5
		java.awt.Point viewLocation = new java.awt.Point();
		for (int x = 0; x < cells.length; ++x) {
			viewLocation.x = x - Soar2D.config.kEaterVision + location.x;
			for (int y = 0; y < cells[x].length; ++y) {
				viewLocation.y = y - Soar2D.config.kEaterVision + location.y;

				// get the current soarcell to update
				SoarCell soarCell = cells[x][y];
				
				// Get real cell if in bounds.
				Cell cell = null;
				if (world.isInBounds(viewLocation)) {
					cell = world.map.getCell(viewLocation.x, viewLocation.y);
				}
				
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
					if (cell == null) {
						createContent(soarCell.content, soarCell, Names.kWallID);
						continue;
					}
					
					// If cell is wall, do the wall
					if (!cell.enterable()) {
						// get all things that block
						ArrayList<CellObject> walls = cell.getAllWithProperty(Names.kPropertyBlock);
						
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
					Player player = cell.getPlayer();
					if (player != null) {
						// ther is a player in the cell, use the eaterid
						createContent(soarCell.content, soarCell, Names.kEaterID);
					}
					
					// food test
					// get all foods
					ArrayList<CellObject> foodList = Soar2D.simulation.world.map.getObjectManager().getTemplatesWithProperty(Names.kPropertyEdible);
					Iterator<CellObject> objectIter = foodList.iterator();
					boolean hadFood = false;
					while (objectIter.hasNext()) {
						// for each food
						CellObject food = objectIter.next();
						// see if it is in the cell
						if (cell.hasObject(food.getName())) {
							// yes
							hadFood = true;
							// create it using its id property on the link
							assert food.hasProperty(Names.kPropertyID);
							createContent(soarCell.content, soarCell, food.getProperty(Names.kPropertyID));
						}
					}
					
					// box test
					// get all boxes
					ArrayList<CellObject> boxes = cell.getAllWithProperty(Names.kPropertyBox);
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
					if (cell == null) {
						continue;
					}
	
					// check for a player
					Player player = cell.getPlayer();
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
						if (cell.hasObject(food.getName())) {
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
					ArrayList<CellObject> boxes = cell.getAllWithProperty(Names.kPropertyBox);
					
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
		*/
		
		//Double val = new Double() ;
		//val.SetFromDouble(time) ;
		
		Value value = new Value(main) ;
		SetValue(value, time) ;
	}
	
	protected Value GetCurrentValue() { return m_Value ; }
}
