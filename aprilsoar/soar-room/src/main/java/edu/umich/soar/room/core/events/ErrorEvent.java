package edu.umich.soar.room.core.events;

public class ErrorEvent extends AbstractNotifyEvent {
	public ErrorEvent(String message) {
		super(message);
	}	
	
	public ErrorEvent(String title, String message) {
		super(title, message);
	}
}
