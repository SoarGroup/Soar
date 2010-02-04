/*
 * Template.java
 *
 * Created on October 26, 2003, 11:18 PM
 */

package edu.umich.soar.visualsoar.misc;

import java.util.*;
import java.io.*;
import java.text.SimpleDateFormat;

import edu.umich.soar.visualsoar.operatorwindow.OperatorRootNode;
import edu.umich.soar.visualsoar.operatorwindow.OperatorWindow;
import edu.umich.soar.visualsoar.ruleeditor.RuleEditor;

/**
 * Represents an insertable code template.
 * 
 * Each template is either a plain old template or a directory of templates
 * (detectable by isDirectory method). Directory templates contain a set of sub
 * templates and directories.
 * 
 * The hierarchy of templates mirrors the hierarchy of directories and .vsoart
 * files in the templates directory.
 * 
 * Here are the macros:
 * 
 * - $super-operator$ Super operator
 * 
 * - $operator$ The current operator
 * 
 * - $production$ The name of the production nearest to the cursor
 * 
 * - $date$ Today's date
 * 
 * - $time$ The time right now
 * 
 * - $project$ or $agent$ The name of the project
 * 
 * - $user$ The current user name
 * 
 * - $caret$ or $cursor$ Place where the cursor should be after the template is
 * expanded.
 */
public class Template {

	private String resource; // Path to the resource
	private List<Template> templates; // Sub templates
	private String name; // Name of directory or template
	private int caretOffset; // offset of last $caret$ macro

	// Formatter for the $date$ macro
	private static SimpleDateFormat dateFormat = new SimpleDateFormat(
			"yyyy/MM/dd");

	// Formatter for the $time$ macro
	private static SimpleDateFormat timeFormat = new SimpleDateFormat(
			"HH:mm:ss");

	/**
	 * Constructor for a .vsoart template
	 */
	private Template(String name, String resource) {
		this.resource = resource;
		this.name = name;
	}

	/**
	 * Constructor for a directory template
	 * 
	 * Warning: does not copy list.
	 */
	private Template(String name, List<Template> childTemplates) {
		this.name = name;
		templates = childTemplates;
	}

	/** Returns true if this is a directory template */
	public boolean isDirectory() {
		return resource == null;
	}

	/** name of template */
	public String getName() {
		return name;
	}

	/** Iterator over child .vsoart templates */
	public Iterator<Template> getChildTemplates() {
		return templates.iterator();
	}

	/**
	 * Instantiate the template, expanding all macros.
	 * 
	 * The rule editor reference is to get access to things like the current
	 * operator and production name, etc.
	 * 
	 * @param editor
	 *            The rule editor that is instantiating the template
	 * @return the instantiated template
	 */
	public String instantiate(RuleEditor editor)
			throws TemplateInstantiationException {
		// can't instantiate directories, sorry.
		if (isDirectory()) {
			throw new TemplateInstantiationException("Not a template");
		}
		caretOffset = -1;
		int c; // used as a char
		int offset = 0;
		try {
			InputStream s = Template.class.getResourceAsStream(resource);
			Reader r = new InputStreamReader(s);
			StringWriter insertText = new StringWriter();

			for (c = r.read(); c != -1; c = r.read()) {
				if (c == '$') {
					StringWriter varWriter = new StringWriter();
					for (c = r.read(); c != '$'; c = r.read()) {
						if (c == -1) {
							throw new TemplateInstantiationException(
									"Unmatched $'s");
						}
						varWriter.write(c);
					}
					insertText.write(lookupVariable(varWriter.toString(),
							editor, offset));
				} else if (c == '\r') {

				} else {
					insertText.write(c);
					offset++;
				}
			}
			if (caretOffset == -1) {
				caretOffset = insertText.toString().length();
			}
			return insertText.toString();
		} catch (IOException ioe) {
			throw new TemplateInstantiationException(
					"There is a problem with resource " + resource);
		}
	}

	public int getCaretOffset() {
		return caretOffset;
	}

	/**
	 * A helper function to help with variable name lookup
	 * 
	 * @param varName
	 *            Name of macro to expand
	 * @param editor
	 *            The rule editor
	 * @return the expanded macro
	 */
	private String lookupVariable(String varName, RuleEditor editor, int offset)
			throws TemplateInstantiationException {
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
			OperatorRootNode root = (OperatorRootNode) OperatorWindow
					.getOperatorWindow().getModel().getRoot();
			return root.toString();
		} else if (varName.equals("user")) {
			return System.getProperty("user.name", "user");
		} else if (varName.equals("caret") || varName.equals("cursor")) {
			caretOffset = offset;
			return "";
		} else {
			throw new TemplateInstantiationException(
					"Undefined template variable: " + varName);
		}
	}

	/**
	 * Loads the templates from the jar and returns a root directory template
	 * 
	 * @return A root template or null if error
	 */
	public static Template loadFromJar() {
		Properties properties = new Properties();
		try {
			properties.load(ClassLoader.getSystemResourceAsStream("templates.properties"));
		} catch (IOException e) {
			e.printStackTrace();
			return null;
		}

		Set<String> keys = properties.stringPropertyNames();
		List<Template> templates = new ArrayList<Template>();
		
		for (String key : keys) {
			String value = properties.getProperty(key);
			Template template = new Template(key, value);
			templates.add(template);
		}
		
		if (templates.isEmpty()) {
			return null;
		}
		
		return new Template("templates", templates);
		
//		File[] filesAndDirs = directory.listFiles();
//
//		if (filesAndDirs == null) {
//			return null;
//		}
//
//		List childDirs = new ArrayList();
//		List childFiles = new ArrayList();
//		Template t = null;
//		for (int i = 0; i < filesAndDirs.length; ++i) {
//			File f = filesAndDirs[i];
//			if (f.isDirectory()) {
//				t = loadDirectory(f);
//				if (t != null) {
//					childDirs.add(t);
//				}
//			} else if (f.isFile() && f.getName().endsWith(".vsoart")) {
//				childFiles.add(new Template(f));
//			}
//		}
//		Template r = null;
//		if (!childDirs.isEmpty() || !childFiles.isEmpty()) {
//			r = new Template(directory, childDirs, childFiles);
//		}
//		return r;
	}
}
