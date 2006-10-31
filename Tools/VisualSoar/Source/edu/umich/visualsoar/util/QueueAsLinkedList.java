package edu.umich.visualsoar.util;
import java.util.LinkedList;

/**
 * This is basically just a wrapper class to make a Linked
 * list behave like a queue
 */

public class QueueAsLinkedList implements Queue {
	LinkedList line = new LinkedList();
	
	public void enqueue(Object o) {
		line.add(o);
	}
	
	public Object dequeue() {
		return line.removeFirst();		
	}
	
	public boolean isEmpty() {
		return line.isEmpty();
	}
}
