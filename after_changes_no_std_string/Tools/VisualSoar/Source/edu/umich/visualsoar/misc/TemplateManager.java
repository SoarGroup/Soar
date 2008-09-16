package edu.umich.visualsoar.misc;
import edu.umich.visualsoar.operatorwindow.OperatorNode;
import java.util.*;
import java.io.*;

/**
 * This class manages/encapsulates working with templates
 *
 * Refactored by Dave Ray... this is pretty boring now because most of its
 * functionality has been moved into the Template class...
 *
 * @author Brad Jones Jon Bauman
 */

public class TemplateManager {
   

	///////////////////////////////////
	// Data Members
	///////////////////////////////////
   private Template rootTemplate;
   
	///////////////////////////////////
	// Accessors
	//////////////////////////////////
   public Template getRootTemplate() {
      return rootTemplate;
   }
   	
	////////////////////////////////////////////////////////////
	// Modifiers
	////////////////////////////////////////////////////////////
	
	/**
	 * Loads in files from the specified directory
	 */
	public void load(File directory) {
      rootTemplate = Template.loadDirectory(directory);
	}
}
