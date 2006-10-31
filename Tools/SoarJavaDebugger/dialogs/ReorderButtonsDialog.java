/********************************************************************************************
*
* ReorderButtonsDialog.java
* 
* Description:	
* 
* Created on 	Oct 1, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package dialogs;

import java.util.ArrayList;

import helpers.FormDataHelper;

import modules.ButtonView;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import debugger.MainFrame;

/************************************************************************
* 
* A dialog that allows the user to define the order of a list of buttons.
* 
************************************************************************/
public class ReorderButtonsDialog extends BaseDialog
{
	private ArrayList       m_Result ;
	private List            m_ButtonListControl ;
	private ButtonView.ButtonInfo[] m_Buttons ;
	private String[] 		m_ButtonNames ;
	private boolean			m_ModifiedList = false ;
		
	/********************************************************************************************
	* 
	* Create a simple dialog asking the user to create a new sequence of buttons.
	* 
	* @param parent			The parent for this dialog (we'll center the dialog within this window)
	* @param title			The title for the dialog
	* @param prompt			The message used to prompt for input
	* @param buttonInfoList	The list of buttons (described as ButtonInfo objects) to re-order
	* @return A new sequence of buttonInfo objects or null if they cancelled the dialog 
	********************************************************************************************/
	public static ArrayList showDialog(Composite parent, String title, ArrayList buttonInfoList)
	{
		// Create the dialog window
		ReorderButtonsDialog dialog = new ReorderButtonsDialog(parent, title, buttonInfoList) ;
				
		dialog.getDialog().pack() ;
		dialog.centerDialog(parent) ;
		dialog.open() ;
		
		dialog.pumpMessages() ;
		
		return dialog.m_Result ;
	}

	/********************************************************************************************
	* 
	* Create the dialog -- the constructor is private because we use a static method to build this.
	* 
	********************************************************************************************/
	private ReorderButtonsDialog(Composite parent, String title, ArrayList buttonInfoList)
	{
		// Create a basic dialog with OK and Cancel buttons
		super(parent, title, true) ;
				
		int margin = 10 ;
		
		// Copy the array list into an array
		m_Buttons = new ButtonView.ButtonInfo[buttonInfoList.size()] ;
		buttonInfoList.toArray(m_Buttons) ;
		
		m_ButtonNames = new String[m_Buttons.length] ;
		for (int i = 0 ; i < m_Buttons.length ; i++)
			m_ButtonNames[i] = m_Buttons[i].m_Name ;

		// Create arrows to scroll up and down
		Group left = new Group(getOpenArea(), SWT.SHADOW_ETCHED_IN) ;
		RowLayout layout = new RowLayout(SWT.VERTICAL) ;
		layout.fill = true ;
		left.setLayout(layout) ;
		
		Button top = new Button(left, SWT.PUSH) ;
		top.setText("Top") ;
		top.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { pressed(true, true) ; }}) ;
		
		Button up = new Button(left, SWT.PUSH) ;
		up.setText("Up") ;
		up.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { pressed(true, false) ; }}) ;
		
		Button down = new Button(left, SWT.PUSH) ;
		down.setText("Down") ;
		down.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { pressed(false, false) ; }}) ;
		
		Button bottom = new Button(left, SWT.PUSH) ;
		bottom.setText("Bottom") ;
		bottom.addSelectionListener(new SelectionAdapter() { public void widgetSelected(SelectionEvent e) { pressed(false, true) ; }}) ;
		
		// Create the list of buttons and anchor it to the top
		Group right = new Group(getOpenArea(), SWT.SHADOW_ETCHED_IN) ;
		right.setLayout(new FillLayout()) ;
		m_ButtonListControl = new List(right, SWT.SINGLE | SWT.V_SCROLL | SWT.BORDER) ;
		m_ButtonListControl.setItems(m_ButtonNames) ;

		getOpenArea().setLayout(new FormLayout()) ;

		FormData leftData = FormDataHelper.anchorTopLeft(margin) ;
		FormData rightData = FormDataHelper.anchorTopRight(margin) ;
		leftData.right = new FormAttachment(right, -margin*2) ;
		
		// Move the positioning buttons down if we have a decent sized
		// list of buttons to re-order.
		if (m_Buttons.length > 9)
			leftData.top   = new FormAttachment(30) ;
				
		left.setLayoutData(leftData) ;
		right.setLayoutData(rightData) ;
	}
	
	protected void pressed(boolean up, boolean toLimit)
	{
		int selection = m_ButtonListControl.getSelectionIndex() ;
		if (selection == -1)
		{
			MainFrame.ShowMessageBox(getDialog().getShell(), "Error", "Select a button first before trying to move it.", SWT.OK) ;
			return ;
		}
		
		// Decide where the selection is going
		int limit = m_Buttons.length - 1 ;
		int dest = selection ;
		
		if (up && toLimit)  dest = 0 ;
		if (up && !toLimit && selection > 0) dest = selection-1 ;
		if (!up && toLimit) dest = limit ;
		if (!up && !toLimit && selection < limit) dest = selection+1 ;
		
		// Nothing to move
		if (dest == selection)
			return ;
		
		// Make the shift
		ButtonView.ButtonInfo temp = m_Buttons[selection] ;
		if (up)
		{
			for (int i = selection ; i > dest ; i--)
				m_Buttons[i] = m_Buttons[i-1] ;
			m_Buttons[dest] = temp ;
		}
		else
		{
			for (int i = selection ; i < dest ; i++)
				m_Buttons[i] = m_Buttons[i+1] ;
			m_Buttons[dest] = temp ;
		}
		
		// Update the control
		for (int i = 0 ; i <= limit ; i++)
			this.m_ButtonNames[i] = m_Buttons[i].m_Name ;
		m_ButtonListControl.setItems(m_ButtonNames) ;
		m_ButtonListControl.setSelection(dest) ;

		this.m_ModifiedList = true ;	
	}
	
	/********************************************************************************************
	* 
	* Close the dialog -- either successfully or cancelled.
	* 
	********************************************************************************************/
	protected void endDialog(boolean ok)
	{
		// If the user cancelled (or made no changes) we're done
		if (!ok || !m_ModifiedList)
		{
			m_Result = null ;
		}
		else
		{
			m_Result = new ArrayList() ;
			
			for (int i = 0 ; i < m_Buttons.length ; i++)
				m_Result.add(m_Buttons[i]) ;
		}
		
		// Close the dialog
		super.endDialog(ok) ;
	}

}
