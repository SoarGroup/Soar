/*
 * Created on Feb 11, 2004
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap;


import edu.rosehulman.soar.datamap.items.*;

import org.eclipse.core.runtime.*;
import org.eclipse.core.resources.*;

import java.io.*;

//	Everything we need to handle XML
import javax.xml.parsers.*;
import org.w3c.dom.*;


/**
 * 
 * Represents a DataMap file. Handles such functions as loading and saving the 
 *  DataMap. To edit the DataMap, <code>getRoot</code> and perform the necessary
 *  modifications on that <code>DMItem</code> and its children.
 * 
 * @author Tim Jasko
 * @see DMItem
 */
public class DataMap {
	private DMSpecial _root;
	private IFile _file;
	
	
	/**
	 * Constructs the DataMap from the file.
	 * 
	 * @param file The file to load the DataMap from.
	 * @throws Exception Any exception encountered while attempting to load the file.
	 */
	public DataMap(IFile file) throws Exception {
		_file = file;
		
		try { 
			loadXML(_file);
		} catch (Exception e) {
			throw(e);
		} // catch
		
	} // DataMap(IFile file)
	
	/**
	 * Constructs a new Datamap, allowing you not to load from the file.
	 *  This is useful in such cases where a new datamap file has just been 
	 *  created and you still need to initialize the contents.
	 * 
	 * @param file The file to load the DataMap from.
	 * @param load <code>true</code> to load from the file, <code>false</code> not to.
	 * @throws Exception Any exception encountered while attempting to load the file.
	 */
	public DataMap(IFile file, boolean load) throws Exception {
		_file = file;
		
		if (load) {
			try { 
				loadXML(_file);
			} catch (Exception e) {
				throw(e);
			} // catch
		} // if
		
	} // DataMap(IFile file, boolean load)
	
	
	/**
	 * Gets the file this DataMap is associated with.
	 * 
	 * @return The file this DataMap is associated with.
	 */
	public IFile getFile() {
		return _file;
	}


	/**
	 * Gets the root of the loaded DataMap.
	 * 
	 * @return The root of the loaded DataMap.
	 */
	public DMSpecial getRoot() {
		return _root;
	}


	/**
	 * Sets the root (and consequently the whole DataMap).
	 * 
	 * @param newRoot The root of the new DataMap.
	 */
	public void setRoot(DMSpecial newRoot) {
		_root = newRoot;
	} // void setRoot(DMItem newRoot)
	
	
	/**
	 * Saves the DataMap to its file in XML format.
	 * 
	 * @param monitor The progress monitor.
	 * @throws Exception Any exception encountered while saving.
	 */
	public void saveXML(IProgressMonitor monitor) throws Exception {
		try {
			saveAsXML(monitor, _file);
		} catch (Exception e) {
			throw(e);
		}
	} // void saveXML()
	
	
	/**
	 * Saves the DataMap to the given file.
	 * 
	 * @param monitor The progress monitor.
	 * @param newFile The file to save to.
	 * @throws Exception Any exception encountered while saving.
	 */
	public void saveAsXML(IProgressMonitor monitor, IFile newFile)
		throws Exception {
		
		try {
			
			//such a tiny line, but such power!
			String xml = "<?xml version=\"1.0\"?>\n" + _root.getXML(0);
	
			InputStream is = new ByteArrayInputStream(xml.getBytes());
	
			newFile.setContents(is, IFile.KEEP_HISTORY, monitor);
	
		} catch (Exception e) {
			monitor.isCanceled();
			throw(e);
		} // catch
	}


	/**
	 * Loads the DataMap from the given file.
	 *  Called automatically by the constructor.
	 * 
	 * @param file The file to load the DataMap from.
	 * @throws Exception Any exception encountered in loading the DataMap.
	 */
	public void loadXML(IFile file) throws Exception {
		_file = file;
		
		try {
			DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
			DocumentBuilder db = dbf.newDocumentBuilder();
			
			InputStream is = file.getContents();
			
			Document doc = db.parse(is);
			
			parseRoot(doc.getDocumentElement());
			
			// free things up.
			is.close();
			
		} catch (Exception e) {
			throw(e);
		} // catch
		
	} // void loadXML()
	
	
	/**
	 * Parses the root node and branches outward from there.
	 * 
	 * @param rootNode The root node of the datamap.
	 */
	private void parseRoot(Element rootNode) {
		_root = new DMSpecial();
		
		String name = rootNode.getAttribute("name");
		String comment = rootNode.getAttribute("comment");
		
		_root.setName(name);
		_root.setComment(comment);
		
		NodeList kids = rootNode.getChildNodes();
		
		for (int i=0; i<kids.getLength(); i++) {
			
			if (kids.item(i).getNodeType() == Node.ELEMENT_NODE) {
				DMItem temp = parseNode((Element) kids.item(i));
				if (temp != null) {
					_root.addChild(temp);
				}
			} // if
		} // for i
	} // void parseRoot(Element rootNode)
	
	
	/**
	 * Parses the node and its children into a DMItem.
	 * 
	 * @param node The Element to parse
	 * @return A DMItem representing the Element that was parsed.
	 */
	private DMItem parseNode(Element node) {
		DMItem ret = new DMIdentifier();
		
		String type = node.getNodeName();
		
		//*****************************************
		//           Enumeration
		if (type.equals("Enumeration")) {
			DMEnumeration temp = new DMEnumeration();
			
			NodeList enums = node.getElementsByTagName("enum");
			
			//Load the enumerations
			for (int i=0; i<enums.getLength(); i++) {
				Element enum = (Element) enums.item(i);
				
				temp.getEnums().add(enum.getAttribute("value"));
			} // for i
			
			ret = temp;
			
		//*****************************************
		//                Float
		} else if (type.equals("Float")) {
			DMFloat temp = new DMFloat();
			
			String lower = node.getAttribute("lower");
			String upper = node.getAttribute("upper");
			
			if (lower.equals("-infinity")) {
				temp.setLowerBound(new Double(Double.MIN_VALUE));
			} else {
				temp.setLowerBound(new Double(lower));
			}
			
			if (upper.equals("infinity")) {
				temp.setUpperBound(new Double(Double.MAX_VALUE));
			} else {
				temp.setUpperBound(new Double(upper));
			}
			
			
			ret = temp;
		
		//*****************************************
		//                Identifier
		} else if (type.equals("Identifier")) {
			DMIdentifier temp = new DMIdentifier();
			
			
			NodeList kids = node.getChildNodes();
		
			//traverse the Identifier's children and add them
			for (int i=0; i<kids.getLength(); i++) {
				if (kids.item(i).getNodeType() == Node.ELEMENT_NODE) {
					DMItem kid = parseNode((Element) kids.item(i));
					if (kid != null) {
						temp.addChild(kid);
					}
				} // if
			} // for i
			
			ret = temp;
		
		//*****************************************
		//                Integer
		} else if (type.equals("Integer")) {
			DMInteger temp = new DMInteger();
			
			temp.setLowerBound(new Integer(node.getAttribute("lower")));
			temp.setUpperBound(new Integer(node.getAttribute("upper")));

			ret = temp;
		
		
		//*****************************************
		//                Special
		} else if (type.equals("Special")) {
			DMSpecial temp = new DMSpecial();
			
			
			NodeList kids = node.getChildNodes();

			//traverse the Identifier's children and add them
			for (int i=0; i<kids.getLength(); i++) {
				if (kids.item(i).getNodeType() == Node.ELEMENT_NODE) {
					DMItem kid = parseNode((Element) kids.item(i));
					if (kid != null) {
						temp.addChild(kid);
					}
				} // if
			} // for i

			ret = temp;
			
		} else if (type.equals("String")) {
			DMString temp = new DMString();
			
			ret = temp;
			
		//*****************************************
		//             TopState
		} else if (type.equals("TopState")) {
			DMTopState temp = new DMTopState();
			
			//Link this back to the top state
			temp.setChildren(_root.getChildren());
			
			ret = temp;
		
		//*****************************************
		//               Link
		} else if (type.equals("Link")) {
			DMLink temp = new DMLink();
			
			IPath filePath = new Path(node.getAttribute("path"));
			
			IFile file = ResourcesPlugin.getWorkspace().getRoot().getFile(filePath);
		
			temp.setFile(file);
		
			ret = temp;
		
		//*****************************************
		//             Bad value
		} else {
			System.out.println("Invalid Datamap entry: " + node);
			return null;
		}
		
		//Every node has these properties
		String name = node.getAttribute("name");
		String comment = node.getAttribute("comment");
		
		ret.setName(name);
		ret.setComment(comment);
		
		return ret;
	} // public DMItem parseNode(Element node)
	




	/**
	 * 
	 * Imports from the given Visual Soar datamap file. Doesn't quite work yet.
	 * 
	 * @param filename The full path to the file.
	 * @throws Exception Any exception encountered in loading the datamap.
	 */
	public void importVS(String filename) throws Exception {
		DMSpecial root;
		
		try {
			// Yes, it does take three levels to get to the class we need.
			//  Seriously, Java, what were you thinking?
			InputStream is = new FileInputStream(filename);
			InputStreamReader isr = new InputStreamReader(is);
			BufferedReader inFile = new BufferedReader(isr);

			
			root = new DMSpecial();
			
			// The first line is the number of items in the file
			int num = Integer.parseInt(inFile.readLine());
			DMItem items[] = new DMItem[num];
			
			System.out.println("num: " + num);
			
			/* The first half of the file gives the types and values of
				the items in the datamap.
			   
				The format is:
			   
				type id [values] 
			*/
			for (int i=0; i<num; i++) {
				String line = inFile.readLine();
				System.out.println(line);
				String tokens[] = line.split("\\s");
				
				System.out.println("tokens length: " + tokens.length);
				
				for (int i2=0;i2< tokens.length; i2++) {
					System.out.println(tokens[i2]);
				} // for i2
				
				String type = tokens[0];
				 
				int id = Integer.parseInt(tokens[1]);
				DMItem temp = new DMIdentifier();
				
				System.out.println("type: " + type);
				System.out.println("id: " + id);
				
				
				if (type.equals("SOAR_ID")) {
					switch (id) {
						case 0:
							temp = new DMTopState();
							temp.setChildren(root.getChildren());
						break;
						case 1:
						case 2:
							temp = new DMSpecial();
						break;
						default:
							temp = new DMIdentifier();
						break;
					} // switch

				} else if (type.equals("ENUMERATION")) {
					temp = new DMEnumeration();
					
					//everything else on the line is the contents of the enumeration
					for (int i2 = 3; i2 < tokens.length; i2++) {
						((DMEnumeration) temp).getEnums().add(tokens[i2]);
					} // for
				} else if (type.equals("INTEGER_RANGE")) {
					DMInteger tempI = new DMInteger();
					
					tempI.setLowerBound(new Integer(tokens[2]));
					tempI.setUpperBound(new Integer(tokens[3]));
					
					temp = tempI;
					
				} else if (type.equals("FLOAT_RANGE")) {
					DMFloat tempF = new DMFloat();
					
					if (tokens[2].equals("-Infinity")) {
						tempF.setLowerBound(new Double(Double.MIN_VALUE));
					} else {
						tempF.setLowerBound(new Double(tokens[2]));
					}
					
					if (tokens[3].equals("Infinity")) {
						tempF.setUpperBound(new Double(Double.MAX_VALUE));
					} else {
						tempF.setUpperBound(new Double(tokens[3]));
					}
					
					temp = tempF;
				} else if (type.equals("STRING")) {
					temp = new DMString();
					
				} else {
					System.out.println("*ERROR: Unknown datamap type: " + type);
				} // else
				
				items[id] = temp;
			} // for i
			
			num = Integer.parseInt(inFile.readLine());
			
			System.out.println( "num: " + num);
			
			/*
			 * The second half of the file tells the name and parent of each node.
			 * The format:
			 * 
			 * parent name id
			 * 
			 * Start at 1 because the root never gets listed here.
			 */
			for (int i=1; i<num; i++) {
				String tokens[] = inFile.readLine().split("\\s");
				
				int id = Integer.parseInt(tokens[2]);
				int parent = Integer.parseInt(tokens[0]);
				String name = tokens[1];
				
				items[id].setName(name);
				items[id].setParent(items[parent]);
				
				
			} // for i
			
			
			// free those puppies up.
			inFile.close();
			isr.close();
			is.close();
			
		} catch (Exception e) {
			throw(e);
		} // catch
		
		_root = root;
	} // static DMItem importVS(String filename) throws Exception
	
	
	/**
	 * Generates a basic Datamap for the given project.
	 * 
	 * @param project The project to generate the Datamap for.
	 */
	public void generateDefault(IProject project) {
		DMSpecial root;
		
		root = new DMSpecial(project.getName());
		
		
		DMIdentifier io = new DMSpecial("io");
		io.addChild(new DMSpecial("input-link"));
		io.addChild(new DMIdentifier("output-link"));
		root.addChild(io);
		
		DMEnumeration name = new DMEnumeration("name");
		name.getEnums().add(project.getName());
		root.addChild(name);
		
		DMEnumeration superstate = new DMEnumeration("superstate");
		superstate.getEnums().add("nil");
		root.addChild(superstate);
		
		DMIdentifier top = new DMTopState("top-state");
		top.setChildren(root.getChildren()); // weird but necessary
		root.addChild(top);
		
		DMEnumeration type = new DMEnumeration("type");
		type.getEnums().add("state");
		root.addChild(type);
		
		_root = root;
	} // static DMItem getDefault(IProject project)
	
	
	
	/**
	 * Generates a basic Datamap for the given project.
	 * 
	 * @param project The project to generate the Datamap for.
	 */
	public void generateDefault(IFolder folder) {
		DMSpecial root;
		
		root = new DMSpecial(folder.getName());
		
		DMEnumeration name = new DMEnumeration("name");
		name.getEnums().add(folder.getName());
		root.addChild(name);
		
		
		//Link in to the parent datamap
		IContainer parentFolder = folder.getParent();
		IResource parentDM = parentFolder.findMember("datamap.xdm");
		
		if (parentDM instanceof IFile) {
			DMLink superstate = new DMLink("superstate");
			superstate.setFile( (IFile) parentDM );
			
			root.addChild(superstate);
		} else {
			DMEnumeration superstate = new DMEnumeration("superstate");
			superstate.getEnums().add("nil");
			
			root.addChild(superstate);
		} // else
		
		
		DMIdentifier top = new DMTopState("top-state");
		top.setChildren(root.getChildren()); // weird but necessary
		root.addChild(top);
		
		DMEnumeration type = new DMEnumeration("type");
		type.getEnums().add("state");
		root.addChild(type);
		
		_root = root;
	} // static DMItem getDefault(IFolder project)
	


} // class
