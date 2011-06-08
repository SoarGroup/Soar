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

import java.io.File;
import java.net.URL;

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
            open("http://sitemaker.umich.edu/soar");
        }
    };

    private AbstractAction m_Wiki = new AbstractAction(
            "Soar Wiki (many topics)")
    {
        public void actionPerformed(ActionEvent e)
        {
            open("http://code.google.com/p/soar/wiki/Home?tm=6");
        }
    };

    private AbstractAction m_CLI = new AbstractAction("Soar Command Line Help")
    {
        public void actionPerformed(ActionEvent e)
        {
            open("http://code.google.com/p/soar/wiki/CommandIndex");
        }
    };

    private AbstractAction m_Manual = new AbstractAction("Soar Manual")
    {
        public void actionPerformed(ActionEvent e)
        {
            local("/share/soar/Documentation/SoarManual.pdf");
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

        menu.add(m_Manual);
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

        if (kernelVersion != null)
            aboutText += kernelVersion;
        else
            aboutText += "no kernel is running.";

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

    private void local(String relativeUrl)
    {
        String path = m_Frame.getLibraryLocation();

        if (path == null || path == "")
        {
            m_Frame.ShowMessageBox("Error launching URL " + relativeUrl,
                    "Can't find Soar library location");
            return;
        }

        try
        {
            File file = new File(path, relativeUrl);

            if (!file.exists())
            {
                m_Frame.ShowMessageBox("Could not find this file", file
                        .toString());
                return;
            }

            URL url = file.toURI().toURL();
            String urlString = url.toString();
            open(urlString);
        }
        catch (Exception e)
        {
            m_Frame.ShowMessageBox("Error launching URL " + relativeUrl, e
                    .getLocalizedMessage());
        }
    }
}
