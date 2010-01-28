package edu.umich.soar.room.core.events;

import edu.umich.soar.room.map.Robot;

public class PlayerRemovedEvent extends AbstractPlayerEvent {

	public PlayerRemovedEvent(Robot player) {
		super(player);
	}
}
