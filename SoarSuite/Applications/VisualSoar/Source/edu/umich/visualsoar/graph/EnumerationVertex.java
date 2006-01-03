package edu.umich.visualsoar.graph;

import java.util.*;
import edu.umich.visualsoar.dialogs.*;

/**
 * This class encapsulates the notion of an enumeration
 * in working memory
 * @author Brad Jones
 * @version 0.9a 6/5/00
 */

public class EnumerationVertex extends SoarVertex {
///////////////////////////////////////////////
// Data Members
///////////////////////////////////////////////
	private TreeSet theStrings = null;
	
///////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////
	public EnumerationVertex(int id, Vector strings) {
		super(id);
		theStrings = new TreeSet(strings);
	}
	
	public EnumerationVertex(int id, TreeSet strings) {
		super(id);
		theStrings = strings;
	}
	
	public EnumerationVertex(int id, String singleton) {
		super(id);
		theStrings = new TreeSet();
		theStrings.add(singleton);
	}
	
///////////////////////////////////////////////
// Accessors
///////////////////////////////////////////////
	public SoarVertex copy(int newId) {
		return new EnumerationVertex(newId, theStrings);
	}

	public void add(String s) {
		theStrings.add(s);
	}

    public void remove(String s)
    {
        Iterator iter = theStrings.iterator();
        while(iter.hasNext())
        {
            String t = (String)iter.next();
            if (t.equals(s))
            {
                iter.remove();
            }
        }
    }

    public boolean contains(String s)
    {
        Iterator iter = theStrings.iterator();
        while(iter.hasNext())
        {
            String t = (String)iter.next();
            if (t.equals(s))
            {
                return true;
            }
        }
        return false;
    }

	public boolean allowsEmanatingEdges() {
		return false;
	}
	
	public Iterator getEnumeration() {
		return theStrings.iterator();
	}
		
	public boolean isEditable() { 
		return true; 
	}
	
	public boolean isValid(String s) {
		Iterator i = theStrings.iterator();
		while(i.hasNext()) {
			String cs = (String)i.next();
			if(cs.compareTo(s) == 0) 
				return true;
		}
		return false;
	}
	
	public String toString() {
		String s = ": [ ";
		Iterator i = theStrings.iterator(); 
		while(i.hasNext())
			s += i.next() + " ";
		s += "]";
		return s;
	}

///////////////////////////////////////////////
// Modifiers
///////////////////////////////////////////////
	public boolean edit(java.awt.Frame owner) {
		EditEnumerationDialog theDialog = new EditEnumerationDialog(owner,theStrings);
		theDialog.setVisible(true);
		if (theDialog.wasApproved()) {
			theStrings = theDialog.getTreeSet();
			return true;
		}
		return false;
	}
	
	public void write(java.io.Writer w) throws java.io.IOException {
		w.write("ENUMERATION " + number + " " + theStrings.size());
		Iterator i = theStrings.iterator();
		while(i.hasNext()) {
			w.write(" " + i.next().toString());
		}
		w.write('\n');
	}
}
