package edu.umich.visualsoar.statemap;
import edu.umich.visualsoar.graph.*;
import java.util.*;
import javax.swing.event.TreeModelEvent;
import javax.swing.*;
import java.awt.Component;

/**
 * This class takes graph nodes and cleverly (or not so cleverly) disguises
 * as tree nodes, to prevent infinite recursion, the children are loaded when
 * needed
 * @author Brad Jones
 */

public class PseudoTreeNode  {
/////////////////////////////////////////
// Data Members
/////////////////////////////////////////
	private boolean hasLoaded = false;
	private String representation;
	private SoarVertex enumeratingVertex;
	private StateMap stateMap;
	private PseudoTreeNode parent;
	private Vector children = new Vector();
	private NamedEdge theEdge;
	
	
/////////////////////////////////////////
// Constructors
/////////////////////////////////////////
	public PseudoTreeNode(StateMap inStateMap) {
		representation = inStateMap.getTopState().toString();
		enumeratingVertex = inStateMap.getTopState();
		stateMap = inStateMap;
	}
	
	public PseudoTreeNode(StateMap inStateMap,NamedEdge ne) {
		representation = ne.toString();
		enumeratingVertex = (SoarVertex)ne.V1();
		stateMap = inStateMap;
		theEdge = ne;
	}

//////////////////////////////////////////
// Accessors
//////////////////////////////////////////

	public PseudoTreeNode getChildAt(int index) {
		return ((PseudoTreeNode)children.get(index));
	}
	
	public int getChildCount() {
		if (!hasLoaded) {
			int count = 0;
			Enumeration e = stateMap.getEmanatingEdges(enumeratingVertex);
			while(e.hasMoreElements()) {
				++count;
				NamedEdge edge = (NamedEdge)e.nextElement();
				PseudoTreeNode aChild = new PseudoTreeNode(stateMap,edge);
				aChild.setParent(this);
				children.add(aChild);
			}	
			hasLoaded = true;
			return count;
		}
		return children.size();	
	}

	public NamedEdge getEdge() {
		return theEdge;
	}

	public SoarVertex getEnumeratingVertex() {
		return enumeratingVertex;
	}

	public int getIndex(PseudoTreeNode ftn) {
		return children.indexOf(ftn);
	}

	public PseudoTreeNode getParent() {
		return parent;
	}
	
	public Vector getTreePath() {
		if (parent == null) {
			Vector v = new Vector();
			v.add(this);
			return v;
		}
		else {
			Vector v = parent.getTreePath();
			v.add(this);
			return v;
		}
	}

	public boolean hasLoaded() {
		return hasLoaded;
	}
	
	public boolean isLeaf() {
		return !enumeratingVertex.allowsEmanatingEdges();
	}

	public boolean isRoot() {
		return (parent == null);
	}

	public String toString() {
		return representation;
	}


//////////////////////////////////////////
// Manipulators
//////////////////////////////////////////
	public TreeModelEvent add(NamedEdge ne) {
		int[] indices = new int[1];
		PseudoTreeNode aChild = new PseudoTreeNode(stateMap,ne);
		aChild.setParent(this);
		boolean found = false;
		int foundAt = 0;
		for(int i = 0; i < children.size() && !found; ++i) {
			NamedEdge current = getChildAt(i).getEdge();		
			if (current.getName().compareTo(ne.getName()) >= 0) {
				found = true;
				foundAt = i;
			}
		}
		if (found) {
			children.add(foundAt,aChild);
			indices[0] = foundAt;
		}
		else {
			children.add(aChild);
			indices[0] = children.size() - 1;
		}
		
		return new TreeModelEvent(stateMap,getTreePath().toArray(),indices,children.toArray());
	}


	public void setParent(PseudoTreeNode ftn) {
		parent = ftn;
	}

	public TreeModelEvent remove(NamedEdge ne) {
		int[] indices = new int[1];
		boolean found = false;
		int count = 0;
		Enumeration e = children.elements();
		while(!found && e.hasMoreElements()) {
			PseudoTreeNode currentChild = (PseudoTreeNode)e.nextElement();
			if(ne.equals(currentChild.getEdge())) {
				found = true;	
				indices[0] = count;
			}
			++count;
		}
		children.removeElementAt(indices[0]);
		return new TreeModelEvent(stateMap,getTreePath().toArray(),indices,children.toArray());
	}

	public void visitChildren(edu.umich.visualsoar.util.Visitor v) {
		Enumeration e = children.elements();
		while(e.hasMoreElements()) {
			PseudoTreeNode currentChild = (PseudoTreeNode)e.nextElement();
			v.visit(currentChild);
			currentChild.visitChildren(v);
		}
	} 
}
