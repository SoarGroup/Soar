/********************************************************************************************
*
* NewWindowDialog.java
* 
* Created on 	Nov 22, 2003
*
* @author 		Doug
* @version
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package dialogs;

import javax.swing.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;

import doc.* ;
import general.* ;

/********************************************************************************************
* 
* This dialog offers a list of available modules (window classes) and allows the user to select one.
* 
* This dialog (like most I write) is designed as a panel so it can be used in a dialog or
* placed on another window if desired.
* 
********************************************************************************************/
public class NewWindowDialog extends JPanel
{
	/** The parent dialog (if there is one) */
	private JDialog m_ParentDialog = null ;

	private ModuleList	m_ModuleList ;
	private Module		m_Module ;

	private boolean m_Inited = false ;				// Has this dialog been initialized
	private boolean	m_InitedFixedWidth = false ;	// The list of fixed width fonts is initialized later
	
	private JList		m_Modules ;
	private JTextArea	m_Description ;
		
	/**
	 * Called when this window is added to a parent
	 */
	public void addNotify()
	{
		super.addNotify();
		
		Init() ;
	}
		
	/********************************************************************************************
	* 
	* Initialize all of the windows and controls in the dialog.
	* 
	********************************************************************************************/
	private void Init()
	{
		if (m_Inited)
			return ;
		m_Inited = true ;
		
		m_Modules = new JList(m_ModuleList.toArray()) ;
		m_Modules.setVisibleRowCount(20) ;
		m_Modules.addListSelectionListener(new ListSelectionListener() { public void valueChanged(ListSelectionEvent e)  { selectionChanged() ; } } ) ;
		JScrollPane scrollModules = new JScrollPane(m_Modules) ;

		m_Description = new JTextArea() ;
		m_Description.setLineWrap(true) ;
		m_Description.setWrapStyleWord(true) ;

		JPanel leftPanel  = new LabelPanel("Window Types", scrollModules ,BorderLayout.NORTH) ;
		JPanel rightPanel = new LabelPanel("Description", m_Description, BorderLayout.NORTH) ;
		
		JPanel buttonPanel = new JPanel() ;

		JButton ok = new JButton("OK") ;
		JButton cancel = new JButton("Cancel") ;

		ok.addActionListener(new ActionListener()     { public void actionPerformed(ActionEvent e) { okPressed() ; } } ) ;
		cancel.addActionListener(new ActionListener() { public void actionPerformed(ActionEvent e) { cancelPressed() ; } } ) ;
				
		buttonPanel.setLayout(new GridLayout(1, 0, 10, 10)) ;
		buttonPanel.add(ok) ;
		buttonPanel.add(cancel) ;

		if (this.m_ParentDialog != null)
		{
			this.m_ParentDialog.getRootPane().setDefaultButton(ok) ;
		}
		
		JPanel top = new JPanel() ;
		top.setLayout(new GridLayout(1, 0, 10, 10)) ;
		top.add(leftPanel) ;
		top.add(rightPanel) ;
				
		this.setLayout(new BorderLayout()) ;
		this.add(top, BorderLayout.CENTER) ;
		this.add(buttonPanel, BorderLayout.SOUTH) ;
	}
	
	/********************************************************************************************
	* 
	* One of the lists or fields used to describe the font has changed.
	* 
	********************************************************************************************/
	private void selectionChanged()
	{
		int selection = m_Modules.getSelectedIndex() ;
		
		if (selection == -1)
			return ;

		// Retrieve the selected module			
		m_Module = m_ModuleList.get(selection) ;
		
		// Change the description
		m_Description.setText(m_Module.getDescription()) ;	
	}
	
	/********************************************************************************************
	* 
	* The user pressed OK.
	* 
	********************************************************************************************/
	private void okPressed()
	{
		EndDialog() ;
	}
	
	/********************************************************************************************
	* 
	* The user pressed Cancel.
	* 
	********************************************************************************************/
	private void cancelPressed()
	{
		m_Module = null ;
		EndDialog() ;
	}
	
	/********************************************************************************************
	* 
	* Close the dialog window.
	* 
	********************************************************************************************/
	private void EndDialog()
	{
		JDialog dialog = this.m_ParentDialog ;
		if (dialog == null)
			return ;
		
		// Close the dialog
		dialog.setVisible(false) ;
		dialog.dispose() ;
	}

	/**************************************************************************
	 * 
	 * Creates the panel inside a dialog box, shows the dialog (modally)
	 * and returns the results of the user's selection.
	 * 
	 * @param	frame		The frame which that will own this dialog
	 * @param	title		The title of the dialog window
	 * @param	moduleList	The list of available modules
	 * 
	 * @return Module		The module the user chose (or null if cancelled)
	 *************************************************************************/
	public static Module ShowDialog(Frame frame, String title, ModuleList moduleList)
	{		
		if (moduleList == null)
			throw new IllegalArgumentException("Must pass in the list of available modules") ;
	
		// Create a new, modal dialog
		boolean modal = true ;
		JDialog dialog = new JDialog(frame, title, modal) ;
		
		// Set the size of the window and center it
		int width = 400 ;
		int height = 400 ;
		dialog.setSize(width, height) ;
		dialog.setLocation(frame.getWidth() / 2 - width / 2, frame.getHeight() / 2 - height / 2) ;
		
		dialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE) ;
		
		// Build the content for the dialog
		NewWindowDialog panel = new NewWindowDialog() ;
		panel.m_ModuleList	 = moduleList ;
		panel.m_ParentDialog = dialog ;
		
		// Make the view the contents of the dialog box.
		dialog.getContentPane().add(panel) ;
		
		// Show everything -- the dialog becomes modal at this point and
		// the flow of control stops until the window is closed.
		dialog.setVisible(true) ;

		// Return the user's choice
		return panel.m_Module ;
	}
}

