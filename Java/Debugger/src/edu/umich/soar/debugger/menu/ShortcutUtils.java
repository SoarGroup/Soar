package edu.umich.soar.debugger.menu;

import org.eclipse.swt.SWT;

import java.util.Locale;

public class ShortcutUtils {
    public static final String OS = System.getProperty("os.name", "unknown").toLowerCase(Locale.ROOT);
    public static final int SHORTCUT_KEY;
    public static final String SHORTCUT_HINT;

    static {
        if (OS.contains("mac")) {
            SHORTCUT_KEY = SWT.COMMAND;
            SHORTCUT_HINT = "Cmd+";
        } else {
            SHORTCUT_KEY = SWT.CTRL;
            SHORTCUT_HINT = "Ctrl+";
        }
    }
}
