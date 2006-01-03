package edu.umich.visualsoar.graph;

import java.awt.Frame;
import edu.umich.visualsoar.dialogs.EditNumberDialog;

public class IntegerRangeVertex extends SoarVertex {
	int low, high;
	String rep;
	public IntegerRangeVertex(int id,int _low, int _high) {
		super(id);
		if (high < low)
			throw new IllegalArgumentException("the low cannot be greater than the high");
		low = _low;
		high = _high;
		calculateRep();
	}

	public SoarVertex copy(int newId) {
		return new IntegerRangeVertex(newId, low, high);
	}
	
	public boolean isValid(String s) {
		try {
			int i = Integer.parseInt(s);
			if (i >= low && i <= high)
				return true;
			return false;
		}
		catch(NumberFormatException nfe) {
			return false;
		}
	}
			
	
	public boolean allowsEmanatingEdges() {
		return false;
	}
	
	public String toString() {
		return rep;
	}
	
	public void write(java.io.Writer w) throws java.io.IOException {
		w.write("INTEGER_RANGE " + number + ' ' + low + ' ' + high + '\n');
	}
	
	public boolean isEditable() { return true; }	
	
	public boolean edit(Frame owner) {
		EditNumberDialog theDialog = new EditNumberDialog(owner,"Integer");
		theDialog.setLow(new Integer(low));
		theDialog.setHigh(new Integer(high));
		theDialog.setVisible(true);
		
		if (theDialog.wasApproved()) {
			low = theDialog.getLow().intValue();
			high = theDialog.getHigh().intValue();
			calculateRep();
			return true;
		}
		return false;
	}
	
	private void calculateRep() {
		rep = ": integer";
		if(low != Integer.MIN_VALUE || high != Integer.MAX_VALUE) {
			rep += " [ ";
			if(low == Integer.MIN_VALUE) {
				rep += "... ";
			}
			else {
				rep += low + " ";
			}
			rep += "- ";
			if(high == Integer.MAX_VALUE) {
				rep += "... ";
			}
			else {
				rep += high + " ";
			}
			rep += " ]";
		}
	}
}
