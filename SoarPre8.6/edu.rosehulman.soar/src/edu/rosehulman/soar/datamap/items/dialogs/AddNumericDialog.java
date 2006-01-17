/*
 * Created on Dec 30, 2003
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap.items.dialogs;

import edu.rosehulman.soar.datamap.validators.*;

import org.eclipse.jface.dialogs.Dialog;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.jface.dialogs.IInputValidator;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.graphics.Image;


/**
 *
 * A Dialog box allowing the user to initialize the falues of a Numeric 
 * datamap attribute.
 * 
 * @author Tim Jasko
 * @author Zach Lute
 */
public class AddNumericDialog extends Dialog {
	Text txtName, txtRangeLower, txtRangeUpper;
	IInputValidator validator;
	
	String lowerBound, upperBound, name;
	
	Image _icon;

	public AddNumericDialog(Shell parentShell, IInputValidator v) {
		super(parentShell);
		
		validator = v;
	}
	
	public String getName() {
		return name;
	}
	
	public String getLowerBound() {
		return lowerBound;
	}
	
	public String getUpperBound() {
		return upperBound;
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
		grp2.setText("Range");
		GridLayout layout = new GridLayout();
		layout.numColumns = 3;
		grp2.setLayout(layout);
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		grp2.setLayoutData(gridData);
		
		txtRangeLower = new Text(grp2, SWT.LEFT | SWT.BORDER);
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		txtRangeLower.setLayoutData(gridData);
		
		Label lblTo = new Label(grp2, SWT.CENTER);
		lblTo.setText("to");
		
		txtRangeUpper = new Text(grp2, SWT.LEFT | SWT.BORDER);
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		txtRangeUpper.setLayoutData(gridData);
		
		return comp;
	}
	
	protected void configureShell(Shell newShell) {
		super.configureShell(newShell);
		if (validator instanceof IntegerInputValidator) {
			newShell.setText("Add Integer");
			//newShell.setImage(ItemImages.getImage(ItemImages.ITEM_INT));
		} else if (validator instanceof FloatInputValidator) {
			newShell.setText("Add Float");
			//newShell.setImage(ItemImages.getImage(ItemImages.ITEM_FLOAT));
		}
	}
	
	
	protected void okPressed() {
		String lb = txtRangeLower.getText();
		String ub = txtRangeUpper.getText();
		
		String res1 = validator.isValid(lb);
		String res2 = validator.isValid(ub);
		
		IInputValidator nameVal = new AttributeNameValidator();
		String res3 = nameVal.isValid(txtName.getText());
		
		if (res3 != null) {
			MessageDialog.openError(this.getShell(), "Invalid Attribute Name", res3);
			return;
		} // if
		
		if (res1 != null) {
			MessageDialog.openError(this.getShell(), "Invalid lower limit", res1);
			return;
		} // if
		
		if (res2 != null) {
			MessageDialog.openError(this.getShell(), "Invalid upper limit", res2);
			return;
		} // if
		
		if (!(ub == null || ub.equals("")) && !(lb == null || lb.equals(""))) {
			Double dlb = new Double(lb);
			Double dub = new Double(ub);
			
			if (dlb.compareTo(dub) > 0) {
				MessageDialog.openError(this.getShell(), "Invalid Range",
					"The left field must be less than the right field");
				return;
			}  // if
		} // if
			
		
		lowerBound = txtRangeLower.getText();
		upperBound = txtRangeUpper.getText();
		name = txtName.getText();
		
		super.okPressed();
		
	}


}
