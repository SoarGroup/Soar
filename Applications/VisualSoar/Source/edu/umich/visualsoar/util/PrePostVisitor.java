package edu.umich.visualsoar.util;

/**
 * This class follows the visitor pattern
 * it is based on Object-Oriented Design patterns in C++, later converted to Java
 * We might want to visit a traversal in Pre, Post, or In Order a PrePostVistor
 * allows this operation by letting a derived class support that operation
 * @author Brad Jones
 */


public abstract class PrePostVisitor extends Visitor {
	
	// Over-ride this operation if you want a preorder traversal
	public void preVisit(Object o) {}
	
	// Over-ride this operation if you want a Inorder traversal
	public void visit(Object o) {}
	
	
	// Over-ride this operation if you want a PostOrder traversal
	public void postVisit(Object o) {}
}
