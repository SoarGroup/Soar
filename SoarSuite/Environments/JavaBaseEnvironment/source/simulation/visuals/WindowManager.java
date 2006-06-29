package simulation.visuals;

import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Shell;
import java.util.logging.*;

public class WindowManager {
	public static final String kMapPrefix = "Map: ";
	public static final String kColors[] = { "red", "blue", "purple", "yellow", "orange", "black", "green" };
	
	public static Color white;
	public static Color blue;
	public static Color red;
	public static Color widget_background;
	public static Color yellow;
	public static Color orange;
	public static Color black;
	public static Color green;
	public static Color purple;

	public static void initColors(Display d) {
	    white = d.getSystemColor(SWT.COLOR_WHITE);
		widget_background = d.getSystemColor(SWT.COLOR_WIDGET_BACKGROUND);
		blue = d.getSystemColor(SWT.COLOR_BLUE);
		red = d.getSystemColor(SWT.COLOR_RED);
		yellow = d.getSystemColor(SWT.COLOR_YELLOW);
		green = d.getSystemColor(SWT.COLOR_GREEN);
		purple = d.getSystemColor(SWT.COLOR_DARK_MAGENTA);
		orange = new Color(d, 255, 127, 0);
		black = d.getSystemColor(SWT.COLOR_BLACK);
	}
	
	public static Color getColor(String color) {
		if (color.equalsIgnoreCase("white")) {
			return white;
		}
		if (color.equalsIgnoreCase("blue")) {
			return blue;
		}
		if (color.equalsIgnoreCase("red")) {
			return red;
		}
		if (color.equalsIgnoreCase("yellow")) {
			return yellow;
		}
		if (color.equalsIgnoreCase("green")) {
			return green;
		}
		if (color.equalsIgnoreCase("purple")) {
			return purple;
		}
		if (color.equalsIgnoreCase("orange")) {
			return orange;
		}
		if (color.equalsIgnoreCase("black")) {
			return black;
		}
		return null;
	}

	protected static Logger logger = Logger.getLogger("simulation.visuals");
	protected Display m_Display;
	protected Shell m_Shell;

	public WindowManager() {
		m_Display = new Display();
		m_Shell = new Shell(m_Display, SWT.BORDER | SWT.CLOSE | SWT.MIN | SWT.TITLE);
		initColors(m_Display);
	}
}
