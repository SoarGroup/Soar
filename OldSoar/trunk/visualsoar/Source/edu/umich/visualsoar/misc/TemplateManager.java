package edu.umich.visualsoar.misc;
import edu.umich.visualsoar.operatorwindow.OperatorNode;
import java.util.*;
import java.io.*;

/**
 * This class manages/encapsulates working with templates
 * @author Brad Jones Jon Bauman
 */

public class TemplateManager {
	///////////////////////////////////
	// Data Members
	///////////////////////////////////
	private Map templateNameFileAssoc = new TreeMap();
	
	///////////////////////////////////
	// Accessors
	//////////////////////////////////
	public Iterator getTemplateNames() {
		return templateNameFileAssoc.keySet().iterator();
	}
	
	////////////////////////////////////////////////////////////
	// Modifiers
	////////////////////////////////////////////////////////////
	/**
	 * Given a name and a place to start, this is instaniate a template
	 * @param name the name of the template
	 * @param operatorNode the node for which this template is being instantiated for
	 * @return the instantiated template
	 */
	public String instantiate(String name,OperatorNode operatorNode) throws TemplateInstantiationException {
		int c; // used as a char			
		File templateFile = (File)templateNameFileAssoc.get(name);
		if(templateFile == null)
			throw new IllegalArgumentException("No such template " + name);
			
		try {	
			FileReader r = new FileReader(templateFile);
			StringWriter insertText = new StringWriter();
				
			for (c = r.read(); c != -1; c = r.read()) {
				if (c == '$') {
					StringWriter varWriter = new StringWriter();
					for (c = r.read(); c != '$'; c = r.read()) {
						if (c == -1) {
					 		throw new TemplateInstantiationException("Unmatched $'s");
					 	}
					 	varWriter.write(c);
					}
					insertText.write(lookupVariable(varWriter.toString(),operatorNode));
				}
				else if(c == '\r') {
					
				}
				else 
					insertText.write(c);
			}
			return insertText.toString();
		}  
		catch (IOException ioe) {
			throw new TemplateInstantiationException("There is a problem with file " + templateFile.getName());
		}
	}

	
	/**
	 * Loads in files from the specified directory
	 */
	public void load(File directory) {
		File[] templateFiles = directory.listFiles(new FileFilter() {
			public boolean accept(File pathname) {
				return pathname.getName().endsWith(".vst");
			} 
		});
		
		if(templateFiles == null)
			return;
		
		for(int i = 0; i < templateFiles.length; ++i) {
			String templateName = templateFiles[i].getName().substring(0,templateFiles[i].getName().length()-4);
			templateNameFileAssoc.put(templateName,templateFiles[i]);
		}
	}
	
	/**
	 * A helper function to help with variable name lookup
	 */
	private String lookupVariable(String varName,OperatorNode operatorNode) throws TemplateInstantiationException {
		if (varName.equals("super-operator")) 
			return operatorNode.getParent().toString();
		else if (varName.equals("operator")) 
			return operatorNode.toString();
		else 
			throw new TemplateInstantiationException("Undefined Variable: " + varName); 
	}
}
