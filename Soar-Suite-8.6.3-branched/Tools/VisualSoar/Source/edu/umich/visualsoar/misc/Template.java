/*
 * Template.java
 *
 * Created on October 26, 2003, 11:18 PM
 */

package edu.umich.visualsoar.misc;

import java.util.*;
import java.io.*;
import java.text.SimpleDateFormat;
import edu.umich.visualsoar.operatorwindow.*;
import edu.umich.visualsoar.ruleeditor.RuleEditor;

/**
 * Represents an insertable code template. 
 *  
 * Each template is either a plain old template or a directory of templates
 * (detectable by isDirectory method). Directory templates contain a set of
 * sub templates and directories.
 *
 * The hierarchy of templates mirrors the hierarchy of directories and .vst
 * files in the templates directory.
 *  
 * Here are the macros:
 * 
 * - $super-operator$
 *    Super operator
 *
 * - $operator$
 *    The current operator
 *
 * - $production$
 *    The name of the production nearest to the cursor
 *
 * - $date$
 *    Today's date
 *
 * - $time$
 *    The time right now
 *
 * - $project$ or $agent$
 *    The name of the project
 *
 * - $user$
 *    The current user name
 *
 * - $caret$ or $cursor$
 *    Place where the cursor should be after the template is expanded.
  */
public class Template {
   
   private File file;         // Path to the template file or directory
   private List directories;  // Sub directories (Template objects)
   private List templates;    // Sub templates
   private String name;       // Name of directory or template
   private int caretOffset;   // offset of last $caret$ macro
   
   // Formatter for the $date$ macro
   private static SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy/MM/dd");
   
   // Formatter for the $time$ macro
   private static SimpleDateFormat timeFormat = new SimpleDateFormat("HH:mm:ss");
   
   /**
    * Constructor for a .vst template
    * 
    * @param f Path to the .vst file
    */
   public Template(File f) {
      file = f;
      String n = f.getName();
      name = n.substring(0, n.lastIndexOf('.')); // strip off .vst extension
   }
   
   /**
    * Constructor for a directory template
    *
    * @param dir Path of directory
    * @param childDirs List of child directory Templates
    * @param childTemplates List of child .vst Templates
    */
   public Template(File dir, List childDirs, List childTemplates){
      file = dir;
      directories = new ArrayList(childDirs);
      templates = new ArrayList(childTemplates);
      name = dir.getName();
   }
   
   /** Returns true if this is a directory template */
   public boolean isDirectory() {
      return file.isDirectory();
   }
   
   /** name of template */
   public String getName() {
      return name;
   }
   
   /** Iterator over child directory templates */
   public Iterator getChildDirectories() {
      return directories.iterator();
   }
   
   /** Iterator over child .vst templates */
   public Iterator getChildTemplates() {
      return templates.iterator();
   }
   
	/**
	 * Instantiate the template, expanding all macros.
    *
    * The rule editor reference is to get access to things like the current
    * operator and production name, etc.
    *
	 * @param editor The rule editor that is instantiating the template
	 * @return the instantiated template
	 */
	public String instantiate(RuleEditor editor) throws TemplateInstantiationException {
      // can't instantiate directories, sorry.
      if(isDirectory()){
         throw new TemplateInstantiationException("Not a template");
      }
      caretOffset = -1;
		int c; // used as a char
      int offset = 0;
		try {	
			FileReader r = new FileReader(file);
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
					insertText.write(lookupVariable(varWriter.toString(), editor, offset));
				}
				else if(c == '\r') {
					
				}
				else {
					insertText.write(c);
               offset++;
            }
			}
         if (caretOffset == -1) {
            caretOffset = insertText.toString().length();
         }
			return insertText.toString();
		}  
		catch (IOException ioe) {
			throw new TemplateInstantiationException("There is a problem with file " + file.getName());
		}
	}
   
   public int getCaretOffset() {
      return caretOffset;
   }
   
	/**
	 * A helper function to help with variable name lookup
    *
    * @param varName Name of macro to expand
    * @param editor The rule editor
    * @return the expanded macro
	 */
	private String lookupVariable(String varName, RuleEditor editor, int offset) throws TemplateInstantiationException {
      if (varName.equals("super-operator")) {
         return editor.getNode().getParent().toString();
      } else if (varName.equals("operator")) {
         return editor.getNode().toString();
      } else if (varName.equals("production")) {
         String p = editor.getProductionNameNearCaret();
         return p != null ? p : "";
      } else if (varName.equals("date")) {
         return dateFormat.format(Calendar.getInstance().getTime());
      } else if (varName.equals("time")) {
         return timeFormat.format(Calendar.getInstance().getTime());
      } else if (varName.equals("project") || varName.equals("agent")) {
         OperatorRootNode root = (OperatorRootNode)OperatorWindow.getOperatorWindow().getModel().getRoot();
         return root.toString();
      } else if (varName.equals("user")) {
         return System.getProperty("user.name", "user");
      } else if (varName.equals("caret") || varName.equals("cursor")) {
         caretOffset = offset;
         return "";
      } else { 
         throw new TemplateInstantiationException("Undefined template variable: " + varName); 
      }
	}
   
   /**
    * Loads the templates/ directory and returns a root directory template
    *
    * @param directory Path to the templates directory
    * @return A root template or null if the directory is valid
    */
   public static Template loadDirectory(File directory) {
      File[] filesAndDirs = directory.listFiles();
      
      if(filesAndDirs == null){
         return null;
      }
      
      List childDirs = new ArrayList();
      List childFiles = new ArrayList();
      Template t = null;
      for(int i = 0; i < filesAndDirs.length; ++i) {
         File f = filesAndDirs[i];
         if(f.isDirectory()){
            t = loadDirectory(f);
            if(t != null){
               childDirs.add(t);
            }
         } else if(f.isFile() && f.getName().endsWith(".vst")) {
            childFiles.add(new Template(f));
         }
      }
      Template r = null;
      if(!childDirs.isEmpty() || !childFiles.isEmpty()){
         r = new Template(directory, childDirs, childFiles);
      }
      return r;
   }   
}
