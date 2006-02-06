package edu.umich.visualsoar.util;
import edu.umich.visualsoar.graph.*;
import edu.umich.visualsoar.datamap.FakeTreeNode;
import java.util.*;

/**
 * This class follows the visitor pattern
 * it is based on Object-Oriented Design patterns in C++, later converted to Java
 * Something has been removed to working memory so remove the edge to the datamap models
 * and produce the proper change event that can be iterated through later
 * @author Brad Jones
 */

public class RemovingVisitor extends Visitor {
/////////////////////////
// Data Members
/////////////////////////
	private NamedEdge edge;
	private LinkedList changeEvents = new LinkedList();
	
////////////////////////
// Constructors
////////////////////////
	// Deny default construction
	private RemovingVisitor() {}
	public RemovingVisitor(NamedEdge ne) {
		edge = ne;
	}
	
////////////////////////////////////////
// Accessors
////////////////////////////////////////
	public Enumeration changeEvents() {
		return new EnumerationIteratorWrapper(changeEvents.iterator());
	}
	
////////////////////////
// Manipulators
////////////////////////
	public void visit(Object o) {
		if(o instanceof FakeTreeNode) {
			FakeTreeNode ftn = (FakeTreeNode)o;
			if (ftn.hasLoaded() == false)
				return;
			SoarVertex ev = ftn.getEnumeratingVertex();
			if (edge.V0().getValue() == ftn.getEnumeratingVertex().getValue()) 
				changeEvents.add(ftn.remove(edge));
		}
	}
}
