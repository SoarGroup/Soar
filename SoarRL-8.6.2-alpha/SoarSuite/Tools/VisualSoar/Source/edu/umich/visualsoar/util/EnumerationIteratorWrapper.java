package edu.umich.visualsoar.util;

/**
 * This is a wrapper class to make a iterator behave like an
 * Enumeration
 * @author Brad Jones
 */
public class EnumerationIteratorWrapper implements java.util.Enumeration {
	java.util.Iterator i = null;
	
	public EnumerationIteratorWrapper(java.util.Iterator _i) {
		i = _i;
	}
	
	public boolean hasMoreElements() {
		return i.hasNext();
	}
	
	public Object nextElement() {
		return i.next();
	}
}
