package edu.umich.soar.room.core.events;

import edu.umich.soar.room.map.Robot;

public class PlayerAddedEvent extends AbstractPlayerEvent {

	public PlayerAddedEvent(Robot player) {
		super(player);
	}
	
	
}
