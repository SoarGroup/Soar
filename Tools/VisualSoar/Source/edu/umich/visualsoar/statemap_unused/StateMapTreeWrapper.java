package edu.umich.visualsoar.statemap;

import javax.swing.tree.*;
import javax.swing.event.TreeModelListener;
import javax.swing.event.TreeModelEvent;
import javax.swing.*;
import java.util.*;
import edu.umich.visualsoar.graph.NamedEdge;
import edu.umich.visualsoar.misc.*;
import edu.umich.visualsoar.util.*;

public class StateMapTreeWrapper implements TreeModel, StateMapListener {
	private List listeners = new LinkedList();
	private StateMap stateMap;
	private PseudoTreeNode root;
	
	public StateMapTreeWrapper(StateMap inStateMap) {
		stateMap = inStateMap;
		inStateMap.addStateMapListener(this);
		root = new PseudoTreeNode(stateMap);
	}
	
	public void addTreeModelListener(TreeModelListener listener) {
		listeners.add(listener);
	}
	
	public Object getChild(Object parent, int index) {
		return ((PseudoTreeNode)parent).getChildAt(index);
	}
	
	public int getChildCount(Object parent) {
		return ((PseudoTreeNode)parent).getChildCount();
	}
	
	public int getIndexOfChild(Object parent,Object child) {
		return ((PseudoTreeNode)parent).getIndex((PseudoTreeNode)child);
	}
	
	public Object getRoot() {
		return root;
	}
	
	public boolean isLeaf(Object node) {
		return ((PseudoTreeNode)node).isLeaf();
	}
	
	public void valueForPathChanged(TreePath path,Object newValue) {}
	
	public void removeTreeModelListener(TreeModelListener listener) {
		listeners.remove(listener);
	}
	
	public void edgeAdded(StateMapEvent sme) {
		NamedEdge edge = sme.getEdge();
		AddingVisitor av = new AddingVisitor(edge);
		av.visit(root);
		root.visitChildren(av);
		Enumeration e = av.changeEvents();
		while(e.hasMoreElements()) {
			TreeModelEvent tme = (TreeModelEvent)e.nextElement();
			notifyListenersOfAdd(tme);
		}
	}
	public void edgeRemoved(StateMapEvent sme) {
		NamedEdge edge = sme.getEdge();
		RemovingVisitor rv = new RemovingVisitor(edge);
		rv.visit(root);
		root.visitChildren(rv);
		Enumeration e = rv.changeEvents();
		while(e.hasMoreElements()) {
			TreeModelEvent tme = (TreeModelEvent)e.nextElement();
			notifyListenersOfRemove(tme);
		}
	}
	
	private void notifyListenersOfAdd(TreeModelEvent tme) {
		Iterator i = listeners.iterator();
		while(i.hasNext()) {
			TreeModelListener tml = (TreeModelListener)i.next();
			tml.treeNodesInserted(tme);
		}
	}
	
	private void notifyListenersOfRemove(TreeModelEvent tme) {
		Iterator i = listeners.iterator();
		while(i.hasNext()) {
			TreeModelListener tml = (TreeModelListener)i.next();
			tml.treeNodesRemoved(tme);
		}
	}
}
