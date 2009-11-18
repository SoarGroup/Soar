package edu.umich.soar.room.core.events;

import edu.umich.soar.room.events.SimEvent;

public class AbstractNotifyEvent implements SimEvent {

	private final String title;
	private final String message;
	
	AbstractNotifyEvent(String title, String message) {
		this.title = title;
		this.message = message;
	}
	
	AbstractNotifyEvent(String message) {
		this.title = null;
		this.message = message;
	}
	
	public String getTitle() {
		return title;
	}
	
	public String getMessage() {
		return message;
	}
}
