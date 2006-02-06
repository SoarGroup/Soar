package edu.umich.visualsoar.operatorwindow;

/**
 * This is an abstract class that is the basis for all tree nodes
 * within visual soar
 * @author Brad Jones
 * @version 0.5a 5 Aug 1999
 */
public abstract class TreeNode extends javax.swing.tree.DefaultMutableTreeNode  {
///////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////
	/**
	 * Given a Writer you must write out a representation of yourself
	 * that can be read back in later
	 * @param w the writer 
	 * @throws IOException if there is an error writing to the writer
	 */
	abstract public void write(java.io.Writer w) throws java.io.IOException;
}
