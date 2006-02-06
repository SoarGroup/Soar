package edu.umich.visualsoar.util;

/**
 * This class is based on Object-Oriented Design patterns in C++, later converted to Java
 * This is an interface to a Queue
 * @author Brad Jones
 */

public interface Queue {
	void enqueue(Object o);
	Object dequeue();
	boolean isEmpty();
}
