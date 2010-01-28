package edu.umich.soar.room.core.events;

import edu.umich.soar.room.map.Robot;


public class HumanInputEvent extends AbstractPlayerEvent {

	HumanInputEvent(Robot player) {
		super(player);
	}
}
