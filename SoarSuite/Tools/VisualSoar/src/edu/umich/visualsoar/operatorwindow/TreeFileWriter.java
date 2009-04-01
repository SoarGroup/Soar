package edu.umich.visualsoar.operatorwindow;

/**
 * This class saves the contents of a @see DefaultTreeModel DefaultTreeModel containing
 * DataMap objects to a file to later be read in by DataMapReader
 * @author Brad Jones Jon Bauman
 * @version 0.5a 5 Aug 1999
 */
import java.io.*;
import javax.swing.tree.*;
import java.util.*;

public class TreeFileWriter {


	/**
	 * Traverses the tree and writes out the data in the VSA file format
   * Format of vsa file is:
   * nodeId  parentId  NodeType  name  filename  nextId
	 * @param treeWriter the writer for the vsa file
   * @param tree the operator hierarchy tree that is being read.
   * @see DefaultTreeModel DefaultTreeModel
	 */
	public static void write(Writer treeWriter, DefaultTreeModel tree) throws IOException {

		final String TAB = "\t";

		// This hash table is used to associate pointers with id's
		// given a pointer you can lookup the id for the that node
		// this is used for parent id lookup
		Hashtable ht = new Hashtable();
		Integer nodeID = new Integer(0);

		DefaultMutableTreeNode root = (DefaultMutableTreeNode)tree.getRoot();
		
		// Doing this enumeration guarentees that we will never reach a
		// child without first processing its parent
		Enumeration e = root.preorderEnumeration();

		TreeNode node = (TreeNode)e.nextElement();
		ht.put(node, nodeID);

		// special case for the root node
		// write out tree specific stuff
		treeWriter.write(nodeID + TAB);
		
		// tell the node to write it self
		node.write(treeWriter);
		
		// terminate the line
		treeWriter.write("\n");

		while(e.hasMoreElements()) {
			nodeID = new Integer(nodeID.intValue() + 1);
			node = (TreeNode)e.nextElement();
			ht.put(node, nodeID);

			// Again the same technique write out the tree information, then the node specific stuff, then
			// terminate the line
			treeWriter.write(nodeID + TAB + ht.get(node.getParent()) + TAB);
			node.write(treeWriter);
			treeWriter.write("\n");
		}
		treeWriter.write("END\n");

	}


	/**
	 * Traverses the tree and writes out the data in the VSA file format
   * Format of vsa file is:<br>
   * nodeId -- parentId -- NodeType -- name -- filename -- nextId
   * The difference from the standard write method is this version 5 method
   * writes each node with a unique id based on its path in the tree.
   * This enables copies of the project to be individually modified by different
   * users and merged together later.
   * This version five method is not currently implemented in Visual Soar 4.0
	 * @param treeWriter the writer for the vsa file
   * @param tree the operator hierarchy tree that is being read.
   * @see DefaultTreeModel DefaultTreeModel
	 */

	public static void write5(Writer treeWriter, DefaultTreeModel tree) throws IOException {

		final String TAB = "\t";

		// This hash table is used to associate pointers with id's
		// given a pointer you can lookup the id for the that node
		// this is used for parent id lookup
    // A unique id is created by concatenating name with all parent's name
		Hashtable ht = new Hashtable();
		//Integer nodeID = new Integer(0);
    String nodeID = "Root";

		DefaultMutableTreeNode root = (DefaultMutableTreeNode)tree.getRoot();
		
		// Doing this enumeration guarentees that we will never reach a
		// child without first processing its parent
		Enumeration e = root.preorderEnumeration();

    OperatorNode node = (OperatorNode)e.nextElement();

		ht.put(node, nodeID);

		// special case for the root node
		// write out tree specific stuff
		treeWriter.write(nodeID + TAB);
		
		// tell the node to write it self
		node.write(treeWriter);
		
		// terminate the line
		treeWriter.write("\n");

		while(e.hasMoreElements()) {
			//nodeID = new Integer(nodeID.intValue() + 1);
			//node = (TreeNode)e.nextElement();
      node = (OperatorNode)e.nextElement();
      nodeID = ht.get(node.getParent()) + "_" + node.toString();
			ht.put(node, nodeID);

			// Again the same technique write out the tree information, then the node specific stuff, then
			// terminate the line
			treeWriter.write(nodeID + TAB + ht.get(node.getParent()) + TAB);
			node.write(treeWriter);
			treeWriter.write("\n");
		}
		treeWriter.write("END\n");

	}

}  // end of TreeFileWriter
