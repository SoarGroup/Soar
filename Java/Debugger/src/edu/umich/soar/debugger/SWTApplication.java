/********************************************************************************************
 *
 * SWTApplication.java
 *
 * Description:
 *
 * Created on 	Mar 2, 2005
 * @author 		Douglas Pearson
 *
 * Developed by ThreePenny Software <a href="http://www.threepenny.net">www.threepenny.net</a>
 ********************************************************************************************/
package edu.umich.soar.debugger;

import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.layout.FillLayout;
import org.eclipse.swt.layout.RowLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.CoolBar;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Menu;
import org.eclipse.swt.widgets.Shell;

import sml.Agent;
import sml.Kernel;
import edu.umich.soar.debugger.doc.Document;

/************************************************************************
 *
 * SWT based application (we used to have a Swing version too)
 *
 ************************************************************************/
public class SWTApplication
{
    private Document m_Document = null;

    static Display display;

    static Shell shell;

    static CoolBar coolBar;

    static Menu chevronMenu = null;

    int m_OffsetY;

    int m_MinY;

    int m_MaxY;

    // This is just a little place for me to easily test SWT concepts
    public void startTest(String[] args) {
        final Display display = new Display();
        Shell shell = new Shell(display);
        shell.setLayout(new FillLayout());

        // The container lets us control the layout of the controls
        // within this window
        Composite container = new Composite(shell, SWT.NULL);

        RowLayout layout = new RowLayout(SWT.HORIZONTAL);
        // layout.wrap = true ;
        layout.fill = true;
        container.setLayout(layout);

        for (int i = 0; i < 10; i++)
        {
            Button button = new Button(container, SWT.PUSH);
            button.setText("Button " + i);
        }

        shell.pack();
        shell.open();
        while (!shell.isDisposed())
        {
            if (!display.readAndDispatch())
                display.sleep();
        }
        display.dispose();
    }

    /*
     * private void clearClipboard() { Clipboard clipboard = new
     * Clipboard(display); String textData = ""; // Copying an empty string to
     * the clipboard to erase it TextTransfer textTransfer =
     * TextTransfer.getInstance(); clipboard.setContents(new Object[]{textData},
     * new Transfer[]{textTransfer}); clipboard.dispose(); }
     */

    // Returns true if a given option appears in the list
    // (Use this for simple flags like -remote)
    private boolean hasOption(String[] args, String option)
    {
        for (String arg : args) {
            if (arg.equalsIgnoreCase(option))
                return true;
        }

        return false;
    }

    // Returns the next argument after the matching option.
    // (Use this for parameters like -port ppp)
    private String getOptionValue(String[] args, String option)
    {
        for (int i = 0; i < args.length - 1; i++)
        {
            if (args[i].equalsIgnoreCase(option))
                return args[i + 1];
        }

        return null;
    }

    // Command line options:
    // -remote => use a remote connection (with default ip and port values)
    // -ip xxx => use this IP value (implies remote connection)
    // -port ppp => use this port (implies remote connection)
    // Without any remote options we start a local kernel
    //
    // -agent <name> => on a remote connection select this agent as initial
    // agent
    // -agent <name> => on a local connection use this as the name of the
    // initial agent
    //
    // -source "<path>" => load this file of productions on launch (only valid
    // for local kernel)
    // -quitonfinish => when combined with source causes the debugger to exit
    // after sourcing that one file
    // -listen ppp => use this port to listen for remote connections (only valid
    // for a local kernel)
    //
    // -maximize => start with maximized window
    // -width <width> => start with this window width
    // -height <height> => start with this window height
    // -x <x> -y <y> => start with this window position
    // -cascade => cascade each window that starts (offseting from the -x <x> -y
    // <y> if given). This option now always on.
    // (Providing width/height/x/y => not a maximized window)
    public void startApp(String[] args) throws Exception
    {
        startApp(args, null);
    }

    public void startApp(String[] args, Display display) {
        // The document manages the Soar process
        m_Document = new Document();

        // Check for command line options
        boolean remote = hasOption(args, "-remote");
        String ip = getOptionValue(args, "-ip");
        String port = getOptionValue(args, "-port");
        String agentName = getOptionValue(args, "-agent");
        String source = getOptionValue(args, "-source");
        boolean quitOnFinish = hasOption(args, "-quitonfinish");
        String listen = getOptionValue(args, "-listen");
        boolean cascade = hasOption(args, "-cascade");
        String layout = getOptionValue(args, "-layout");

        // quitOnFinish is only valid if sourcing a file
        if (source == null)
            quitOnFinish = false;

        boolean maximize = hasOption(args, "-maximize");

        // Remote args
        if (ip != null || port != null)
            remote = true;

        String errorMsg = null;

        int portNumber = Kernel.GetDefaultPort();
        if (port != null)
        {
            try
            {
                portNumber = Integer.parseInt(port);
            }
            catch (NumberFormatException e)
            {
                errorMsg = "Passed invalid port value " + port;
                System.out.println(errorMsg);
            }
        }

        int listenPort = Kernel.GetDefaultPort();
        if (listen != null)
        {
            try
            {
                listenPort = Integer.parseInt(listen);
            }
            catch (NumberFormatException e)
            {
                errorMsg = "Passed invalid listen value " + listen;
                System.out.println(errorMsg);
            }
        }

        // Window size args
        // We'll override the existing app property values
        if (maximize)
            m_Document.getAppProperties()
                    .setAppProperty("Window.Max", maximize);

        m_Document.getAppProperties().setAppProperty("Window.Cascade", cascade);

        String[] options = new String[] { "-width", "-height", "-x", "-y" };
        String[] props = new String[] { "Window.width", "Window.height",
                "Window.x", "Window.y" };

        for (int i = 0; i < options.length; i++)
        {
            String value = getOptionValue(args, options[i]);
            if (value != null)
            {
                try
                {
                    int val = Integer.parseInt(value);
                    m_Document.getAppProperties().setAppProperty(props[i], val);

                    // If provide any window information we'll start
                    // not-maximized
                    m_Document.getAppProperties().setAppProperty("Window.Max",
                            false);
                }
                catch (NumberFormatException e)
                {
                    System.out.println("Passed invalid value " + value
                            + " for option " + options[i]);
                }
            }
        }

        boolean owned = true;
        if (display == null)
        {
            owned = false;
            display = new Display();
        }
        Shell shell = new Shell(display);

        Image small = new Image(display, SWTApplication.class
                .getResourceAsStream("/images/debugger16.png"));
        Image large = new Image(display, SWTApplication.class
                .getResourceAsStream("/images/debugger.png"));
        shell.setImages(new Image[] { small, large });

        // We default to showing the contents of the clipboard in the search
        // dialog
        // so clear it when the app launches, so we don't get random junk in
        // there.
        // Once the app is going, whatever the user is copying around is a
        // reasonable
        // thing to start from, but things from before are presumably unrelated.
        // This currently fails on Linux version of SWT
        // clearClipboard() ;

        final MainFrame frame = new MainFrame(shell, m_Document);
        frame.initComponents(layout);

        // We wait until we have a frame up before starting the kernel
        // so it's just as if the user chose to do this manually
        // (saves some special case logic in the frame)
        if (!remote)
        {
            Agent agent = m_Document.startLocalKernel(listenPort, agentName,
                    source, quitOnFinish);
            frame.setAgentFocus(agent);
        }
        else
        {
            // Start a remote connection
            try
            {
                m_Document.remoteConnect(ip, portNumber, agentName);
            }
            catch (Exception e)
            {
                errorMsg = e.getMessage();
            }
        }

        shell.open();

        // We delay any error message until after the shell has been opened
        if (errorMsg != null)
            MainFrame.ShowMessageBox(shell,
                    "Error connecting to remote kernel", errorMsg, SWT.OK);

        if (!owned)
        {
            m_Document.pumpMessagesTillClosed(display);

            display.dispose();
        }
    }

}
