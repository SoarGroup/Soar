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
 * The dialog box to edit the values of DMIntgers and DMFloats
 * 
 * @author Zach Lute
 * @author Tim Jasko
 */
public class EditNumericDialog extends Dialog {
	Text txtRangeLower, txtRangeUpper;
	IInputValidator validator;
	
	String _lowerBound, _upperBound;
	
	Image _icon;

	public EditNumericDialog(Shell parentShell, IInputValidator v, String lower, String upper) {
		super(parentShell);
		_lowerBound = lower;
		_upperBound = upper;
		
		validator = v;
	}

	// So they can't call this
	private EditNumericDialog(Shell parentShell)
	{
		super(parentShell);
	}
	
	public String getLowerBound() {
		return _lowerBound;
	}
	
	public String getUpperBound() {
		return _upperBound;
	}
	
	protected Control createDialogArea(Composite parent) {
		Composite comp = (Composite)super.createDialogArea(parent);
		
		comp.setLayout(new GridLayout());
		
		GridData gridData;

		Group grpNum = new Group(comp, SWT.SHADOW_ETCHED_IN);
		grpNum.setText("Range");
		GridLayout layout = new GridLayout();
		layout.numColumns = 3;
		grpNum.setLayout(layout);
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		grpNum.setLayoutData(gridData);
		
		txtRangeLower = new Text(grpNum, SWT.LEFT | SWT.BORDER);
		txtRangeLower.setText(_lowerBound);
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		txtRangeLower.setLayoutData(gridData);
		
		Label lblTo = new Label(grpNum, SWT.CENTER);
		lblTo.setText("to");
		
		txtRangeUpper = new Text(grpNum, SWT.LEFT | SWT.BORDER);
		txtRangeUpper.setText(_upperBound);
		gridData = new GridData();
		gridData.grabExcessHorizontalSpace = true;
		gridData.horizontalAlignment = GridData.FILL;
		txtRangeUpper.setLayoutData(gridData);
		
		return comp;
	}
	
	protected void configureShell(Shell newShell) {
		super.configureShell(newShell);
		if (validator instanceof IntegerInputValidator) {
			newShell.setText("Edit Integer");
			//newShell.setImage(ItemImages.getImage(ItemImages.ITEM_INT));
		} else if (validator instanceof FloatInputValidator) {
			newShell.setText("Edit Float");
			//newShell.setImage(ItemImages.getImage(ItemImages.ITEM_FLOAT));
		}
	}
	
	
	protected void okPressed() {
		String lb = txtRangeLower.getText();
		String ub = txtRangeUpper.getText();
		
		String res1 = validator.isValid(lb);
		String res2 = validator.isValid(ub);
		
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
		
		_lowerBound = txtRangeLower.getText();
		_upperBound = txtRangeUpper.getText();
		
		super.okPressed();
		
	}


}
