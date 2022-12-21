/********************************************************************************************
 *
 * HelpMenu.java
 *
 * Description:	Help menu
 *
 * Created on 	Apr 10, 2006
 * @author 		Douglas Pearson
 *
 * Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
 ********************************************************************************************/
package edu.umich.soar.debugger.menu;

import java.util.Objects;

import org.eclipse.swt.widgets.Menu;

import sml.sml_Names;
import edu.umich.soar.debugger.MainFrame;
import edu.umich.soar.debugger.doc.Document;
import edu.umich.soar.debugger.general.OSName;
import edu.umich.soar.debugger.general.StartBrowser;

public class HelpMenu
{
    private Document m_Document = null;

    private MainFrame m_Frame = null;

    private AbstractAction m_About = new AbstractAction("About Soar's Debugger")
    {
        public void actionPerformed(ActionEvent e)
        {
            about();
        }
    };

    private AbstractAction m_Homepage = new AbstractAction("Soar Home page")
    {
        public void actionPerformed(ActionEvent e)
        {
            open("http://soar.eecs.umich.edu/");
        }
    };

    private AbstractAction m_Wiki = new AbstractAction(
            "Soar Wiki (many topics)")
    {
        public void actionPerformed(ActionEvent e)
        {
            open("https://github.com/SoarGroup");
        }
    };

    private AbstractAction m_CLI = new AbstractAction("Soar Command Line Help")
    {
        public void actionPerformed(ActionEvent e)
        {
            open("http://soar.eecs.umich.edu/articles/articles/general/73-command-line-help");
        }
    };

    /** Create this menu */
    public static HelpMenu createMenu(MainFrame frame, Document doc,
            String title)
    {
        HelpMenu menu = new HelpMenu();
        menu.m_Frame = frame;
        menu.m_Document = doc;

        menu.makeMenu(frame.getMenuBar(), title);

        return menu;
    }

    private BaseMenu makeMenu(Menu parent, String title)
    {
        BaseMenu menu = new BaseMenu(parent, title);

        menu.add(m_Homepage);
        menu.add(m_Wiki);
        menu.add(m_CLI);
        menu.addSeparator();
        menu.add(m_About);

        return menu;
    }

    /**
     * Enable/disable the menu items depending on the current state of the
     * kernel
     */
    public void updateMenu()
    {
        // All are always enabled for now
    }

    private void about()
    {
        String newLine = OSName.kSystemLineSeparator;

        // This version is based on the version of the client interface compiled
        // into the debugger
        // and is always available.
        String aboutText = newLine + "\tSoar Debugger version: \t"
                + sml_Names.getKSoarVersionValue() + newLine;

        // This version is based on the runtime
        aboutText += "\tSoar Kernel version: \t";

        String kernelVersion = m_Document.getKernelVersion();

        aboutText += Objects.requireNonNullElse(kernelVersion, "no kernel is running.");

        // This version is for the communication library
        aboutText += newLine + "\tSML version: \t\t"
                + sml_Names.getKSMLVersionValue() + newLine;

        aboutText += newLine + "\tDeveloped by: " + newLine
                + "\t\tDouglas Pearson" + newLine + "\t\tBob Marinier"
                + newLine + "\t\tKaren Coulter" + newLine
                + "\t\tJonathan Voigt" + newLine + "\t\tand others." + newLine;

        aboutText += newLine
                + "For help with the debugger or other aspects of Soar "
                + sml_Names.getKSMLVersionValue()
                + newLine
                + "application development send email to soar-sml-list@lists.sourceforge.net"
                + newLine + newLine;
        aboutText += "For help with writing Soar systems (production rules) or for theoretical discussions "
                + newLine
                + "of the Soar cognitive architecture send email to soar-group@lists.sourceforge.net"
                + newLine;

        m_Frame.ShowMessageBox("About Soar's Debugger", aboutText);
    }

    private void open(String url)
    {
        try
        {
            StartBrowser.openURL(url);
        }
        catch (Exception e)
        {
            m_Frame.ShowMessageBox("Error launching URL " + url, e
                    .getLocalizedMessage());
        }
    }
}
