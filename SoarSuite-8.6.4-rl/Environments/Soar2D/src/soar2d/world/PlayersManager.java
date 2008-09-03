package soar2d.world;

import java.awt.Point;
import java.awt.geom.Point2D;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.ListIterator;

import soar2d.Soar2D;
import soar2d.player.MoveInfo;
import soar2d.player.Player;

public class PlayersManager {
	private ArrayList<Player> players = new ArrayList<Player>(7);
	private ArrayList<Player> humanPlayers = new ArrayList<Player>(7);
	private HashMap<String, Player> playersMap = new HashMap<String, Player>(7);
	private HashMap<Player, Point> initialLocations = new HashMap<Player, Point>(7);
	private HashMap<Player, Point> locations = new HashMap<Player, Point>(7);
	private HashMap<Player, Point2D.Double> floatLocations = new HashMap<Player, Point2D.Double>(7);
	private HashMap<Player, MoveInfo> lastMoves = new HashMap<Player, MoveInfo>(7);
	
	public int numberOfPlayers() {
		return players.size();
	}

	public Iterator<Player> humanIterator() {
		return humanPlayers.iterator();
	}
	
	public Iterator<Player> iterator() {
		return players.iterator();
	}
	
	public ListIterator<Player> listIterator() {
		return players.listIterator();
	}
	
	public ListIterator<Player> listIterator(int index) {
		return players.listIterator(index);
	}
	
	public Point getLocation(Player player) {
		return locations.get(player);
	}
	
	void setLocation(Player player, Point location) {
		locations.put(player, location);
	}
	
	public Point2D.Double getFloatLocation(Player player) {
		return floatLocations.get(player);
	}
	
	void setFloatLocation(Player player, Point2D.Double location) {
		floatLocations.put(player, location);
	}
	
	public MoveInfo getMove(Player player) {
		return lastMoves.get(player);
	}
	
	void setMove(Player player, MoveInfo move) {
		lastMoves.put(player, move);
	}
	
	public Player get(String name) {
		return playersMap.get(name);
	}
	
	public Player get(int index) {
		return players.get(index);
	}
	
	public int indexOf(Player player) {
		return players.indexOf(player);
	}
	
	void remove(Player player) {
		players.remove(player);
		humanPlayers.remove(player);
		playersMap.remove(player.getName());
		initialLocations.remove(player);
		locations.remove(player);
		floatLocations.remove(player);
		lastMoves.remove(player);
	}
	
	void add(Player player, Point initialLocation, boolean human) {
		players.add(player);
		playersMap.put(player.getName(), player);
		
		if (initialLocation != null) {
			initialLocations.put(player, initialLocation);
		}
		
		if (human) {
			humanPlayers.add(player);
		}
	}
	
	public boolean hasInitialLocation(Player player) {
		return initialLocations.containsKey(player);
	}

	public Point getInitialLocation(Player player) {
		return initialLocations.get(player);
	}
	
	public boolean exists(Player player) {
		return playersMap.containsKey(player.getName());
	}
	
	public double angleOff(Player left, Player right) {
		Point2D.Double target = new Point2D.Double();
		target.x = floatLocations.get(right).x;
		target.y = floatLocations.get(right).y;
		
		return angleOff(left, target);
	}

	public int size() {
		return players.size();
	}
	

	public double angleOff(Player left, Point2D.Double target) {
		Point2D.Double playerVector = new Point2D.Double();
		playerVector.x = floatLocations.get(left).x;
		playerVector.y = floatLocations.get(left).y;

		if (Soar2D.bConfig.getContinuous() == false) {
			// translate the player's location back a little bit to increase peripheral vision
			playerVector.x -= Math.cos(left.getHeadingRadians());
			playerVector.y -= Math.sin(left.getHeadingRadians());
		}
			
		Point2D.Double targetVector = new Point2D.Double();
		targetVector.x = target.x;
		targetVector.y = target.y;
		
		// translate target so i'm the origin
		targetVector.x -= playerVector.x;
		targetVector.y -= playerVector.y;
		
		// make target unit vector
		double targetVectorLength = Math.sqrt(Math.pow(targetVector.x, 2) + Math.pow(targetVector.y, 2));
		if (targetVectorLength > 0) {
			targetVector.x /= targetVectorLength;
			targetVector.y /= targetVectorLength;
		} else {
			targetVector.x = 0;
			targetVector.y = 0;
		}
		
		// make player facing vector
		playerVector.x = Math.cos(left.getHeadingRadians());
		playerVector.y = Math.sin(left.getHeadingRadians());
		
		double dotProduct = (targetVector.x * playerVector.x) + (targetVector.y * playerVector.y);
		double crossProduct = (targetVector.x * playerVector.y) - (targetVector.y * playerVector.x);
		
		// calculate inverse cosine of that for angle
		if (crossProduct < 0) {
			return Math.acos(dotProduct);
		}
		return Math.acos(dotProduct) * -1;
	}

}
