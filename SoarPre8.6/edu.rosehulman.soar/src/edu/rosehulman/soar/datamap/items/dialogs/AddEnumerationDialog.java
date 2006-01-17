/*
 * Created on Jan 27, 2004
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap.items.dialogs;

import edu.rosehulman.soar.datamap.validators.*;

import org.eclipse.jface.dialogs.Dialog;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.dialogs.IInputValidator;
import org.eclipse.swt.events.*;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;

import java.util.ArrayList;

/**
 * The Dialog to create a new datamap enumeration.
 * 
 * @author Tim Jasko
 */
public class AddEnumerationDialog extends Dialog {
	Text txtName, txtEnum;
	List lstEnums;
	String _name;
	ArrayList _enums;
	

	public AddEnumerationDialog(Shell parentShell) {
		super(parentShell);
	}

	public String getName() {
		return _name;
	}
	
	public ArrayList getEnums() {
		return _enums;
	}


	protected Control createDialogArea(Composite parent) {
		Composite comp = (Composite)super.createDialogArea(parent);
	
		comp.setLayout(new GridLayout());
	
		GridData gridData;

		Group grp1 = new Group(comp, SWT.SHADOW_ETCHED_IN);
		grp1.setText("Attribute Name");
		grp1.setLayout(new GridLayout());
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		grp1.setLayoutData(gridData);
	
		txtName = new Text(grp1, SWT.LEFT | SWT.BORDER);
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		txtName.setLayoutData(gridData);
	
		Group grp2 = new Group(comp, SWT.SHADOW_ETCHED_IN);
		grp2.setText("Enumeration");
		GridLayout layout = new GridLayout();
		layout.numColumns = 1;
		
		
		grp2.setLayout(layout);
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		grp2.setLayoutData(gridData);
	
		txtEnum = new Text(grp2, SWT.LEFT | SWT.BORDER);
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		txtEnum.setLayoutData(gridData);
	
		lstEnums = new List(grp2, SWT.LEFT | SWT.BORDER | SWT.V_SCROLL);
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.grabExcessVerticalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		gridData.verticalAlignment = GridData.FILL;
		gridData.heightHint = 100;
		lstEnums.setLayoutData(gridData);
		
		Composite grpButtons = new Composite(grp2, SWT.SHADOW_NONE);
		GridLayout layButtons = new GridLayout();
		layButtons.numColumns = 2;
		grpButtons.setLayout(layButtons);
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		grpButtons.setLayoutData(gridData);
		
		Button btnAdd = new Button(grpButtons, SWT.PUSH);
		btnAdd.setText("Add");
		btnAdd.addSelectionListener(new AddSelectionListener());
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		btnAdd.setLayoutData(gridData);
		
		Button btnRemove = new Button(grpButtons, SWT.PUSH);
		btnRemove.setText("Remove");
		btnRemove.addSelectionListener(new RemoveSelectionListener());
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		btnRemove.setLayoutData(gridData);
	
		return comp;
	}

	protected void configureShell(Shell newShell) {
		super.configureShell(newShell);
		newShell.setText("Add Enumeration");
		//newShell.setImage(ItemImages.getImage(ItemImages.ITEM_ENUM));
	}

	protected void okPressed() {
		
		IInputValidator nameVal = new AttributeNameValidator();
		String res3 = nameVal.isValid(txtName.getText());

		if (res3 != null) {
			MessageDialog.openError(this.getShell(), "Invalid Attribute Name", res3);
			return;
		} // if
		
		if (lstEnums.getItemCount() == 0) {
			MessageDialog.openError(this.getShell(), "Invalid Enumeration", 
				"Enumeration may not have zero elements");
			return;
		}
		
		_name = txtName.getText();
		String[] enums = lstEnums.getItems();
		_enums = new ArrayList(enums.length);
		for (int i=0; i<enums.length; i++) {
			_enums.add(enums[i]);
		} // for i
	
		super.okPressed();
	
	}
	
	private class AddSelectionListener implements SelectionListener {
		public void widgetSelected(SelectionEvent e) {
			String temp = txtEnum.getText();
			if (temp == null || temp.equals("")) {
				MessageDialog.openError(AddEnumerationDialog.this.getShell(),
					"Invalid Enumeration Data",
					"Zero length strings are not allowed in enumerations");
			} else if (temp.indexOf(" ") != -1) {
				MessageDialog.openError(AddEnumerationDialog.this.getShell(),
					"Invalid Enumeration Data",
					"Spaces are not allowed in enumerations");
			} else {
				lstEnums.add(temp);
				txtEnum.setText("");
				txtEnum.setFocus();
			} // else
		}
		public void widgetDefaultSelected(SelectionEvent e) {
		}
	}
	
	private class RemoveSelectionListener implements SelectionListener {
		public void widgetSelected(SelectionEvent e) {
			int sel = lstEnums.getSelectionIndex();
			
			if (sel != -1) {
				lstEnums.remove(sel);
			}

		}
		public void widgetDefaultSelected(SelectionEvent e) {
		}
	}
}
