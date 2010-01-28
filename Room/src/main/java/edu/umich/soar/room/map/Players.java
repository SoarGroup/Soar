package edu.umich.soar.room.map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

class Players {
	private static Log logger = LogFactory.getLog(Players.class);

	public static void adjustPoints(Robot player, Points points, int delta, String comment) {
		int previous = points.getPoints();
		points.adjustPoints(delta);
		scoreComment(player, points, comment, previous);
	}

	public static void setPoints(Robot player, Points points, int amount, String comment) {
		int previous = points.getPoints();
		points.setPoints(amount);
		scoreComment(player, points, comment, previous);
	}
	
	private static void scoreComment(Robot player, Points points, String comment, int previous) {
		if (comment != null) {
			logger.info(player + " score: " + Integer.toString(previous) + " -> " + Integer.toString(points.getPoints()) + " (" + comment + ")");
		} else {
			logger.info(player + " score: " + Integer.toString(previous) + " -> " + Integer.toString(points.getPoints()));
		}
	}
}