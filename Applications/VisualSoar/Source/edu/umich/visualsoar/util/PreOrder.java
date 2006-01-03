package edu.umich.visualsoar.util;

/**
 * This class follows the visitor pattern
 * it is based on Object-Oriented Design patterns in C++, later converted to Java
 * Visit the elements in PreOrder
 * @author Brad Jones
 */


public class PreOrder extends PrePostVisitor {
	Visitor v = null;
	public PreOrder(Visitor _v) {
		v = _v;
	}
	
	public void preVisit(Object o) {
		v.visit(o);
	}
}
