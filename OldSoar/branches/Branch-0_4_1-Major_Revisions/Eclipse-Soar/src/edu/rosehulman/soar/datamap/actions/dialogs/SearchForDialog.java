/*
 * Created on Feb 6, 2004
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap.actions.dialogs;

import edu.rosehulman.soar.datamap.*;
import edu.rosehulman.soar.datamap.items.*;

import org.eclipse.jface.dialogs.Dialog;
import org.eclipse.jface.dialogs.MessageDialog;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.jface.viewers.*;

import java.util.ArrayList;


/**
 *
 * A dialog box that will search the datamap according to the 
 *  criteria input by the user.
 * 
 * @author Tim Jasko
 */
public class SearchForDialog extends Dialog {
	private DataMapEditor _editor;
	private DMItem _target;
	private ArrayList _skipList;
	
	//Controls we'll need to keep an eye on
	private Text txtSearch;
	private Button chkCase, chkIdentifiers, chkEnums, chkStrings,
		chkIntegers, chkFloats, chkKeep;
	
	/**
	 * Constructor.
	 * 
	 * @param editor The editor this was called from.
	 * @param target This item and its children will be searched.
	 */
	public SearchForDialog(DataMapEditor editor, DMItem target) {
		super(editor.getSite().getShell());
		//super(new Shell(SWT.MODELESS));
		
		_editor = editor;
		_target = target;
		_skipList = new ArrayList();
		
		setBlockOnOpen(false);
	}
	
	protected Control createDialogArea(Composite parent) {
		Composite comp = (Composite)super.createDialogArea(parent);
			
		comp.setLayout(new GridLayout());
			
		GridData gridData;
	
		// The "Find" group
		Group grpFind = new Group(comp, SWT.SHADOW_ETCHED_IN);
		grpFind.setText("Find");
		grpFind.setLayout(new GridLayout());
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		grpFind.setLayoutData(gridData);
		
		txtSearch = new Text(grpFind, SWT.LEFT | SWT.BORDER);
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		txtSearch.setLayoutData(gridData);
		
		chkCase = new Button(grpFind, SWT.CHECK);
		chkCase.setText("&Match Case");
		chkCase.setSelection(false);
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		chkCase.setLayoutData(gridData);
		
		// The "Search Includes" group
		Group grpSI = new Group(comp, SWT.SHADOW_ETCHED_IN);
		grpSI.setText("Search Includes");
		GridLayout siLayout = new GridLayout();
		siLayout.numColumns = 5;
		grpSI.setLayout(siLayout);
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		grpSI.setLayoutData(gridData);
		
		chkIdentifiers = new Button(grpSI, SWT.CHECK);
		chkIdentifiers.setText("Identifiers");
		chkIdentifiers.setSelection(true);
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		chkIdentifiers.setLayoutData(gridData);

		chkEnums = new Button(grpSI, SWT.CHECK);
		chkEnums.setText("Enumerations");
		chkEnums.setSelection(true);
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		chkEnums.setLayoutData(gridData);
		
		chkStrings = new Button(grpSI, SWT.CHECK);
		chkStrings.setText("Strings");
		chkStrings.setSelection(true);
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		chkStrings.setLayoutData(gridData);
		
		chkIntegers = new Button(grpSI, SWT.CHECK);
		chkIntegers.setText("Integers");
		chkIntegers.setSelection(true);
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		chkIntegers.setLayoutData(gridData);
				
		chkFloats = new Button(grpSI, SWT.CHECK);
		chkFloats.setText("Floats");
		chkFloats.setSelection(true);
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		chkFloats.setLayoutData(gridData);
			
		return comp;
	}
	
	
	protected void createButtonsForButtonBar(Composite parent) {
		GridLayout layout = new GridLayout();
		layout.numColumns = 3;
		parent.setLayout(layout);
		
		chkKeep = new Button(parent, SWT.CHECK);
		chkKeep.setText("&Keep Dialog");
		chkKeep.setSelection(true);
		
		createButton(parent, Dialog.OK, "&Find Next", true);
		createButton(parent, Dialog.CANCEL, "&Cancel", false);
		
	}
	
	
	protected void configureShell(Shell newShell) {
		super.configureShell(newShell);
		
		setShellStyle(SWT.MODELESS);
		
		newShell.setText("Search " + _target.getName());
		
	}
	
	protected void okPressed() {
		
		if (! txtSearch.getText().equals("")) { // make sure something was entered
			boolean found = startSearch(_target, txtSearch.getText());
			
			if (!found) {
				MessageDialog.openInformation(this.getShell(),
					"Search Completed",
					"No more instances found");
			} // if
		} // if
		
		if (! chkKeep.getSelection()) {
			this.close();
		} // if
	}
	
	/**
	 * Run a search on each of the node's children. 
	 * We start with this function instead of <code>searchNode</code> to deal 
	 *  with that special case where the use has chosen to search top-state.
	 *  As we descend, we don't want to continually search through top-state, 
	 *  because that will never end. <code>searchNode</code> ignores the top-state.
	 * 
	 * @param node The node to begin the search on.
	 * @param clause The node name to look for.
	 * @return <code>true</code> if we found a match, <code>false</code> if not. 
	 */
	private boolean startSearch(DMItem node, String clause) {
		if (node.hasChildren()) {
			ArrayList kids = node.getChildren();
			for (int i=0; i< kids.size(); i++) {
				if ( searchNode( (DMItem)kids.get(i), clause) ) {
					return true;
				} // if
			} // for i
		} // if
		return false;
	}
	
	/**
	 * Recurses through this node and its children to find a match. 
	 * Ignores the top-state's children to avoid infinite recursion. 
	 * @param node The node to search.
	 * @param clause The node name to look for.
	 * @return <code>true</code> if we found a match, <code>false</code> if not.
	 */
	private boolean searchNode(DMItem node, String clause) {

		//System.out.println("Searching: " + node);
		
		//Does this node match? if so, break out and return true.
		if (chkIdentifiers.getSelection() && node instanceof DMIdentifier 
			|| chkEnums.getSelection() && node instanceof DMEnumeration
			|| chkStrings.getSelection() && node instanceof DMString
			|| chkIntegers.getSelection() && node instanceof DMInteger
			|| chkFloats.getSelection() && node instanceof DMFloat) {
			
			if (chkCase.getSelection() && node.getName().equals(clause)
				|| !chkCase.getSelection() 
				&& node.getName().equalsIgnoreCase(clause)) {
				//We found a match.
				
				//This part I am not particularly proud of. Currently, I 
				// maintain a list of all previously found nodes so that
				// I can skip them when finding the next result. Alas, this
				// means I recurse through the tree from the start every time.
				// There has to be a better way. If only Java had continuations...
				if (! _skipList.contains(node)) {
					nodeFound(node);
					return true;
				} // if
			} // if
		} // if
		
		//The node didn't match, check its kids.
		if (node.hasChildren() && !(node instanceof DMTopState)) {
			ArrayList kids = node.getChildren();
			for (int i=0; i< kids.size(); i++) {
				if ( searchNode( (DMItem)kids.get(i), clause) ) {
					return true;
				} // if
			} // for i
		} // if
		
		return false;
	} // boolean searchNode(DMItem node)
	
	
	/**
	 * This is the node we were looking for! Do stuff.
	 * @param node The node that matched the search criteria.
	 */
	protected void nodeFound(DMItem node) {
		ISelection sel = new StructuredSelection(node);
		_editor.getViewer().setSelection(sel);
		
		_skipList.add(node);
		
		if (! chkKeep.getSelection()) {
			this.close();
		} // if
	} // void nodeFound(DMItem node)
	
} // class
