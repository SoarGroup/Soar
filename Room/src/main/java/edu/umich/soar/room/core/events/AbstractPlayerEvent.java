package edu.umich.soar.room.core.events;

import edu.umich.soar.room.events.SimEvent;
import edu.umich.soar.room.map.Robot;

public abstract class AbstractPlayerEvent implements SimEvent {
	private final Robot player;
	
	AbstractPlayerEvent(Robot player) {
		this.player = player;
	}
	
	public Robot getPlayer() {
		return player;
	}
}
