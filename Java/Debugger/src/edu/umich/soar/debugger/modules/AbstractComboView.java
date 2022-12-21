/********************************************************************************************
 *
 * AbstractComboView.java
 *
 * Description:
 *
 * Created on 	Mar 29, 2005
 * @author 		Douglas Pearson
 *
 * Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
 ********************************************************************************************/
package edu.umich.soar.debugger.modules;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.KeyAdapter;
import org.eclipse.swt.events.KeyEvent;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.FormAttachment;
import org.eclipse.swt.layout.FormData;
import org.eclipse.swt.layout.FormLayout;
import org.eclipse.swt.widgets.Combo;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Event;
import org.eclipse.swt.widgets.Listener;

import sml.Agent;
import sml.smlAgentEventId;
import sml.smlPrintEventId;
import sml.smlRunEventId;
import sml.smlSystemEventId;
import edu.umich.soar.debugger.MainFrame;
import edu.umich.soar.debugger.doc.Document;
import edu.umich.soar.debugger.general.JavaElementXML;
import edu.umich.soar.debugger.helpers.CommandHistory;
import edu.umich.soar.debugger.helpers.FormDataHelper;
import edu.umich.soar.debugger.manager.Pane;

/********************************************************************************************
 *
 * This is a base class designed to part of a view. This part handles the combo
 * box. Another derived class handles the display of those commands which may be
 * in text or something else.
 *
 ********************************************************************************************/
public abstract class AbstractComboView extends AbstractUpdateView implements
        Agent.PrintEventInterface
{
    protected Composite m_ComboContainer;

    protected Combo m_CommandCombo;

    /** If true, clear the text window each time we execute a command. */
    protected boolean m_ClearEachCommand = true;

    /**
     * If true, we clear the command entry (combo box) line each time we execute
     * a command
     */
    protected boolean m_ClearComboEachCommand = false;

    /** If true, will display output from "run" commands */
    protected boolean m_ShowTraceOutput = false;

    /** If true, will display output from "run" commands */
    protected boolean m_ShowEchoOutput = false;

    /** Prompt the user to type commands in the combo box with this text */
    protected String m_PromptForCommands;

    /**
     * If true, the combo box is at the top of the window (otherwise at the
     * bottom)
     */
    protected boolean m_ComboAtTop = true;

    /**
     * Record a copy of the text in the combo box. We do this so we can look up
     * this value when executing in a non-UI thread.
     */
    protected String m_CurrentCommand;

    /** The history of commands for this window */
    protected CommandHistory m_CommandHistory = new CommandHistory();

    protected long m_PrintCallback = -1;

    protected long m_EchoCallback = -1;

    /**
     * the position in the history list the up/down scrolling is at (SBW,
     * 7/1/08)
     **/
    protected int m_PositionInHistory = -1;

    /**
     * for history scrolling: save the command in progress before we replace it
     * with the most recent history: so user can hit up to see last command, but
     * restore what was there if down is hit again (this is essentially
     * history[-1]) SBW 7/1/08
     */
    protected String m_UnexecutedNewCommand = "";

    // The constructor must take no arguments so it can be called
    // from the loading code and the new window dialog
    public AbstractComboView()
    {
    }

    /************************************************************************
     *
     * Returns true if this window can display output from commands executed
     * through the "executeAgentCommand" method.
     *
     *************************************************************************/
    @Override
    public boolean canDisplayOutput()
    {
        return true;
    }

    /**
     * Windows that do auto-updates can be usefully primed with an initial
     * command (e.g. print --stack) when defining a default behavior
     */
    public void setInitialCommand(String command)
    {
        m_CommandHistory.UpdateHistoryList(command, true);
        makeComboBoxMatchHistory(!m_ClearComboEachCommand);
        m_CurrentCommand = command;
    }

    /**
     * Separate out the laying out of the combo box as we might want to put
     * controls next to it
     */
    protected void layoutComboBar(boolean top)
    {
        m_ComboContainer.setLayout(new FillLayout());
        FormData comboData = top ? FormDataHelper.anchorTop(0) : FormDataHelper
                .anchorBottom(0);
        m_ComboContainer.setLayoutData(comboData);
    }

    /** Layout the combo box and the main display window */
    protected void layoutControls()
    {
        // I'll use forms everywhere for consistency and so it's easier
        // to extend them later if we wish to add something.
        m_Container.setLayout(new FormLayout());

        if (!this.m_ComboAtTop)
        {
            FormData attachBottom = FormDataHelper.anchorFull(0);
            attachBottom.bottom = new FormAttachment(m_ComboContainer);

            getDisplayWindow().setLayoutData(attachBottom);
            layoutComboBar(m_ComboAtTop);
        }
        else
        {
            FormData attachTop = FormDataHelper.anchorFull(0);
            attachTop.top = new FormAttachment(m_ComboContainer);

            getDisplayWindow().setLayoutData(attachTop);
            layoutComboBar(m_ComboAtTop);
        }

        m_Container.layout();
        m_ComboContainer.layout();

        // Create a context menu for m_Text.
        // It will be filled in via a call to fillInContextMenu when the menu is
        // popped up
        // (this allows for dynamic content)
        createContextMenu(getDisplayControl());
    }

    /********************************************************************************************
     *
     * Scroll the display control to the bottom
     *
     ********************************************************************************************/
    public abstract void scrollBottom();

    /********************************************************************************************
     *
     * Initialize this window and its children. Should call setValues() at the
     * start to complete initialization of the abstract view.
     *
     ********************************************************************************************/
    public void init(MainFrame frame, Document doc, Pane parentPane)
    {
        setValues(frame, doc, parentPane);
        Composite parent = parentPane.getWindow();

        // The container lets us control the layout of the controls
        // within this window
        m_Container = new Composite(parent, SWT.NULL);

        m_ComboContainer = new Composite(m_Container, 0);
        m_CommandCombo = new Combo(m_ComboContainer, 0);

        if (m_PromptForCommands != null)
            m_CommandCombo.setText(m_PromptForCommands);

        m_CommandCombo.addKeyListener(new KeyAdapter()
        {
            // Listen for key presses on the combo box so we know when the user
            // presses return
            public void keyPressed(KeyEvent e)
            {
                comboKeyPressed(e);
            }
        });

        // When we scroll up and down the text is selected. We don't want that
        // as it makes it harder to modify an earlier command.
        // So clear the selection explicitly.
        m_CommandCombo.addSelectionListener(new SelectionAdapter()
        {
            public void widgetSelected(SelectionEvent e)
            {
                m_CommandCombo.clearSelection();
            }
        });

        // Listen for changes to the text in the combo box so we can keep track
        // of what's currently entered
        m_CommandCombo.addModifyListener(this::comboTextModified);

        // Decide how many rows to show in the combo list
        m_CommandCombo
                .setVisibleItemCount(Math.min(CommandHistory.kMaxHistorySize, 10));

        // Listen for when this window is disposed and unregister for anything
        // we registered for
        m_Container.addDisposeListener(e -> removeListeners());

        m_Updating = false;

        // Create the control that will display output from the commands
        createDisplayControl(m_Container);

        getDisplayControl().setBackground(getBackgroundColor());

        // Listen for Ctrl-V and execute our paste method (so paste goes to
        // command buffer, not into the window directly)
        getDisplayControl().addListener(SWT.KeyDown, m_ControlV);

        // When the user presses TAB in a trace window, set the focus to the
        // combo box (for command entry)
        // (Figuring out how to do this correctly required a lot of stepping
        // through the SWT code).
        getDisplayControl().addListener(SWT.Traverse, m_Tab);

        layoutControls();
    }

    // If the user presses TAB in the display control (text window) set the
    // focus to the combo box directly
    // and suppress the normal tab selection logic. Listening for the
    // SWT.TRAVERSE_TAB_NEXT event (through event.type)
    // is too late in the sequence it turns out. That's why we listen for
    // SWT.Traverse and then look at the detail.
    protected Listener m_Tab = new Listener()
    {
        public void handleEvent(Event e)
        {
            if (e.detail == SWT.TRAVERSE_TAB_NEXT)
            {
                m_CommandCombo.setFocus();

                // Suppress the normal tab logic
                e.doit = false;
            }
        }
    };

    /************************************************************************
     *
     * Set the focus to this window so the user can type commands easily. Return
     * true if this window wants the focus.
     *
     *************************************************************************/
    @Override
    public boolean setFocus()
    {
        // For us, we focus on the combo box, where the user types commands.
        m_CommandCombo.setFocus();

        return true;
    }

    @Override
    public boolean hasFocus()
    {
        return m_CommandCombo.isFocusControl()
                || getDisplayControl().isFocusControl();
    }

    private void comboKeyPressed(KeyEvent e)
    {
        Combo combo = (Combo) e.getSource();

        // This character may be platform specific...I wonder how we look it up
        // from SWT?
        if (e.character == '\r')
        {
            String command = combo.getText();
            commandEntered(command, true);
        }
        // override ComboBox's default up/down behavior (which is platform
        // specific)
        // to make it behave like a command prompt: up goes back in history,
        // down forward
        // SBW, 7/1/08
        else if (e.keyCode == SWT.ARROW_UP)
        {
            String[] history = m_CommandHistory.getHistory();

            if (m_PositionInHistory == -1)
            { // can't be below -1
                // currently not viewing history,
                // save away the command here so it can be restored
                m_UnexecutedNewCommand = combo.getText();
            }

            if (m_PositionInHistory < (history.length - 1))
            {
                m_PositionInHistory++; // can't increment past the end (minus 1
                                       // above)
                m_CommandCombo.setItems(history);
                m_CommandCombo.setText(history[m_PositionInHistory]);
                moveComboCursorToEnd();
                m_CurrentCommand = m_CommandCombo.getText();
            }
            // otherwise (not enough history), up is ignored

            e.doit = false; // don't apply the default combobox behavior
        }
        else if (e.keyCode == SWT.ARROW_DOWN)
        {
            String[] history = m_CommandHistory.getHistory();
            if (m_PositionInHistory > 0)
            {
                m_PositionInHistory--; // can't decrement below 0
                m_CommandCombo.setItems(history);
                m_CommandCombo.setText(history[m_PositionInHistory]);
                moveComboCursorToEnd();
                m_CurrentCommand = m_CommandCombo.getText();
            }
            else if (m_PositionInHistory == 0)
            {
                m_PositionInHistory--; // -1 is legal, it means a new command
                m_CommandCombo.setItems(history);
                m_CommandCombo.setText(m_UnexecutedNewCommand);
                moveComboCursorToEnd();
                m_CurrentCommand = m_CommandCombo.getText();
            }
            // otherwise down is ignored

            e.doit = false; // don't apply the default combobox behavior
        }

    }

    private void moveComboCursorToEnd()
    {
        // set the cursor at the far right of the text in the box
        // SBW, 7/1/08
        int position = m_CommandCombo.getText().length();
        m_CommandCombo.setSelection(new Point(position, position));
    }

    private void comboTextModified(ModifyEvent e)
    {
        Combo combo = (Combo) e.getSource();
        m_CurrentCommand = combo.getText();
        // System.out.println("Current command is " + m_CurrentCommand) ;
    }

    private void makeComboBoxMatchHistory(boolean placeTopItemInCombo)
    {
        // Changing the list of items clears the text entry field
        // which we may not wish to do, so we'll keep it and manually
        // reset it.
        String text = m_CommandCombo.getText();

        String[] history = m_CommandHistory.getHistory();
        this.m_CommandCombo.setItems(history);

        if (placeTopItemInCombo && history[0] != null)
            m_CommandCombo.setText(history[0]);
        else
            m_CommandCombo.setText(text);

        // Make sure our cache of what's stored in the combo box is up to date
        m_CurrentCommand = m_CommandCombo.getText();
    }

    private String lookupCommand(String originalCommand)
    {
        // !! => Get last command
        if (originalCommand.equals("!!"))
        {
            return m_CommandHistory.getMostRecent();
        }

        return m_CommandHistory.getMatch(originalCommand.substring(1));
    }

    private void commandEntered(String command, boolean updateHistory)
    {
        // If the command starts with "!" interpret it in the "shell" style,
        // matching it against the command history
        if (command.length() >= 1 && command.charAt(0) == '!')
        {
            command = lookupCommand(command);
        }

        // Update the combo box history list
        if (updateHistory)
        {
            this.m_CommandHistory.UpdateHistoryList(command, true);

            // Update the combo box to match the history list
            makeComboBoxMatchHistory(false);
        }

        // Clear the text from the edit control in the combo box if we're
        // configured that way.
        if (m_ClearComboEachCommand)
        {
            this.m_CommandCombo.setText("");
            m_CurrentCommand = "";
        }
        else
        {
            // SBW 7/1/08: prevent cursor from jumping left if there is still
            // text in the box
            moveComboCursorToEnd();
        }

        // clear command history scrolling state (SBW 7/1/08)
        m_UnexecutedNewCommand = "";
        m_PositionInHistory = -1;

        // Send the command to Soar and echo into the trace
        if (command.length() > 0)
            executeAgentCommand(command, true);
        else
            m_Updating = false;
    }

    /************************************************************************
     *
     * Converts this object into an XML representation.
     *
     *************************************************************************/
    @Override
    public edu.umich.soar.debugger.general.JavaElementXML convertToXML(
            String title, boolean storeContent)
    {
        JavaElementXML element = new JavaElementXML(title);

        // It's useful to record the class name to uniquely identify the type
        // of object being stored at this point in the XML tree.
        Class<? extends AbstractComboView> cl = this.getClass();
        element.addAttribute(JavaElementXML.kClassAttribute, cl.getName());

        if (m_Name == null)
            throw new IllegalStateException(
                    "We've created a view with no name -- very bad");

        // Store this object's properties.
        element.addAttribute("Name", m_Name);
        element.addAttribute("UpdateOnStop", Boolean.toString(m_UpdateOnStop));
        element.addAttribute("ClearEachCommand", Boolean
                .toString(m_ClearEachCommand));
        element.addAttribute("ClearComboEachCommand", Boolean
                .toString(this.m_ClearComboEachCommand));
        element.addAttribute("ComboAtTop", Boolean.toString(this.m_ComboAtTop));
        element.addAttribute("ShowTraceOutput", Boolean
                .toString(m_ShowTraceOutput));
        element.addAttribute("UpdateEveryNthDecision", Integer
                .toString(m_UpdateEveryNthDecision));

        if (storeContent)
            storeContent(element);

        if (this.m_CommandCombo != null)
        {
            String command = m_CommandCombo.getText();
            if (command != null && command.length() != 0)
                this.m_CommandHistory.UpdateHistoryList(command, true);
        }

        element.addChildElement(this.m_CommandHistory.ConvertToXML("History"));
        element.addChildElement(this.m_Logger.convertToXML("Logger"));

        return element;
    }

    /************************************************************************
     *
     * Rebuild the object from an XML representation.
     *
     * @param frame
     *            The top level window that owns this window
     * @param doc
     *            The document we're rebuilding
     * @param parent
     *            The pane window that owns this view
     * @param element
     *            The XML representation of this command
     *
     *************************************************************************/
    @Override
    public void loadFromXML(MainFrame frame,
            edu.umich.soar.debugger.doc.Document doc, Pane parent,
            edu.umich.soar.debugger.general.JavaElementXML element)
            throws Exception
    {
        setValues(frame, doc, parent);

        m_Name = element.getAttribute("Name");
        m_UpdateOnStop = element.getAttributeBooleanThrows("UpdateOnStop");
        m_ClearEachCommand = element
                .getAttributeBooleanThrows("ClearEachCommand");
        m_ClearComboEachCommand = element
                .getAttributeBooleanThrows("ClearComboEachCommand");
        m_ComboAtTop = element.getAttributeBooleanThrows("ComboAtTop");
        m_ShowTraceOutput = element
                .getAttributeBooleanThrows("ShowTraceOutput");
        m_UpdateEveryNthDecision = element
                .getAttributeIntThrows("UpdateEveryNthDecision");

        JavaElementXML history = element.findChildByName("History");
        if (history != null)
            this.m_CommandHistory.LoadFromXML(doc, history);

        JavaElementXML log = element.findChildByName("Logger");
        if (log != null)
            this.m_Logger.loadFromXML(doc, log);

        // Register that this module's name is in use
        frame.registerViewName(m_Name, this);

        // Actually create the window
        init(frame, doc, parent);

        // Reset the combo box to match the history list we just loaded
        makeComboBoxMatchHistory(!m_ClearComboEachCommand);

        // Restore the text we saved (if we chose to save it)
        restoreContent(element);
    }

    protected void updateNow()
    {
        if (m_Updating)
            return;

        // System.out.println("Updating window's contents") ;

        // Retrieve the current command in the combo box
        // (We use a cached value so we don't need to go to the UI thread)
        final String command = getCurrentCommand();

        // We don't want to execute "run" commands when Soar stops--or we'll get
        // into an
        // infinite loop.
        if (!getDocument().getSoarCommands().isRunCommand(command))
        {
            m_Updating = true;

            // If Soar is running in the UI thread we can make
            // the update directly.
            if (!Document.kDocInOwnThread)
                commandEntered(command, false);
            else
            {
                // Have to make update in the UI thread.
                // Callback comes in the document thread.
                Display.getDefault().asyncExec(() -> commandEntered(command, false));
            }
        }
    }

    /************************************************************************
     *
     * Execute a command (send it to Soar) and display the output in a manner
     * appropriate to this view.
     *
     * @param Command
     *            The command line to execute
     * @param echoCommand
     *            If true, display the command in the output window as well.
     *
     *            The result (if any) is also returned to the caller.
     *
     *************************************************************************/
    @Override
    public String executeAgentCommand(String command, boolean echoCommand)
    {
        // Check to see if this is a local command (handled directly by the
        // debugger not Soar)
        // We now allow for command line expansion of this too, so we can
        // support aliases
        String expanded = m_Frame.getExpandedCommand(command);
        expanded = expanded.trim();
        if (m_Frame.isDebuggerCommand(expanded))
        {
            m_Updating = false;
            String result = m_Frame.executeDebuggerCommand(this, expanded,
                    echoCommand);
            if (result != null)
            {
                return result;
            }
        }

        if (echoCommand && !m_ClearEachCommand)
        {
            appendTextSafely(kLineSeparator + command);
        }

        String result = getDocument()
                .sendAgentCommand(getAgentFocus(), command);

        if (this.m_Container.isDisposed())
            return "Window closed";

        // Clear the text area if this window is configured that way.
        if (this.m_ClearEachCommand)
            clearDisplay();

        // Scroll the display window to the bottom so we can see the new
        // command and any output
        scrollBottom();

        // Output from Soar doesn't include newlines and assumes that we insert
        // a newline before the result of the command is displayed.
        if (result != null && result.length() > 0)
            appendTextSafely(kLineSeparator + result);

        m_Updating = false;

        return result;
    }

    /************************************************************************
     *
     * Display the given text in this view (if possible).
     *
     * This method is used to programmatically insert text that Soar doesn't
     * generate into the output window.
     *
     *************************************************************************/
    @Override
    public void displayText(String text)
    {
        appendTextSafely(text);
    }

    /************************************************************************
     *
     * Add the text to the view (this method assumes always called from UI
     * thread)
     *
     *************************************************************************/
    protected abstract void appendText(String text);

    /************************************************************************
     *
     * Add the text to the view in a thread safe way (switches to UI thread)
     *
     *************************************************************************/
    protected void appendTextSafely(final String text)
    {
        // If Soar is running in the UI thread we can make
        // the update directly.
        if (!Document.kDocInOwnThread)
        {
            appendText(text);
            return;
        }

        // Have to make update in the UI thread.
        // Callback comes in the document thread.
        Display.getDefault().asyncExec(() -> appendText(text));
    }

    private String getCurrentCommand()
    {
        return m_CurrentCommand;
    }

    public void printEventHandler(int eventID, Object data, Agent agent,
            String message)
    {
        if (getDisplayControl().isDisposed())
        {
            System.out
                    .println("Naughty -- we're still registered for the print event although our window has been closed: agent "
                            + agent.GetAgentName());
            return;
        }

        if (eventID == smlPrintEventId.smlEVENT_PRINT.swigValue()
                || eventID == smlPrintEventId.smlEVENT_ECHO.swigValue())
            appendTextSafely(message);
    }

    @Override
    protected void registerForAgentEvents(Agent agent)
    {
        if (agent == null)
            return;

        if (m_StopCallback == -1)
        {
            // Update on stop, init-soar or when we make a new connection
            m_StopCallback = agent.GetKernel().RegisterForSystemEvent(
                    smlSystemEventId.smlEVENT_SYSTEM_STOP, this, this);
            m_InitCallback = agent.GetKernel().RegisterForAgentEvent(
                    smlAgentEventId.smlEVENT_AFTER_AGENT_REINITIALIZED, this,
                    this);
        }

        if (m_ShowTraceOutput && m_PrintCallback == -1)
            m_PrintCallback = agent.RegisterForPrintEvent(
                    smlPrintEventId.smlEVENT_PRINT, this, this);

        // If we issue a command which is echoed, ignore that echo event.
        // Otherwise we'll get 2 sets of output from that command.
        boolean ignoreOwnEchos = true;
        if (m_ShowEchoOutput && m_EchoCallback == -1)
            m_EchoCallback = agent.RegisterForPrintEvent(
                    smlPrintEventId.smlEVENT_ECHO, this, this, ignoreOwnEchos);

        if (m_UpdateEveryNthDecision > 0 && m_DecisionCallback == -1)
            m_DecisionCallback = agent.RegisterForRunEvent(
                    smlRunEventId.smlEVENT_AFTER_DECISION_CYCLE, this, this);
        else if (m_UpdateEveryNthDecision <= 0 && m_DecisionCallback != -1)
        {
            // This is a bit naughty, but it's helpful to unregister for the
            // decision event
            // if we no longer are interested in updates every n decisions.
            // This allows us to just call "registerForAgentEvents" again after
            // updating the properties
            // and get the right result. We don't currently worry about the
            // other events in the same way as
            // they won't generate as much traffic.
            agent.UnregisterForRunEvent(m_DecisionCallback);
            m_DecisionCallback = -1;
        }

        registerForViewAgentEvents(agent);

        // If this is a new remote connection, update windows marked for auto
        // updating.
        if (m_UpdateOnStop && m_Document.isConnected() && m_Document.isRemote())
            updateNow();
    }

    /**
     * Agent gone, so clear any callback references we have (we can't unregister
     * because agent object already destroyed)
     */
    @Override
    protected void clearAgentEvents()
    {
        m_PrintCallback = -1;
        m_EchoCallback = -1;
        super.clearAgentEvents();
    }

    @Override
    protected void unregisterForAgentEvents(Agent agent)
    {
        if (agent == null)
            return;

        boolean ok = true;

        if (m_StopCallback != -1)
        {
            ok = agent.GetKernel().UnregisterForSystemEvent(m_StopCallback);
        }

        if (m_InitCallback != -1)
            ok = agent.GetKernel().UnregisterForAgentEvent(m_InitCallback)
                    && ok;

        if (m_PrintCallback != -1)
            ok = agent.UnregisterForPrintEvent(m_PrintCallback) && ok;

        if (m_EchoCallback != -1)
            ok = agent.UnregisterForPrintEvent(m_EchoCallback) && ok;

        if (m_DecisionCallback != -1)
            ok = agent.UnregisterForRunEvent(m_DecisionCallback) && ok;

        m_StopCallback = -1;
        m_InitCallback = -1;
        m_PrintCallback = -1;
        m_DecisionCallback = -1;
        m_EchoCallback = -1;

        ok = unregisterForViewAgentEvents(agent) && ok;

        if (!ok)
            throw new IllegalStateException("Problem unregistering for events");
    }
}
