package  edu.umich.visualsoar.graph;
import javax.swing.*;

/**
 * This class is the base class for all Soar Working memory
 * vertices
 * @author Brad Jones
 * @version 0.9a 6/5/00
 */ 

public abstract class SoarVertex extends Vertex {
///////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////
	/**
	 * Constructs a SoarVertex with the given id
	 */
	public SoarVertex(int id) {
		super(id);
	}
	
///////////////////////////////////////////////
// Accessors
///////////////////////////////////////////////
	/**
	 * This method is used to determine whether or not this
	 * node allows children
	 * @return whether or not this Vertex allows emanating edges
	 */
	public abstract boolean allowsEmanatingEdges();
	
	/**
	 * This method tells us whether or not the edit method will work
	 * @return whether or not this node is editable
	 */
	public boolean isEditable() { 
		return false; 
	}
	
	/**
	 * This method determines whether or not a given value is valid
	 * for this particular node
	 * @param value the string we are checking the validity of
	 * @return is the string a valid value
	 */
	public abstract boolean isValid(String value);
	
	/**
	 * Method returns a new copy of the same data, but with
	 * a new id
	 * @param the new ID to use
	 * @return the new vertex
	 */
	public abstract SoarVertex copy(int newId);
	
///////////////////////////////////////////////
// Modifiers
///////////////////////////////////////////////
	/**
	 * This method allows the user to edit the contents of this node
	 */
	public boolean edit(java.awt.Frame owner) { 
		JOptionPane.showMessageDialog(owner, 
			"This element has no values to edit,\n use \"Rename Attribute...\" to edit the attribute name", 
			"Invalid Edit", JOptionPane.ERROR_MESSAGE);	
		return false;
	}
	
	/**
	 * This method writes a description of this node to the
	 * stream pointed to by the writer
	 * @param w the stream where this node is described to
	 * @throws IOException if there was an error writing to the stream
	 */
	public abstract void write(java.io.Writer w) throws java.io.IOException;
	
	public boolean isUnknown() {
		return false;
	}

}
	
