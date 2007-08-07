package soar2d.world;

import java.awt.Point;
import java.util.ArrayList;
import java.util.Iterator;

import soar2d.Direction;
import soar2d.Soar2D;
import soar2d.configuration.Configuration;
import soar2d.map.CellObject;
import soar2d.map.GridMap;
import soar2d.map.KitchenMap;
import soar2d.player.MoveInfo;
import soar2d.player.Player;

public class KitchenWorld implements IWorld {

	public void fragPlayer(Player player, GridMap map, PlayersManager players,
			Point location) {

	}

	public boolean postLoad(GridMap newMap) {
		return true;
	}

	public void putInStartingLocation(Player player, GridMap map,
			PlayersManager players, Point location) {
	}

	public void reset() {

	}

	public boolean update(GridMap _map, PlayersManager players) {
		KitchenMap map = (KitchenMap)_map;
		
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			MoveInfo move = players.getMove(player);
			
			// check for break-out
			if (Soar2D.control.isShuttingDown()) {
				return false;
			}

			if (move.move) {
				// Calculate new location
				Point oldLocation = players.getLocation(player);
				Point newLocation = new Point(oldLocation);
				Direction.translate(newLocation, move.moveDirection);
				
				// Verify legal move and commit move
				if (map.isInBounds(newLocation) && map.enterable(newLocation)) {
					// remove from cell
					map.setPlayer(oldLocation, null);
					players.setLocation(player, newLocation);
					
					// todo: collisions not handled
					map.setPlayer(newLocation, player);

					// move stuff with the player
					if (move.moveWithObject) {
						ArrayList<CellObject> stuff = map.getAllWithProperty(oldLocation, "smell");
						if (stuff.size() > 0) {
							map.removeAllWithProperty(oldLocation, "smell");
							
							Iterator<CellObject> stuffIter = stuff.iterator();
							while (stuffIter.hasNext()) {
								map.addObjectToCell(newLocation, stuffIter.next());
							}
						}
					}
				}
			}
			
			if (move.mix) {
				ArrayList<CellObject> stuff = map.getAllWithProperty(players.getLocation(player), "smell");
				if (stuff.size() > 1) {
					map.removeAllWithProperty(players.getLocation(player), "smell");

					CellObject mixture = map.createObjectByName("mixture");
					mixture.addProperty("shape", "triangle");

					Iterator<CellObject> stuffIter;

					// texture
					boolean powder = false;
					mixture.addProperty("texture", "solid");
					stuffIter = stuff.iterator();
					while (stuffIter.hasNext()) {
						CellObject ingredient = stuffIter.next();
						if (ingredient.getProperty("texture").equals("liquid")) {
							mixture.addProperty("texture", "liquid");
							break;
							
						} else if (!powder && ingredient.getProperty("texture").equals("powder")) {
							mixture.addProperty("texture", "powder");
							powder = true;
						}
					}
					
					// color
					boolean brown = false;
					boolean yellow = false;
					mixture.addProperty("color", "white");
					stuffIter = stuff.iterator();
					while (stuffIter.hasNext()) {
						CellObject ingredient = stuffIter.next();
						if (ingredient.getProperty("color").equals("black")) {
							mixture.addProperty("color", "black");
							break;
						} else if (!brown) {
							if (ingredient.getProperty("color").equals("brown")) {
								mixture.addProperty("color", "brown");
								brown = true;
							} else if (!yellow && ingredient.getProperty("color").equals("yellow")) {
								mixture.addProperty("color", "yellow");
								yellow = true;
							}
						}
					}
					
					// smell
					stuffIter = stuff.iterator();
					CellObject left = stuffIter.next();
					Smell newSmell = toSmell(left.getProperty("smell"));
					while (stuffIter.hasNext()) {
						CellObject right = stuffIter.next();
						newSmell = mixSmell(newSmell, toSmell(right.getProperty("smell")));
					}
					switch (newSmell) {
					case None:
						mixture.addProperty("smell", "none");
						break;
					case Mild:
						mixture.addProperty("smell", "mild");
						break;
					case Strong:
						mixture.addProperty("smell", "strong");
						break;
					}
					
					Soar2D.logger.info("New mixture: " + mixture.getProperty("texture") 
							+ "/" + mixture.getProperty("color") 
							+ "/" + mixture.getProperty("smell"));
					
					map.addObjectToCell(players.getLocation(player), mixture);
				} else {
					Soar2D.logger.info("Ignoring mix of less than 2 ingredients");
				}
			}
			
			if (move.cook) {
				ArrayList<CellObject> stuff = map.getAllWithProperty(players.getLocation(player), "smell");
				if (stuff.size() > 0) {
					map.removeAllWithProperty(players.getLocation(player), "smell");
				}
			}
			
			if (move.eat) {
				ArrayList<CellObject> stuff = map.getAllWithProperty(players.getLocation(player), "smell");
				if (stuff.size() > 0) {
					map.removeAllWithProperty(players.getLocation(player), "smell");
				}
			}
		}
		
		updatePlayers(false, map, players);

		map.updateObjects(null);
		
		map.spawnBasics();

		// do not reset after this frame
		return false;
	}

	private enum Smell { None, Mild, Strong };
	
	private Smell toSmell(String smell) {
		if (smell.equalsIgnoreCase("strong")) {
			return Smell.Strong;
		}
		if (smell.equalsIgnoreCase("mild")) {
			return Smell.Mild;
		}
		return Smell.None;
	}
	
	private Smell mixSmell(Smell left, Smell right) {
		
		switch (left) {
		case None:
			switch (right) {
			case None:
				return Smell.None;
			case Mild:
			case Strong:
				return Smell.Mild;
			}
		case Mild:
			switch (right) {
			case None:
				return Smell.Mild;
			case Mild:
			case Strong:
				return Smell.Strong;
			}
		case Strong:
			switch (right) {
			case None:
				return Smell.Mild;
			case Mild:
			case Strong:
				return Smell.Strong;
			}
		}
		assert false;
		return Smell.None;
	}
	
	public void updatePlayers(boolean playersChanged, GridMap map, PlayersManager players) {
		Iterator<Player> iter = players.iterator();
		while (iter.hasNext()) {
			Player player = iter.next();
			player.update(players.getLocation(player));
		}
	}
	
	public int getMinimumAvailableLocations() {
		return 1;
	}

	public void resetPlayer(GridMap map, Player player, PlayersManager players, boolean resetDuringRun) {
		player.reset();
	}

	public GridMap newMap(Configuration config) {
		return new KitchenMap(config);
	}
}
