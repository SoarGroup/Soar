/********************************************************************************************
*
* PropertiesDialog.java
* 
* Description:	
* 
* Created on 	Mar 23, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package dialogs;

import helpers.FormDataHelper;
import modules.AbstractView;

import org.eclipse.swt.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.custom.*;
import org.eclipse.swt.graphics.Point;

import debugger.MainFrame;

/************************************************************************
 * 
 * The base dialog used to show properties for a view
 * 
 ************************************************************************/
public class PropertiesDialog extends BaseDialog
{
	public abstract static class Property
	{
		protected String m_PropertyName ;
		
		public Property(String propertyName)
		{
			m_PropertyName = propertyName ;
		}
		
		public abstract TableItem addTableItem(Table table) ;

		// Copy the value from the control to the property
		// Return false if its not valid for some reason.
		public abstract boolean update() ;

		protected void showError(Composite parent, String error)
		{
			// Display an SWT message box
			MessageBox msg = new MessageBox(parent.getShell(), 0);
			msg.setText("Error entering property " + m_PropertyName) ;
			msg.setMessage(error);
			msg.open();
		}
	}
	
	public static class IntProperty extends Property
	{
		protected int m_Value ;
		protected int m_MinValue ;
		protected int m_MaxValue ;
		protected Text m_Text ;
		
		public IntProperty(String propertyName, int initialValue, int minValue, int maxValue)
		{
			super(propertyName) ;
			m_Value = initialValue ;
			m_MinValue = minValue ;
			m_MaxValue = maxValue ;
		}
		
		public IntProperty(String propertyName, int initialValue)
		{
			this(propertyName, initialValue, Integer.MIN_VALUE, Integer.MAX_VALUE) ;
		}
		
		public int getValue() { return m_Value ; }
		
		public TableItem addTableItem(final Table table)
		{
			TableItem item = new TableItem (table, SWT.NULL);
			item.setText (0, m_PropertyName);
			
			// Create a text field for this property
			m_Text = new Text (table, SWT.NONE);
			m_Text.setText(Integer.toString(m_Value)) ;
			TableEditor editor = new TableEditor (table);
			editor.grabHorizontal = true;
			editor.setEditor(m_Text, item, 1);

			return item ;
		}
		
		public boolean update()
		{
			try
			{
				String value = m_Text.getText() ;
				int newValue = Integer.parseInt(value) ;
				
				if (newValue < m_MinValue || newValue > m_MaxValue)
				{
					showError(m_Text.getParent(), "The value for " + m_PropertyName + " must be in the range " + m_MinValue + " to " + m_MaxValue) ;
					return false ;
				}
				else
				{
					// Only record the new value if its valid
					m_Value = newValue ;
					return true ;
				}
			}
			catch (NumberFormatException ex)
			{
				showError(m_Text.getParent(), "The value for " + m_PropertyName + " must be a number") ;
				return false ;
			}
		}
	}
	
	public static class StringProperty extends Property
	{
		protected String m_Value ;
		protected Text	 m_Text ;
		
		public StringProperty(String propertyName, String initialValue)
		{
			super(propertyName) ;
			m_Value = initialValue ;
			
			if (initialValue == null)
				m_Value = "" ;
		}
		
		public String getValue() { return m_Value ; }

		public TableItem addTableItem(final Table table)
		{
			TableItem item = new TableItem (table, SWT.NULL);
			item.setText (0, m_PropertyName);
			
			// Create a text field for this property
			m_Text = new Text (table, SWT.NONE);
			m_Text.setText(m_Value) ;
			TableEditor editor = new TableEditor (table);
			editor.grabHorizontal = true;
			editor.setEditor(m_Text, item, 1);

			return item ;
		}

		// Copy the value from the control to the property
		// Return false if its not valid for some reason.
		public boolean update()
		{
			String value = m_Text.getText() ;
			m_Value = value ;
				
			return true ;
		}
	}
	
	public static class MultiLineStringProperty extends Property
	{
		protected String m_Value ;
		protected Button m_Button ;
		protected Text	 m_Text ;
		
		public MultiLineStringProperty(String propertyName, String initialValue)
		{
			super(propertyName) ;
			m_Value = initialValue ;
			
			if (initialValue == null)
			{
				m_Value = "" ;
			}
		}
		
		public String getValue() { return m_Value ; }
		
		public String[] getLines()
		{
			String[] lines = m_Value.split(AbstractView.kLineSeparator) ;
			return lines ;
		}

		private void updateButtonLabel()
		{
			String[] lines = getLines() ;
			String first = lines.length == 1 ? lines[0] : lines.length > 0 ? lines[0] + "..." : "<command>" ;
			m_Button.setText(first) ;
		}
		
		public TableItem addTableItem(final Table table)
		{
			TableItem item = new TableItem (table, SWT.NULL);
			item.setText (0, m_PropertyName);
			
			// Create a text field for this property
			// Can't have a multi-line edit control in a table (SWT limitation)
			// so we'll use a button and pop up the edit control.
			m_Button = new Button(table, SWT.PUSH) ;
			updateButtonLabel() ;
			
			// For some reason using the selectionAdapter doesn't work here, but a generic listener does.
			m_Button.addListener(SWT.Selection, new Listener() { public void handleEvent(Event e) {  
				String newValue = SwtInputDialog.showMultiLineDialog(table, "Enter button actions", "Actions", m_Value) ;

				if (newValue != null)
				{
					// We store our commands separated with \n since that's what Soar uses (not \r\n which is what Windows uses, sometimes)
					m_Value = newValue.replaceAll(AbstractView.kSystemLineSeparator, AbstractView.kLineSeparator) ;
					updateButtonLabel() ;
				}
			} } ) ;
			
			TableEditor editor = new TableEditor (table);
			editor.grabHorizontal = true;
			editor.setEditor(m_Button, item, 1);

			return item ;
		}

		// Copy the value from the control to the property
		// Return false if its not valid for some reason.
		public boolean update()
		{
			//String value = m_Text.getText() ;
			//m_Value = value ;
				
			return true ;
		}
	}

	public static class BooleanProperty extends Property
	{
		protected boolean m_Value ;
		protected CCombo  m_Combo ;
		
		public BooleanProperty(String propertyName, boolean initialValue)
		{
			super(propertyName) ;
			m_Value = initialValue ;
		}
		
		public boolean getValue() { return m_Value ; }

		public TableItem addTableItem(Table table)
		{
			TableItem item = new TableItem (table, SWT.NULL);
			item.setText (0, m_PropertyName);
			
			// Create a combo box for this property
			m_Combo = new CCombo (table, SWT.NONE);
			m_Combo.setItems(new String[] { "True", "False"} ) ;
			m_Combo.setEditable(false) ;
			TableEditor editor = new TableEditor (table);
			editor.grabHorizontal = true;
			editor.setEditor(m_Combo, item, 1);
			
			if (m_Value)
				m_Combo.select(0) ;
			else
				m_Combo.select(1) ;
			
			return item ;
		}

		// Copy the value from the control to the property
		// Return false if its not valid for some reason.
		public boolean update()
		{
			int selection = m_Combo.getSelectionIndex() ;
			m_Value = (selection == 0) ;
			
			return true ;
		}
	}
	
	public static class SetProperty extends Property
	{
		protected int      m_Index ;
		protected Object[] m_SetValues ;
		protected String[] m_SetDescriptions ;
		protected CCombo   m_Combo ;
		
		public SetProperty(String propertyName, Object initialValue, Object[] setValues, String[] setDescriptions)
		{
			super(propertyName) ;

			if (setValues.length != setDescriptions.length)
				throw new IllegalStateException("The arrays of objects and descriptions don't match") ;
			
			m_Index = -1 ;
			for (int i = 0 ; i < setValues.length ; i++)
			{
				// Allow null as a value
				if ((initialValue == null && setValues[i] == null) || setValues[i].equals(initialValue))
					m_Index = i ;
			}
	
			// Want to allow for a value that is null initially and then is set later
//			if (m_Index == -1)
//				throw new IllegalStateException("Initial value isn't in the set") ;
			
			m_SetValues = setValues ;
			m_SetDescriptions = setDescriptions ;
		}
		
		public Object getValue() { return (m_Index >= 0) ? m_SetValues[m_Index] : null ; }

		public TableItem addTableItem(Table table)
		{
			TableItem item = new TableItem (table, SWT.NULL);
			item.setText (0, m_PropertyName);
			
			// Create a combo box for this property
			m_Combo = new CCombo (table, SWT.NONE);
			m_Combo.setItems(m_SetDescriptions) ;
			m_Combo.setEditable(false) ;
			TableEditor editor = new TableEditor (table);
			editor.grabHorizontal = true;
			editor.setEditor(m_Combo, item, 1);
			
			m_Combo.select(m_Index) ;
			
			return item ;
		}

		// Copy the value from the control to the property
		// Return false if its not valid for some reason.
		public boolean update()
		{
			m_Index = m_Combo.getSelectionIndex() ;
			return true ;
		}
	}
	
	protected Property[] m_BaseProperties ;
	protected Composite  m_Container ;
	
	public PropertiesDialog(MainFrame frame, String title, boolean modal, Property[] baseProperties)
	{
		super(frame.getWindow(), title, modal) ;
		m_BaseProperties = baseProperties ;

		m_Container = new Group(getOpenArea(), SWT.NULL) ;
		m_Container.setLayout(new FillLayout()) ;
		
		FormData data = FormDataHelper.anchorTop(0) ;
		data.bottom = new FormAttachment(100) ;
		m_Container.setLayoutData(data) ;

		final Table table = new Table (m_Container, SWT.BORDER | SWT.FULL_SELECTION);
		table.setLinesVisible (true);
		table.setHeaderVisible (true);
		String[] titles = {"Property Name", "Property Value"};
		for (int i=0; i<titles.length; i++) {
			TableColumn column = new TableColumn (table, SWT.NULL);
			column.setText (titles [i]);
		}	

		for (int i = 0 ; i < baseProperties.length ; i++)
		{
			Property property = baseProperties[i] ;
			property.addTableItem(table) ;
		}
		
		for (int i=0; i<titles.length; i++) {
			table.getColumn (i).pack ();
		}
	}
	
	/********************************************************************************************
	* 
	* Create a simple dialog asking the user for input (a single string).
	* 
	* @param parent			The parent for this dialog (we'll center the dialog within this window)
	* @param title			The title for the dialog
	* @return True if user closed by pressing OK.  The properties passed in will have been updated.
	********************************************************************************************/
	public static boolean showDialog(MainFrame frame, String title, Property[] baseProperties)
	{
		// Create the dialog window
		Composite parent = frame.getWindow() ;
		PropertiesDialog dialog = new PropertiesDialog(frame, title, true, baseProperties) ;
				
		Point size = dialog.getDialog().computeSize(SWT.DEFAULT, SWT.DEFAULT) ;
		size.x = size.x + 20 ;
		size.y = size.y + 50 ;
		dialog.getDialog().setSize(size) ;
		dialog.centerDialog(parent) ;
		dialog.open() ;
		
		dialog.pumpMessages() ;
		
		return !dialog.m_Cancelled ;
	}
	
	/********************************************************************************************
	* 
	* Close the dialog -- either successfully or cancelled.
	* 
	********************************************************************************************/
	protected void endDialog(boolean ok)
	{
		if (ok)
		{
			boolean valid = true ;
			for (int i = 0 ; i < m_BaseProperties.length && valid ; i++)
			{
				Property property = m_BaseProperties[i] ;
				valid = property.update() ;
			}
			
			// If some field is not valid, don't close the dialog
			if (!valid)
				return ;
		}

		// Close down now
		super.endDialog(ok) ;
	}

}
