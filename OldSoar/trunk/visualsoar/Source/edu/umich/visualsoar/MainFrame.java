package edu.umich.visualsoar;
import edu.umich.visualsoar.misc.*;
import edu.umich.visualsoar.dialogs.*;
import edu.umich.visualsoar.operatorwindow.*;
import edu.umich.visualsoar.graph.*;
import edu.umich.visualsoar.datamap.*;
import edu.umich.visualsoar.ruleeditor.*;
import edu.umich.visualsoar.util.*;
import edu.umich.visualsoar.parser.ParseException;
import edu.umich.visualsoar.parser.TokenMgrError;

// 3P
import threepenny.*;

import java.awt.*;
import java.awt.dnd.*;
import java.awt.event.*;
import java.awt.datatransfer.*;
import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.text.*;
import java.io.*;
import javax.swing.undo.*;
import javax.swing.event.*;
import javax.swing.border.TitledBorder;
import java.util.*;

// The global application class

/** 
 * This is the main project window of VisualSoar
 * @author Brad Jones
 */
public class MainFrame extends JFrame 
{

/////////////////////////////////////////
// Static Members
/////////////////////////////////////////
	private static MainFrame s_mainFrame;

////////////////////////////////////////
// Data Members
////////////////////////////////////////
	private OperatorWindow operatorWindow;

	private CustomDesktopPane DesktopPane = new CustomDesktopPane();
	private TemplateManager d_templateManager = new TemplateManager();
	private JSplitPane operatorDesktopSplit = new JSplitPane(JSplitPane.HORIZONTAL_SPLIT);
    private JSplitPane feedbackDesktopSplit = new JSplitPane(JSplitPane.VERTICAL_SPLIT);
	public FeedbackList feedbackList = new FeedbackList();
	Preferences prefs = Preferences.getInstance();

	// 3P
	// Soar Tool Interface (STI) object, message time, and menu objects
	private SoarToolJavaInterface soarToolJavaInterface = null;
	private javax.swing.Timer soarToolPumpMessageTimer = null;
	private JMenu soarRuntimeAgentMenu = null;
	
////////////////////////////////////////
// Access to data members
////////////////////////////////////////
	// 3P
	// Access to the STI object
	public SoarToolJavaInterface	GetSoarToolJavaInterface()	{ return soarToolJavaInterface; }
	
// Dialogs
	AboutDialog aboutDialog = new AboutDialog(this);
	public NameDialog nameDialog = new NameDialog(this);
	
//Actions
	Action newProjectAction = new NewProjectAction();
	Action openProjectAction = new OpenProjectAction();
    Action openFileAction = new OpenFileAction();
	PerformableAction closeProjectAction = new CloseProjectAction();
	PerformableAction saveAllFilesAction = new SaveAllFilesAction();
	PerformableAction exportAgentAction = new ExportAgentAction();
	PerformableAction saveDataMapAndProjectAction = new SaveDataMapAndProjectAction();
	Action preferencesAction = new PreferencesAction();
	PerformableAction commitAction = new CommitAction();
	Action exitAction = new ExitAction();
    Action cascadeAction = new CascadeAction();
    Action tileWindowsAction = new TileWindowsAction();
    Action reTileWindowsAction = new ReTileWindowsAction();
    Action sendProductionsAction = new SendProductionsAction();
	Action checkSyntaxErrorsAction = new CheckSyntaxErrorsAction();
	PerformableAction checkAllProductionsAction = new CheckAllProductionsAction();
    Action searchDataMapCreateAction = new SearchDataMapCreateAction();
    Action searchDataMapTestAction = new SearchDataMapTestAction();
    Action searchDataMapCreateNoTestAction = new SearchDataMapCreateNoTestAction();
    Action searchDataMapTestNoCreateAction = new SearchDataMapTestNoCreateAction();
    Action searchDataMapNoTestNoCreateAction = new SearchDataMapNoTestNoCreateAction();

    Action generateDataMapAction = new GenerateDataMapAction();
	Action saveProjectAsAction = new SaveProjectAsAction();
	Action contactUsAction = new ContactUsAction();
    Action viewKeyBindingsAction = new ViewKeyBindingsAction();
	Action findInProjectAction = new FindInProjectAction();
    Action replaceInProjectAction = new ReplaceInProjectAction();

	// 3P
	// Menu handlers for STI init, term, and "Send Raw Command"
	Action soarRuntimeInitAction = new SoarRuntimeInitAction();
	Action soarRuntimeTermAction = new SoarRuntimeTermAction();
	Action soarRuntimeSendRawCommandAction = new SoarRuntimeSendRawCommandAction();

////////////////////////////////////////
// Constructors
////////////////////////////////////////


    /**
     * Private constructor not used
     */
	private MainFrame() {}
	/**
	 * Constructs the operatorWindow, the DesktopPane, the SplitPane, the menubar and the file chooser
	 * 3P Also initializes the STI library
	 * @param s the name of the window
	 */
	public MainFrame(String s) 
    {

		// Set the Title of the window
		super(s);
		File	templateFolder = Preferences.getInstance().getTemplateFolder();

        // Use Java toolkit to access user's screen size and set VisualSoar window to 90% of that size
        Toolkit tk = Toolkit.getDefaultToolkit();
        Dimension d = tk.getScreenSize();
		setSize( ((int) (d.getWidth() * .9)), ((int) (d.getHeight() * .9)) );

		setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);

		Container contentPane = getContentPane();
		operatorDesktopSplit.setRightComponent(DesktopPane);
		operatorDesktopSplit.setOneTouchExpandable(true);

		JScrollPane sp = new JScrollPane(feedbackList);
		sp.setBorder(new TitledBorder("Feedback"));

        feedbackDesktopSplit.setTopComponent(operatorDesktopSplit);
        feedbackDesktopSplit.setBottomComponent(sp);
        feedbackDesktopSplit.setOneTouchExpandable(true);
        feedbackDesktopSplit.setDividerLocation( ((int) (d.getHeight() * .65)) );

        contentPane.add(feedbackDesktopSplit, BorderLayout.CENTER);

		setJMenuBar(createMainMenu());
		addWindowListener(
            new WindowAdapter() 
            {
                public void windowClosing(WindowEvent e) 
                {
                    checkForUnsavedProjectOnClose();
                    
                    exitAction.actionPerformed(
                        new ActionEvent(e.getSource(),e.getID(),"Exit"));
                }
            });//addWindowListener()
        
		if(templateFolder != null)
        d_templateManager.load(templateFolder);

		// 3P Initialize the STI library
		SoarRuntimeInit();
	}

////////////////////////////////////////
// Methods
////////////////////////////////////////

	/**
     * This method scans open windows on the VisualSoar desktop for unsaved
     * changes.  It returns true if any are found.
     */
    public boolean isModified()
    {
        CustomInternalFrame[] frames = DesktopPane.getAllCustomFrames();
        for(int i = 0; i < frames.length; ++i) 
        {
            if (frames[i].isModified()) return true;
        }

        return false;
    }
    
    
	/**
     * Method updates the FeedBack list window
     * @param v the vector list of feedback data
     */
	public void setFeedbackListData(Vector v) 
    {

		feedbackList.setListData(v);
	}

	/**
     * Gets the project TemplateManager
     * @return a <code>TemplateManager</code> in charge of all template matters.
     */
	public TemplateManager getTemplateManager() 
    {

		return d_templateManager;
	}

	/**
     * Gets the project OperatorWindow
     * @return the project's only <code>OperatorWindow</code>.
     */
	public OperatorWindow getOperatorWindow() 
    {

		return operatorWindow;
	}

    /**
     * Gets the project CustomDesktopPane
     * @see CustomDesktopPane
     * @return the project's only <code>CustomDesktopPane</code>.
     */
    public CustomDesktopPane getDesktopPane() 
    {

        return DesktopPane;
    }

	/**
	 * A helper function to create the file menu
	 * @return The file menu
	 */
	private JMenu createFileMenu() 
    {

		JMenu fileMenu = new JMenu("File");
		// The File Menu
		JMenuItem newProjectItem = new JMenuItem("New Project...");
		newProjectItem.addActionListener(newProjectAction);
		newProjectAction.addPropertyChangeListener(
            new ActionButtonAssociation(newProjectAction,newProjectItem));
			
		JMenuItem openProjectItem = new JMenuItem("Open Project...");
		openProjectItem.addActionListener(openProjectAction);
		openProjectAction.addPropertyChangeListener(
            new ActionButtonAssociation(openProjectAction,openProjectItem));

        JMenuItem openFileItem = new JMenuItem("Open File...");
        openFileItem.addActionListener(openFileAction);
        openFileAction.addPropertyChangeListener(
            new ActionButtonAssociation(openFileAction, openFileItem));

		JMenuItem closeProjectItem = new JMenuItem("Close Project");
		closeProjectItem.addActionListener(closeProjectAction);
		closeProjectAction.addPropertyChangeListener(
            new ActionButtonAssociation(closeProjectAction,closeProjectItem));
				
		JMenuItem commitItem = new JMenuItem("Save");
		commitItem.addActionListener(commitAction);
        commitItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_S,Event.CTRL_MASK));
		commitAction.addPropertyChangeListener(
            new ActionButtonAssociation(commitAction,commitItem));
				
		JMenuItem exitItem = new JMenuItem("Exit");
		exitItem.addActionListener(exitAction);
		exitAction.addPropertyChangeListener(
            new ActionButtonAssociation(exitAction,exitItem));
		
		JMenuItem saveProjectAsItem = new JMenuItem("Save Project As...");
		saveProjectAsItem.addActionListener(saveProjectAsAction);
		saveProjectAsAction.addPropertyChangeListener(
            new ActionButtonAssociation(saveProjectAsAction,saveProjectAsItem));
		
		fileMenu.add(newProjectItem);
		fileMenu.add(openProjectItem);
        fileMenu.add(openFileItem);
		fileMenu.add(closeProjectItem);
		
		fileMenu.addSeparator();
		
		fileMenu.add(commitItem);
		fileMenu.add(saveProjectAsItem);

		fileMenu.addSeparator();

		fileMenu.add(exitItem);
		
		// register mnemonics and accelerators
		
		fileMenu.setMnemonic('F');
		newProjectItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_N,Event.CTRL_MASK));
		newProjectItem.setMnemonic(KeyEvent.VK_N);
		openProjectItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_O,Event.CTRL_MASK));
		openProjectItem.setMnemonic(KeyEvent.VK_O);
        openFileItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_F,Event.CTRL_MASK));
        openFileItem.setMnemonic(KeyEvent.VK_F);

		exitItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_X,Event.ALT_MASK));
		exitItem.setMnemonic(KeyEvent.VK_X);
		
		return fileMenu;
	}
	
	/**
	 * A helper function to create the edit menu
	 * @return The edit menu
	 */
	private JMenu createEditMenu() 
    {

		JMenu editMenu = new JMenu("Edit");
		
		JMenuItem preferencesItem = new JMenuItem("Preferences...");
		preferencesItem.addActionListener(preferencesAction);
		editMenu.add(preferencesItem);

		return editMenu;
	}
	
	/**
	 * A helper function to create the search menu
	 * @return The search menu
	 */
	private JMenu createSearchMenu() 
    {

		JMenu searchMenu = new JMenu("Search");

		JMenuItem findInProjectItem = new JMenuItem("Find in Project");
		findInProjectItem.addActionListener(findInProjectAction);
		findInProjectAction.addPropertyChangeListener(
            new ActionButtonAssociation(findInProjectAction,findInProjectItem));

        JMenuItem replaceInProjectItem = new JMenuItem("Replace in Project");
        replaceInProjectItem.addActionListener(replaceInProjectAction);
        replaceInProjectAction.addPropertyChangeListener(
            new ActionButtonAssociation(replaceInProjectAction, replaceInProjectItem));

		searchMenu.add(findInProjectAction);
        searchMenu.add(replaceInProjectAction);
		searchMenu.setMnemonic(KeyEvent.VK_A);


		return searchMenu;
	}

	/**
	 * A helper function to create the Datamap menu
	 * @return The datamap menu
	 */
	private JMenu createDatamapMenu() 
    {

		JMenu datamapMenu = new JMenu("Datamap");

		JMenuItem checkAllProductionsItem = new JMenuItem("Check All Productions Against the Datamap");
		checkAllProductionsItem.addActionListener(checkAllProductionsAction);
		checkAllProductionsAction.addPropertyChangeListener(
            new ActionButtonAssociation(checkAllProductionsAction,checkAllProductionsItem));

		JMenuItem checkSyntaxErrorsItem = new JMenuItem("Check All Productions for Syntax Errors");
		checkSyntaxErrorsItem.addActionListener(checkSyntaxErrorsAction);
		checkSyntaxErrorsAction.addPropertyChangeListener(
            new ActionButtonAssociation(checkSyntaxErrorsAction,checkSyntaxErrorsItem));

		JMenuItem searchDataMapTestItem = new JMenuItem("Search the Datamap for WMEs that are Never Tested");
		searchDataMapTestItem.addActionListener(searchDataMapTestAction);
		searchDataMapTestAction.addPropertyChangeListener(
            new ActionButtonAssociation(searchDataMapTestAction,searchDataMapTestItem));

		JMenuItem searchDataMapCreateItem = new JMenuItem("Search the Datamap for WMEs that are Never Created");
		searchDataMapCreateItem.addActionListener(searchDataMapCreateAction);
		searchDataMapCreateAction.addPropertyChangeListener(
            new ActionButtonAssociation(searchDataMapCreateAction,searchDataMapCreateItem));

		JMenuItem searchDataMapTestNoCreateItem = new JMenuItem("Search the Datamap for WMEs that are Tested but Never Created");
		searchDataMapTestNoCreateItem.addActionListener(searchDataMapTestNoCreateAction);
		searchDataMapTestNoCreateAction.addPropertyChangeListener(
            new ActionButtonAssociation(searchDataMapTestNoCreateAction,searchDataMapTestNoCreateItem));

		JMenuItem searchDataMapCreateNoTestItem = new JMenuItem("Search the Datamap for WMEs that are Created but Never Tested");
		searchDataMapCreateNoTestItem.addActionListener(searchDataMapCreateNoTestAction);
		searchDataMapCreateNoTestAction.addPropertyChangeListener(
            new ActionButtonAssociation(searchDataMapCreateNoTestAction,searchDataMapCreateNoTestItem));

        JMenuItem searchDataMapNoTestNoCreateItem = new JMenuItem("Search the Datamap for WMEs that are Never Tested and Never Created");
		searchDataMapNoTestNoCreateItem.addActionListener(searchDataMapNoTestNoCreateAction);
		searchDataMapNoTestNoCreateAction.addPropertyChangeListener(
            new ActionButtonAssociation(searchDataMapNoTestNoCreateAction,searchDataMapNoTestNoCreateItem));

		JMenuItem generateDataMapItem = new JMenuItem("Generate the Datamap from the Current Operator Hierarchy");
        generateDataMapItem.addActionListener(generateDataMapAction);
        generateDataMapAction.addPropertyChangeListener(
            new ActionButtonAssociation(generateDataMapAction, generateDataMapItem));

		datamapMenu.add(checkAllProductionsItem);
		datamapMenu.add(checkSyntaxErrorsItem);
		datamapMenu.addSeparator();
        datamapMenu.add(generateDataMapItem);
		datamapMenu.addSeparator();
        datamapMenu.add(searchDataMapTestItem);
        datamapMenu.add(searchDataMapCreateItem);
        datamapMenu.add(searchDataMapTestNoCreateItem);
        datamapMenu.add(searchDataMapCreateNoTestItem);
        datamapMenu.add(searchDataMapNoTestNoCreateItem);
        datamapMenu.setMnemonic(KeyEvent.VK_D);

        return datamapMenu;
    }
	
	/**
	 * A helper function to create the view menu
	 * @return The view menu
	 */
	private JMenu createViewMenu() 
    {

		final JMenu viewMenu = new JMenu("View");
		// View Menu
		JMenuItem cascadeItem = new JMenuItem("Cascade Windows");
		cascadeItem.addActionListener(cascadeAction);
		cascadeItem.addPropertyChangeListener(
            new ActionButtonAssociation(cascadeAction,cascadeItem));
		viewMenu.add(cascadeItem);
		
		JMenuItem tileWindowItem = new JMenuItem("Tile Windows");
		tileWindowItem.addActionListener(tileWindowsAction);
		tileWindowItem.addPropertyChangeListener(
            new ActionButtonAssociation(tileWindowsAction,tileWindowItem));
		viewMenu.add(tileWindowItem);
		
		JMenuItem reTileWindowItem = new JMenuItem("Re-Tile Windows");
		reTileWindowItem.addActionListener(reTileWindowsAction);
		reTileWindowItem.addPropertyChangeListener(
            new ActionButtonAssociation(reTileWindowsAction,reTileWindowItem));
		viewMenu.add(reTileWindowItem);
		viewMenu.addSeparator();
				
		viewMenu.addMenuListener(
            new MenuListener() 
            {

                public void menuCanceled(MenuEvent e) {} 
                public void menuDeselected(MenuEvent e) 
                    {

                        for(int i = viewMenu.getMenuComponentCount() - 1; i > 2; --i) 
                        {

                            viewMenu.remove(i);
                        }
                    }
          	 	
                public void menuSelected(MenuEvent e) 
                    {

                        JInternalFrame[] frames = DesktopPane.getAllFrames();
                        for(int i = 0; i < frames.length; ++i) 
                        {

                            JMenuItem menuItem = new JMenuItem(frames[i].getTitle());
                            menuItem.addActionListener(new WindowMenuListener(frames[i]));
                            viewMenu.add(menuItem);
                        }
                    }   	 		
            });		
		viewMenu.setMnemonic('V');
		//cascadeWindowItem.setMnemonic(KeyEvent.VK_C);
		tileWindowItem.setMnemonic(KeyEvent.VK_T);
		tileWindowItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_T,Event.CTRL_MASK));
		reTileWindowItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_T,Event.CTRL_MASK | Event.SHIFT_MASK));
		return viewMenu;
	}

    /**
     * This function is called when the project is being closed (i.e.,
     * due to window close, opening new project or selecting "Close Project"
     * from the File menu).  It detects unsaved changes and asks the user
     * what to do about it.
     */
    void checkForUnsavedProjectOnClose()
    {
        //If the user has modifed files that have not been saved,
        //we need to ask the user what to do about it.
        if (isModified())
        {
            String[] buttons = { "Save all files, then exit.",
                                 "Exit without Saving" };

            String selectedValue = (String)
                JOptionPane.showInputDialog(
                    getMainFrame(),
                    "This project has unsaved files.  What should I do?",
                    "Abandon Project",
                    JOptionPane.QUESTION_MESSAGE,
                    null,
                    buttons,
                    buttons[0]);

            //If user hits Cancel button
            if (selectedValue == null)
            {
                return;
            }

            //If user selects Save all files, then exit
            if (selectedValue.equals(buttons[0]))
            {
                saveAllFilesAction.perform();
            }
                        
            //If user selects Exit without Saving
            if (selectedValue.equals(buttons[1]))
            {
                //Make all files as unchanged
                CustomInternalFrame[] frames = DesktopPane.getAllCustomFrames();
                for(int i = 0; i < frames.length; ++i) 
                {
                    frames[i].setModified(false);
                }
                            
            }
        }//if
                    
    }//checkForUnsavedProjectOnClose()


    /**
     * This function opens a file (assumed to be a non-Soar file).
     */
    void OpenFile(File file)
    {
        try
        {
            boolean oldPref = Preferences.getInstance().isHighlightingEnabled();
            Preferences.getInstance().setHighlightingEnabled(false);  // Turn off highlighting
            RuleEditor ruleEditor = new RuleEditor(file);
            ruleEditor.setVisible(true);
            addRuleEditor(ruleEditor);
            ruleEditor.setSelected(true);
            Preferences.getInstance().setHighlightingEnabled(oldPref);  // Turn it back to what it was
        }
        catch(IOException IOE) 
        {

            JOptionPane.showMessageDialog(MainFrame.this, "There was an error reading file: " +
                                          file.getName(), "I/O Error", JOptionPane.ERROR_MESSAGE);
        }
        catch(java.beans.PropertyVetoException pve)
        {
            //No sweat. This just means the new window failed to get focus.
        }
    }//OpenFile()

    /**
     * When the Soar Runtime|Agent menu is selected, this listener
     * populates the menu with the agents that are currently
     * connected to via the STI.
     * @Author ThreePenny
     */
	class SoarRuntimeAgentMenuListener extends MenuAdapter
	
    {

		public void menuSelected(MenuEvent e)
		
        {

			// Remove our existing items
			soarRuntimeAgentMenu.removeAll();
			
			// Check to see if we have an STI connection
			SoarToolJavaInterface sti=GetSoarToolJavaInterface();
			if (sti == null)
			
            {

				// Add a "not connected" menu item
				JMenuItem menuItem=new JMenuItem("<not connected>");
				menuItem.setEnabled(false);
				
				soarRuntimeAgentMenu.add(menuItem);
				return;
			}
			
			// Get the connection names
			String[] connectionNames=sti.GetConnectionNames();
			
			// If we don't have any connections then display the
			// appropriate menu item.
			if (connectionNames == null || connectionNames.length == 0)
			
            {

				// Add the "no agents" menu item
				JMenuItem menuItem=new JMenuItem("<no agents>");
				menuItem.setEnabled(false);
				
				soarRuntimeAgentMenu.add(menuItem);
				return;
			}
			
			// Add each name
			int i;
			for (i=0; i < connectionNames.length; i++)
			
            {

				// Get this connection name
				String sConnectionName=connectionNames[i];
				
				// Create the connection menu and add a listener to it
				// which contains the connection index.
				JCheckBoxMenuItem connectionMenuItem=new JCheckBoxMenuItem(sConnectionName);
				connectionMenuItem.addActionListener(
                    new AgentConnectionActionListener(connectionMenuItem, sConnectionName));
				
				// Set the state based on whether or not we are connected to this agent
				boolean bConnected=sti.IsConnectionEnabledByName(sConnectionName);
				connectionMenuItem.setState(bConnected);

				// Add the menu item
				soarRuntimeAgentMenu.add(connectionMenuItem);	
			}
		}
	}


    /**
     * Listener activated when the user clicks on an Agent in the Soar
     * Runtime |Agents menu.  When the user clicks on an agent in the menu,
     * it is activated/deactivated.
     * @Author ThreePenny
     */
	class AgentConnectionActionListener implements ActionListener
	
    {

		// Index of this agent connection in the menu
		private String				m_sAgentConnectionName;
		
		// Menu item that this action is for
		// TODO: Is there a way to just retrieve this without storing it?
		private JCheckBoxMenuItem	m_assocatedMenuItem;
		
		// Constructor
		public AgentConnectionActionListener(JCheckBoxMenuItem assocatedMenuItem, String sAgentConnectionName)
		
        {

			m_sAgentConnectionName = sAgentConnectionName;
			m_assocatedMenuItem = assocatedMenuItem;
		}
		
		// Disable the default constructor
		private AgentConnectionActionListener() {}
		
		// Called when the action has been performed
		public void actionPerformed(ActionEvent e)
		
        {

			// Check to see if we have an STI connection
			SoarToolJavaInterface sti=GetSoarToolJavaInterface();
			if (sti == null)
			
            {

				// TODO
				// Assert or throw some kind of exception?
				return;
			}
			
			// Set the connection state
			sti.EnableConnectionByName(m_sAgentConnectionName, m_assocatedMenuItem.getState() == true);
		}
	}


    /**
     * Creates the "Soar Runtime" menu which appears in the MainFrame
     * @Author ThreePenny
     * @return a <code>soarRuntimeMenu</code> JMenu
     */
	private JMenu createSoarRuntimeMenu()
	
    {

		// Add the menu as set the mnemonic
		JMenu soarRuntimeMenu = new JMenu("Soar Runtime");
		soarRuntimeMenu.setMnemonic('S');
		
		// Add the menu items
		JMenuItem connectMenuItem=soarRuntimeMenu.add(soarRuntimeInitAction);
		JMenuItem disconnectMenuItem=soarRuntimeMenu.add(soarRuntimeTermAction);
		JMenuItem sendRawCommandMenuItem=soarRuntimeMenu.add(soarRuntimeSendRawCommandAction);
			
		// Build the "Connected Agents" menu
		soarRuntimeAgentMenu = new JMenu("Connected Agents");
		soarRuntimeAgentMenu.addMenuListener(
            new SoarRuntimeAgentMenuListener());
		soarRuntimeAgentMenu.setEnabled(false);

		// Add the "Connected Agents" menu
		soarRuntimeMenu.add(soarRuntimeAgentMenu);

		// Set the mnemonics
		connectMenuItem.setMnemonic('C');
		disconnectMenuItem.setMnemonic('D');
		sendRawCommandMenuItem.setMnemonic('R');
		soarRuntimeAgentMenu.setMnemonic('A');

		return soarRuntimeMenu;
	}

	/**
	 * A helper function to create the help menu
	 * @return The help menu
	 */
	private JMenu createHelpMenu() 
    {

		JMenu helpMenu = new JMenu("Help");
		// Help menu
		helpMenu.add(contactUsAction); 
        helpMenu.add(viewKeyBindingsAction);
		helpMenu.setMnemonic('H');
		return helpMenu;
	}
	
	/**
	 * A Helper function that creates the main menu bar
	 * @return The MenuBar just created
	 */
	private JMenuBar createMainMenu() 
    {

		JMenuBar MenuBar = new JMenuBar();
		
		// The Main Menu Bar
		MenuBar.add(createFileMenu());
		MenuBar.add(createEditMenu());
		MenuBar.add(createSearchMenu());
        MenuBar.add(createDatamapMenu());
		MenuBar.add(createViewMenu());
		
		// 3P
		// Add the Soar Runtime menu
		MenuBar.add(createSoarRuntimeMenu());

		MenuBar.add(createHelpMenu());
		
		return MenuBar;
	}
	
	/**
	 * Creates a rule window opening with the given file name
	 * @param re the ruleeditor file that the rule editor should open
	 */
	public void addRuleEditor(RuleEditor re) 
    {

		DesktopPane.add(re);
		re.moveToFront();
		if (Preferences.getInstance().isAutoTilingEnabled()) 
        {

			DesktopPane.performTileAction();
		}
		else 
        {

			re.reshape(0, 0 , 300, 200);
		}
		DesktopPane.revalidate();
	}

	/**
	 * Creates a datamap window with the given datamap.
	 * @param dm the datamap that the window should open
     * @see DataMap
	 */
	public void addDataMap(DataMap dm) 
    {

		DesktopPane.add(dm);
		dm.moveToFront();
		DesktopPane.revalidate();
		if (Preferences.getInstance().isAutoTilingEnabled()) 
        {

			DesktopPane.performTileAction();
		}
		else 
        {

			dm.reshape(0, 0, 200, 300);
		}
	}

	/**
     * Makes the specified rule editor window the selected window
     * and brings the window to the front of the frame.
     * @param re the specified rule editor window
     */
	public void showRuleEditor(RuleEditor re) 
    {
		try 
        {
			if (re.isIcon())
            re.setIcon(false);
			re.setSelected(true);
			re.moveToFront();
		}
		catch (java.beans.PropertyVetoException pve)
        {
            System.err.println("Guess we can't do that");
        }				
	}
						
	/**
     * Selects a currently open editor window and brings it to the front.
     * If none are open then no action is taken.
     */
    public void selectNewInternalFrame()
    {
        JInternalFrame[] jif = DesktopPane.getAllFrames();
        RuleEditor re = null;
        for(int i = jif.length-1; i >= 0; i--)
        {
            if(jif[i].isShowing()) 
            {
                try 
                {
                    if (jif[i].isIcon())
                    {
                        jif[i].setIcon(false);
                    }
                    jif[i].setSelected(true);
                    jif[i].moveToFront();
                    break;
                }
                catch (java.beans.PropertyVetoException pve)
                {
                    //Don't break;
                }				
            }
        }//for

	}//selectNewInternalFrame()
						

	/**
	 * Gets rid of the operator window
	 */
    public void removeOperatorWindow() 
    {

	 	operatorDesktopSplit.setLeftComponent(null);
    }


	/**
	 * This class is used to bring an internal frame to the front
	 * if it is selected from the view menu
	 */
	class WindowMenuListener implements ActionListener
    {

		JInternalFrame internalFrame;
		
		public WindowMenuListener(JInternalFrame jif) 
        {

			internalFrame = jif;
		}

		private WindowMenuListener() {}
		
		public void actionPerformed(ActionEvent e) 
        {

			internalFrame.toFront();
			try 
            {

				internalFrame.setIcon(false);
				internalFrame.setSelected(true);
				internalFrame.moveToFront();
			} catch (java.beans.PropertyVetoException pve) { 
				System.err.println("Guess we can't do that"); }
		}
	}

	
  	/**
  	 * Sets the main window
  	 */
  	public static void setMainFrame(MainFrame mainFrame) 
    {

  		s_mainFrame = mainFrame;
  	}
  	
  	/**
  	 * Gets the main window
  	 */
  	public static MainFrame getMainFrame() 
    {

  		return s_mainFrame;
  	}
  	/**
  	 * enables the corresponding actions for when a project is opened
  	 */
  	private void projectActionsEnable(boolean areEnabled) 
    {

  		// Enable various actions
		saveAllFilesAction.setEnabled(areEnabled);
		checkAllProductionsAction.setEnabled(areEnabled);
		checkSyntaxErrorsAction.setEnabled(areEnabled);
        searchDataMapTestAction.setEnabled(areEnabled);
        searchDataMapCreateAction.setEnabled(areEnabled);
        searchDataMapTestNoCreateAction.setEnabled(areEnabled);
        searchDataMapCreateNoTestAction.setEnabled(areEnabled);
        searchDataMapNoTestNoCreateAction.setEnabled(areEnabled);    
        generateDataMapAction.setEnabled(areEnabled);
		//checkCoverageAction.setEnabled(true);
		closeProjectAction.setEnabled(areEnabled);
		findInProjectAction.setEnabled(areEnabled);
        replaceInProjectAction.setEnabled(areEnabled);
		commitAction.setEnabled(areEnabled);
		saveProjectAsAction.setEnabled(areEnabled);
  	}
  	  	
	
/*########################################################################################
  Actions
  ########################################################################################/*
	
/**
* Runs through all the Rule Editors in the Desktop Pane and tells them to save
* themselves.
*/
	class SaveAllFilesAction extends PerformableAction 
    {

		public SaveAllFilesAction() 
        {

			super("Save All");
			setEnabled(false);
		}
		
		public void perform() 
        {

			try 
            {

				JInternalFrame[] jif = DesktopPane.getAllFrames();
				for(int i = 0; i < jif.length; ++i) 
                {

					if(jif[i] instanceof RuleEditor) 
                    {

						RuleEditor re = (RuleEditor)jif[i];
						re.write();
					}
				}
			}
			catch(java.io.IOException ioe) 
            {

				JOptionPane.showMessageDialog(MainFrame.this, "Error Writing File", "IO Error", JOptionPane.ERROR_MESSAGE);
			}
		}

		public void actionPerformed(ActionEvent event) 
        {

			perform();
		}
		
	}
	
	/**
	 * Exit command
	 * First closes all the RuleEditor windows
	 * if all the closes go successfully, then it closes
	 * the operator hierarchy then exits
	 */
	class ExitAction extends AbstractAction 
    {

		public ExitAction() 
        {

			super("Exit");
		}
		public void actionPerformed(ActionEvent event) 
        {

			JInternalFrame[] frames = DesktopPane.getAllFrames();
			prefs.write();
			try 
            {

				for(int i = 0; i < frames.length; ++i) 
                {

					frames[i].setClosed(true);
				}
				
				// 3P
				// Close down the STI library
				SoarRuntimeTerm();
								
				dispose();
				commitAction.perform();
				System.exit(0);
			}
			catch (java.beans.PropertyVetoException pve) {}
		}
	}

	/**
     * Attempts to save the datamap
     * @see OperatorWindow.saveHierarchy()
     */
	class SaveDataMapAndProjectAction extends PerformableAction 
    {

		public SaveDataMapAndProjectAction() 
        {

			super("Save DataMap And Project Action");
		}
		
		public void perform() 
        {

			if(operatorWindow != null) 
            {

				operatorWindow.saveHierarchy();
			}
		}
		
		public void actionPerformed(ActionEvent event) 
        {

			perform();	
			Vector v = new Vector();
			v.add("DataMap and Project Saved");
			setFeedbackListData(v);
		}
	}

    /**
     * Attempts to open a new project by creating a new OperatorWindow
     * @param file .vsa project file that is to be opened
     * @see OperatorWindow
     */
	public void tryOpenProject (File file) throws FileNotFoundException, IOException
    {

		operatorWindow = new OperatorWindow(file);

		operatorDesktopSplit.setLeftComponent(operatorWindow);
		
		projectActionsEnable(true);

		operatorDesktopSplit.setDividerLocation(.30);
	}
	
	/**
	 * Open Project Action
     * a filechooser is created to determine project file
     * Opens a project by creating a new OperatorWindow
     * @see OperatorWindow
     * @see SoarFileFilter
	 */
	class OpenProjectAction extends AbstractAction 
    {

		public OpenProjectAction() 
        {

			super("Open Project...");
		}	
		public void actionPerformed(ActionEvent event) 
        {

			try 
            {

				JFileChooser fileChooser = new JFileChooser();
				fileChooser.setFileFilter(new SoarFileFilter());
				fileChooser.setCurrentDirectory(Preferences.getInstance().getOpenFolder());
				int state = fileChooser.showOpenDialog(MainFrame.this);
				File file = fileChooser.getSelectedFile();
				if (file != null && state == JFileChooser.APPROVE_OPTION) 
                {

                    //Get rid of the old project (if it exists)
                    if (operatorWindow != null)
                    {
                        MainFrame.getMainFrame().closeProjectAction.perform();
                    }
                    
                    //Open the new project
					operatorWindow = new OperatorWindow(file);
					if(file.getParent() != null)
                    Preferences.getInstance().setOpenFolder(file.getParentFile());
					operatorDesktopSplit.setLeftComponent(new JScrollPane(operatorWindow));
					
					projectActionsEnable(true);

                    //%%%This value should be in preferences
					operatorDesktopSplit.setDividerLocation(.30);

                    //Set the title bar to include the project name
                    setTitle(file.getName().replaceAll(".vsa", ""));
				}
			}
			
			catch(FileNotFoundException fnfe) 
            {

				JOptionPane.showMessageDialog(MainFrame.this, "File Not Found!", "File Not Found", JOptionPane.ERROR_MESSAGE);
			}
			catch(IOException ioe) 
            {

				JOptionPane.showMessageDialog(MainFrame.this, "Error Reading File", "IOException", JOptionPane.ERROR_MESSAGE);
				ioe.printStackTrace();
			}
			catch(NumberFormatException nfe) 
            {

                nfe.printStackTrace();
                JOptionPane.showMessageDialog(MainFrame.this, "Error Reading File, Data Incorrectly Formatted", "Bad File", JOptionPane.ERROR_MESSAGE);
			}
		}
	}

    /**
     * Open a text file unrelated to the project in a rule editor
     * Opened file is not necessarily part of project and not soar formatted
     */
    class OpenFileAction extends AbstractAction 
    {

        public OpenFileAction() 
        {

            super("Open File...");
        }
        public void actionPerformed(ActionEvent event) 
        {

            try 
            {

                JFileChooser fileChooser = new JFileChooser();
                fileChooser.setFileFilter(new TextFileFilter());
                fileChooser.setCurrentDirectory(Preferences.getInstance().getOpenFolder());
                int state = fileChooser.showOpenDialog(MainFrame.this);
                File file = fileChooser.getSelectedFile();
                if(file != null && state == JFileChooser.APPROVE_OPTION) 
                {
                    OpenFile(file);
                }
            }

			catch(NumberFormatException nfe) 
            {

                nfe.printStackTrace();
                JOptionPane.showMessageDialog(MainFrame.this, "Error Reading File, Data Incorrectly Formatted", "Bad File", JOptionPane.ERROR_MESSAGE);
			}

        }
    }


	/**
	 * New Project Action
     * Creates a dialog that gets the new project name and then creates the new 
     * project by creating a new Operator Window.
     * @see NewAgentDialog
     * @see OperatorWindow
	 */
	class NewProjectAction extends AbstractAction 
    {

		public NewProjectAction() 
        {

			super("New Project...");
		}
		
		public void actionPerformed(ActionEvent event) 
        {

			// redo this a dialog should just pass back data to the main window for processing
			NewAgentDialog newAgentDialog = new NewAgentDialog(MainFrame.this);
			newAgentDialog.show();
			if (newAgentDialog.wasApproved()) 
            {

				String agentName = newAgentDialog.getNewAgentName();
				String path = newAgentDialog.getNewAgentPath();
				String agentFileName = path + File.separator + agentName + ".vsa";
				operatorWindow = new OperatorWindow(agentName,agentFileName,true);
				
				Preferences.getInstance().setOpenFolder(new File(path));
				operatorDesktopSplit.setLeftComponent(new JScrollPane(operatorWindow));
				
				projectActionsEnable(true);
                exportAgentAction.perform();
				
				operatorDesktopSplit.setDividerLocation(.30);
			}
		}
	}
	
	/**
	 * Close Project Action
     * Closes all open windows in the desktop pane
	 */
	class CloseProjectAction extends PerformableAction 
    {

		public CloseProjectAction() 
        {

			super("Close Project");
			setEnabled(false);
		}
		
		public void perform()
        {
            checkForUnsavedProjectOnClose();

			JInternalFrame[] frames = DesktopPane.getAllFrames();
			try 
            {

				for(int i = 0; i < frames.length; ++i) 
                {

					frames[i].setClosed(true);
				}
				commitAction.perform();
				operatorDesktopSplit.setLeftComponent(null);

				projectActionsEnable(false);
			}
			catch (java.beans.PropertyVetoException pve) {}

            setTitle("VisualSoar");
		}//perform()

        
        public void actionPerformed(ActionEvent event) 
        {
            perform();
        }

	}
	
	/**
	 * Export Agent
	 * Writes all the <operator>_source.soar files necesary for sourcing agent
	 * files written in  into the TSI
	 */
	class ExportAgentAction extends PerformableAction 
    {

		public ExportAgentAction() 
        {

			super("Export Agent");
		}
		
		public void perform() 
        {

			DefaultTreeModel tree = (DefaultTreeModel)operatorWindow.getModel();
			OperatorRootNode root = (OperatorRootNode)tree.getRoot();
			try 
            {

				root.startSourcing();
			}
			catch (IOException exception) 
            {

				JOptionPane.showMessageDialog(MainFrame.this, "IO Error exporting agent.", "Agent Export Error", JOptionPane.ERROR_MESSAGE);
				return;
			}
		}
		
		public void actionPerformed(ActionEvent event)
        {
			perform();
			Vector v = new Vector();
			v.add("Export Finished");
			setFeedbackListData(v);
		}
	}
	
	/**
	 * Creates and shows the preferences dialog
	 */
	class PreferencesAction extends AbstractAction 
    {

		public PreferencesAction() 
        {

			super("Preferences Action");
		}
		
		public void actionPerformed(ActionEvent e) 
        {

			PreferencesDialog	theDialog = new PreferencesDialog(MainFrame.getMainFrame());
			theDialog.setVisible(true);
		
			//Update all open source code files
            //COMMENTED OUT: this takes too long
//  			if (theDialog.wasApproved()) 
//              {
//                  // make the change realized...
//                  JInternalFrame[] theFrames = DesktopPane.getAllFrames();

//                  for (int i = 0; i < theFrames.length; i++) 
//                  {
//                      if (theFrames[i] instanceof RuleEditor) 
//                      {
//                          ((RuleEditor)theFrames[i]).recolorSyntax();
//                      }
//                  }
//  			}//if
		}//actionPerformed()
	}
	
	/**
	 * This is where the user user wants some info about the authors
     * @see AboutDialog
	 */	
	class ContactUsAction extends AbstractAction 
    {

		public ContactUsAction() 
        {

			super("About VisualSoar");
		}
		
		public void actionPerformed(ActionEvent e) 
        {

			aboutDialog.setVisible(true);		
		}
  	}

	/**
	 * This is where the user user wants a list of keybindings.  The action
     * loads the docs/KeyBindings.txt file.
	 */	
	class ViewKeyBindingsAction extends AbstractAction 
    {

		public ViewKeyBindingsAction() 
        {

			super("VisualSoar Keybindings");
		}
		
		public void actionPerformed(ActionEvent e) 
        {
            File f = new File("./docs/KeyBindings.txt");
            if (f.exists())
            {
                OpenFile(f);
            }
            else
            {
				JOptionPane.showMessageDialog(
                    MainFrame.this,
                    "The keybindings documentation file appears to be missing.\n"
                    + "I am expecting to find a file called \"KeyBindings.txt\" in \n"
                    + "the \"docs\" subdirectory of the VisualSoar directory.",
                    "Could Not Find VisualSoar Keybindings",
                    JOptionPane.ERROR_MESSAGE);
            }
		}
  	}//class ViewKeyBindingsAction


    /**
     * Message timer listener which is used to pump the STI messages
     * in the background.  Incoming commands are received here and
     * processeed through the ProcessSoarToolJavaCommand method.
     * @author ThreePenny
     */
	class SoarRuntimePumpMessagesTimerListener implements ActionListener
	
    {

		public void actionPerformed(ActionEvent evt)
		
        {

			// Return if we don't have a tools interface
			if (soarToolJavaInterface == null)
			
            {

				return;
			}
			
			// Pump our messages
			soarToolJavaInterface.PumpMessages(true /* bProcessAllPendingMessages */);
			
			// Process all of our commands
			while (soarToolJavaInterface.IsIncomingCommandAvailable() == true)
			
            {

				// Get our command object
				SoarToolJavaCommand commandObject=new SoarToolJavaCommand();
				soarToolJavaInterface.GetIncomingCommand(commandObject);
	
				// Process the command
				ProcessSoarToolJavaCommand(commandObject);
						
				// Pop the command
				soarToolJavaInterface.PopIncomingCommand();
			}
		}
	}


    /**
     * Processes incoming STI commands from the runtime
     * @author ThreePenny
     */
	protected void ProcessSoarToolJavaCommand(SoarToolJavaCommand command)
	
    {

		switch (command.GetCommandID())
		
        {

            // Edit Production command
            case 1001:
                // BUGBUG: JDK complains that STI_kEditProduction needs to be a constant
                // Seems defined as 'final' which I thought was a Java constant.
                //case command.STI_kEditProduction:
			
            {

				// Edit the production
				EditProductionByName(command.GetStringParam());
				break;
			}
		}
	}
	

    /**
     * Edits a production (in the project) based on its name.
     * @author ThreePenny
     */
	protected void EditProductionByName(String sProductionName)
	
    {

		// TODO: Should we match case?
		
		// Find the rule and open it
		getOperatorWindow().findInProjectAndOpenRule(sProductionName, false /* match case */);
		
		// Bring our window to the front
		toFront();
	}
	

    /**
     * Handles Soar Runtime|Connect menu option
     * @author ThreePenny
     */
	class SoarRuntimeInitAction extends AbstractAction
	
    {

		public SoarRuntimeInitAction()
		
        {

			super("Connect");
		}
		
		public void actionPerformed(ActionEvent e)
		
        {

			// Initialize the soar runtime
			if (!SoarRuntimeInit())
            {

				// Unable to connect!
				soarToolJavaInterface=null;
				JOptionPane.showMessageDialog(MainFrame.this, "Unable to connect to the Soar Tool Interface (STI)", "STI Error", JOptionPane.ERROR_MESSAGE);
			}
		}	
	}


    /**
     * Initializes the Soar Tool Interface (STI) object and enabled/disables
     * menu items as needed.
     * @author ThreePenny
     */
	boolean SoarRuntimeInit()
	
    {

		// Stop any "pump messages" timers that are currently running
		if (soarToolPumpMessageTimer != null)
		
        {

			soarToolPumpMessageTimer.stop();
			soarToolPumpMessageTimer=null;
		}
		
		// Term our interface if we have one already
		if (soarToolJavaInterface != null)
		
        {

			soarToolJavaInterface.Term();
			soarToolJavaInterface=null;
		}
		
		// Load the interface object
		try 
        {
            soarToolJavaInterface = new SoarToolJavaInterface();
		}
		catch (java.lang.UnsatisfiedLinkError ule)
        {
			// Disable all related menu items
			soarRuntimeTermAction.setEnabled(false);
			soarRuntimeInitAction.setEnabled(false);
			soarRuntimeSendRawCommandAction.setEnabled(false);
			soarRuntimeAgentMenu.setEnabled(false);
            
            return false;
        }				
        
        //Initialize the interface object
        if (soarToolJavaInterface.Init("VisualSoar", false /* bIsRuntime */) == true)
		{			
			// Create our pump messages timer to be fired every 1000 ms
			soarToolPumpMessageTimer = new javax.swing.Timer(1000, new SoarRuntimePumpMessagesTimerListener());
			soarToolPumpMessageTimer.start();
			
			// Enable/Disable menu items
			soarRuntimeTermAction.setEnabled(true);
			soarRuntimeInitAction.setEnabled(false);
			soarRuntimeSendRawCommandAction.setEnabled(true);
			soarRuntimeAgentMenu.setEnabled(true);
	
			// Success!
			return true;
		}
		else
		
        {

			// Failure
			return false;
		}
	}


    /**
     * Terminates the Soar Tool Interface (STI) object and enabled/disables
     * menu items as needed.
     * @author ThreePenny
     */
	void SoarRuntimeTerm()
	
    {

		// Stop any "pump messages" timers that are currently running
		if (soarToolPumpMessageTimer != null)
		
        {

			soarToolPumpMessageTimer.stop();
			soarToolPumpMessageTimer=null;
		}

		// Term our interface object
		if (soarToolJavaInterface != null)
		
        {

			soarToolJavaInterface.Term();
			soarToolJavaInterface=null;
		}
					
		// Enable/Disable menu items
		soarRuntimeTermAction.setEnabled(false);
		soarRuntimeInitAction.setEnabled(true);
		soarRuntimeSendRawCommandAction.setEnabled(false);
		soarRuntimeAgentMenu.setEnabled(false);
	}


    /**
     * Handles Soar Runtime|Disconnect menu option
     * @author ThreePenny
     */
	class SoarRuntimeTermAction extends AbstractAction
	
    {

		public SoarRuntimeTermAction()
		
        {

			// Set the name and default to being disabled
			super("Disconnect");
			setEnabled(false);
		}
		
		public void actionPerformed(ActionEvent e)
		
        {

			// Terminate the soar runtime
			SoarRuntimeTerm();
		}	
	}
	

    /**
     * Handles Soar Runtime|Send Raw Command menu option
     * @author ThreePenny
     */
	class SoarRuntimeSendRawCommandAction extends AbstractAction 
    {

		public SoarRuntimeSendRawCommandAction()
		
        {

			// Set the name and default to being disabled
			super("Send Raw Command");
			setEnabled(false);
		}
		
		public void actionPerformed(ActionEvent e)
		
        {

			SoarRuntimeSendRawCommandDialog theDialog = new SoarRuntimeSendRawCommandDialog(MainFrame.this, GetSoarToolJavaInterface());
			theDialog.setVisible(true);
		}	
	}
		
	class SendProductionsAction extends AbstractAction 
    {

		public SendProductionsAction() 
        {

			super("Send Productions");
		}
	
		public void actionPerformed(ActionEvent e) 
        {

			try 
            {

			
				String serverName = JOptionPane.showInputDialog(MainFrame.getMainFrame(),"Server Name");
				String serverPort = JOptionPane.showInputDialog(MainFrame.getMainFrame(),"Server Port");
				int port = Integer.parseInt(serverPort);
				java.net.Socket socket = new java.net.Socket(serverName,port);
				//operatorWindow.sendProductions(
                //    new BufferedWriter(
                //        new OutputStreamWriter(socket.getOutputStream())));
			}
			catch(IOException ioe) 
            {

				System.err.println("IOError");
			}
			catch(NumberFormatException nfe) 
            {

				System.err.println("Hey, how about you enter a number?");
			}
		}
	
	}

    /**
     * This is a generic class for scanning a set of entities for errors in a
     * separate thread and providing a progress dialog while you do so.  You
     * must subclass this class to use it.
     */
    abstract class UpdateThread extends Thread
    {
        Runnable update, finish;
        int value, min, max;
		JProgressBar progressBar;
		JDialog progressDialog;
        Vector vecEntities;
        Vector parsedProds, vecErrors = new Vector();
        int entityNum = 0;

        public UpdateThread(Vector v, String title)
        {
            vecEntities = v;
            max = v.size();
			progressBar = new JProgressBar(0, max);
			progressDialog = new JDialog(MainFrame.this, title);
			progressDialog.getContentPane().setLayout(new FlowLayout());
			progressDialog.getContentPane().add(progressBar);
			progressBar.setStringPainted(true);
			progressDialog.setLocationRelativeTo(MainFrame.this);
			progressDialog.pack();
			progressDialog.show();
            progressBar.getMaximum();
            progressBar.getMinimum();
				
            update = new Runnable() 
            {

                public void run() 
                {
                    value = progressBar.getValue() + 1;
                    updateProgressBar(value);
                }
            };
            finish = new Runnable() 
            {

                public void run() 
                {

                    updateProgressBar(min);
                    progressDialog.dispose();
                }
            };
        }

        public void run() 
        {
            checkEntities();
        }

        private void updateProgressBar(int value) 
        {

            progressBar.setValue(value);
        }

        /**
         * Override this function in your subclass.  It scans the given entity
         * for errors and places them in the vecErrors vector.  vecErrors can
         * either contain Strings or FeedbackListObjects
         * @param o object to scan
         * @return true if any errors were found
         */
        abstract public boolean checkEntity(Object o) throws IOException;

        public void checkEntities()
        {
            try 
            {
                boolean anyErrors = false;
                for(int i = 0; i < max; i++)
                {
                    boolean errDetected = checkEntity(vecEntities.elementAt(i));
                    if (errDetected)
                    {
                        anyErrors = true;
                    }
                    updateProgressBar(++entityNum);
                    SwingUtilities.invokeLater(update);
                }

                if(!anyErrors)
                {
                    vecErrors.add("There were no errors detected in this project.");
                }
                setFeedbackListData(vecErrors);
                SwingUtilities.invokeLater(finish);
            }
            catch (IOException ioe) 
            {

                ioe.printStackTrace();
            }
        }//checkEntities()

    }//class UpdateThread


    
 	/**
     * This action searches all productions in the project for syntax
     * errors only.   Operation status is displayed in a progress bar.
     * Results are displayed in the feedback list
     */
	class CheckSyntaxErrorsAction extends AbstractAction 
    {
		public CheckSyntaxErrorsAction() 
        {

			super("Check All Productions for Syntax Errors");
			setEnabled(false);
		}
	
		public void actionPerformed(ActionEvent ae)
        {
			Enumeration bfe = operatorWindow.breadthFirstEnumeration();
            Vector vecNodes = new Vector(10, 50);
			while(bfe.hasMoreElements())
            {
				vecNodes.add(bfe.nextElement());
			}
			(new CheckSyntaxThread(vecNodes, "Checking Productions...")).start();
		}

        class CheckSyntaxThread extends UpdateThread
        {
            public CheckSyntaxThread(Vector v, String title)
            {
                super(v, title);
            }
                
            public boolean checkEntity(Object node) throws IOException
            {
                OperatorNode opNode = (OperatorNode)node;
                
                try
                {
                    opNode.parseProductions();
                }
                catch(ParseException pe)
                {
                    vecErrors.add(opNode.parseParseException(pe));
                    return true;
                }
                catch(TokenMgrError tme) 
                {
                    tme.printStackTrace();
                }

                return false;
            }
        }//class CheckSyntaxThread
	
	}//class CheckSyntaxErrorsAction


    

 	/**
     * This class is responsible for comparing all productions in the project
     * with the project's model of working memory - the datamap.
     * Operation status is displayed in a progress bar.
     * Results are displayed in the feedback list
     */
	class CheckAllProductionsAction extends PerformableAction
    {
		public CheckAllProductionsAction() 
        {
			super("Check All Productions");
			setEnabled(false);
		}

        //Same as actionPerformed() but this function waits for the thread to
        //complete before returning (i.e., it's effectively not threaded)
        public void perform()
        {
            Vector vecNodes = new Vector(10, 50);
			Enumeration bfe = operatorWindow.breadthFirstEnumeration();
			while(bfe.hasMoreElements()) 
            {
				vecNodes.add(bfe.nextElement());
			}

            CheckProductionsThread cpt =
                new CheckProductionsThread(vecNodes,
                                           "Checking Productions...");
            cpt.start();
        }

		public void actionPerformed(ActionEvent ae)
        {
            perform();
		}

        class CheckProductionsThread extends UpdateThread
        {
            public CheckProductionsThread(Vector v, String title)
            {
                super(v, title);
            }
                
            public boolean checkEntity(Object node) throws IOException
            {
                return ((OperatorNode)node).CheckAgainstDatamap(vecErrors);
            }
            
        }

	}//class CheckAllProductionsAction
    

	/**
     * This action provides a framework for searching all datamaps for errors.
     * It is intended to be subclassed.  Operation status is displayed in
     * a progress bar.  Results are displayed in the feedback list
     * Double-clicking on an item in the feedback list should display the rogue
     * node in the datamap.
     */
	abstract class SearchDataMapAction extends AbstractAction
    {
        int numNodes = 0;       // number of operator nodes in the project
        int numChecks = 0;      // number of nodes scanned so far
        
		public SearchDataMapAction() 
        {
			super("Check All Productions");
			setEnabled(false);
		}

		public void actionPerformed(ActionEvent ae) 
        {
            initializeEdges();
            numNodes = 0;
            numChecks = 0;
            
			Enumeration bfe = operatorWindow.breadthFirstEnumeration();
            Vector vecNodes = new Vector(10, 50);
			while(bfe.hasMoreElements())
            {
				vecNodes.add(bfe.nextElement());
                numNodes++;
			}

            //Add the nodes a second time because we'll be scanning them twice,
            //once to check productions against the datamap and again to check
            //the datamap for untested WMEs.  (See checkEntity() below.)
			bfe = operatorWindow.breadthFirstEnumeration();
			while(bfe.hasMoreElements())
            {
				vecNodes.add(bfe.nextElement());
			}
            
			(new DatamapTestThread(vecNodes, "Scanning Datamap...")).start();

		}//actionPerformed()

        /**
             *  This initializes the status of all the edges to zero, which
             *  means that the edges have not been used by a production in any
             *  way.
             */
        public void initializeEdges()
        {

            Enumeration edges = operatorWindow.getDatamap().getEdges();
            while(edges.hasMoreElements()) 
            {

                NamedEdge currentEdge = (NamedEdge)edges.nextElement();
                currentEdge.resetTestedStatus();
                currentEdge.resetErrorNoted();
                // initialize the output-link as already tested
                if(currentEdge.getName().equals("output-link")) 
                {
                    currentEdge.setOutputLinkTested(operatorWindow.getDatamap());
                }
            }
        }// initializeEdges()

        //This function performs the actual error check
        //The datamap associated with the given operator node is scanned and a
        //list of errors is placed in the given Vector.
        abstract public void searchDatamap(OperatorNode opNode, Vector v);
        
		class DatamapTestThread extends UpdateThread 
        {
			public DatamapTestThread(Vector v, String title) 
            {
                super(v, title);
			}

            /**
             *  Search through the datamap and look for extra WMEs by looking at
             *  the status of the named edge (as determined by the check nodes
             *  function) and the edge's location within the datamap.  Extra
             *  WMEs are classified in this action by never being tested by a
             *  production, not including any item within the output-link.
             */
            public boolean checkEntity(Object node) throws IOException
            {
                OperatorNode opNode = (OperatorNode)node;

                //For the first run, do a normal production check
                if (numChecks < numNodes)
                {
                    Vector v = new Vector();
                    boolean rc = opNode.CheckAgainstDatamap(v);
                    if (rc)
                    {
                        vecErrors.add("WARNING:  datamap errors were found in "
                                      + opNode.getFileName()
                                      + "'s productions.  This may invalid the current scan.");
                    }

                    numChecks++;
                    return rc;
                }//if

                //For the second run, do the requested datamap scan
                Vector v = new Vector();
                searchDatamap(opNode, v);
                numChecks++;
                
                if (!v.isEmpty())
                {
                    vecErrors.addAll(v);
                    return true;
                }

                return false;
            }//checkEntity()
           
		}//class DatamapTestThread
	}// end of SearchDataMapAction

    
	/**
     * Search for WMEs that are never tested
     */
	class SearchDataMapTestAction extends SearchDataMapAction
    {
        public void searchDatamap(OperatorNode opNode, Vector v)
        {
            opNode.searchTestDataMap(operatorWindow.getDatamap(), v);
        }//searchDatamap
	}// end of SearchDataMapTestAction

    
	/**
     * Search for WMEs that are never created
     */
	class SearchDataMapCreateAction extends SearchDataMapAction
    {
        public void searchDatamap(OperatorNode opNode, Vector v)
        {
            opNode.searchCreateDataMap(operatorWindow.getDatamap(), v);
        }//searchDatamap
	}// class SearchDataMapCreateAction

	/**
     * Search for WMEs that are tested but never created
     */
	class SearchDataMapTestNoCreateAction extends SearchDataMapAction
    {
        public void searchDatamap(OperatorNode opNode, Vector v)
        {
            opNode.searchTestNoCreateDataMap(operatorWindow.getDatamap(), v);
        }//searchDatamap
	}//class SearchDataMapTestNoCreateAction

	/**
     * Search for WMEs that are created but never tested
     */
	class SearchDataMapCreateNoTestAction extends SearchDataMapAction
    {
        public void searchDatamap(OperatorNode opNode, Vector v)
        {
            opNode.searchCreateNoTestDataMap(operatorWindow.getDatamap(), v);
        }//searchDatamap
	}//class SearchDataMapCreateNoTestAction

	/**
     * Search for WMEs that are never created and never tested
     */
	class SearchDataMapNoTestNoCreateAction extends SearchDataMapAction
    {
        public void searchDatamap(OperatorNode opNode, Vector v)
        {
            opNode.searchNoTestNoCreateDataMap(operatorWindow.getDatamap(), v);
        }//searchDatamap
	}//class SearchDataMapNoTestNoCreateAction


    /**
     * This class is responsible for comparing all productions in the project
     * with the project's datamaps and 'fixing' any discrepencies
     * by adding missing productions to the datamap.
     * Operation status is displayed in a progress bar.
     * Add productions in the datamap are displayed as green until the user validates them.
     * Results are displayed in the feedback list
     */
    class GenerateDataMapAction extends AbstractAction 
    {

		JProgressBar progressBar;
		JDialog progressDialog;

		public GenerateDataMapAction() 
        {

			super("Generate Datamap from Operator Hierarchy");
			setEnabled(false);
		}

		public void actionPerformed(ActionEvent ae) 
        {

			int numNodes = 0;
			Enumeration bfe = operatorWindow.breadthFirstEnumeration();
			while(bfe.hasMoreElements()) 
            {

				numNodes++;
				bfe.nextElement();
			}
			System.out.println("Nodes: " + numNodes);
			progressBar = new JProgressBar(0, numNodes * 7);
			progressDialog = new JDialog(MainFrame.this, "Generating Datamap from Productions");
			progressDialog.getContentPane().setLayout(new FlowLayout());
			progressDialog.getContentPane().add(progressBar);
			progressBar.setStringPainted(true);
			progressDialog.setLocationRelativeTo(MainFrame.this);
			progressDialog.pack();
			progressDialog.show();
			(new UpdateThread()).start();
		}

		class UpdateThread extends Thread 
        {

			Runnable update, finish;
			int value, min, max;
			 

			java.util.List errors = new LinkedList();
            java.util.List generations = new LinkedList();
            int repCount = 0;
			Enumeration bfe = operatorWindow.breadthFirstEnumeration();
			OperatorNode current;
			Vector parsedProds, vecErrors = new Vector();
			int nodeNum = 0;
			
			public UpdateThread() 
            {

				progressBar.getMaximum();
				progressBar.getMinimum();

				update = new Runnable() 
                {

					public void run() 
                    {

						value = progressBar.getValue() + 1;
						updateProgressBar(value);
						//System.out.println("Value is " + value);
					}
				};
				finish = new Runnable() 
                {

					public void run() 
                    {

						updateProgressBar(min);
						System.out.println("Done");
						progressDialog.dispose();
					}
				};
			}

			public void run() 
            {

                checkNodes();
                repCount = 0;

                JOptionPane.showMessageDialog(null, "DataMap Generation Completed", "DataMap Generator", JOptionPane.INFORMATION_MESSAGE);
			}

			private void updateProgressBar(int value) 
            {

				progressBar.setValue(value);
			}
						
			public void checkNodes() 
            {

				try 
                {

                    do 
                    {

                        repCount++;
                        errors.clear();
                        bfe = operatorWindow.breadthFirstEnumeration();

                      checkingNodes: while(bfe.hasMoreElements()) 
                      {

                          current = (OperatorNode)bfe.nextElement();
                          generations.clear();
                          // If the node has a rule editor open, get the rules from the
                          // window, otherwise, open the associated file

                          // is this how to get the rule editor?

                          try 
                          {

                              parsedProds = current.parseProductions();
                          }
                          catch(ParseException pe) 
                          {

                              String errString;
                              String parseError = pe.toString();
                              int i = parseError.lastIndexOf("line ");
                              String lineNum = parseError.substring(i + 5);
                              i = lineNum.indexOf(',');
                              lineNum = "(" + lineNum.substring(0, i) + "): ";
                              errString = current.getFileName() + lineNum + "Unable to generate datamap due to parse error";
                              errors.add(errString);
                          }
                          catch(TokenMgrError tme) 
                          {

                              tme.printStackTrace();
                          }

                          if (parsedProds!= null)  
                          {

							  operatorWindow.generateProductions((OperatorNode)current.getParent(), parsedProds, generations, current);
                              operatorWindow.checkProductions((OperatorNode)current.getParent(), parsedProds, errors);
                          }

                          Enumeration e = new EnumerationIteratorWrapper(generations.iterator());
                          while(e.hasMoreElements()) 
                          {

                              try 
                              {

                                  String errorString = e.nextElement().toString();
                                  String numberString = errorString.substring(errorString.indexOf("(")+1,errorString.indexOf(")"));
                                  vecErrors.add(
                                      new FeedbackListObject(current,Integer.parseInt(numberString),errorString,true,true, true));
                              }
                              catch(NumberFormatException nfe) 
                              {

                                  System.out.println("Never happen");
                              }
                          }
                          setFeedbackListData(vecErrors);
                          value = progressBar.getValue() + 1;
                          updateProgressBar(value);
                          //if(errors.isEmpty())
                          //  updateProgressBar(progressBar.getMaximum());
                          SwingUtilities.invokeLater(update);
                      } // while parsing operator nodes
          
                    } while(!(errors.isEmpty()) && repCount < 5);


                    SwingUtilities.invokeLater(finish);
				}   // end of try
				catch (IOException ioe) 
                {

					ioe.printStackTrace();
				}
			}

		}
	}    // End of class GenerateDataMapAction


	class FindInProjectAction extends AbstractAction 
    {

  		public FindInProjectAction()
        {

  			super("Find in Project");
			setEnabled(false);
  		}
  	
        /**
         * Prompts the user for a string to find in the project
         * finds all instances of the string in the project
         * and displays a FindInProjectList in the DesktopPane with all instances
         * or tells the user that no instances were found
         * @see FindDialog
         */  		
		public void actionPerformed(ActionEvent e) 
        {

			FindDialog theDialog = new FindDialog(MainFrame.this, operatorWindow);
			theDialog.setVisible(true);
		}
	}


    class ReplaceInProjectAction extends AbstractAction 
    {

        public ReplaceInProjectAction() 
        {

            super("Replace in Project");
            setEnabled(false);
        }

        /**
         * Prompts the user for a string to find in the project
         * and a string to replace the found string with.
         * Goes through all found instances and opens rules editors
         * for all files with matching strings.
         * @see ReplaceInProjectDialog
         */
        public void actionPerformed(ActionEvent e) 
        {

            ReplaceInProjectDialog replaceDialog = new ReplaceInProjectDialog(MainFrame.this, operatorWindow);
            replaceDialog.setVisible(true);
        }
    }

	
	class CommitAction extends PerformableAction 
    {

		public CommitAction() 
        {

			super("Commit");
			setEnabled(false);
		}
		
		public void perform() 
        {

			saveAllFilesAction.perform();
			if(operatorWindow != null) 
            {

				exportAgentAction.perform();
				saveDataMapAndProjectAction.perform();
			}
		}
		
		public void actionPerformed(ActionEvent e) 
        {

			perform();
			Vector v = new Vector();
			v.add("Save Finished");
			setFeedbackListData(v);
		}
	}
	
	class SaveProjectAsAction extends AbstractAction 
    {

		public SaveProjectAsAction() 
        {

			super("Save Project As");
			setEnabled(false);
		}

		public void actionPerformed(ActionEvent e) 
        {

			SaveProjectAsDialog spad = new SaveProjectAsDialog(MainFrame.getMainFrame());
            spad.show();

            OperatorRootNode orn = (OperatorRootNode)(operatorWindow.getModel().getRoot());
            File oldProjectFile = new File(orn.getProjectFile());

			if (spad.wasApproved()) 
            {

				String newName = spad.getNewAgentName();
                String newPath = spad.getNewAgentPath();
				if(OperatorWindow.isProjectNameValid(newName)) 
                {

					operatorWindow.saveProjectAs(newName, newPath);

                    // Regenerate the *_source.soar files in the old project
                    try 
                    {

                        OperatorWindow oldOpWin = new OperatorWindow(oldProjectFile);
                        OperatorRootNode oldOrn = (OperatorRootNode)oldOpWin.getModel().getRoot();
                        oldOrn.startSourcing();
                    }
                    catch (IOException exception) 
                    {

                        JOptionPane.showMessageDialog(MainFrame.this, "IO Error exporting agent.", "Agent Export Error", JOptionPane.ERROR_MESSAGE);
                        return;
                    }

                    JInternalFrame[] jif = DesktopPane.getAllFrames();
                    for(int i = 0; i < jif.length; ++i)
          
                    {

                        if(jif[i] instanceof RuleEditor) 
                        {

                            RuleEditor oldRuleEditor = (RuleEditor)jif[i];
                            OperatorNode newNode = oldRuleEditor.getNode();
                            oldRuleEditor.fileRenamed( newNode.getFileName() );         // Update the Rule editor with the correct updated file name
                        }
                    }
                    saveAllFilesAction.perform();     // Save all open Rule Editors to the new project directory
                    exportAgentAction.perform();
                    saveDataMapAndProjectAction.perform();    // Save DataMap and Project file (.vsa)
				}
				else 
                {

					JOptionPane.showMessageDialog(MainFrame.this, "That is not a valid name for the project", "Invalid Name", JOptionPane.ERROR_MESSAGE);				
				}
			}
		}
	
	}

    class TileWindowsAction extends AbstractAction 
    {

        public TileWindowsAction() 
        {

            super("Tile Windows");
        }

        public void actionPerformed(ActionEvent e) 
        {

            DesktopPane.performTileAction();
        }
    }

    class ReTileWindowsAction extends AbstractAction 
    {

        public ReTileWindowsAction() 
        {

            super("Re-Tile Windows");
        }

        public void actionPerformed(ActionEvent e) 
        {

            DesktopPane.performReTileAction();
        }
    }

    class CascadeAction extends AbstractAction 
    {

        public CascadeAction() 
        {

            super("Cascade Windows");
        }

        public void actionPerformed(ActionEvent e) 
        {

            DesktopPane.performCascadeAction();
        }
    }
}	
		
