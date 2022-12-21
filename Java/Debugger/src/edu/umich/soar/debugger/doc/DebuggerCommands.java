/********************************************************************************************
 *
 * DebuggerCommands.java
 *
 * Description:
 *
 * Created on 	Apr 6, 2005
 * @author 		Douglas Pearson
 *
 * Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
 ********************************************************************************************/
package edu.umich.soar.debugger.doc;

import edu.umich.soar.debugger.MainFrame;
import edu.umich.soar.debugger.modules.AbstractView;

/************************************************************************
 *
 * A set of commands that extend the Soar command set.
 *
 * I'm not sure if these should include the script commands or not so for now
 * I'm handling these separately.
 *
 ************************************************************************/
public class DebuggerCommands
{
    protected MainFrame m_Frame;

    protected Document m_Document;

    public final static String kClear = "clear";

    public final static String kQuit = "quit";

    public final static String kExit = "exit";

    public final static String kEditProduction = "edit-production";

    protected String[] kCommands = new String[] { kClear, kQuit, kExit,
            kEditProduction };

    public DebuggerCommands(MainFrame frame, Document doc)
    {
        m_Frame = frame;
        m_Document = doc;
    }

    // Providing a method for expanding the command line (so we can support
    // aliases)
    public String getExpandedCommand(String commandLine)
    {
        return commandLine;
        // return m_Document.getExpandedCommandLine(commandLine) ;
    }

    public boolean isCommand(String commandLine)
    {
        for (String kCommand : kCommands) {
            if (commandLine.toLowerCase().startsWith(kCommand))
                return true;
        }

        return false;
    }

    public String executeCommand(AbstractView view, String commandLine,
            boolean echoCommand)
    {
        String line = commandLine.toLowerCase();
        if (line.startsWith(kClear))
        {
            view.clearDisplay();
        }
        else if (line.startsWith(kQuit) || line.startsWith(kExit))
        {
            m_Frame.close();
        }
        else if (line.startsWith(kEditProduction))
        {
            if (!m_Document.isVisualSoarConnected())
            {
                m_Frame
                        .displayTextInPrimeView("Error: Visual Soar is not connected.");
            }
            return null; // this means DO send the command to Soar, that we are
                         // adding behavior
        }
        return ""; // this means do not send the command to Soar
    }
}
