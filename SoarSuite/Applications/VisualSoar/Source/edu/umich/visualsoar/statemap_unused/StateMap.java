package edu.umich.visualsoar.statemap;
import edu.umich.visualsoar.graph.*;
import java.util.*;
import edu.umich.visualsoar.datamap.*;

public class StateMap {
	private DirectedGraph rep = new OrderedDirectedGraphAsAdjacencyLists();
	private List listeners = new LinkedList();
	private StateVertex topState;

	public StateMap() {}
	
	public StateMap(String projectName) {
		topState = createNewStateVertex(projectName);
	}
	
	public StateVertex getTopState() {
		return topState;
	}
	
	public StateVertex getVertexForId(int id) {
		return (StateVertex)rep.selectVertex(id);
	}
	
	public void addEdge(SoarVertex v0,SoarVertex v1) {
		if (!v0.allowsEmanatingEdges()) 
			throw new IllegalArgumentException("The First SoarVertex does not allow emanating edges");
		NamedEdge edge = new StateMapEdge(v0,v1);
		rep.addEdge(edge);
		notifyListenersOfAdd(edge);
	}
	
	public void removeEdge(SoarVertex v0,SoarVertex v1) {
		NamedEdge edge = new StateMapEdge(v0,v1);
		rep.removeEdge(edge);
		notifyListenersOfRemove(edge);
	}
	
	public StateVertex createNewStateVertex(String name) {
		int id = getNextVertexId();
		StateVertex sv = new StateVertex(id,name);
		rep.addVertex(sv);
		return sv;
	}
		
	public int getNextVertexId() {
		return rep.numberOfVertices();
	}
	
	private void notifyListenersOfAdd(NamedEdge e) {
		Iterator i = listeners.iterator();
		StateMapEvent sme = new StateMapEvent(e);
		while(i.hasNext()) {
			StateMapListener sml = (StateMapListener)i.next();
			sml.edgeAdded(sme);
		}
	}
	
	private void notifyListenersOfRemove(NamedEdge e) {
		Iterator i = listeners.iterator();
		StateMapEvent sme = new StateMapEvent(e);
		while(i.hasNext()) {
			StateMapListener sml = (StateMapListener)i.next();
			sml.edgeRemoved(sme);
		}
	}
	
	public void addStateMapListener(StateMapListener sml) {
		listeners.add(sml);
	}
	
	public void removeStateMapListener(StateMapListener sml) {
		listeners.remove(sml);
	}
	
	public Enumeration getEmanatingEdges(SoarVertex sv) {
		return rep.emanatingEdges(sv);	
	}
}
