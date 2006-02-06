package edu.umich.visualsoar.statemap;

import edu.umich.visualsoar.graph.NamedEdge;

public class StateMapEvent {
	private NamedEdge edge;

	private StateMapEvent() {}
	
	public StateMapEvent(NamedEdge inEdge) {
		edge = inEdge;
	}
	
	public NamedEdge getEdge() {
		return edge;
	}
}
