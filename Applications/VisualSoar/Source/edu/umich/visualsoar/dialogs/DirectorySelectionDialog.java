package edu.umich.visualsoar.dialogs;

import java.awt.event.*;
import java.awt.*;
import javax.swing.*;
import javax.swing.event.*;
import java.io.*;
import javax.swing.tree.*;
import java.util.*;

public class DirectorySelectionDialog extends JDialog {
	private JPanel rootSelectionPanel;
	private JComboBox rootSelector;
	private DirectoryTree directoryTree;
	private JPanel buttonPanel = new JPanel();
	private JButton okButton = new JButton("Ok");
	private JButton cancelButton = new JButton("Cancel");
	private JButton refreshButton = new JButton("Refresh");
	private JButton createNewFolderButton = new JButton("Create New Folder...");
	private JLabel volumeLabel = new JLabel("Volume:");
	private boolean wasApproved = false;
	private File selectedDirectory;

	public DirectorySelectionDialog(Frame owner) {
		super(owner,"Select A Directory",true);
		Container contentPane = getContentPane();
		contentPane.setLayout(new BorderLayout());

		File[] roots = File.listRoots();
		if(roots.length != 1) {
			createRootSelector(roots);
			contentPane.add(rootSelectionPanel,BorderLayout.NORTH);
		}
		directoryTree = new DirectoryTree(roots[0]);
		contentPane.add(new JScrollPane(directoryTree),BorderLayout.CENTER);
		
		createButtonPanel();
		contentPane.add(buttonPanel,BorderLayout.SOUTH);

		pack();
	}

	public void setPath(File directory) {
		String fullPath = directory.getAbsolutePath();
		int pos = fullPath.indexOf(File.separator);
		if(pos != -1) {
			String volumeName = fullPath.substring(0,pos+1);
			File[] roots = File.listRoots();
			if(roots != null && roots.length != 1) {
				for(int i = 0; i < roots.length; ++i) {
					if(roots[i].getPath().compareTo(volumeName) == 0) {
						rootSelector.setSelectedItem(roots[i]);
						directoryTree.changeRoot(roots[i]);
						directoryTree.setPath(directory);			
					}
				}
			}
		}
	}
	
	public boolean wasApproved() {
		return wasApproved;
	}

	public File getSelectedDirectory() {
		return selectedDirectory;
	}

	private void createRootSelector(File[] roots) {
		rootSelector = new JComboBox();

		for(int i = 0; i < roots.length; ++i)
			rootSelector.addItem(roots[i]);

		rootSelector.addItemListener(new ItemListener() {
			public void itemStateChanged(ItemEvent event) {
				if(event.getStateChange() == ItemEvent.SELECTED) {
					File newRoot = (File)event.getItem();
					directoryTree.changeRoot(newRoot);	
				}
			}
		});
		rootSelectionPanel = new JPanel();
		rootSelectionPanel.setLayout(new BorderLayout());
		rootSelectionPanel.add(volumeLabel,BorderLayout.WEST);
		rootSelectionPanel.add(rootSelector,BorderLayout.CENTER);	
	}

	private void createButtonPanel() {
		buttonPanel.setLayout(new FlowLayout());
		buttonPanel.add(okButton);
		buttonPanel.add(cancelButton);
		buttonPanel.add(refreshButton);
		buttonPanel.add(createNewFolderButton);
		
		okButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent ae) {
				TreePath selectionPath = directoryTree.getSelectionPath();
				if(selectionPath == null) {
					JOptionPane.showMessageDialog(DirectorySelectionDialog.this,
												  "A directory was not selected.",
												  "Selection Not Found",
												   JOptionPane.ERROR_MESSAGE);
					return;

				}
				wasApproved = true;
				DirectoryNode selectedNode = (DirectoryNode)selectionPath.getLastPathComponent();
				selectedDirectory = selectedNode.getDirectory();
				dispose();
			}
		});

		cancelButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent ae) {
				dispose();
			}
		});
		
		refreshButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent ae) {
				TreePath selectionPath = directoryTree.getSelectionPath();
				if(selectionPath == null) {
					JOptionPane.showMessageDialog(DirectorySelectionDialog.this,
												  "A directory was not selected to refresh.",
												  "Selection Not Found",
												  JOptionPane.ERROR_MESSAGE);
					return;
				}
				DirectoryNode selectedNode = (DirectoryNode)selectionPath.getLastPathComponent();
				selectedNode.explore();
				DefaultTreeModel model = (DefaultTreeModel)directoryTree.getModel();
				model.nodeStructureChanged(selectedNode);
			}
		});
		
		createNewFolderButton.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent ae) {
				TreePath selectionPath = directoryTree.getSelectionPath();
				if(selectionPath == null) {
					JOptionPane.showMessageDialog(DirectorySelectionDialog.this,
												  "A directory was not selected for the parent of the new directory.",
												  "Selection Not Found",
												   JOptionPane.ERROR_MESSAGE);
					return;
				}
				String newFolderName = JOptionPane.showInputDialog(DirectorySelectionDialog.this,
																   "Enter Name For New Folder:");
				if(newFolderName == null)
					return;
				if(newFolderName == "") {
					getToolkit().beep();
					return;
				}
				DirectoryNode parentNode = (DirectoryNode)selectionPath.getLastPathComponent();
				File parentFolder = parentNode.getDirectory();
				File newFolder = new File(parentFolder,newFolderName);
				boolean result = newFolder.mkdir();
				if(result == true) {
					parentNode.explore();
					DefaultTreeModel model = (DefaultTreeModel)directoryTree.getModel();
					model.nodeStructureChanged(parentNode);
				}
				else {
					JOptionPane.showMessageDialog(DirectorySelectionDialog.this,
												  "Error creating folder \"" + newFolder.getName() + "\", either a folder of that name doesn't exist, or it may be an invalid name.",
												  "Folder Creation Failed",
												   JOptionPane.ERROR_MESSAGE);
				}
			}
		});
	}
}

class DirectoryTree extends JTree {
	public DirectoryTree(File root) {
		super(new DirectoryNode(root));
		addTreeExpansionListener(new TreeExpansionListener() {
			public void treeCollapsed(TreeExpansionEvent e) {}
			public void treeExpanded(TreeExpansionEvent e) {
				TreePath path = e.getPath();
				DirectoryNode directoryNode = (DirectoryNode)path.getLastPathComponent();
				if(!directoryNode.isExplored()) {
					DefaultTreeModel model = (DefaultTreeModel)getModel();
					directoryNode.explore();
					model.nodeStructureChanged(directoryNode);
				}
			}
		});

		DefaultTreeSelectionModel selectionModel = new DefaultTreeSelectionModel();
		selectionModel.setSelectionMode(DefaultTreeSelectionModel.SINGLE_TREE_SELECTION);
		setSelectionModel(selectionModel);
	}

	public void changeRoot(File newRoot) {
		setModel(new DefaultTreeModel(new DirectoryNode(newRoot)));
	}
	
	public void setPath(File directory) {
		DirectoryNode parent = (DirectoryNode)getModel().getRoot();
		TreePath expansionPath = new TreePath(parent);
		int index = directory.getAbsolutePath().indexOf(File.separator);
		if(index == -1)
			return;
		String name = getNextName(directory,expansionPath);
		while(name != null) {
			DirectoryNode child = null;
			for(int i = 0;child == null && i < parent.getChildCount(); ++i) {
				if(parent.getChildAt(i).toString().compareTo(name) == 0) {
					child = (DirectoryNode)parent.getChildAt(i);
				}
			}
			if(child != null) {
				expansionPath = expansionPath.pathByAddingChild(child);          
				name = getNextName(directory,expansionPath);
				parent = child;
			}
			else {
				name = null;
			}
		}
		expandPath(expansionPath);
		setSelectionPath(expansionPath);
		
	}
	
	private final String getNextName(File directory,TreePath treePath) {
		DirectoryNode node = (DirectoryNode)treePath.getLastPathComponent();
		int length = node.getDirectory().getPath().length();
		String directoryPath = directory.getAbsolutePath();
		if(directoryPath.length() <= length + 1)
			return null;
		int pos = directoryPath.indexOf(File.separator,length+1);
		if(pos == -1)
			return directory.getName();
		else {
			return directoryPath.substring(length,pos);
		}
	}
}

class DirectoryNode extends DefaultMutableTreeNode implements Comparable {
	private static DirectoryFilter directoryFilter = new DirectoryFilter();
	private boolean explored = false;
	private String name;

	/**
	 assumptions: the File Passed in must be a directory
	*/
	public DirectoryNode(File inFile) {
		setUserObject(inFile);
		if(inFile.getParentFile() == null)
			name = inFile.getPath();
		else
			name = inFile.getName();
	}

	public boolean getAllowsChildren() { return true; }
	public boolean isLeaf() { return false; }
	public File getDirectory() { return (File)getUserObject(); }
	
	public boolean isExplored() {
		return explored;
	}
	
	public int getChildCount() {
		if(!explored)
			explore();
		return super.getChildCount();
	}

	public void explore() {
		if(explored) {
			removeAllChildren();
		}
		explored = true;
		File file = getDirectory();
		File[] children = file.listFiles(directoryFilter);
		Set directoryNodes = new TreeSet();
		if(children != null) {
			for(int i = 0; i < children.length; ++i)
				directoryNodes.add(new DirectoryNode(children[i]));
		}
		Iterator i = directoryNodes.iterator();
		while(i.hasNext()) {
			add((DirectoryNode)i.next());
		}
	}
	
	public String toString() {
		return name;
	}
	
	public int compareTo(Object o) {
		String s = o.toString();
		return name.compareTo(s);
	}	
}

class DirectoryFilter implements FileFilter {
	public boolean accept(File inPathName) {
		return inPathName.isDirectory();
	}
}
