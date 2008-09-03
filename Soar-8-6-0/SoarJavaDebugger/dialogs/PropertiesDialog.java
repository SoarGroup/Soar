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

import org.eclipse.swt.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;
import org.eclipse.swt.custom.*;
import org.eclipse.swt.events.*;
import org.eclipse.swt.graphics.Point;

import debugger.MainFrame;
import dialogs.RemoteDialog.RemoteInfo;

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
			
			if (m_Index == -1)
				throw new IllegalStateException("Initial value isn't in the set") ;
			
			m_SetValues = setValues ;
			m_SetDescriptions = setDescriptions ;
		}
		
		public Object getValue() { return m_SetValues[m_Index] ; }

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
