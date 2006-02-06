package edu.umich.visualsoar.statemap;

import javax.swing.*;
import javax.swing.tree.*;
import java.awt.*;
import java.awt.event.*;
import java.awt.dnd.*;
import java.util.Vector;
import java.awt.datatransfer.*;
import edu.umich.visualsoar.MainFrame;
import edu.umich.visualsoar.dialogs.*;
import edu.umich.visualsoar.graph.*;
import edu.umich.visualsoar.datamap.TransferableVertex;

public class StateMapTree extends JTree {
	/* Data Members */

	private JPopupMenu menu = new JPopupMenu();
	private JMenuItem addSubStateItem = new JMenuItem("Add SubState...");
	private JMenuItem addFileItem = new JMenuItem("Add File...");
	private JMenuItem propertiesItem = new JMenuItem("Properties...");
	private StateMap stateMap;

	private DragGestureListener dragGestureListener = new SMTDragGestureListener();
	private DragSourceListener dragSourceListener = new SMTDragSourceListener();
	private DropTargetListener dropTargetListener = new SMTDropTargetListener();
	private DropTarget dropTarget = new DropTarget(this,DnDConstants.ACTION_LINK,dropTargetListener,true);
	

	/* Constructors */
	public StateMapTree(StateMap inStateMap) {
		super(new StateMapTreeWrapper(inStateMap));	
		stateMap = inStateMap;
		createContextMenu();
		addMouseListener(new MouseAdapter() {
			public void mousePressed(MouseEvent me) {
				if ((me.getModifiers() & me.BUTTON3_MASK) == me.BUTTON3_MASK) {
						TreePath treePath = getPathForLocation(me.getX(),me.getY());
						if(treePath != null) {
							addSelectionPath(treePath);
							menu.show(StateMapTree.this,me.getX(),me.getY());
						}
				}
			}
		});
		
		getSelectionModel().setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);
		DragSource.getDefaultDragSource().createDefaultDragGestureRecognizer(this,DnDConstants.ACTION_LINK,dragGestureListener);
	}
		
	/* Methods */
	public void addSubState() {
		NameDialog nameDialog = new NameDialog(edu.umich.visualsoar.MainFrame.getMainFrame());
		nameDialog.makeVisible("");
		if(nameDialog.wasApproved()) {
			String name = nameDialog.getText();
			if(name.equals("")) {
				//Invalid Name
			}
			else {
				TreePath tp = getSelectionPath();
				if(tp != null) {
					PseudoTreeNode ptn = (PseudoTreeNode)tp.getLastPathComponent();
					StateVertex sv0 = (StateVertex)ptn.getEnumeratingVertex();
					StateVertex sv1 = stateMap.createNewStateVertex(name);
					stateMap.addEdge(sv0,sv1);
				}
			}
		}
	}
	
	public void addFile() {
		AddFilesDialog theDialog = new AddFilesDialog(MainFrame.getMainFrame()); 
		theDialog.setVisible(true);
	}
	
	public void showProperties() {
		TreePath tp = getSelectionPath();
		if(tp != null) {
			PseudoTreeNode ptn = (PseudoTreeNode)tp.getLastPathComponent();
			PropertiesDialog propertiesDialog = new PropertiesDialog(MainFrame.getMainFrame(),(StateVertex)ptn.getEnumeratingVertex());
			propertiesDialog.setVisible(true);
		}
	}
	
	
	private void createContextMenu() {
		menu.add(addSubStateItem);
		addSubStateItem.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent ae) {
				addSubState();
			}
		});
		menu.add(addFileItem);
		addFileItem.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent ae) {
				addFile();
			}
		});
		menu.add(propertiesItem);
		propertiesItem.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent ae) {
				showProperties();
			}
		});
	}
	
	// Inner Classes
	class SMTDragGestureListener implements DragGestureListener {
		public void dragGestureRecognized(DragGestureEvent dge) {
			int action = dge.getDragAction();
			TreePath path = getSelectionPath();
			if(path == null)
				return;
				
			PseudoTreeNode ptn = (PseudoTreeNode)path.getLastPathComponent();
			NamedEdge e = ptn.getEdge();
			Transferable t;
			if(e == null)
				t = new TransferableVertex(ptn.getEnumeratingVertex(),ptn.toString());
			else
				t = new TransferableVertex(ptn.getEnumeratingVertex(),e.getName(),e);
				
			if(action == DnDConstants.ACTION_LINK) {
				DragSource.getDefaultDragSource().startDrag(dge,DragSource.DefaultLinkNoDrop,t,dragSourceListener);
			}
		}
	}
	
	class SMTDropTargetListener implements DropTargetListener {
		public void dragEnter(DropTargetDragEvent dtde) {}
		public void dragExit(DropTargetEvent dte) {}
		public void dragOver(DropTargetDragEvent dtde) {}
		public void dropActionChanged(DropTargetDragEvent dtde) {}
		
		public void drop(DropTargetDropEvent e) {
			Point loc = e.getLocation();
			int x = (int)loc.getX(), y = (int)loc.getY();
			
			int action = e.getDropAction();
			
			DataFlavor[] flavors = e.getCurrentDataFlavors();
			DataFlavor chosen = flavors[0];
						
			if (isDropOK(x, y, action)) {
				e.acceptDrop(action);
			}
			else {
				e.rejectDrop();
				return;
			}
			
			Vector data = null;
			
			try {
				data = (Vector)e.getTransferable().getTransferData(chosen);
			} 
			catch(Throwable t) {
				t.printStackTrace();
				e.dropComplete(false);
				return;
			}
			TreePath path = getPathForLocation(x, y);
			SoarVertex vertex = ((PseudoTreeNode)path.getLastPathComponent()).getEnumeratingVertex();
			stateMap.addEdge(vertex,stateMap.getVertexForId(((Integer)data.get(0)).intValue()));
			e.dropComplete(true);
		}
		
		boolean isDropOK(int x, int y, int action) {
			TreePath path = getPathForLocation(x, y);
			if (path == null) return false;
			if (action == DnDConstants.ACTION_LINK) {
				PseudoTreeNode ptn = (PseudoTreeNode)path.getLastPathComponent();
				if (ptn.isLeaf())
					return false;
				return true;
			}
			return false;
			
		}
	}
	
	class SMTDragSourceListener implements DragSourceListener {
		public void dragEnter(DragSourceDragEvent dsde) {}
		public void dragOver(DragSourceDragEvent dsde) {}
		public void dragExit(DragSourceEvent dse) {}
		public void dragDropEnd(DragSourceDropEvent e) {}
		public void dropActionChanged(DragSourceDragEvent dsde) {}
	}
	
	
}
