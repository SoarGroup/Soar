package edu.umich.visualsoar.filemanager;
import javax.swing.*;
import javax.swing.tree.*;
import java.awt.event.*;
import java.io.*;
import java.awt.*;
import java.awt.dnd.*;
import java.awt.datatransfer.*;

public class FileManager extends JTree {
	private JPopupMenu folderContextMenu = new JPopupMenu();
	private JPopupMenu fileContextMenu = new JPopupMenu();

	private DragGestureListener dragGestureListener = new FMDragGestureListener();
	private DragSourceListener dragSourceListener = new FMDragSourceListener();
	private DropTargetListener dropTargetListener = new FMDropTargetListener();
	private DropTarget dropTarget = new DropTarget(this,DnDConstants.ACTION_MOVE,dropTargetListener,true);
	

	public FileManager() {}
	
	public FileManager(String projectName) {
		createContextMenus();
		getSelectionModel().setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);
		registerMouseListener();
		FolderNode root = new FolderNode(projectName);
		root.add(new FolderNode("all"));
		root.add(new FolderNode("common"));
		root.add(new FolderNode("elaborations"));
		try {
			File fileLoadFile = new File("firstload.soar");
			fileLoadFile.createNewFile();
			root.add(new FileNode(fileLoadFile));
		}
		catch(IOException ioe) {
			ioe.printStackTrace();
		}
		setModel(new DefaultTreeModel(root));
		DragSource.getDefaultDragSource().createDefaultDragGestureRecognizer(this,DnDConstants.ACTION_MOVE,dragGestureListener);
	}
	
	public void showFileContextMenu(int x,int y) {
		fileContextMenu.show(this,x,y);
	}
	
	public void showFolderContextMenu(int x,int y) {
		folderContextMenu.show(this,x,y);	
	}
	
	private void createContextMenus() {
		createFolderContextMenu();
		createFileContextMenu();
	}
	
	
	private void createFolderContextMenu() {
		JMenuItem createFileItem = new JMenuItem("Create a File...");
		JMenuItem addFileItem = new JMenuItem("Add a File...");
		JMenuItem addFolderItem = new JMenuItem("Add a Folder...");
		JMenuItem deleteItem = new JMenuItem("Remove");
		JMenuItem renameItem = new JMenuItem("Rename...");
		folderContextMenu.add(createFileItem);
		folderContextMenu.add(addFileItem);
		folderContextMenu.add(addFolderItem);
		folderContextMenu.add(deleteItem);
		folderContextMenu.add(renameItem);
		
		createFileItem.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent ae) {
				
			}
		});
		
		addFileItem.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent ae) {
			
			}
		});
		
		addFolderItem.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent ae) {
			
			}
		});
		
		deleteItem.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent ae) {
				TreePath tp = getSelectionPath();
				if(tp == null)
					return;
				Object o = tp.getLastPathComponent();
				JOptionPane.showConfirmDialog(FileManager.this,"Do you really want to remove " + o.toString() + " ?","Confirm Remove",JOptionPane.YES_NO_OPTION);
			}
		});
		
		renameItem.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent ae) {
			
			}
		});
	}
	
	private void createFileContextMenu() {
		JMenuItem openRulesItem = new JMenuItem("Open Rules");
		JMenuItem deleteItem = new JMenuItem("Delete");
		JMenuItem renameItem = new JMenuItem("Rename...");
		fileContextMenu.add(openRulesItem);
		fileContextMenu.add(deleteItem);
		fileContextMenu.add(renameItem);
		
		openRulesItem.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent ae) {
				FileNode fileNode = (FileNode)getSelectionPath().getLastPathComponent();
				fileNode.openRules();
			}
		});
		
		deleteItem.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent ae) {
			
			}
		});
		
		renameItem.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent ae) {
			
			}
		});
		
	}
	
	public void registerMouseListener() {
		addMouseListener(new MouseAdapter() {
			public void mousePressed(MouseEvent me) {
				if ((me.getModifiers() & me.BUTTON3_MASK) == me.BUTTON3_MASK) {
						TreePath treePath = getPathForLocation(me.getX(),me.getY());
						if(treePath != null) {
							addSelectionPath(treePath);
							FileManagerNode fmn = (FileManagerNode)treePath.getLastPathComponent();
							fmn.showContext(FileManager.this,me.getX(),me.getY());
						}
				}
			}
		});
	}
	
	class FMDragGestureListener implements DragGestureListener {
		public void dragGestureRecognized(DragGestureEvent dge) {
			int action = dge.getDragAction();
			TreePath path = getSelectionPath();
			if(path == null)
				return;
			FileManagerNode fmn = (FileManagerNode)path.getLastPathComponent();
			if(fmn.isDragOk(action)) {
				Transferable t = fmn.getTransferable();
				if(action == DnDConstants.ACTION_MOVE) {
					DragSource.getDefaultDragSource().startDrag(dge,DragSource.DefaultMoveNoDrop,t,dragSourceListener);
				}
			}
			
			
		}
	}
	
	class FMDropTargetListener implements DropTargetListener {
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
			
			FileNode fileNode = null;
			
			try {
				fileNode = (FileNode)e.getTransferable().getTransferData(chosen);
			} 
			catch(Throwable t) {
				t.printStackTrace();
				e.dropComplete(false);
				return;
			}
			TreePath path = getPathForLocation(x, y);
			FolderNode folderNode = (FolderNode)path.getLastPathComponent();
			folderNode.add(fileNode);
			e.dropComplete(true);
		}
		
		boolean isDropOK(int x, int y, int action) {
			TreePath path = getPathForLocation(x, y);
			if (path == null) 
				return false;
			
			if (action == DnDConstants.ACTION_MOVE) {
				Object o = path.getLastPathComponent();
				if(o instanceof FolderNode) {
					return true;	
				}
			}
			return false;
		}
	}
	
	class FMDragSourceListener implements DragSourceListener {
		public void dragEnter(DragSourceDragEvent dsde) {}
		public void dragOver(DragSourceDragEvent dsde) {}
		public void dragExit(DragSourceEvent dse) {}
		public void dragDropEnd(DragSourceDropEvent e) {}
		public void dropActionChanged(DragSourceDragEvent dsde) {}
	}
	
	
}
