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
 * This is the main project window of Visual Soar
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
	Action closeProjectAction = new CloseProjectAction();
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
	Action checkAllProductionsAction = new CheckAllProductionsAction();
    Action searchDataMapCreateAction = new SearchDataMapCreateAction();
    Action searchDataMapTestAction = new SearchDataMapTestAction();
    Action searchDataMapCreateNoTestAction = new SearchDataMapCreateNoTestAction();
    Action searchDataMapTestNoCreateAction = new SearchDataMapTestNoCreateAction();
    Action searchDataMapNoTestNoCreateAction = new SearchDataMapNoTestNoCreateAction();

    Action generateDataMapAction = new GenerateDataMapAction();
	Action saveProjectAsAction = new SaveProjectAsAction();
	Action contactUsAction = new ContactUsAction();
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

        // Use Java toolkit to access user's screen size and set Visual Soar window to 90% of that size
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
                    //If the user has modifed files that have not been saved,
                    //we need to ask the user what to do about it.
                    if (MainFrame.getMainFrame().isModified())
                    {
                        String[] buttons = { "Save all files, then exit.",
                                             "Exit without Saving" };

                        String selectedValue = (String)
                            JOptionPane.showInputDialog(
                                MainFrame.getMainFrame(),
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
                            MainFrame.getMainFrame().saveAllFilesAction.perform();
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
                    
                    exitAction.actionPerformed(
                        new ActionEvent(e.getSource(),e.getID(),"Exit"));
                }//windowClosing()
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
		commitAction.addPropertyChangeListener(
            new ActionButtonAssociation(commitAction,commitItem));
				
		JMenuItem exitItem = new JMenuItem("Exit");
		exitItem.addActionListener(exitAction);
		exitAction.addPropertyChangeListener(
            new ActionButtonAssociation(exitAction,exitItem));
		
		JMenuItem checkAllProductionsItem = new JMenuItem("Check All Productions Against DataMap");
		checkAllProductionsItem.addActionListener(checkAllProductionsAction);
		checkAllProductionsAction.addPropertyChangeListener(
            new ActionButtonAssociation(checkAllProductionsAction,checkAllProductionsItem));

		JMenuItem generateDataMapItem = new JMenuItem("Generate Datamap from Operator Hierarchy");
        generateDataMapItem.addActionListener(generateDataMapAction);
        generateDataMapAction.addPropertyChangeListener(
            new ActionButtonAssociation(generateDataMapAction, generateDataMapItem));

		JMenuItem saveProjectAsItem = new JMenuItem("Save Project As...");
		saveProjectAsItem.addActionListener(saveProjectAsAction);
		saveProjectAsAction.addPropertyChangeListener(
            new ActionButtonAssociation(saveProjectAsAction,saveProjectAsItem));
		
		/*
          JMenuItem checkCoverageItem = new JMenuItem("Check Coverage");
          checkCoverageItem.addActionListener(checkCoverageAction);
          checkCoverageAction.addPropertyChangeListener(
              new ActionButtonAssociation(checkCoverageAction,checkCoverageItem));
		*/
		
//		JMenuItem sendProductionsItem = new JMenuItem("Send Productions");
//		sendProductionsItem.addActionListener(sendProductionsAction);
		
		fileMenu.add(newProjectItem);
		fileMenu.add(openProjectItem);
        fileMenu.add(openFileItem);
		fileMenu.add(closeProjectItem);
		
		fileMenu.addSeparator();
		
		fileMenu.add(commitItem);
		fileMenu.add(saveProjectAsItem);

//		fileMenu.add(checkCoverageItem);
//		fileMenu.add(sendProductionsItem);

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

		JMenuItem findInProjectItem = new JMenuItem("Find In Project");
		findInProjectItem.addActionListener(findInProjectAction);
		findInProjectAction.addPropertyChangeListener(
            new ActionButtonAssociation(findInProjectAction,findInProjectItem));

        JMenuItem replaceInProjectItem = new JMenuItem("Replace In Project");
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

		JMenuItem checkAllProductionsItem = new JMenuItem("Check All Productions Against DataMap");
		checkAllProductionsItem.addActionListener(checkAllProductionsAction);
		checkAllProductionsAction.addPropertyChangeListener(
            new ActionButtonAssociation(checkAllProductionsAction,checkAllProductionsItem));

		JMenuItem searchDataMapTestItem = new JMenuItem("Search Datamap for untested WMEs");
		searchDataMapTestItem.addActionListener(searchDataMapTestAction);
		searchDataMapTestAction.addPropertyChangeListener(
            new ActionButtonAssociation(searchDataMapTestAction,searchDataMapTestItem));

		JMenuItem searchDataMapCreateItem = new JMenuItem("Search Datamap for non Created WMEs");
		searchDataMapCreateItem.addActionListener(searchDataMapCreateAction);
		searchDataMapCreateAction.addPropertyChangeListener(
            new ActionButtonAssociation(searchDataMapCreateAction,searchDataMapCreateItem));

		JMenuItem searchDataMapTestNoCreateItem = new JMenuItem("Search Datamap for WME's tested but never created");
		searchDataMapTestNoCreateItem.addActionListener(searchDataMapTestNoCreateAction);
		searchDataMapTestNoCreateAction.addPropertyChangeListener(
            new ActionButtonAssociation(searchDataMapTestNoCreateAction,searchDataMapTestNoCreateItem));

		JMenuItem searchDataMapCreateNoTestItem = new JMenuItem("Search Datamap for WME's created but never tested");
		searchDataMapCreateNoTestItem.addActionListener(searchDataMapCreateNoTestAction);
		searchDataMapCreateNoTestAction.addPropertyChangeListener(
            new ActionButtonAssociation(searchDataMapCreateNoTestAction,searchDataMapCreateNoTestItem));

        JMenuItem searchDataMapNoTestNoCreateItem = new JMenuItem("Search Datamap for WME's never tested and never created");
		searchDataMapNoTestNoCreateItem.addActionListener(searchDataMapNoTestNoCreateAction);
		searchDataMapNoTestNoCreateAction.addPropertyChangeListener(
            new ActionButtonAssociation(searchDataMapNoTestNoCreateAction,searchDataMapNoTestNoCreateItem));

		JMenuItem generateDataMapItem = new JMenuItem("Generate Datamap from Operator Hierarchy");
        generateDataMapItem.addActionListener(generateDataMapAction);
        generateDataMapAction.addPropertyChangeListener(
            new ActionButtonAssociation(generateDataMapAction, generateDataMapItem));

		datamapMenu.add(checkAllProductionsItem);
        datamapMenu.add(generateDataMapItem);
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
		
		JMenuItem reTileWindowItem = new JMenuItem("Re-tile Windows");
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
		catch (java.beans.PropertyVetoException pve) { System.err.println("Guess we can't do that");}				
	}
						
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
                                                                                             * themselves
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

					operatorWindow = new OperatorWindow(file);
					if(file.getParent() != null)
                    Preferences.getInstance().setOpenFolder(file.getParentFile());
					operatorDesktopSplit.setLeftComponent(new JScrollPane(operatorWindow));
					
					projectActionsEnable(true);
					
					operatorDesktopSplit.setDividerLocation(.30);
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

                    try 
                    {

                        boolean oldPref = Preferences.getInstance().isHighlightingEnabled();
                        Preferences.getInstance().setHighlightingEnabled(false);  // Turn off highlighting
                        RuleEditor ruleEditor = new RuleEditor(file);
                        ruleEditor.setVisible(true);
                        addRuleEditor(ruleEditor);
                        Preferences.getInstance().setHighlightingEnabled(oldPref);  // Turn it back to what it was
                    }
                    catch(IOException IOE) 
                    {

                        JOptionPane.showMessageDialog(MainFrame.this, "There was an error reading file: " +
                                                      file.getName(), "I/O Error", JOptionPane.ERROR_MESSAGE);
                    }
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
	class CloseProjectAction extends AbstractAction 
    {

		public CloseProjectAction() 
        {

			super("Close Project");
			setEnabled(false);
		}
		
		public void actionPerformed(ActionEvent event) 
        {

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
		
		public void actionPerformed(ActionEvent event) {	
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
		
			//to realize the change immediately... takes too long				
/*			if (theDialog.wasApproved()) 
            {

            // make the change realized...
            JInternalFrame[] theFrames = DesktopPane.getAllFrames();

            for (int i = 0; i < theFrames.length; i++) 
            {

            if (theFrames[i] instanceof RuleEditor) 
            {

            ((RuleEditor)theFrames[i]).recolorSyntax();
            }

            }
			}
*/		
		}
	}
	
	/**
	 * This is where the user user wants some info about the authors
     * @see AboutDialog
	 */	
	class ContactUsAction extends AbstractAction 
    {

		public ContactUsAction() 
        {

			super("About Visual Soar");
		}
		
		public void actionPerformed(ActionEvent e) 
        {

			aboutDialog.setVisible(true);		
		}
  	}


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
            JOptionPane.showMessageDialog(
                MainFrame.this,
                "I was unable to load the Soar Tools Interface (STI) library.\n"
                + "Therefore this part of the VisualSoar functionality will not \n"
                + "be available while you are running VisualSoar.  To enable \n"
                + "the STI, make sure that the SoarToolJavaInterface1 library \n"
                + "is in your PATH and restart VisualSoar.",
                "DLL Load Error",
                JOptionPane.ERROR_MESSAGE);

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
	/*
      class CheckCoverageAction extends AbstractAction 
      {

      public CheckCoverageAction() 
      {

      super("Check Coverage");
      setEnabled(false);
      }
		
      public void actionPerformed(ActionEvent ae) 
      {

      DataMapCoverageChecker dmcc = new DataMapCoverageChecker(getOperatorWindow().getDatamap());
      Enumeration bfe = operatorWindow.breadthFirstEnumeration();
      while(bfe.hasMoreElements()) 
      {

      OperatorNode on = (OperatorNode)bfe.nextElement();
      try 
      {

      Vector productions = on.parseProductions();
      if(productions != null) 
      {

      OperatorNode parent = (OperatorNode)on.getParent();
      SoarIdentifierVertex siv = parent.getStateIdVertex(getOperatorWindow().getDatamap());
      if(siv == null)
      dmcc.check(getOperatorWindow().getDatamap().getTopstate(),productions);
      else
      dmcc.check(siv,productions);
      }
      }
      catch(ParseException pe) {}
      catch(java.io.IOException ioe) {}
      }
      System.out.println("Place to Break when done");
      }
      }
 	*/

 	/**
     * This class is responsible for comparing all productions in the project
     * with the project's model of working memory - the datamap.
     * Operation status is displayed in a progress bar.
     * Results are displayed in the feedback list
     */
	class CheckAllProductionsAction extends AbstractAction 
    {

		JProgressBar progressBar;
		JDialog progressDialog;
        int numNodes;
	
		public CheckAllProductionsAction() 
        {

			super("Check All Productions");
			setEnabled(false);
		}

		public void actionPerformed(ActionEvent ae) 
        {

			numNodes = 0;
			Enumeration bfe = operatorWindow.breadthFirstEnumeration();
			while(bfe.hasMoreElements()) 
            {

				numNodes++;
				bfe.nextElement();
			}
			System.out.println("Nodes: " + numNodes);
			progressBar = new JProgressBar(0, numNodes);
			progressDialog = new JDialog(MainFrame.this, "Checking Productions");
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
                //checkNodesLog();
			}

			private void updateProgressBar(int value) 
            {

				progressBar.setValue(value);
			}

            public void checkNodes() 
            {

				try 
                {

                    boolean anyErrors = false;
                  checkingNodes: while(bfe.hasMoreElements()) 
                  {

                      current = (OperatorNode)bfe.nextElement();

                      // If the node has a rule editor open, get the rules from the
                      // window, otherwise, open the associated file

                      try 
                      {

                          parsedProds = current.parseProductions();

                      }
                      catch(ParseException pe) 
                      {


                          anyErrors = true;
                          String errString;
                          String parseError = pe.toString();
                          int i = parseError.lastIndexOf("line ");
                          String lineNum = parseError.substring(i + 5);
                          i = lineNum.indexOf(',');
                          lineNum = "(" + lineNum.substring(0, i) + "): ";
                          errString = current.getFileName() + lineNum + "Unable to check productions due to parse error";
                          errors.add(errString);
                      }
                      catch(TokenMgrError tme) 
                      {

                          tme.printStackTrace();
                      }

                      if (parsedProds!= null) 
                      {

                          operatorWindow.checkProductions((OperatorNode)current.getParent(), parsedProds, errors);
                      }
                      if (errors.isEmpty()) 
                      {

                          if(parsedProds != null) 
                          {


                              vecErrors.add("No errors detected in " + current.getFileName());
                          }
                      }
                      else 
                      {

                          anyErrors = true;
                          Enumeration e = new EnumerationIteratorWrapper(errors.iterator());
                          while(e.hasMoreElements()) 
                          {

                              try 
                              {

                                  String errorString = e.nextElement().toString();
                                  String numberString = errorString.substring(errorString.indexOf("(")+1,errorString.indexOf(")"));
                                  if(errorString.endsWith("Unable to check productions due to parse error"))
                                  {
                                      vecErrors.add(
                                          new FeedbackListObject(current,Integer.parseInt(numberString),errorString,true,true));
                                  }
                                  else
                                  {
                                      vecErrors.add(
                                          new FeedbackListObject(current,Integer.parseInt(numberString),errorString,true));
                                  }
                              } catch(NumberFormatException nfe) 
                              {

                                  System.out.println("Never happen");
                              }
                          }
                      }
                      errors.clear();
                      updateProgressBar(++nodeNum);
                      SwingUtilities.invokeLater(update);
                  } // while parsing operator nodes

                    if(!anyErrors)
                    vecErrors.add("There were no errors detected in this project.");
					feedbackList.setListData(vecErrors);
					SwingUtilities.invokeLater(finish);
				}
				catch (IOException ioe) 
                {

					ioe.printStackTrace();
				}
			}          // end of checkNodes()

			/**
             * Similar to checkNodes(), but this generates a log file.
             */
			public void checkNodesLog() 
            {

				try 
                {

                    FileWriter fw = new FileWriter("CheckingProductions.log");

                    boolean anyErrors = false;
                  checkingNodes: while(bfe.hasMoreElements()) 
                  {

                      current = (OperatorNode)bfe.nextElement();

                      try 
                      {

                          fw.write("Looking at node " + current.toString());
                          fw.write('\n');
                      }
                      catch(IOException e) 
                      {

                          e.printStackTrace();
                      }

                      // If the node has a rule editor open, get the rules from the
                      // window, otherwise, open the associated file


                      try 
                      {

                          parsedProds = current.parseProductions();
                          try 
                          {

                              fw.write("Parsed Productions for node " + current.toString());
                              fw.write('\n');
                          }
                          catch(IOException e) 
                          {

                              e.printStackTrace();
                          }
                      }
                      catch(ParseException pe) 
                      {


                          try 
                          {

                              fw.write("There was a parse error when attempting to parse productions for node " + current.toString());
                              fw.write('\n');
                          }
                          catch(IOException e) 
                          {

                              e.printStackTrace();
                          }
                          anyErrors = true;
                          String errString;
                          String parseError = pe.toString();
                          int i = parseError.lastIndexOf("line ");
                          String lineNum = parseError.substring(i + 5);
                          i = lineNum.indexOf(',');
                          lineNum = "(" + lineNum.substring(0, i) + "): ";
                          errString = current.getFileName() + lineNum + "Unable to check productions due to parse error";
                          errors.add(errString);
                      }
                      catch(TokenMgrError tme) 
                      {

                          tme.printStackTrace();
                      }

                      if (parsedProds!= null) 
                      {

                          try 
                          {

                              fw.write("About to check individual productions for node " + current.toString());
                              fw.write('\n');
                          }
                          catch(IOException e) 
                          {

                              e.printStackTrace();
                          }
                          operatorWindow.checkProductionsLog((OperatorNode)current.getParent(), parsedProds, errors, fw);
                      }
                      if (errors.isEmpty()) 
                      {

                          if(parsedProds != null) 
                          {

                              try 
                              {

                                  fw.write("No errors detected for node " + current.toString());
                                  fw.write('\n');
                              }
                              catch(IOException e) 
                              {

                                  e.printStackTrace();
                              }
                              vecErrors.add("No errors detected in " + current.getFileName());
                          }
                      }
                      else 
                      {

                          try 
                          {

                              fw.write("Errors were detected for node " + current.toString());
                              fw.write('\n');
                          }
                          catch(IOException e) 
                          {

                              e.printStackTrace();
                          }

                          anyErrors = true;
                          Enumeration e = new EnumerationIteratorWrapper(errors.iterator());
                          while(e.hasMoreElements()) 
                          {

                              try 
                              {

                                  String errorString = e.nextElement().toString();
                                  String numberString = errorString.substring(errorString.indexOf("(")+1,errorString.indexOf(")"));
                                  if(errorString.endsWith("Unable to check productions due to parse error"))
                                  {
                                      vecErrors.add(
                                          new FeedbackListObject(current,Integer.parseInt(numberString),errorString,true,true));
                                  }
                                  else
                                  {
                                      vecErrors.add(
                                          new FeedbackListObject(current,Integer.parseInt(numberString),errorString,true));
                                  }
                              } catch(NumberFormatException nfe) 
                              {

                                  System.out.println("Never happen");
                              }
                          }
                      }
                      errors.clear();
                      updateProgressBar(++nodeNum);
                      SwingUtilities.invokeLater(update);

                      try 
                      {

                          fw.write("Check for the node " + current.toString() + " completed.  Getting next node\n");
                          fw.write( nodeNum + " nodes out of " + numNodes + " total nodes have been completed.");
                          fw.write('\n');
                          fw.write('\n');
                      }
                      catch(IOException e) 
                      {

                          e.printStackTrace();
                      }
                  } // while parsing operator nodes

                    if(!anyErrors)
                    vecErrors.add("There were no errors detected in this project.");
					feedbackList.setListData(vecErrors);
					SwingUtilities.invokeLater(finish);

                    fw.close();
				}
				catch (IOException ioe) 
                {

					ioe.printStackTrace();
				}
			}

		}       // end of CheckNodesLog()
	}      // end of CheckProductions Action


	/**
     * This action searches all Datamaps to find WMEs that
     * are never tested by any production within the project.
     * i.e. not in the condition side of any production and not in the output-link.
     * Operation status is displayed in a progress bar.
     * Results are displayed in the feedback list
     * Double-clicking on an item in the feedback list
     * should display the rogue node in the datamap.
     */
	class SearchDataMapTestAction extends AbstractAction 
    {

		JProgressBar progressBar;
		JDialog progressDialog;
        int numNodes;

		public SearchDataMapTestAction() 
        {

			super("Check All Productions");
			setEnabled(false);
		}

		public void actionPerformed(ActionEvent ae) 
        {

			numNodes = 0;
			Enumeration bfe = operatorWindow.breadthFirstEnumeration();
			while(bfe.hasMoreElements()) 
            {

				numNodes++;
				bfe.nextElement();
			}
			System.out.println("Nodes: " + (numNodes * 2));
			progressBar = new JProgressBar(0, numNodes * 2);
			progressDialog = new JDialog(MainFrame.this, "Checking Productions");
			progressDialog.getContentPane().setLayout(new FlowLayout());
			progressDialog.getContentPane().add(progressBar);
			progressBar.setStringPainted(true);
			progressDialog.setLocationRelativeTo(MainFrame.this);
			progressDialog.pack();
			progressDialog.show();
			(new UpdateThread()).start();
		}   // end of constructor

		class UpdateThread extends Thread 
        {

			Runnable update, finish;
			int value, min, max;


			java.util.List errors = new LinkedList();
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

                initializeNodes();
				if(checkNodes())
                searchDataMap();
                SwingUtilities.invokeLater(finish);
			}

			private void updateProgressBar(int value) 
            {

				progressBar.setValue(value);
			}

            /**
             *  This initializes the status of all the edges to zero, which means that
             *  the edges have not been used by a production in any way.
             */
            public void initializeNodes() 
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
            }     // end of searchDataMap()


            /**
             *  Simply go through all the nodes, checking productions
             *  and marking the named edges in the datamap when either tested
             *  or created by a production.
             *  @return  false if encountered a parse error
             */
            public boolean checkNodes() 
            {

				try 
                {

                    boolean anyErrors = false;
                  checkingNodes: while(bfe.hasMoreElements()) 
                  {

                      current = (OperatorNode)bfe.nextElement();

                      // If the node has a rule editor open, get the rules from the
                      // window, otherwise, open the associated file

                      try 
                      {

                          parsedProds = current.parseProductions();

                      }
                      catch(ParseException pe) 
                      {


                          anyErrors = true;
                          String errString;
                          String parseError = pe.toString();
                          int i = parseError.lastIndexOf("line ");
                          String lineNum = parseError.substring(i + 5);
                          i = lineNum.indexOf(',');
                          lineNum = "(" + lineNum.substring(0, i) + "): ";
                          errString = current.getFileName() + lineNum + "Unable to Search DataMap for extra WMEs due to parse error";
                          errors.add(errString);
                      }
                      catch(TokenMgrError tme) 
                      {

                          tme.printStackTrace();
                      }

                      if (parsedProds!= null) 
                      {

                          operatorWindow.checkProductions((OperatorNode)current.getParent(), parsedProds, errors);
                      }
                      if (!errors.isEmpty()) 
                      {

                          anyErrors = true;
                          Enumeration e = new EnumerationIteratorWrapper(errors.iterator());
                          while(e.hasMoreElements()) 
                          {

                              try 
                              {

                                  String errorString = e.nextElement().toString();
                                  String numberString = errorString.substring(errorString.indexOf("(")+1,errorString.indexOf(")"));
                                  if(errorString.endsWith("Unable to Search DataMap for extra WMEs due to parse error")) 
                                  {

                                      vecErrors.add(
                                          new FeedbackListObject(current,Integer.parseInt(numberString),errorString,true,true));
                                      feedbackList.setListData(vecErrors);
                                      SwingUtilities.invokeLater(finish);
                                      return false;
                                  }
                              } catch(NumberFormatException nfe) 
                              {

                                  System.out.println("Never happen");
                              }
                          }
                      }
                      errors.clear();
                      updateProgressBar(++nodeNum);
                      SwingUtilities.invokeLater(update);
                  } // while parsing operator nodes

					feedbackList.setListData(vecErrors);
                    return true;
				}
				catch (IOException ioe) 
                {

					ioe.printStackTrace();
				}
                return false;  // should never get here unless try failed
			}     // end of checkNodes()

            /**
             *  Search through the datamap and look for extra WMEs
             *  by looking at the status of the named edge (as determined
             *  by the check nodes function) and the edge's location within
             *  the datamap.
             *  Extra WMEs are classified in this action by never being tested by a
             *  production, not including any item within the output-link.
             *  Send results to the feedback list
             */
            public void searchDataMap() 
            {

                OperatorNode operatorNode;
                vecErrors.clear();    // clear out errors just in case
                vecErrors.add("Attributes never tested in productions in this agent:");

                // Go through all of the nodes examining the nodes' datamap if they have one
                Enumeration bfe = operatorWindow.breadthFirstEnumeration();
                while(bfe.hasMoreElements()) 
                {

                    operatorNode = (OperatorNode)bfe.nextElement();
                    operatorNode.searchTestDataMap(operatorWindow.getDatamap(), vecErrors);
                    updateProgressBar(++nodeNum);
                    SwingUtilities.invokeLater(update);
                }   // end of while looking at all nodes

                // do case of no errors
                if(vecErrors.size() == 1) 
                {

                    vecErrors.clear();
                    vecErrors.add("No errors.  All attributes were tested in productions within this agents");
                }

                // Add list of errors to feedback list
                feedbackList.setListData(vecErrors);
            }     // end of searchDataMap()

		}   // end of UpdateThread class
	}   // end of SearchDataMapTestAction

	/**
     * This action searches all Datamaps to find WMEs that
     * are never created by any production within the project.
     * i.e. not in the action side of any production and not in the output-link.
     * Operation status is displayed in a progress bar.
     * Results are displayed in the feedback list
     * Double-clicking on an item in the feedback list
     * should display the rogue node in the datamap.
     */
	class SearchDataMapCreateAction extends AbstractAction 
    {

		JProgressBar progressBar;
		JDialog progressDialog;
        int numNodes;

		public SearchDataMapCreateAction() 
        {

			super("Check All Productions");
			setEnabled(false);
		}

		public void actionPerformed(ActionEvent ae) 
        {

			numNodes = 0;
			Enumeration bfe = operatorWindow.breadthFirstEnumeration();
			while(bfe.hasMoreElements()) 
            {

				numNodes++;
				bfe.nextElement();
			}
			System.out.println("Nodes: " + (numNodes * 2));
			progressBar = new JProgressBar(0, numNodes * 2);
			progressDialog = new JDialog(MainFrame.this, "Checking Productions");
			progressDialog.getContentPane().setLayout(new FlowLayout());
			progressDialog.getContentPane().add(progressBar);
			progressBar.setStringPainted(true);
			progressDialog.setLocationRelativeTo(MainFrame.this);
			progressDialog.pack();
			progressDialog.show();
			(new UpdateThread()).start();
		}   // end of constructor

		class UpdateThread extends Thread 
        {

			Runnable update, finish;
			int value, min, max;


			java.util.List errors = new LinkedList();
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

                initializeNodes();
				if(checkNodes())
                searchDataMap();
                SwingUtilities.invokeLater(finish);
			}

			private void updateProgressBar(int value) 
            {

				progressBar.setValue(value);
			}

            /**
             *  This initializes the status of all the edges to zero, which means that
             *  the edges have not been used by a production in any way.
             */
            public void initializeNodes() 
            {

                Enumeration edges = operatorWindow.getDatamap().getEdges();
                while(edges.hasMoreElements()) 
                {

                    NamedEdge currentEdge = (NamedEdge)edges.nextElement();
                    currentEdge.resetTestedStatus();
                    currentEdge.resetErrorNoted();
                    // initialize the input-link as already tested
                    if(currentEdge.getName().equals("input-link"))
                    currentEdge.setInputLinkCreated(operatorWindow.getDatamap());
                }
            }     // end of searchDataMap()

            /**
             *  Simply go through all the nodes, checking productions
             *  and marking the named edges in the datamap when either tested
             *  or created by a production.
             *  @return  false if encountered a parse error
             */
            public boolean checkNodes() 
            {

				try 
                {

                    boolean anyErrors = false;
                  checkingNodes: while(bfe.hasMoreElements()) 
                  {

                      current = (OperatorNode)bfe.nextElement();

                      // If the node has a rule editor open, get the rules from the
                      // window, otherwise, open the associated file

                      try 
                      {

                          parsedProds = current.parseProductions();
                      }
                      catch(ParseException pe) 
                      {

                          anyErrors = true;
                          String errString;
                          String parseError = pe.toString();
                          int i = parseError.lastIndexOf("line ");
                          String lineNum = parseError.substring(i + 5);
                          i = lineNum.indexOf(',');
                          lineNum = "(" + lineNum.substring(0, i) + "): ";
                          errString = current.getFileName() + lineNum + "Unable to Search DataMap for extra WMEs due to parse error";
                          errors.add(errString);
                      }
                      catch(TokenMgrError tme) 
                      {

                          tme.printStackTrace();
                      }

                      if (parsedProds!= null) 
                      {

                          operatorWindow.checkProductions((OperatorNode)current.getParent(), parsedProds, errors);
                      }

                      if (!errors.isEmpty()) 
                      {

                          anyErrors = true;
                          Enumeration e = new EnumerationIteratorWrapper(errors.iterator());
                          while(e.hasMoreElements()) 
                          {

                              try 
                              {

                                  String errorString = e.nextElement().toString();
                                  String numberString = errorString.substring(errorString.indexOf("(")+1,errorString.indexOf(")"));
                                  if(errorString.endsWith("Unable to Search DataMap for extra WMEs due to parse error")) 
                                  {

                                      vecErrors.add(
                                          new FeedbackListObject(current,Integer.parseInt(numberString),errorString,true,true));
                                      feedbackList.setListData(vecErrors);
                                      SwingUtilities.invokeLater(finish);
                                      return false;
                                  }
                              } catch(NumberFormatException nfe) 
                              {

                                  System.out.println("Never happen");
                              }
                          }
                      }
                      errors.clear();
                      updateProgressBar(++nodeNum);
                      SwingUtilities.invokeLater(update);

                  } // while parsing operator nodes

					feedbackList.setListData(vecErrors);
                    return true;
				}
				catch (IOException ioe) 
                {

					ioe.printStackTrace();
				}
                return false;  // should never get here unless try failed
			}     // end of checkNodes()


            /**
             *  Search through the datamap and look for extra WMEs
             *  by looking at the status of the named edge (as determined
             *  by the check nodes function) and the edge's location within
             *  the datamap.
             *  Extra WMEs are classified in this action by never being created by a
             *  production, not including any item within the input-link.
             *  Send results to the feedback list
             */
            public void searchDataMap() 
            {

                OperatorNode operatorNode;
                vecErrors.clear();    // clear out errors just in case
                vecErrors.add("Attributes never created in productions in this agent:");

                // Go through all of the nodes examining the nodes' datamap if they have one
                Enumeration bfe = operatorWindow.breadthFirstEnumeration();
                while(bfe.hasMoreElements()) 
                {

                    operatorNode = (OperatorNode)bfe.nextElement();
                    operatorNode.searchCreateDataMap(operatorWindow.getDatamap(), vecErrors);
                    updateProgressBar(++nodeNum);
                    SwingUtilities.invokeLater(update);
                }   // end of while looking at all nodes

                // do case of no errors
                if(vecErrors.size() == 1) 
                {

                    vecErrors.clear();
                    vecErrors.add("No errors.  All attributes were created in productions within this agents");
                }

                // Add list of errors to feedback list
                feedbackList.setListData(vecErrors);
            }     // end of searchDataMap()

		}   // end of UpdateThread class
	}   // end of SearchDataMapCreateAction


	/**
     * This action searches all Datamaps to find WMEs that
     * are never tested by some production within the project, but never created.
     * i.e. not in the action side of any production and not in the output-link.
     * Operation status is displayed in a progress bar.
     * Results are displayed in the feedback list
     * Double-clicking on an item in the feedback list
     * should display the rogue node in the datamap.
     */
	class SearchDataMapTestNoCreateAction extends AbstractAction 
    {

		JProgressBar progressBar;
		JDialog progressDialog;
        int numNodes;

		public SearchDataMapTestNoCreateAction() 
        {

			super("Searching DataMap");
			setEnabled(false);
		}

		public void actionPerformed(ActionEvent ae) 
        {

			numNodes = 0;
			Enumeration bfe = operatorWindow.breadthFirstEnumeration();
			while(bfe.hasMoreElements()) 
            {

				numNodes++;
				bfe.nextElement();
			}
			System.out.println("Nodes: " + (numNodes * 2));
			progressBar = new JProgressBar(0, numNodes * 2);
			progressDialog = new JDialog(MainFrame.this, "Checking Productions");
			progressDialog.getContentPane().setLayout(new FlowLayout());
			progressDialog.getContentPane().add(progressBar);
			progressBar.setStringPainted(true);
			progressDialog.setLocationRelativeTo(MainFrame.this);
			progressDialog.pack();
			progressDialog.show();
			(new UpdateThread()).start();
		}   // end of constructor

		class UpdateThread extends Thread 
        {

			Runnable update, finish;
			int value, min, max;


			java.util.List errors = new LinkedList();
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

                initializeNodes();
				if(checkNodes())
                searchDataMap();
                SwingUtilities.invokeLater(finish);
			}

			private void updateProgressBar(int value) 
            {

				progressBar.setValue(value);
			}

            /**
             *  This initializes the status of all the edges to zero, which means that
             *  the edges have not been used by a production in any way.
             */
            public void initializeNodes() 
            {

                Enumeration edges = operatorWindow.getDatamap().getEdges();
                while(edges.hasMoreElements()) 
                {

                    NamedEdge currentEdge = (NamedEdge)edges.nextElement();
                    currentEdge.resetTestedStatus();
                    currentEdge.resetErrorNoted();
                    // initialize the input-link as already tested
                    if(currentEdge.getName().equals("input-link"))
                    currentEdge.setInputLinkCreated(operatorWindow.getDatamap());
                    // initialize the output-link as already tested
                    if(currentEdge.getName().equals("output-link"))
                    currentEdge.setOutputLinkTested(operatorWindow.getDatamap());
                }
            }     // end of initializeNodes()

            /**
             *  Simply go through all the nodes, checking productions
             *  and marking the named edges in the datamap when either tested
             *  or created by a production.
             *  @return  false if encountered a parse error
             */
            public boolean checkNodes() 
            {

				try 
                {

                    boolean anyErrors = false;
                  checkingNodes: while(bfe.hasMoreElements()) 
                  {

                      current = (OperatorNode)bfe.nextElement();

                      // If the node has a rule editor open, get the rules from the
                      // window, otherwise, open the associated file

                      try 
                      {

                          parsedProds = current.parseProductions();
                      }
                      catch(ParseException pe) 
                      {

                          anyErrors = true;
                          String errString;
                          String parseError = pe.toString();
                          int i = parseError.lastIndexOf("line ");
                          String lineNum = parseError.substring(i + 5);
                          i = lineNum.indexOf(',');
                          lineNum = "(" + lineNum.substring(0, i) + "): ";
                          errString = current.getFileName() + lineNum + "Unable to Search DataMap for extra WMEs due to parse error";
                          errors.add(errString);
                      }
                      catch(TokenMgrError tme) 
                      {

                          tme.printStackTrace();
                      }

                      if (parsedProds!= null) 
                      {

                          operatorWindow.checkProductions((OperatorNode)current.getParent(), parsedProds, errors);
                      }

                      if (!errors.isEmpty()) 
                      {

                          anyErrors = true;
                          Enumeration e = new EnumerationIteratorWrapper(errors.iterator());
                          while(e.hasMoreElements()) 
                          {

                              try 
                              {

                                  String errorString = e.nextElement().toString();
                                  String numberString = errorString.substring(errorString.indexOf("(")+1,errorString.indexOf(")"));
                                  if(errorString.endsWith("Unable to Search DataMap for extra WMEs due to parse error")) 
                                  {

                                      vecErrors.add(
                                          new FeedbackListObject(current,Integer.parseInt(numberString),errorString,true,true));
                                      feedbackList.setListData(vecErrors);
                                      SwingUtilities.invokeLater(finish);
                                      return false;
                                  }
                              } catch(NumberFormatException nfe) 
                              {

                                  System.out.println("Never happen");
                              }
                          }
                      }
                      errors.clear();
                      updateProgressBar(++nodeNum);
                      SwingUtilities.invokeLater(update);

                  } // while parsing operator nodes

					feedbackList.setListData(vecErrors);
                    return true;
				}
				catch (IOException ioe) 
                {

					ioe.printStackTrace();
				}
                return false;  // should never get here unless try failed
			}     // end of checkNodes()


            /**
             *  Search through the datamap and look for extra WMEs
             *  by looking at the status of the named edge (as determined
             *  by the check nodes function) and the edge's location within
             *  the datamap.
             *  Extra WMEs are classified in this action by being tested by some production
             *  within the project, but never created by any production;
             *  not including any item within the input-link.
             *  Send results to the feedback list
             */
            public void searchDataMap() 
            {

                OperatorNode operatorNode;
                vecErrors.clear();    // clear out errors just in case
                vecErrors.add("Attributes that were tested but never created in the productions of this agent:");

                // Go through all of the nodes examining the nodes' datamap if they have one
                Enumeration bfe = operatorWindow.breadthFirstEnumeration();
                while(bfe.hasMoreElements()) 
                {

                    operatorNode = (OperatorNode)bfe.nextElement();
                    operatorNode.searchTestNoCreateDataMap(operatorWindow.getDatamap(), vecErrors);
                    updateProgressBar(++nodeNum);
                    SwingUtilities.invokeLater(update);
                }   // end of while looking at all nodes

                // do case of no errors
                if(vecErrors.size() == 1) 
                {

                    vecErrors.clear();
                    vecErrors.add("No errors.  All attributes that were tested, were also created.");
                }

                // Add list of errors to feedback list
                feedbackList.setListData(vecErrors);
            }     // end of searchDataMap()

		}   // end of UpdateThread class
	}   // end of SearchDataMapTestNoCreateAction


	/**
     * This action searches all Datamaps to find WMEs that
     * are created by some production within the project, but never tested.
     * i.e. not in the action side of any production and not in the output-link.
     * Operation status is displayed in a progress bar.
     * Results are displayed in the feedback list
     * Double-clicking on an item in the feedback list
     * should display the rogue node in the datamap.
     */
	class SearchDataMapCreateNoTestAction extends AbstractAction 
    {

		JProgressBar progressBar;
		JDialog progressDialog;
        int numNodes;

		public SearchDataMapCreateNoTestAction() 
        {

			super("Searching DataMap");
			setEnabled(false);
		}

		public void actionPerformed(ActionEvent ae) 
        {

			numNodes = 0;
			Enumeration bfe = operatorWindow.breadthFirstEnumeration();
			while(bfe.hasMoreElements()) 
            {

				numNodes++;
				bfe.nextElement();
			}
			System.out.println("Nodes: " + (numNodes * 2));
			progressBar = new JProgressBar(0, numNodes * 2);
			progressDialog = new JDialog(MainFrame.this, "Checking Productions");
			progressDialog.getContentPane().setLayout(new FlowLayout());
			progressDialog.getContentPane().add(progressBar);
			progressBar.setStringPainted(true);
			progressDialog.setLocationRelativeTo(MainFrame.this);
			progressDialog.pack();
			progressDialog.show();
			(new UpdateThread()).start();
		}   // end of constructor

		class UpdateThread extends Thread 
        {

			Runnable update, finish;
			int value, min, max;


			java.util.List errors = new LinkedList();
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

                initializeNodes();
				if(checkNodes())
                searchDataMap();
                SwingUtilities.invokeLater(finish);
			}

			private void updateProgressBar(int value) 
            {

				progressBar.setValue(value);
			}

            /**
             *  This initializes the status of all the edges to zero, which means that
             *  the edges have not been used by a production in any way.
             */
            public void initializeNodes() 
            {

                Enumeration edges = operatorWindow.getDatamap().getEdges();
                while(edges.hasMoreElements()) 
                {

                    NamedEdge currentEdge = (NamedEdge)edges.nextElement();
                    currentEdge.resetTestedStatus();
                    currentEdge.resetErrorNoted();
                    // initialize the input-link as already tested
                    if(currentEdge.getName().equals("input-link"))
                    currentEdge.setInputLinkCreated(operatorWindow.getDatamap());
                    // initialize the output-link as already tested
                    if(currentEdge.getName().equals("output-link"))
                    currentEdge.setOutputLinkTested(operatorWindow.getDatamap());
                }
            }     // end of initializeNodes()

            /**
             *  Simply go through all the nodes, checking productions
             *  and marking the named edges in the datamap when either tested
             *  or created by a production.
             *  @return  false if encountered a parse error
             */
            public boolean checkNodes() 
            {

				try 
                {

                    boolean anyErrors = false;
                  checkingNodes: while(bfe.hasMoreElements()) 
                  {

                      current = (OperatorNode)bfe.nextElement();

                      // If the node has a rule editor open, get the rules from the
                      // window, otherwise, open the associated file

                      try 
                      {

                          parsedProds = current.parseProductions();
                      }
                      catch(ParseException pe) 
                      {

                          anyErrors = true;
                          String errString;
                          String parseError = pe.toString();
                          int i = parseError.lastIndexOf("line ");
                          String lineNum = parseError.substring(i + 5);
                          i = lineNum.indexOf(',');
                          lineNum = "(" + lineNum.substring(0, i) + "): ";
                          errString = current.getFileName() + lineNum + "Unable to Search DataMap for extra WMEs due to parse error";
                          errors.add(errString);
                      }
                      catch(TokenMgrError tme) 
                      {

                          tme.printStackTrace();
                      }

                      if (parsedProds!= null) 
                      {

                          operatorWindow.checkProductions((OperatorNode)current.getParent(), parsedProds, errors);
                      }

                      if (!errors.isEmpty()) 
                      {

                          anyErrors = true;
                          Enumeration e = new EnumerationIteratorWrapper(errors.iterator());
                          while(e.hasMoreElements()) 
                          {

                              try 
                              {

                                  String errorString = e.nextElement().toString();
                                  String numberString = errorString.substring(errorString.indexOf("(")+1,errorString.indexOf(")"));
                                  if(errorString.endsWith("Unable to Search DataMap for extra WMEs due to parse error")) 
                                  {

                                      vecErrors.add(
                                          new FeedbackListObject(current,Integer.parseInt(numberString),errorString,true,true));
                                      feedbackList.setListData(vecErrors);
                                      SwingUtilities.invokeLater(finish);
                                      return false;
                                  }
                              } catch(NumberFormatException nfe) 
                              {

                                  System.out.println("Never happen");
                              }
                          }
                      }
                      errors.clear();
                      updateProgressBar(++nodeNum);
                      SwingUtilities.invokeLater(update);

                  } // while parsing operator nodes

					feedbackList.setListData(vecErrors);
                    return true;
				}
				catch (IOException ioe) 
                {

					ioe.printStackTrace();
				}
                return false;  // should never get here unless try failed
			}     // end of checkNodes()


            /**
             *  Search through the datamap and look for extra WMEs
             *  by looking at the status of the named edge (as determined
             *  by the check nodes function) and the edge's location within
             *  the datamap.
             *  Extra WMEs are classified in this action by being created by some production
             *  within the project, but never tested by any production;
             *  not including any item within the output-link.
             *  Send results to the feedback list
             */
            public void searchDataMap() 
            {

                OperatorNode operatorNode;
                vecErrors.clear();    // clear out errors just in case
                vecErrors.add("Attributes that were created but never tested in the productions of this agent:");

                // Go through all of the nodes examining the nodes' datamap if they have one
                Enumeration bfe = operatorWindow.breadthFirstEnumeration();
                while(bfe.hasMoreElements()) 
                {

                    operatorNode = (OperatorNode)bfe.nextElement();
                    operatorNode.searchCreateNoTestDataMap(operatorWindow.getDatamap(), vecErrors);
                    updateProgressBar(++nodeNum);
                    SwingUtilities.invokeLater(update);
                }   // end of while looking at all nodes

                // do case of no errors
                if(vecErrors.size() == 1) 
                {

                    vecErrors.clear();
                    vecErrors.add("No errors.  All attributes that were created, were also tested.");
                }

                // Add list of errors to feedback list
                feedbackList.setListData(vecErrors);
            }     // end of searchDataMap()

		}   // end of UpdateThread class
	}   // end of SearchDataMapCreateNoTestAction

///////////////////////////
	/**
     * This action searches all Datamaps to find WMEs that
     * are never tested by some production within the project and are never created.
     * i.e. not in the action or condition side of any production and not in the io link.
     * Operation status is displayed in a progress bar.
     * Results are displayed in the feedback list
     * Double-clicking on an item in the feedback list
     * should display the rogue node in the datamap.
     */
	class SearchDataMapNoTestNoCreateAction extends AbstractAction 
    {

		JProgressBar progressBar;
		JDialog progressDialog;
        int numNodes;

		public SearchDataMapNoTestNoCreateAction() 
        {

			super("Searching DataMap");
			setEnabled(false);
		}

		public void actionPerformed(ActionEvent ae) 
        {

			numNodes = 0;
			Enumeration bfe = operatorWindow.breadthFirstEnumeration();
			while(bfe.hasMoreElements()) 
            {

				numNodes++;
				bfe.nextElement();
			}
			System.out.println("Nodes: " + (numNodes * 2));
			progressBar = new JProgressBar(0, numNodes * 2);
			progressDialog = new JDialog(MainFrame.this, "Checking Productions");
			progressDialog.getContentPane().setLayout(new FlowLayout());
			progressDialog.getContentPane().add(progressBar);
			progressBar.setStringPainted(true);
			progressDialog.setLocationRelativeTo(MainFrame.this);
			progressDialog.pack();
			progressDialog.show();
			(new UpdateThread()).start();
		}   // end of constructor

		class UpdateThread extends Thread 
        {

			Runnable update, finish;
			int value, min, max;


			java.util.List errors = new LinkedList();
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

                initializeNodes();
				if(checkNodes())
                searchDataMap();
                SwingUtilities.invokeLater(finish);
			}

			private void updateProgressBar(int value) 
            {

				progressBar.setValue(value);
			}

            /**
             *  This initializes the status of all the edges to zero, which means that
             *  the edges have not been used by a production in any way.
             */
            public void initializeNodes() 
            {

                Enumeration edges = operatorWindow.getDatamap().getEdges();
                while(edges.hasMoreElements()) 
                {

                    NamedEdge currentEdge = (NamedEdge)edges.nextElement();
                    currentEdge.resetTestedStatus();
                    currentEdge.resetErrorNoted();
                    // initialize the input-link as already tested
                    if(currentEdge.getName().equals("input-link"))
                    currentEdge.setInputLinkCreated(operatorWindow.getDatamap());
                    // initialize the output-link as already tested
                    if(currentEdge.getName().equals("output-link"))
                    currentEdge.setOutputLinkTested(operatorWindow.getDatamap());
                }
            }     // end of initializeNodes()

            /**
             *  Simply go through all the nodes, checking productions
             *  and marking the named edges in the datamap when either tested
             *  or created by a production.
             *  @return  false if encountered a parse error
             */
            public boolean checkNodes() 
            {

				try 
                {

                    boolean anyErrors = false;
                  checkingNodes: while(bfe.hasMoreElements()) 
                  {

                      current = (OperatorNode)bfe.nextElement();

                      // If the node has a rule editor open, get the rules from the
                      // window, otherwise, open the associated file

                      try 
                      {

                          parsedProds = current.parseProductions();
                      }
                      catch(ParseException pe) 
                      {

                          anyErrors = true;
                          String errString;
                          String parseError = pe.toString();
                          int i = parseError.lastIndexOf("line ");
                          String lineNum = parseError.substring(i + 5);
                          i = lineNum.indexOf(',');
                          lineNum = "(" + lineNum.substring(0, i) + "): ";
                          errString = current.getFileName() + lineNum + "Unable to Search DataMap for extra WMEs due to parse error";
                          errors.add(errString);
                      }
                      catch(TokenMgrError tme) 
                      {

                          tme.printStackTrace();
                      }

                      if (parsedProds!= null) 
                      {

                          operatorWindow.checkProductions((OperatorNode)current.getParent(), parsedProds, errors);
                      }

                      if (!errors.isEmpty()) 
                      {

                          anyErrors = true;
                          Enumeration e = new EnumerationIteratorWrapper(errors.iterator());
                          while(e.hasMoreElements()) 
                          {

                              try 
                              {

                                  String errorString = e.nextElement().toString();
                                  String numberString = errorString.substring(errorString.indexOf("(")+1,errorString.indexOf(")"));
                                  if(errorString.endsWith("Unable to Search DataMap for extra WMEs due to parse error")) 
                                  {

                                      vecErrors.add(
                                          new FeedbackListObject(current,Integer.parseInt(numberString),errorString,true,true));
                                      feedbackList.setListData(vecErrors);
                                      SwingUtilities.invokeLater(finish);
                                      return false;
                                  }
                              } catch(NumberFormatException nfe) 
                              {

                                  System.out.println("Never happen");
                              }
                          }
                      }
                      errors.clear();
                      updateProgressBar(++nodeNum);
                      SwingUtilities.invokeLater(update);

                  } // while parsing operator nodes

					feedbackList.setListData(vecErrors);
                    return true;
				}
				catch (IOException ioe) 
                {

					ioe.printStackTrace();
				}
                return false;  // should never get here unless try failed
			}     // end of checkNodes()


            /**
             *  Search through the datamap and look for extra WMEs
             *  by looking at the status of the named edge (as determined
             *  by the check nodes function) and the edge's location within
             *  the datamap.
             *  Extra WMEs are classified in this action by being tested by some production
             *  within the project, but never created by any production;
             *  not including any item within the input-link.
             *  Send results to the feedback list
             */
            public void searchDataMap() 
            {

                OperatorNode operatorNode;
                vecErrors.clear();    // clear out errors just in case
                vecErrors.add("Attributes that were never tested and never created in the productions of this agent:");

                // Go through all of the nodes examining the nodes' datamap if they have one
                Enumeration bfe = operatorWindow.breadthFirstEnumeration();
                while(bfe.hasMoreElements()) 
                {

                    operatorNode = (OperatorNode)bfe.nextElement();
                    operatorNode.searchNoTestNoCreateDataMap(operatorWindow.getDatamap(), vecErrors);
                    updateProgressBar(++nodeNum);
                    SwingUtilities.invokeLater(update);
                }   // end of while looking at all nodes

                // do case of no errors
                if(vecErrors.size() == 1) 
                {

                    vecErrors.clear();
                    vecErrors.add("No errors.  All attributes were either tested or created.");
                }

                // Add list of errors to feedback list
                feedbackList.setListData(vecErrors);
            }     // end of searchDataMap()

		}   // end of UpdateThread class
	}   // end of SearchDataMapNoTestNoCreateAction


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
                          feedbackList.setListData(vecErrors);
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

  			super("Find In Project");
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

            super("Replace In Project");
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

            super("Re-tile Windows");
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
		
