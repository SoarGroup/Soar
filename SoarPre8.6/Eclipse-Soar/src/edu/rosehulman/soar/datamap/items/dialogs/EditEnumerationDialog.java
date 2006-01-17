/*
 * Created on Jan 28, 2004
 *
 */
package edu.rosehulman.soar.datamap.items.dialogs;


import org.eclipse.jface.dialogs.Dialog;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.swt.events.*;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;

import java.util.ArrayList;

/**
 *
 * The Dialog to edit a datamap enumeration.
 * 
 * @author Zach Lute
 */
public class EditEnumerationDialog extends Dialog {
	Text txtEnum;
	List lstEnums;
	String _name;
	ArrayList _enums;
	

	public EditEnumerationDialog(Shell parentShell, ArrayList enums) {
		super(parentShell);
		_enums = enums;
	}

	// So they can't call this one
	private EditEnumerationDialog(Shell parentShell)
	{
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

		Group grpEnum = new Group(comp, SWT.SHADOW_ETCHED_IN);
		grpEnum.setText("Enumeration");
		GridLayout layout = new GridLayout();
		layout.numColumns = 1;
		
		grpEnum.setLayout(layout);
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		grpEnum.setLayoutData(gridData);
	
		txtEnum = new Text(grpEnum, SWT.LEFT | SWT.BORDER);
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		txtEnum.setLayoutData(gridData);
	
		lstEnums = new List(grpEnum, SWT.LEFT | SWT.BORDER | SWT.V_SCROLL);
		for(int i = 0; i < _enums.size(); i++)
		{
			lstEnums.add((String) _enums.get(i));
		}
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.grabExcessVerticalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		gridData.verticalAlignment = GridData.FILL;
		gridData.heightHint = 100;
		lstEnums.setLayoutData(gridData);
		
		Composite grpButtons = new Composite(grpEnum, SWT.SHADOW_NONE);
		GridLayout layButtons = new GridLayout();
		layButtons.numColumns = 2;
		grpButtons.setLayout(layButtons);
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		grpButtons.setLayoutData(gridData);
		
		Button btnAdd = new Button(grpButtons, SWT.PUSH);
		btnAdd.setText("Add");
		btnAdd.addSelectionListener(new EditSelectionListener());
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
		newShell.setText("Edit Enumeration");
		//newShell.setImage(ItemImages.getImage(ItemImages.ITEM_ENUM));
	}

	protected void okPressed() {
		
		//IInputValidator nameVal = new AttributeNameValidator();
		//String res3 = nameVal.isValid(txtName.getText());

		//if (res3 != null) {
		//	MessageDialog.openError(this.getShell(), "Invalid Attribute Name", res3);
		//	return;
		//} // if
		
		if (lstEnums.getItemCount() == 0) {
			MessageDialog.openError(this.getShell(), "Invalid Enumeration", 
				"Enumeration may not have zero elements");
			return;
		}
		
		//_name = txtName.getText();
		String[] enums = lstEnums.getItems();
		_enums = new ArrayList(enums.length);
		for (int i=0; i<enums.length; i++) {
			_enums.add(enums[i]);
		} // for i
	
		super.okPressed();
	
	}
	
	private class EditSelectionListener implements SelectionListener {
		public void widgetSelected(SelectionEvent e) {
			String temp = txtEnum.getText();
			
			if (temp == null || temp.equals("")) {
				MessageDialog.openError(EditEnumerationDialog.this.getShell(),
					"Invalid Enumeration Data",
					"Zero length strings are not allowed in enumerations");
			} else if (temp.indexOf(" ") != -1) {
				MessageDialog.openError(EditEnumerationDialog.this.getShell(),
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
