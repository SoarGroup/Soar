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
import java.io.File;
import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;

public class DesktopActions
{
    public static void openURL(String url) throws IOException, URISyntaxException {
        if (Desktop.isDesktopSupported() && Desktop.getDesktop().isSupported(Desktop.Action.BROWSE)) {
            Desktop.getDesktop().browse(new URI(url));
        } else {
            throw new IOException("Opening websites URLs is not supported on this platform or machine");
        }
    }

    public static void openFileOrDirectory(File file) throws IOException {
        if (Desktop.isDesktopSupported() && Desktop.getDesktop().isSupported(Desktop.Action.OPEN)) {
            Desktop desktop = Desktop.getDesktop();
            desktop.open(file);
        } else {
            throw new IOException("Opening directories is not supported on this platform or machine");
        }
    }

}
