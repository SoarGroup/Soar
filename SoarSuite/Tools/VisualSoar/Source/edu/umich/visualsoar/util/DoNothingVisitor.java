package edu.umich.visualsoar.util;

/**
 * This class follows the visitor pattern
 * it is based on Object-Oriented Design patterns in C++, later converted to Java
 * This class is pretty worthless, but it is used to simplify garbage collection in Soar
 * Working Memory 
 */

public class DoNothingVisitor extends Visitor {
	public void visit(Object o) {}
}
