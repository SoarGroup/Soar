package soar2d.player.kitchen;

import java.awt.Point;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.logging.Logger;

import soar2d.Direction;
import soar2d.Simulation;
import soar2d.Soar2D;
import soar2d.map.CellObject;
import soar2d.map.KitchenMap;
import soar2d.player.MoveInfo;
import soar2d.player.PlayerConfig;
import soar2d.tosca2d.*;
import soar2d.world.World;
import tosca.*;

/**
 * @author doug
 *
 * Represents the communication between an agent and Tosca
 */
public class ToscaCook extends Cook {
	protected Logger logger = Soar2D.logger;

	public ToscaCook( PlayerConfig playerConfig ) {
		super(playerConfig);
	}
	
	@Override
	public void update(java.awt.Point location) {
		World world = Soar2D.simulation.world;
		KitchenMap map = (KitchenMap)world.getMap();

		// check to see if we've moved
		super.update(location);
		
		if (moved) {
			// we have moved
			
			// location.x; // column on map
			// location.y; // row on map
			
			Point tempLocation;

			// examine cell to the north
			tempLocation = new Point(location);
			Direction.translate(tempLocation, Direction.kNorthInt);
			examineCell(map, tempLocation);

			// examine cell to the south
			tempLocation = new Point(location);
			Direction.translate(tempLocation, Direction.kSouthInt);
			examineCell(map, tempLocation);
			
			// examine cell to the east
			tempLocation = new Point(location);
			Direction.translate(tempLocation, Direction.kEastInt);
			examineCell(map, tempLocation);
			
			// examine cell to the west
			tempLocation = new Point(location);
			Direction.translate(tempLocation, Direction.kWestInt);
			examineCell(map, tempLocation);
			
			// examine current cell
			// note: we'll look at the specific objects in this cell later
			examineCell(map, location);
		}
		
		// Get all of the objects in this cell. 
		// Anything with a "smell" qualifies as an object we care about.
		ArrayList<CellObject> stuff = map.getAllWithProperty(location, "smell");
		Iterator<CellObject> stuffIter = stuff.iterator();
		while (stuffIter.hasNext()) {
			CellObject item = stuffIter.next();
			item.getName(); // name of the object (e.g., butter, toffee, mixture-5)
			// note: mixtures are named mixture-<id> where <id> is a unique id
			// agents shouldn't be aware of the id number though, they should just know it is a mixture
			
			item.getProperty("texture"); // texture of object (e.g., solid, liquid)
			item.getProperty("color"); // color of object (e.g., white, brown)
			item.getProperty("smell"); // smell of object (e.g., strong, mild)
		}
		
		this.getPointsDelta(); // points changed this frame (reward for last action)
	}
	
	private void examineCell(KitchenMap map, Point tempLocation) {
		if (map.getAllWithProperty(tempLocation, "smell").size() > 0) {
			// there is an object in the cell
		} else {
			// there is not an object in the cell
		}

		if (map.hasObject(tempLocation, "wall")) {
			// the cell is a wall-type cell
		} else if (map.hasObject(tempLocation, "oven")) {
			// the cell is an oven-type cell
		} else if (map.hasObject(tempLocation, "countertop")) {
			// the cell is a countertop-type cell
		} else {
			// the cell is a normal cell
		}
	}
	
	@Override
	public MoveInfo getMove() {
		MoveInfo move = new MoveInfo();
		
		// 1) example of normal move
		move.move = true;
		move.moveDirection = Direction.kNorthInt;
		
		// 2) example of move with object(s)
		move.move = true;
		move.moveDirection = Direction.kSouthInt;
		move.moveWithObject = true;
		
		// 3) example of mix
		move.mix = true;
		
		// 4) example of cook
		move.cook = true;
		
		// 5) example of eat
		move.eat = true;
		
		// only one type of move (1 - 5) can happen in any given frame
		// so:
		move.move = true;
		move.eat = true; // invalid: can't eat and move on same update
		
		return move;
	}
	
	@Override
	public void moveWithObjectFailed() {
		// called if move with object command failed
		// this only means that the object didn't move with the player
		// if the player also failed to move, moveFailed will also be called
	}

	@Override
	public void moveFailed() {
		// called if the move failed (e.g., in to a wall)
	}

	@Override
	public void mixFailed() {
		// called if the mix failed (e.g., not at countertop)
	}

	@Override
	public void cookFailed() {
		// called if the cook action failed (e.g., not at oven)
	}

	@Override
	public void eatFailed() {
		// called if the eat failed (e.g., nothing to eat)
	}
}
