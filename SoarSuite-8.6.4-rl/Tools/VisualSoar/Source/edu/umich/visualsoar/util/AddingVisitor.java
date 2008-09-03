package edu.umich.visualsoar.util;
import edu.umich.visualsoar.graph.*;
import edu.umich.visualsoar.datamap.FakeTreeNode;
import java.util.*;
import javax.swing.*;

/**
 * This class follows the visitor pattern
 * it is based on Object-Oriented Design patterns in C++, later converted to Java
 * Something has been added to working memory so add the edge to the datamap models
 * and produce the proper change event that can be iterated through later
 * @author Brad Jones
 */

public class AddingVisitor extends Visitor {
///////////////////////////////////////////
// Data Members
///////////////////////////////////////////
	private NamedEdge edge;
	private LinkedList changeEvents = new LinkedList();
	
///////////////////////////////////////////
// Constructors
///////////////////////////////////////////
	// Deny Default Construction
	private AddingVisitor() {}
	public AddingVisitor(NamedEdge ne) {
		edge = ne;
	}
	
///////////////////////////////////////////
// Manipulators
///////////////////////////////////////////
	public void visit(Object o) {
		if(o instanceof FakeTreeNode) {
			FakeTreeNode ftn = (FakeTreeNode)o;
			if (ftn.hasLoaded() == false) {
				return;
      }
			if (edge.V0().getValue() == ftn.getEnumeratingVertex().getValue()) {
				changeEvents.add(ftn.add(edge));
			}
		}
	}
	
	public Enumeration changeEvents() {
		return new EnumerationIteratorWrapper(changeEvents.iterator());
	}
}
