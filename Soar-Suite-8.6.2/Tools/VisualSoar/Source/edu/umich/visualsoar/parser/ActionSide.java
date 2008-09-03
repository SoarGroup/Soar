package edu.umich.visualsoar.parser;

import java.util.*;

public class ActionSide {
	// Data Members
	private List d_actions = new LinkedList();
	
	// Constructors
	public ActionSide() {}
	
	// Accessors
	public final void add(Action action) {
		d_actions.add(action);
	}
	
	public final Iterator getActions() {
		return d_actions.iterator();
	}	
}
