/********************************************************************************************
 *
 * AbstractFixedView.java
 *
 * Description:
 *
 * Created on 	Sep 25, 2005
 * @author 		Douglas Pearson
 *
 * Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
 ********************************************************************************************/
package edu.umich.soar.debugger.modules;

import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Menu;

import sml.Agent;
import edu.umich.soar.debugger.MainFrame;
import edu.umich.soar.debugger.general.JavaElementXML;
import edu.umich.soar.debugger.manager.Pane;

/************************************************************************
 *
 * Fixed views generally don't do a whole lot (compared to variable size
 * trace/output windows), so we'll fill in default methods here to save each
 * subclass from providing them.
 *
 * If a class really wants to implement one of these methods directly they can
 * always override it.
 *
 ************************************************************************/
public abstract class AbstractFixedView extends AbstractView
{
    /********************************************************************************************
     *
     * Return true if this view shouldn't be user resizable. E.g. A text window
     * would return false but a bar for buttons would return true.
     *
     ********************************************************************************************/
    @Override
    public boolean isFixedSizeView()
    {
        return true;
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
        return false;
    }

    @Override
    public void setTextFont(Font f)
    {
        // We ignore this as our window doesn't display text.
    }

    /********************************************************************************************
     *
     * Copy/paste current selection to/from the clipboard.
     *
     ********************************************************************************************/
    @Override
    public void copy()
    {
        // Usually nothing to copy in fixed views
    }

    @Override
    public void selectAll() {
        // Usually nothing to select in fixed views
    }

    /************************************************************************
     *
     * Search for the next occurance of 'text' in this view and place the
     * selection at that point.
     *
     * @param text
     *            The string to search for
     * @param searchDown
     *            If true search from top to bottom
     * @param matchCase
     *            If true treat the text as case-sensitive
     * @param wrap
     *            If true continue search from the top after reaching bottom
     * @param searchHidden
     *            If true and this view has hidden text (e.g. unexpanded tree
     *            nodes) search that text
     *
     *************************************************************************/
    @Override
    public boolean find(String text, boolean searchDown, boolean matchCase,
                        boolean wrap, boolean searchHiddenText)
    {
        return false;
    }

    /************************************************************************
     *
     * Clear the display (the text part if any)
     *
     *************************************************************************/
    @Override
    public void clearDisplay()
    {
    }

    /************************************************************************
     *
     * Override and return false if it doesn't make sense to clear this type of
     * view.
     *
     *************************************************************************/
    @Override
    public boolean offerClearDisplay()
    {
        return false;
    }

    /************************************************************************
     *
     * Override and return false if it doesn't make sense to log the contexts of
     * this type of view and so we shouldn't offer it to the user in the context
     * menu.
     *
     *************************************************************************/
    @Override
    public boolean offerLogging()
    {
        return false;
    }

    @Override
    public String executeAgentCommand(String command, boolean echoCommand)
    {
        // Send the command to Soar but there's no where to display the output
        // so we just eat it.
        return getDocument()
                .sendAgentCommand(getAgentFocus(), command);
    }

    @Override
    public void displayText(String text)
    {
        // Nowhere to display it so eat it.
    }

    /************************************************************************
     *
     * Set the focus to this window so the user can type commands easily. Return
     * true if this window wants the focus.
     *
     *************************************************************************/
    @Override
    public boolean setFocus()
    {
        return false;
    }

    @Override
    public boolean hasFocus()
    {
        return false;
    }

    /************************************************************************
     *
     * Register and unregister for Soar events for this agent. (E.g. a trace
     * window might register for the print event)
     *
     *************************************************************************/
    @Override
    protected void registerForAgentEvents(Agent agent)
    {
    }

    @Override
    protected void unregisterForAgentEvents(Agent agent)
    {
    }

    /************************************************************************
     *
     * ClearAgentEvents is called when the agent has already been deleted (so we
     * can't unregister but should just clear our references)
     *
     *************************************************************************/
    @Override
    protected void clearAgentEvents()
    {
    }

    /************************************************************************
     *
     * Converts this object into an XML representation.
     *
     * For the button view there is no content beyond the list of buttons.
     *
     *************************************************************************/
    @Override
    public JavaElementXML convertToXML(String tagName, boolean storeContent)
    {
        JavaElementXML element = new JavaElementXML(tagName);

        // It's useful to record the class name to uniquely identify the type
        // of object being stored at this point in the XML tree.
        Class<? extends AbstractFixedView> cl = this.getClass();
        element.addAttribute(JavaElementXML.kClassAttribute, cl.getName());

        // Store this object's properties.
        element.addAttribute("Name", m_Name);

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

        m_Name = element.getAttributeThrows("Name");

        // Register that this module's name is in use
        frame.registerViewName(m_Name, this);

        // Actually create the window
        init(frame, doc, parent);
    }

    /************************************************************************
     *
     * Given a context menu and a control, fill in the items you want to see in
     * the menu. The simplest is to just call "fillWindowMenu".
     *
     * This call is made after the user has clicked to bring up the menu so we
     * can create a dymanic menu based on the current context.
     *
     * You also have to call createContextMenu() to request a context menu be
     * attached to a specific control.
     *
     *************************************************************************/
    @Override
    protected void fillInContextMenu(Menu contextMenu, Control control,
                                     int mouseX, int mouseY)
    {
        fillWindowMenu(contextMenu, false, false);
    }
}
