package edu.umich.soar.debugger.modules;

import java.util.ArrayList;

import org.eclipse.swt.SWT;
import org.eclipse.swt.custom.StyledText;
import org.eclipse.swt.events.MouseAdapter;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Point;
import org.eclipse.swt.layout.FormData;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Label;

import sml.Kernel;
import edu.umich.soar.debugger.MainFrame;
import edu.umich.soar.debugger.dialogs.PropertiesDialog;
import edu.umich.soar.debugger.doc.Document;
import edu.umich.soar.debugger.general.JavaElementXML;
import edu.umich.soar.debugger.helpers.FormDataHelper;
import edu.umich.soar.debugger.manager.Pane;
import edu.umich.soar.debugger.menu.ParseSelectedText;

public class RHSFunTextView extends AbstractRHSFunView implements
        Kernel.RhsFunctionInterface
{
    @Override
    public void init(MainFrame frame, Document doc, Pane parentPane)
    {
        if (labelText.length() <= 0)
        {
            labelText = getModuleBaseName();
        }
        super.init(frame, doc, parentPane);
    }

    public String getModuleBaseName()
    {
        return "rhs_fun_text";
    }

    protected String labelText = new String();

    protected Label labelTextWidget;

    StyledText textBox;

    boolean clear = false;

    /************************************************************************
     * 
     * Set text in a thread safe way (switches to UI thread)
     * 
     *************************************************************************/
    protected void setTextSafely(final String text)
    {
        // If Soar is running in the UI thread we can make
        // the update directly.
        if (!Document.kDocInOwnThread)
        {
            textBox.setText(text);
            return;
        }

        // Have to make update in the UI thread.
        // Callback comes in the document thread.
        Display.getDefault().asyncExec(new Runnable()
        {
            public void run()
            {
                textBox.setText(text);
            }
        });
    }

    protected void setLabelText(final String text)
    {
        // If Soar is running in the UI thread we can make
        // the update directly.
        if (!Document.kDocInOwnThread)
        {
            labelTextWidget.setText(text);
            return;
        }

        // Have to make update in the UI thread.
        // Callback comes in the document thread.
        Display.getDefault().asyncExec(new Runnable()
        {
            public void run()
            {
                labelTextWidget.setText(text);
            }
        });
    }

    /************************************************************************
     * 
     * Append text in a thread safe way (switches to UI thread)
     * 
     *************************************************************************/
    protected void appendTextSafely(final String text)
    {
        // If Soar is running in the UI thread we can make
        // the update directly.
        if (!Document.kDocInOwnThread)
        {
            textBox.append(text);
            return;
        }

        // Have to make update in the UI thread.
        // Callback comes in the document thread.
        Display.getDefault().asyncExec(new Runnable()
        {
            public void run()
            {
                textBox.append(text);
            }
        });
    }

    StringBuilder output = new StringBuilder();

    public String rhsFunctionHandler(int eventID, Object data,
            String agentName, String functionName, String argument)
    {

        String[] commandLine = argument.split("\\s+");

        if (commandLine.length >= 1 && commandLine[0].equals("--clear"))
        {
            clear = true;
            return debugMessages ? m_Name + ":" + functionName
                    + ": set to clear" : "";
        }

        output = new StringBuilder();

        for (int index = 0; index < commandLine.length; index += 2)
        {
            output.append(commandLine[index]);
            if (index + 1 < commandLine.length)
            {
                output.append(": ");
                output.append(commandLine[index + 1]);
                output.append("\n");
            }
        }

        return debugMessages ? m_Name + ":" + functionName + ": updated "
                + getName() : "";
    }

    @Override
    public boolean find(String text, boolean searchDown, boolean matchCase,
            boolean wrap, boolean searchHiddenText)
    {
        return false;
    }

    Composite rhsFunContainer;

    @Override
    protected void createDisplayControl(Composite parent)
    {

        rhsFunContainer = new Composite(parent, SWT.NULL);
        FormData attachFull = FormDataHelper.anchorFull(0);
        rhsFunContainer.setLayoutData(attachFull);
        {
            GridLayout gl = new GridLayout();
            gl.numColumns = 1;
            gl.verticalSpacing = 0;
            gl.marginHeight = 0;
            gl.marginWidth = 0;
            rhsFunContainer.setLayout(gl);
        }

        labelTextWidget = new Label(rhsFunContainer, SWT.NONE);
        labelTextWidget.setText(labelText);
        {
            GridData gd = new GridData(SWT.FILL, SWT.NONE, true, false);
            labelTextWidget.setLayoutData(gd);
        }

        textBox = new StyledText(rhsFunContainer, SWT.MULTI | SWT.READ_ONLY
                | SWT.H_SCROLL | SWT.V_SCROLL);
        updateNow();
        {
            GridData gd = new GridData(SWT.FILL, SWT.FILL, true, true);
            textBox.setLayoutData(gd);
        }

        // We want a right click to set the selection instantly, so you can
        // right click on an ID
        // rather than left-clicking on the ID and then right click to bring up
        // menu.
        textBox.addMouseListener(new MouseAdapter()
        {
            public void mouseDown(MouseEvent e)
            {
                if (e.button == 2 || e.button == 3)
                    rightButtonPressed(e);
            };
        });

        textBox.setBackground(getBackgroundColor());

        createContextMenu(labelTextWidget);
        createContextMenu(textBox);
    }

    /*******************************************************************************************
     * 
     * When the user clicks the right mouse button, sets the selection to that
     * location (just like a left click). This makes right clicking on a piece
     * of text much easier as it's just one click rather than having to left
     * click to place the selection and then right click to bring up the menu.
     * 
     ********************************************************************************************/
    protected void rightButtonPressed(MouseEvent e)
    {
        try
        {
            Point mouse = new Point(e.x, e.y);
            int offset = textBox.getOffsetAtLocation(mouse);
            textBox.setSelection(offset);
        }
        catch (IllegalArgumentException ex)
        {
            // If the click is out of range of the text ignore it.
        }
    }

    @Override
    public Color getBackgroundColor()
    {
        return getMainFrame().getDisplay().getSystemColor(
                SWT.COLOR_WIDGET_BACKGROUND);
    }

    @Override
    protected Control getDisplayControl()
    {
        // this should return the text control
        return rhsFunContainer;
    }

    @Override
    protected void restoreContent(JavaElementXML element)
    {

    }

    @Override
    protected void storeContent(JavaElementXML element)
    {

    }

    @Override
    public void clearDisplay()
    {
        output = new StringBuilder();
        setTextSafely(output.toString());
    }

    @Override
    public void copy()
    {
        textBox.copy();
    }

    @Override
    protected ParseSelectedText.SelectedObject getCurrentSelection(int mouseX,
            int mouseY)
    {
        // Find out if mouseX, mouseY is over the text at all
        try
        {
            Point mouse = textBox.toControl(mouseX, mouseY);
            textBox.getOffsetAtLocation(mouse);
        }
        catch (IllegalArgumentException ex)
        {
            // If the click wasn't over any letters we come here and return null
            // to indicate
            // no text was selected (we'll just show the default context menu).
            return null;
        }

        // Look up the currently selected text
        int pos = textBox.getCaretOffset();
        if (pos == -1)
            return null;

        ParseSelectedText selection = new ParseSelectedText(textBox.getText(),
                pos);

        return selection.getParsedObject(this.m_Document, this.getAgentFocus());
    }

    @Override
    public void displayText(String text)
    {
    }

    @Override
    public void onInitSoar()
    {
        clearDisplay();
    }

    @Override
    protected void updateNow()
    {
        if (clear)
        {
            clearDisplay();
            clear = false;
        }

        setTextSafely(output.toString());
    }

    private int propertiesStartingIndex;

    protected void initProperties(
            ArrayList<PropertiesDialog.Property> properties)
    {
        super.initProperties(properties);
        propertiesStartingIndex = properties.size();

        properties.add(new PropertiesDialog.StringProperty("Label text",
                labelText));
    }

    protected void processProperties(
            ArrayList<PropertiesDialog.Property> properties)
    {
        super.processProperties(properties);
        labelText = ((PropertiesDialog.StringProperty) properties
                .get(propertiesStartingIndex)).getValue();
        setLabelText(labelText);
    }

    /************************************************************************
     * 
     * Converts this object into an XML representation.
     * 
     *************************************************************************/
    @Override
    public JavaElementXML convertToXML(String title, boolean storeContent)
    {
        JavaElementXML element = new JavaElementXML(title);

        // It's useful to record the class name to uniquely identify the type
        // of object being stored at this point in the XML tree.
        Class<? extends RHSFunTextView> cl = this.getClass();
        element.addAttribute(JavaElementXML.kClassAttribute, cl.getName());

        if (m_Name == null)
            throw new IllegalStateException(
                    "We've created a view with no name -- very bad");

        // Store this object's properties.
        element.addAttribute("Name", m_Name);
        element.addAttribute("UpdateOnStop", Boolean.toString(m_UpdateOnStop));
        element.addAttribute("UpdateEveryNthDecision", Integer
                .toString(m_UpdateEveryNthDecision));
        element.addAttribute("RHSFunctionName", rhsFunName);
        element.addAttribute("LabelText", labelText);
        element.addAttribute("DebugMessages", Boolean.toString(debugMessages));

        if (storeContent)
            storeContent(element);

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
    public void loadFromXML(MainFrame frame, Document doc, Pane parent,
            JavaElementXML element) throws Exception
    {
        setValues(frame, doc, parent);

        m_Name = element.getAttribute("Name");
        m_UpdateOnStop = element.getAttributeBooleanDefault("UpdateOnStop",
                true);
        m_UpdateEveryNthDecision = element.getAttributeIntDefault(
                "UpdateEveryNthDecision", 0);
        rhsFunName = element.getAttribute("RHSFunctionName");
        labelText = element.getAttribute("LabelText");
        debugMessages = element.getAttributeBooleanDefault("DebugMessages",
                true);

        if (rhsFunName == null)
        {
            rhsFunName = new String();
        }

        if (labelText == null)
        {
            labelText = new String();
        }

        JavaElementXML log = element.findChildByName("Logger");
        if (log != null)
            this.m_Logger.loadFromXML(doc, log);

        // Register that this module's name is in use
        frame.registerViewName(m_Name, this);

        // Actually create the window
        init(frame, doc, parent);

        // Restore the text we saved (if we chose to save it)
        restoreContent(element);
    }

}
