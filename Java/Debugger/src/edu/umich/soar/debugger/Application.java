/** 
 * Application.java
 *
 * Title:			Debugger for Soar written in Java
 * Description:	
 * @author			Douglas Pearson, 2005 (www.threepenny.net)
 * @version			
 */

package edu.umich.soar.debugger;

import org.eclipse.swt.widgets.Display;

public class Application
{

    /************************************************************************
     * 
     * Default constructor for the application -- creates the main frame and set
     * the look and feel.
     * 
     *************************************************************************/
    public Application(String[] args, boolean alwaysInstallLibs, Display display)
    {
        try
        {
            // Start the SWT version of the application (we used to have a Swing
            // version too)
            SWTApplication swtApp = new SWTApplication();

            swtApp.startApp(args, display);

            if (display == null)
            {
                System.exit(0);
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
    }

    /*************************************************************************
     * 
     * The application's main entry point.
     * 
     * @param args
     *            Command line arguments. Currently ignored.
     * 
     *************************************************************************/
    static public void main(String[] args)
    {
        new Application(args, false, null);
    }

}
