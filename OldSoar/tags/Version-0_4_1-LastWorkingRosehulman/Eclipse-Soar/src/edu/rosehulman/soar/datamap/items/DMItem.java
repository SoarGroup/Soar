/*
 * Created on Dec 25, 2003
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap.items;



import edu.umich.visualsoar.parser.*;

import java.util.*;

import org.eclipse.ui.part.*;


/**
 *
 * Abstract class to represent any item in the datamap. 
 * I probably should have named it DMAttribute, but it is far too late to 
 * change that now.
 * 
 * @author Tim Jasko
 */
public abstract class DMItem {
	protected String _name;
	protected String _comment = "";
	protected DMItem _parent;
	protected int _id;
	
	/**
	 * The default constructor
	 *
	 */
	public DMItem() {
	}
	
	/**
	 * Creates a new DMItem with the given name.
	 * @param name The name of the item.
	 */
	public DMItem(String name) {
		_name = name;
	}
	
	/**
	 * Sets the item's ID number, which should be unique to the project.
	 * @param id
	 */
	public void setID(int id) {
		_id = id;
	}
	
	/**
	 * Get the item's ID number, which should be unique to the project.
	 * @return
	 */
	public int getID() {
		return _id;
	}
	
	
	/**
	 * Returns an ArrayList of the children of this item.
	 * Only used by {@link DMIdentifier} at this time.
	 * 
	 * @return An ArrayList of the item's children.
	 */
	public ArrayList getChildren() {
		return new ArrayList(0);
	}
	
	
	/**
	 * satisfies tests whether or not this edge could be used to
	 * satisfy the constraint passed in, if so, it returns true
	 * else false
	 * @param triple the constraint to test satisfaction to
	 * @return whether or not this item can satisfy that constraint
	 */
	public boolean satisfies(Triple triple) {
		if (!TripleUtils.isVariable(triple.getAttribute().getString())) {
			if (!triple.getAttribute().getString().equals(getName())) {
				return false;
			}
		}
		if (TripleUtils.isVariable(triple.getValue().getString())) {
			return true;
		}
		
		return isValidValue(triple.getValue().getString());
	}
	
	
	/**
	 * This method determines whether or not a given value is valid
	 * for this particular node
	 * @param value the string we are checking the validity of
	 * @return is the string a valid value
	 */
	public abstract boolean isValidValue(String val);
	
	/**
	 * Sets the item's children to the given ArrayList if the item supports children.
	 * Does nothing, except in {@link DMIdentifier}.
	 * 
	 * @param children The new ArrayList of children.
	 */
	public void setChildren(ArrayList children) {
	}
	
	/**
	 * Add a new item to this item's children.
	 * 
	 * @param child The child to add to this item.
	 */
	public void addChild(DMItem child) {
		if (! getChildren().contains(child)) {
			getChildren().add(child);
			child.setParent(this);
		}
	}
	
	/**
	 * Remove a child from this item.
	 * 
	 * @param child The child to be removed
	 */
	public void removeChild(DMItem child) {
		getChildren().remove(child);
		child._parent = null;
	}
	
	
	/**
	 * Tells whether the item has any children or not.
	 * 
	 * @return true if the item has children, false otherwise
	 */
	public boolean hasChildren() {
		return false;
	}
	
	/**
	 * Indicates whether the item can have children.
	 *  When this was all first designed, only DMIdentifier could have
	 *  children. However, the recently added DMPointer may or may not
	 *  support children. Good thing I left this open for expansion, eh?
	 *  I'm such a genius.
	 * 
	 * @return true if the item supports children, false if it doesn't
	 */
	public boolean acceptsChildren() {
		return false;
	}
	
	
	//It is unlikely anybody will be overriding these
	
	/**
	 * Returns the item's parent item in the datamap
	 * 
	 */
	public DMItem getParent() {
		return _parent;
	}
	
	/**
	 * Sets the item's parent item in the datamap
	 * 
	 * @param parent The item's new parent
	 */
	
	public void setParent(DMItem parent) {
		// things in here should be rearranged someday.
		// someday... yes....
		if (parent != _parent) {
			if (_parent != null) {
				_parent.removeChild(this);
			} // if
			
			_parent = parent;
			
			if (parent != null) {
				if (!parent.getChildren().contains(this)) {
					parent.getChildren().add(this);
				} // if
			} // if
		} // if
	}
	
	/**
	 * Gets the item's name.
	 * 
	 * @return The name of the item.
	 */
	public String getName() {
		return _name;
	}
	
	/**
	 * Sets the item's name.
	 * 
	 * @param newName The new name of the item.
	 */
	public void setName( String newName) {
		_name = newName;
	}
	
	/**
	 * Gets the comment associated with this item.
	 * 
	 * @return The comment associated with this item.
	 */
	public String getComment() {
		return _comment;
	}
	
	/**
	 * Sets the comment associated with this item.
	 * 
	 * @param newComment The comment to be associated with this item.
	 */
	public void setComment( String newComment) {
		_comment = newComment;
	}
	
	/**
	 * Creates a new instance of this class of item.
	 * 
	 * @return A new instance of DMItem
	 */
	public abstract DMItem createNew();
	
	/**
	 * Returns a string indicating the type of this Item.
	 * 
	 * @return The type of this item
	 */
	public abstract String getTypeName();
	
	/**
	 * Displays a dialog enabling the user to initialize this item's values.
	 * 
	 * @param parent The parent view of this new instance
	 * @return The result, either Dialog.OK or Dialog.CANCEL 
	 */
	public abstract int initValuesDialog(WorkbenchPart parent);
	
	/**
	 * Indicates whether this item's calues can be edited or not.
	 * 
	 * @return true if the values are editable, false if not.
	 */
	public abstract boolean canEditValues();
	
	/**
	 * Displays a dialog enabling the user to edit this item's values.
	 * 
	 * @param parent The parent view of this new instance
	 * @return The result, either Dialog.OK or Dialog.CANCEL 
	 */
	public abstract int editValuesDialog(WorkbenchPart parent);
	
	
	/**
	 * Returns the XML for this item and all its children. 
	 * Generally, you will only need to call this method with a parameter of 
	 * 0 on the root node.
	 * 
	 * @param depth How deep the item is in the tree. Used to
	 *  keep the file properly indented;
	 * @return A string containing the XML to represent this item and its children.
	 */
	public abstract String getXML(int depth);
	
	
	protected String getSharedXML() {
		String ret = " name=\"" + getName() + "\""
			+ " comment=\"" + getComment() + "\""
			+ " id=\"" + getID() + "\"";
			
		return ret;
	}
	
	/**
	 * Creates a copy/clone of this node. If the node has children, copies of
	 *  them are created as well. The reference to the node's parent is 
	 *  <i>not</i> retained.
	 * @return A copy of this node.
	 */
	public abstract DMItem copy();
}
