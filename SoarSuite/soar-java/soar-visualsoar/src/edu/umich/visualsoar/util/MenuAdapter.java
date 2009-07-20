package edu.umich.visualsoar.util;
import javax.swing.event.*;

/**
 * This is just a class that implements the 
 * Menu Listener interface with all empty messages,
 * it makes writing the listener a little bit easier
 * @author Brad Jones
 */ 

public class MenuAdapter implements MenuListener {
	public void menuCanceled(MenuEvent e) {}
	public void menuDeselected(MenuEvent e) {}
	public void menuSelected(MenuEvent e) {}
}
