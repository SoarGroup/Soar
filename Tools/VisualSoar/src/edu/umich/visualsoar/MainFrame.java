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
import sml.Agent;
import sml.Kernel;
import sml.smlStringEventId;
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
public class MainFrame extends JFrame implements Kernel.StringEventInterface
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
    String lastWindowViewOperation = "none"; // can also be "tile" or "cascade"

    private JMenu soarRuntimeMenu = null;
	private JMenu soarRuntimeAgentMenu = null;
	
////////////////////////////////////////
// Access to data members
////////////////////////////////////////
	
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
	PerformableAction verifyProjectAction = new VerifyProjectAction();
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
    Kernel m_Kernel = null ;
    String m_ActiveAgent = null ;
    int    m_EditProductionCallback = -1 ;
	// Menu handlers for STI init, term, and "Send Raw Command"
	Action soarRuntimeInitAction = new SoarRuntimeInitAction();
	Action soarRuntimeTermAction = new SoarRuntimeTermAction();
	Action soarRuntimeSendRawCommandAction = new SoarRuntimeSendRawCommandAction();
	Action soarRuntimeSendAllFilesAction = new SendAllFilesToSoarAction() ;

	public Kernel getKernel()			{ return m_Kernel ; }
	public String getActiveAgentName()  { return m_ActiveAgent ; }

	public Agent getActiveAgent()
	{
		if (m_Kernel == null || m_ActiveAgent == null)
			return null ;
		return m_Kernel.GetAgent(m_ActiveAgent) ;
	}

	public void reportResult(String result)
	{
		String lines[] = result.split("\n") ;
		
		Vector v = new Vector();
		
		for (int i = 0 ; i < lines.length ; i++)
			v.add(lines[i]) ;
		
		setFeedbackListData(v);
	}
	
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
            JOptionPane.showMessageDialog(MainFrame.this,
                                          IOE.getMessage(),
                                          "I/O Error Reading " + file.getName(),
                                          JOptionPane.ERROR_MESSAGE);
        }
        catch(java.beans.PropertyVetoException pve)
        {
            //No sweat. This just means the new window failed to get focus.
        }
    }//OpenFile()

    /**
     * When the Soar Runtime|Agent menu is selected, this listener
     * populates the menu with the agents that exist in the kernel
     * we're currently connected to through SML.
     * @Author ThreePenny
     */
	class SoarRuntimeAgentMenuListener extends MenuAdapter
	
    {
		public void menuSelected(MenuEvent e)
		
        {
			// Remove our existing items
			soarRuntimeAgentMenu.removeAll();

			// Check to see if we have a connection
			if (m_Kernel == null)
            {
				// Add a "not connected" menu item
				JMenuItem menuItem=new JMenuItem("<not connected>");
				menuItem.setEnabled(false);
				
				soarRuntimeAgentMenu.add(menuItem);
				return;
			}
			
			// Get the connection names
			int nAgents = m_Kernel.GetNumberAgents() ;

			// If we don't have any connections then display the
			// appropriate menu item.
			if (nAgents == 0)
			
            {
				// Add the "no agents" menu item
				JMenuItem menuItem=new JMenuItem("<no agents>");
				menuItem.setEnabled(false);
				
				soarRuntimeAgentMenu.add(menuItem);
				return;
			}
			
			// Add each name
			int i;
			for (i=0; i < nAgents; i++)
			
            {
				// Get this agent's name
				sml.Agent agent = m_Kernel.GetAgentByIndex(i) ;

				if (agent == null)
					continue ;
				
				String name = agent.GetAgentName() ;
								
				// Create the connection menu and add a listener to it
				// which contains the connection index.
				JRadioButtonMenuItem connectionMenuItem=new JRadioButtonMenuItem(name);

				if (name.equals(m_ActiveAgent))
					connectionMenuItem.setSelected(true) ;
				
				connectionMenuItem.addActionListener(
	                    new AgentConnectionActionListener(connectionMenuItem, name));
					
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
		private JRadioButtonMenuItem	m_assocatedMenuItem;
		
		// Constructor
		public AgentConnectionActionListener(JRadioButtonMenuItem assocatedMenuItem, String sAgentConnectionName)
		
        {
			m_sAgentConnectionName = sAgentConnectionName;
			m_assocatedMenuItem = assocatedMenuItem;
		}
		
		// Disable the default constructor
		private AgentConnectionActionListener() {}
		
		// Called when the action has been performed
		public void actionPerformed(ActionEvent e)
		
        {
			m_ActiveAgent  = m_sAgentConnectionName ;
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
		soarRuntimeMenu = new JMenu("Soar Runtime");
		soarRuntimeMenu.setMnemonic('S');
		
		// Add the menu items
		JMenuItem connectMenuItem=soarRuntimeMenu.add(soarRuntimeInitAction);
		JMenuItem disconnectMenuItem=soarRuntimeMenu.add(soarRuntimeTermAction);
		JMenuItem sendAllFilesMenuItem=soarRuntimeMenu.add(soarRuntimeSendAllFilesAction);
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
		sendAllFilesMenuItem.setMnemonic('S');
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
     * Sets a new JInternalFrame's shape and position based upon the user's
     * apparent preference.
     */
    void setJIFShape(JInternalFrame newJIF)
    {
        JInternalFrame currJIF = DesktopPane.getSelectedFrame();
        if ( (currJIF != null) && (currJIF.isMaximum()) )
        {
            try
            {
                newJIF.setMaximum(true);
            }
            catch (java.beans.PropertyVetoException pve)
            {
                //Unable to maximize window.  Oh well.
            }				
        }
		else if ( (Preferences.getInstance().isAutoTilingEnabled())
                  || (lastWindowViewOperation.equals("tile")) )
        {
			DesktopPane.performTileAction();
		}
        else if (lastWindowViewOperation.equals("cascade"))
        {
            DesktopPane.performCascadeAction();
		}
        else if (currJIF != null)
        {
            Rectangle bounds = currJIF.getBounds();
            newJIF.reshape(bounds.x + 30,
                           bounds.y + 30,
                           bounds.width,
                           bounds.height);
        }
        else
        {
            newJIF.reshape(0, 0 , 400, 400);
        }

    }//setJIFShape
    
	/**
	 * Creates a rule window opening with the given file name
	 * @param re the ruleeditor file that the rule editor should open
	 */
	public void addRuleEditor(RuleEditor re) 
    {
		DesktopPane.add(re);
		re.moveToFront();
		DesktopPane.revalidate();
        setJIFShape(re);
	}

	/**
	 * Creates a datamap window with the given datamap.
	 * @param dm the datamap that the window should open
     * @see DataMap
	 */
	public void addDataMap(DataMap dm) 
    {
        if (!DesktopPane.hasDataMap(dm))
        {
            DesktopPane.add(dm);
            DesktopPane.revalidate();
            setJIFShape(dm);
        }
        else
        {
            dm = DesktopPane.dmGetDataMap(dm.getId());
        }

        try
        {
            dm.setSelected(true);
            dm.setIcon(false);
            dm.moveToFront();
        }
		catch (java.beans.PropertyVetoException pve)
        {
            //no sweat.
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
            RuleEditor re = null;
            
			try 
            {
				JInternalFrame[] jif = DesktopPane.getAllFrames();
				for(int i = 0; i < jif.length; ++i) 
                {
					if(jif[i] instanceof RuleEditor) 
                    {
						re = (RuleEditor)jif[i];
						re.write();
					}
				}
			}
			catch(java.io.IOException ioe) 
            {
				JOptionPane.showMessageDialog(MainFrame.this,
                                              ioe.getMessage(),
                                              "I/O Error",
                                              JOptionPane.ERROR_MESSAGE);
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
            File file = null;
            
			try 
            {
            			//FIXME: This is totally ghetto
            			// Using a JFileChooser seems to cause hangs on OS X (10.4, at least)
            			//  so I've converted the code to use a FileDialog instead
            			// Unfortunately, FilenameFilters don't work on Windows XP, so I have
            			//  to set the file to *.vsa.  Yuck.
            			
				//JFileChooser fileChooser = new JFileChooser();
				//fileChooser.setFileFilter(new SoarFileFilter());
				//fileChooser.setCurrentDirectory(Preferences.getInstance().getOpenFolder());
				//int state = fileChooser.showOpenDialog(MainFrame.this);
				//file = fileChooser.getSelectedFile();
				FileDialog fileChooser = new FileDialog(MainFrame.this, "Open Project", FileDialog.LOAD);
				fileChooser.setFilenameFilter(new FilenameFilter() {
					public boolean accept(File dir, String name) {
						return name.toLowerCase().endsWith("vsa");
					}
				});
				fileChooser.setFile("*.vsa");
				fileChooser.setVisible(true);
				if(fileChooser.getFile() != null) {
				file = new File(fileChooser.getDirectory(), fileChooser.getFile());
				//if (file != null && state == JFileChooser.APPROVE_OPTION) 
				if (file != null) 
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

                    //Verify project integrity
                    verifyProjectAction.perform();

                    //Set the title bar to include the project name
                    setTitle(file.getName().replaceAll(".vsa", ""));
				}
			}
			}
			catch(FileNotFoundException fnfe)
            {
				JOptionPane.showMessageDialog(MainFrame.this,
                                              fnfe.getMessage(),
                                              "File Not Found",
                                              JOptionPane.ERROR_MESSAGE);
			}
			catch(IOException ioe) 
            {
				JOptionPane.showMessageDialog(MainFrame.this,
                                              ioe.getMessage(),
                                              "I/O Exception",
                                              JOptionPane.ERROR_MESSAGE);
				ioe.printStackTrace();
			}
			catch(NumberFormatException nfe) 
            {
                nfe.printStackTrace();
                JOptionPane.showMessageDialog(MainFrame.this,
                                              "Error Reading File, Data Incorrectly Formatted",
                                              "Bad File",
                                              JOptionPane.ERROR_MESSAGE);
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
                JOptionPane.showMessageDialog(MainFrame.this,
                                              "Error Reading File, Data Incorrectly Formatted",
                                              "Bad File",
                                              JOptionPane.ERROR_MESSAGE);
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
			newAgentDialog.setVisible(true);
			if (newAgentDialog.wasApproved()) 
            {
                //Verify that the path exists
				String path = newAgentDialog.getNewAgentPath();
                File pathFile = new File(path);
                if (! pathFile.exists())
                {
                    int choice = JOptionPane.showConfirmDialog(
                        getMainFrame(),
                            path + " does not exist.\nShould I create it for you?",
                            path + " Does Not Exist",
                            JOptionPane.OK_CANCEL_OPTION);

                    if (choice == JOptionPane.CANCEL_OPTION)
                    {
                        return;
                    }

                    pathFile.mkdirs();
                }//if

                //Verify that the project doesn't already exist
				String agentName = newAgentDialog.getNewAgentName();
				String agentFileName = path + File.separator + agentName + ".vsa";
                File agentNameFile = new File(agentFileName);
                if (agentNameFile.exists())
                {
                    JOptionPane.showMessageDialog(
                        getMainFrame(),
                        agentName + " already exists. Please try again with a different project name or path.",
                        agentName + " already exists!",
                        JOptionPane.ERROR_MESSAGE);

                    return;
                }
                
				operatorWindow = new OperatorWindow(agentName,agentFileName,true);
				
				Preferences.getInstance().setOpenFolder(new File(path));
				operatorDesktopSplit.setLeftComponent(new JScrollPane(operatorWindow));
				
				projectActionsEnable(true);
                exportAgentAction.perform();
				
				operatorDesktopSplit.setDividerLocation(.30);

                //Set the title bar to include the project name
                setTitle(agentName);
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
				JOptionPane.showMessageDialog(MainFrame.this,
                                              exception.getMessage(),
                                              "Agent Export Error",
                                              JOptionPane.ERROR_MESSAGE);
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
            //File f = new File("./docs/KeyBindings.txt");
			File f = new File(Preferences.getVisualSoarFolder() + File.separator + "docs" + File.separator + "KeyBindings.txt") ;
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
     * Processes incoming STI commands from the runtime
     * @author ThreePenny
     */
	/*
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
	*/

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
			boolean ok = false ;
			String message = "Error connecting to remote kernel" ;
			
			try
			{
				ok = SoarRuntimeInit() ;
			}
			catch (Throwable ex)
			{
				System.out.println(ex.toString()) ;
				message = "Exception when initializing the SML library.  Check that sml.jar is on the path along with soar-library." ;
			}

			if (!ok)
            {
				JOptionPane.showMessageDialog(MainFrame.this,
											  message,
                                              "SML Connection Error",
                                              JOptionPane.ERROR_MESSAGE);
			}
		}	
	}

	public String stringEventHandler(int eventID, Object userData, Kernel kernel, String callbackData)
	{
		if (eventID == smlStringEventId.smlEVENT_EDIT_PRODUCTION.swigValue())
		{
			String prod = callbackData ;
	
			if (prod != null)
			{
				EditProductionByName(prod) ;
			}
		}
		return "" ;
	}

    /**
     * Initializes the Soar Tool Interface (STI) object and enabled/disables
     * menu items as needed.
     * 
     * @author ThreePenny
     */
	boolean SoarRuntimeInit() throws Throwable
	
    {
		if (m_Kernel != null)
		{
			SoarRuntimeTerm() ;
			m_Kernel = null ;
		}
		
		// This may throw if we can't find the SML libraries.
		m_Kernel = Kernel.CreateRemoteConnection(true, null) ;
		
		if (m_Kernel.HadError())
		{
			m_Kernel = null ;
			
			// Disable all related menu items
			soarRuntimeTermAction.setEnabled(false);
			soarRuntimeInitAction.setEnabled(true);	// Allow us to try again later
			soarRuntimeSendRawCommandAction.setEnabled(false);
			soarRuntimeAgentMenu.setEnabled(false);
			soarRuntimeSendAllFilesAction.setEnabled(false) ;
            
			return false ;
		}
		
		if (m_Kernel.GetNumberAgents() > 0)
		{
			// Select the first agent if there is any as our current agent
			m_ActiveAgent = m_Kernel.GetAgentByIndex(0).GetAgentName() ;
		}
		m_EditProductionCallback = m_Kernel.RegisterForStringEvent(smlStringEventId.smlEVENT_EDIT_PRODUCTION, this, null) ;
		
		soarRuntimeTermAction.setEnabled(true);
		soarRuntimeInitAction.setEnabled(false);
		soarRuntimeSendRawCommandAction.setEnabled(true);
		soarRuntimeAgentMenu.setEnabled(true);
		soarRuntimeSendAllFilesAction.setEnabled(true) ;
		
		return true ;
	}


    /**
     * Terminates the Soar Tool Interface (STI) object and enabled/disables
     * menu items as needed.
     * @author ThreePenny
     */
	void SoarRuntimeTerm()
	
    {
		try
		{
			if (m_Kernel != null)
				m_Kernel.delete() ;
		}
		catch (Throwable ex)
		{
			// Trouble shutting down.
			System.out.println(ex.toString()) ;
		}

		m_EditProductionCallback = -1 ;
		m_Kernel = null ;
		
		// Enable/Disable menu items
		soarRuntimeTermAction.setEnabled(false);
		soarRuntimeInitAction.setEnabled(true);
		soarRuntimeSendRawCommandAction.setEnabled(false);
		soarRuntimeAgentMenu.setEnabled(false);
		soarRuntimeSendAllFilesAction.setEnabled(false) ;
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
	
    // Handles the "Runtime|Send All Files" menu item
    class SendAllFilesToSoarAction extends AbstractAction
    {
        public SendAllFilesToSoarAction()
        {
            super("Send All Files");
			setEnabled(false);
        }

        public void actionPerformed(ActionEvent e)
        {
            // Get the agent
        	Agent agent = MainFrame.getMainFrame().getActiveAgent() ;
            if (agent == null)
            {
                JOptionPane.showMessageDialog(MainFrame.this,"Not connected to an agent.","Error",JOptionPane.ERROR_MESSAGE);
                return;
            }
            
            // Generate the path to the top level source file
            OperatorRootNode root = (OperatorRootNode)(operatorWindow.getModel().getRoot());
            
            if (root == null)
            {
            	System.out.println("Couldn't find the top level project node") ;
            	return ;
            }
            
            String projectFilename = root.getProjectFile() ;	// Includes .vsa
            
            // Swap the extension from .vsa to .soar
            projectFilename = projectFilename.replaceFirst(".vsa", ".soar") ;
            
            // Call source in Soar
            String result = agent.ExecuteCommandLine("source " + "\"" + projectFilename + "\"", true) ;
            
            if (!agent.GetLastCommandLineResult())
            	result = agent.GetLastErrorDescription() ;
            
			MainFrame.getMainFrame().reportResult(result) ;
        }
    }//class SendFileToSoarAction

	
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
			if (m_Kernel != null)
			{
				Agent agent = getActiveAgent() ;
				
				if (agent == null)
				{
					JOptionPane.showMessageDialog(MainFrame.getMainFrame(), "No agent is currently selected, so we can't send the command.\n\nPlease use the Connected Agent menu to select one.") ;
					return ;
				}
				
				SoarRuntimeSendRawCommandDialog theDialog = new SoarRuntimeSendRawCommandDialog(MainFrame.this, m_Kernel, getActiveAgent());
				theDialog.setVisible(true);
			}
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
			progressDialog.setVisible(true);
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
     * This action verifies that a project is intact.  Specifically
     * it checks that all the project's files are present and can
     * be loaded.
     */
	class VerifyProjectAction extends PerformableAction
    {
		public VerifyProjectAction() 
        {
			super("Verify Project Integrity");
			setEnabled(false);
		}

        public void perform()
        {
			Enumeration bfe = operatorWindow.breadthFirstEnumeration();
            Vector vecNodes = new Vector(10, 50);
			while(bfe.hasMoreElements())
            {
				vecNodes.add(bfe.nextElement());
			}
			(new VerifyProjectThread(vecNodes, "Verifiying Project...")).start();
        }
        
		public void actionPerformed(ActionEvent ae)
        {
            perform();
		}

        class VerifyProjectThread extends UpdateThread
        {
            public VerifyProjectThread(Vector v, String title)
            {
                super(v, title);
            }
                
            public boolean checkEntity(Object node) throws IOException
            {
                OperatorNode opNode = (OperatorNode)node;

                //Only file nodes need to be examined
                if ( ! (opNode instanceof FileNode))
                {
                    return false;
                }
                
                File f = new File(opNode.getFileName());
                if (!f.canRead())
                {
                    vecErrors.add("Error!  Project Corrupted:  Unable to open file: "
                                  + opNode.getFileName());
                    return true;
                }
                    
                if (!f.canWrite())
                {
                    vecErrors.add("Error!  Unable to write to file: "
                                  + opNode.getFileName());
                    return true;
                }

                //We lie and say there are errors no matter what so that
                //the "there were no errors..." message won't appear.
                return true;
            }
        }//class VerifyProjectThread
	
	}//class VerifyProjectAction

    
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
                    vecErrors.add(opNode.parseTokenMgrError(tme));
                    return true;
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
			progressDialog.setVisible(true);
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
                do 
                {
                    repCount++;
                    errors.clear();
                    bfe = operatorWindow.breadthFirstEnumeration();

                    while(bfe.hasMoreElements()) 
                    {
                        current = (OperatorNode)bfe.nextElement();

                        operatorWindow.generateDataMap(current, errors, vecErrors);

                        setFeedbackListData(vecErrors);
                        value = progressBar.getValue() + 1;
                        updateProgressBar(value);
                        SwingUtilities.invokeLater(update);
                    } // while parsing operator nodes
          
                } while(!(errors.isEmpty()) && repCount < 5);


                //Instruct all open datamap windows to display
                //the newly generated nodes
                JInternalFrame[] jif = DesktopPane.getAllFrames();
                for(int i = 0; i < jif.length; ++i) 
                {
                    if(jif[i] instanceof DataMap) 
                    {
                        DataMap dm = (DataMap)jif[i];
                        dm.displayGeneratedNodes();
                    }
                }

                SwingUtilities.invokeLater(finish);
                
			}//checkNodes

		}//class UpdateThread
	}//class GenerateDataMapAction


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
			FindInProjectDialog theDialog =
                new FindInProjectDialog(MainFrame.this,
                                        operatorWindow,
                                        (OperatorNode)operatorWindow.getModel().getRoot());
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
            ReplaceInProjectDialog replaceDialog =
                new ReplaceInProjectDialog(MainFrame.this,
                                           operatorWindow,
                                           (OperatorNode)operatorWindow.getModel().getRoot());
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
            spad.setVisible(true);

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
                        JOptionPane.showMessageDialog(MainFrame.this,
                                                      exception.getMessage(),
                                                      "Agent Export Error",
                                                      JOptionPane.ERROR_MESSAGE);
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

                    //Set the title bar to include the project name
                    setTitle(newName);
                    
                }
				else 
                {
					JOptionPane.showMessageDialog(MainFrame.this,
                                                  "That is not a valid name for the project",
                                                  "Invalid Name",
                                                  JOptionPane.ERROR_MESSAGE);				
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
            lastWindowViewOperation = "tile";
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
            lastWindowViewOperation = "tile";
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
            lastWindowViewOperation = "cascade";
        }
    }
}	
		
