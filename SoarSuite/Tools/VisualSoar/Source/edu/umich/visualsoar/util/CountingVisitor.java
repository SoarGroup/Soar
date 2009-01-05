package edu.umich.visualsoar.util;

/**
 * This class follows the visitor pattern
 * it is based on Object-Oriented Design patterns in C++, later converted to Java
 * it counts the number of things that it visits and has an accessor function
 * so the client can recieve the result
 * @author Brad Jones
 */

public class CountingVisitor extends Visitor {
////////////////////////////////////////////
// Data Members
////////////////////////////////////////////
	private int count = 0;

////////////////////////////////////////////
// Accessors
////////////////////////////////////////////
	public int count() {
		return count;
	}

////////////////////////////////////////////
// Manipulators
////////////////////////////////////////////
	public void visit(Object o) {
		++count;
	}
}
