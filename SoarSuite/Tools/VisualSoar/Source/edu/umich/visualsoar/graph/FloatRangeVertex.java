package edu.umich.visualsoar.graph;

import edu.umich.visualsoar.dialogs.*;

public class FloatRangeVertex extends SoarVertex {
//////////////////////////////////////////
// Data Members
//////////////////////////////////////////
	private double low, high;
	String rep;

//////////////////////////////////////////
// Constructors
//////////////////////////////////////////
	public FloatRangeVertex(int id, double _low, double _high) {
		super(id);
		if (low > high)
			throw new IllegalArgumentException("Low cannot be greater than high");
		low = _low;
		high = _high;
		calculateRep();
	}
	
//////////////////////////////////////////
// Accessors
//////////////////////////////////////////	
	public SoarVertex copy(int newId) {
		return new FloatRangeVertex(newId, low, high);
	}

	public boolean allowsEmanatingEdges() {
		return false;
	}
	
	public boolean isEditable() { 
		return true;
	}
	
	public boolean isValid(String s) {
		try {
			float f = Float.parseFloat(s);
			if (f >= low && f <= high)
				return true;
			return false;
		}
		catch(NumberFormatException nfe) {
			return false;
		}
	}
	
	public String toString() {
		return rep;
	}
	
//////////////////////////////////////////
// Manipulators
//////////////////////////////////////////	
	public boolean edit(java.awt.Frame owner) {
		EditNumberDialog theDialog = new EditNumberDialog(owner,"Float");
		theDialog.setLow(new Float(low));
		theDialog.setHigh(new Float(high));
		theDialog.setVisible(true);
		
		if (theDialog.wasApproved()) {
			low = theDialog.getLow().floatValue();
			high = theDialog.getHigh().floatValue();
			calculateRep();
			return true;			
		}
		return false;
	}

	public void write(java.io.Writer w) throws java.io.IOException {
		w.write("FLOAT_RANGE " + number + " " + low + " " + high + '\n');
	}
	
	private void calculateRep() {
		rep = ": float";
		if(low != Float.NEGATIVE_INFINITY || high != Float.POSITIVE_INFINITY) {
			rep += " [ ";
			if(low == Float.NEGATIVE_INFINITY) {
				rep += "... ";
			}
			else {
				rep += low + " ";
			}
			rep += "- ";
			if(high == Float.POSITIVE_INFINITY) {
				rep += "... ";
			}
			else {
				rep += high + " ";
			}
			rep += " ]";
		}
	}

	
	
}
