package edu.umich.soar.debugger.general;

/////////////////////////////////////////////////////////
//Bare Bones Browser Launch                          //
//Version 1.5                                        //
//December 10, 2005                                  //
//Supports: Mac OS X, GNU/Linux, Unix, Windows XP    //
//Example Usage:                                     //
// String url = "http://www.centerkey.com/";       //
// StartBrowser.openURL(url);
//
//Public Domain Software -- Free to Use as You Like  //
/////////////////////////////////////////////////////////

import java.awt.Desktop;
import java.net.URI;
public class StartBrowser
{
    public static void openURL(String url) throws Exception
    {
        if (Desktop.isDesktopSupported() && Desktop.getDesktop().isSupported(Desktop.Action.BROWSE)) {
            Desktop.getDesktop().browse(new URI(url));
        } else {
            System.err.println("Opening websites URLs is not supported on this platform or machine");
        }
    }

}
