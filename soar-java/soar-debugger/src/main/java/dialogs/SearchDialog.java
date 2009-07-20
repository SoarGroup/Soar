/********************************************************************************************
*
* SearchDialog.java
* 
* Description:	
* 
* Created on 	Apr 14, 2005
* @author 		Douglas Pearson
* 
* Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
********************************************************************************************/
package dialogs;

import modules.AbstractView;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.*;
import org.eclipse.swt.layout.*;
import org.eclipse.swt.widgets.*;

import debugger.MainFrame;

/************************************************************************
 * 
 * Search dialog for finding text in a window
 * 
 ************************************************************************/
public class SearchDialog extends BaseDialog
{
	// The find and replace buttons
	private Button 		m_DoFind;
	private AbstractView m_View ;
	private MainFrame 	m_Frame ;
	private Text 		m_FindText ;
	private Button 		m_Down ;
	private Button 		m_Up ;
	private Button 		m_Match ;
	private Button 		m_SearchHidden ;
	private Button		m_KeepWindow ;
	private Button 		m_Wrap ;
	
	private Listener 	m_ControlF ;

	/********************************************************************************************
	* 
	* Create a simple dialog asking the user for input (a single string).
	* 
	* @param parent			The parent for this dialog (we'll center the dialog within this window)
	* @param title			The title for the dialog
	* @return The value the user entered or null if they cancelled the dialog 
	********************************************************************************************/
	public static SearchDialog showDialog(MainFrame frame, String title, AbstractView view)
	{
		// Create the dialog window
		Composite parent = frame.getWindow() ;
		SearchDialog dialog = new SearchDialog(frame, title, view) ;
				
		dialog.getDialog().setSize(550, 180) ;
		dialog.centerDialog(parent) ;
		dialog.open() ;
		
		return dialog ;
	}

	public boolean isDisposed()
	{
		return getDialog().isDisposed() ;
	}
	
	  /**
	   * FindReplaceDialog constructor
	   * 
	   * @param shell the parent shell
	   * @param document the associated document
	   * @param viewer the associated viewer
	   */
	  private SearchDialog(MainFrame frame, String title, AbstractView view) {
	  	super(frame.getWindow().getShell(), title, false) ;
	  	m_View = view ;
	  	m_Frame = frame ;
	  	
	  	createContents(getOpenArea()) ;
	  }
	  
	  /**
	   * Creates the dialog's contents
	   * 
	   * @param window
	   */
	  protected void createContents(final Composite window) {
	    window.setLayout(new GridLayout(2, false));

	    m_OK.setVisible(false) ;
	    m_Cancel.setVisible(false) ;
	    	    
	    // Add the text input fields
	    Composite text = new Composite(window, SWT.NONE);
	    text.setLayoutData(new GridData(GridData.FILL_HORIZONTAL)) ;
	    text.setLayout(new GridLayout(1, true)) ;

	    new Label(text, SWT.RIGHT).setText("Fi&nd:");
	    m_FindText = new Text(text, SWT.BORDER);
	    GridData data = new GridData(GridData.FILL_HORIZONTAL);
	    data.horizontalSpan = 2;
	    m_FindText.setLayoutData(data);
	    
	    // Start off with anything on the clipboard
	    m_FindText.paste() ;

	    // If there was nothing on the clipboard, go with whatever
	    // the user last typed in here.
	    if (m_FindText.getText().length() == 0)
	    {
	    	String prevText = m_Frame.getAppStringProperty("Search.Text") ;
	    	if (prevText != null)
	    		m_FindText.setText(prevText) ;
	    }
	    
	    m_FindText.selectAll() ;

	    Composite options = new Composite(text, SWT.NULL) ;
	    options.setLayout(new GridLayout(3, true)) ;
	    options.setLayoutData(new GridData(GridData.FILL_HORIZONTAL)) ;
	    
	    // Add the match case checkbox
	    m_Match = new Button(options, SWT.CHECK);
	    m_Match.setText("&Match Case");

	    // Default wrap to true
	    m_Wrap = new Button(options, SWT.CHECK) ;
	    m_Wrap.setText("&Wrap") ;

	    // Add the whole word checkbox
	    m_SearchHidden = new Button(options, SWT.CHECK);
	    m_SearchHidden.setText("&Search hidden text");
	    
	    // Add the direction radio buttons
	    m_Down = new Button(options, SWT.RADIO);
	    m_Down.setText("D&own");

	    m_Up = new Button(options, SWT.RADIO);
	    m_Up.setText("&Up");

	    // Add the keep window checkbox
	    m_KeepWindow = new Button(options, SWT.CHECK) ;
	    m_KeepWindow.setText("&Keep window open") ;
	    
	    // Set the initial values for the controls based on the users previous choices
	    // or the defaults defined here.
	    m_Down.setSelection(m_Frame.getAppBooleanProperty("Search.Down", true)) ;
	    m_Match.setSelection(m_Frame.getAppBooleanProperty("Search.MatchCase", false)) ;
	    m_Wrap.setSelection(m_Frame.getAppBooleanProperty("Search.Wrap", true)) ;
	    m_SearchHidden.setSelection(m_Frame.getAppBooleanProperty("Search.Hidden", true)) ;
	    m_KeepWindow.setSelection(m_Frame.getAppBooleanProperty("Search.Keep", true)) ;

	    // Add the buttons
	    Composite buttons = new Composite(window, SWT.NONE);
	    buttons.setLayout(new GridLayout());

	    // Create the Find button
	    m_DoFind = new Button(buttons, SWT.PUSH);
	    m_DoFind.setText("&Find   Ctrl-F");
	    m_DoFind.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));

	    // Listen for Ctrl-F within the dialog and issue a find immediately.
	    // This allows users to hit Ctrl-F to bring up find and then do it again
	    // to search.
	    m_ControlF = new Listener() { public void handleEvent(Event e) {
	    	if (e.type == SWT.KeyDown)
	    	{
		    	int key = e.keyCode ;
		    	int mask = e.stateMask ;
		    	
		    	if (key == 'f' && (mask & SWT.CTRL) > 0 && !getDialog().isDisposed())
		    		find() ;
	    	}
	    } } ;
	    
	    // Registering a filter means it will apply to any keydown within the dialog's display
	    // rather than having to register with each specific control in the dialog
	    // (which would otherwise be necessary as the key strokes only go to the control with focus).
	    getDialog().getDisplay().addFilter(SWT.KeyDown, m_ControlF) ;

	    // Create the Close button
	    Button close = new Button(buttons, SWT.PUSH);
	    close.setText("Close");
	    close.setLayoutData(new GridData(GridData.FILL_HORIZONTAL));
	    close.addSelectionListener(new SelectionAdapter() {
	      public void widgetSelected(SelectionEvent event) {
	      		close() ;
	      }
	    });

	    // Do a find
	    m_DoFind.addSelectionListener(new SelectionAdapter() {
	      public void widgetSelected(SelectionEvent event) {
	      	find() ;
	      	
	      	if (!m_KeepWindow.getSelection())
	      		close() ;
	      }
	    });

	    // Set defaults
	    m_Down.setSelection(true);
	    m_FindText.setFocus();
	    getDialog().setDefaultButton(m_DoFind);
	  }
	  
	  private void close()
	  {
      	m_Frame.setAppProperty("Search.Down", m_Down.getSelection()) ;
      	m_Frame.setAppProperty("Search.MatchCase", m_Match.getSelection()) ;
      	m_Frame.setAppProperty("Search.Wrap", m_Wrap.getSelection()) ;
      	m_Frame.setAppProperty("Search.Hidden", m_SearchHidden.getSelection()) ;
      	m_Frame.setAppProperty("Search.Text", m_FindText.getText()) ;
      	m_Frame.setAppProperty("Search.Keep", m_KeepWindow.getSelection()) ;
      	
        getDialog().getDisplay().removeFilter(SWT.KeyDown, m_ControlF) ;
        getDialog().close();
	  }
	  
	  /********************************************************************************************
	   * 
	   * Trigger the dialog to start a find.
	   * 
	   * This is exposed publicly so we can programmatically trigger a find once the dialog
	   * is up (based on user key presses etc.)
	   * 
	  ********************************************************************************************/
	  public void find()
	  {
      	boolean found = m_View.find(m_FindText.getText(), m_Down.getSelection(), m_Match.getSelection(), 
        		m_Wrap.getSelection(), m_SearchHidden.getSelection());
      	
        if (!found)
        {
        	m_Frame.ShowMessageBox("Find text", "No match was found") ;
        }
	  }
}
