package edu.umich.visualsoar.util;

/**
 * This class follows the visitor pattern
 * it is based on Object-Oriented Design patterns in C++, later converted to Java
 * this is the base class for all other derivation
 * @author Brad Jones
 */

public abstract class Visitor {
	public abstract void visit(Object o);
	public boolean isDone() {
		return false;
	}
}
