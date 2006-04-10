package edu.umich.visualsoar;
import java.io.File;
import java.io.IOException;
import javax.swing.JOptionPane;

/**
 * This is the class that the user runs, all it does is create an instance of the 
 * main frame and sets it visible
 * @author Brad Jones 0.5a 5 Aug 1999
 * @author Jon Bauman
 * @author Brian Harleton
 * @version 4.0 5 Jun 2002
 */
public class VisualSoar {
///////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////
	/**
	 * Since this class can be run this is the starting point, it constructs an instance of
	 * of the MainFrame
	 * @param args an array of strings representing command line arguments
	 */
	public static void main(String[] args) throws Exception {
		MainFrame mainFrame = new MainFrame("VisualSoar");
		MainFrame.setMainFrame(mainFrame);
		mainFrame.setVisible(true);

		if(args.length == 1){
			try{
				mainFrame.tryOpenProject(new File(args[0]));
			}
			catch(IOException ioe) {
				JOptionPane.showMessageDialog(mainFrame,
					"File " + args[0] + " not found",
					"Open Project Error",
					JOptionPane.ERROR_MESSAGE);
			}
		}
	}
	
}
