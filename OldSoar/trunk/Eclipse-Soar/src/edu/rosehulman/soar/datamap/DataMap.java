/*
 * Created on Feb 11, 2004
 *
 * To change the template for this generated file go to
 * Window&gt;Preferences&gt;Java&gt;Code Generation&gt;Code and Comments
 */
package edu.rosehulman.soar.datamap;


import edu.rosehulman.soar.*;
import edu.rosehulman.soar.datamap.items.*;

import org.eclipse.core.runtime.*;
import org.eclipse.core.resources.*;

import java.io.*;
import java.util.*;

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
	private HashMap _ids;
	private int _currID=1;
	
	/**
	 * Qualified name for the persistent property on a resource identifying the
	 *  datamap item that is associated with it.
	 */
	public static final QualifiedName VERTEX_ID
		= new QualifiedName("edu.rosehulman.soar", "vertexID");
	
	
	/**
	 * Gets a unique id to be assigned to a new node.
	 *  If the id is given to a node, this should be followed by a
	 *  call to <code>incrementCurrentID()</code>.
	 * @return The id.
	 */
	public int getCurrentID() {
		return _currID;
	}
	
	/**
	 * Increments the id. Should be called after using the value
	 *  given by <code>getCurrentID()</code>.
	 */
	public void incrementCurrentID() {
		++_currID;
	}
	
	/**
	 * Returns a unique id to be assigned to new nodes, then automatically
	 *  increments it.
	 * @return The id to use.
	 */
	public int getAndIncrementID() {
		return _currID++;
	}
	
	/**
	 * Tells the datamap to associate this item with its id, enabling you
	 *  to later retrieve it by its id.
	 * @param newItem
	 */
	public void register(DMItem newItem) {
		_ids.put(new Integer(newItem.getID()), newItem);
	}
	
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
	 * Gets the node with the given id. Not guaranteed to be up to date.
	 * @param id The id. Duh.
	 * @return The node with that id.
	 */
	public DMItem getItem(int id) {
		return getItem(new Integer(id));
	}
	
	
	/**
	 * Gets the node with the given id. Not guaranteed to be up to date.
	 * @param id The id.
	 * @return The node with that id.
	 */
	public DMItem getItem(Integer id) {
		return (DMItem) _ids.get(id);
	}
	
	

	
	
	/**
	 * Replaces the item with a new item of a different type but the same name.
	 * @param doomed The item to be replaced.
	 * @param replacementType An item of the type to replace this with.
	 */
	public void replace(DMItem doomed, DMItem replacementType) {
		DMItem temp = replacementType.createNew();
	
		temp.setName(doomed.getName());
		
		DMItem dmParent = doomed.getParent();
		ArrayList siblings = dmParent.getChildren(); 
		
		//Get the exact position of the item in the child list so we 
		// can insert the replacement there.
		int loc = siblings.indexOf(doomed);
		 
		//Make the new item recognize its parent.
		temp.setParent(dmParent);
		//Deal with the fact that it was automatically added to the 
		// end of the child list in the last call.
		siblings.remove(temp);
		//Replace the old child with a newer, better one.
		// Don't you wish you could do that with your kids?
		siblings.set(loc, temp);
		
		_ids.put(new Integer(temp.getID()), temp);
	}
	
	
	/**
	 * Removes the given item from the datamap.
	 * @param doomed The item to be removed.
	 */
	public void remove (DMItem doomed) {
		doomed.setParent(null);
		
		_ids.remove(new Integer(doomed.getID()));
	}
	
	
	/**
	 * Finds all attributes matching the given path. Since names
	 * are not necessarily unique, there may be several such matches.
	 * 
	 * @param names An ArrayList of Strings representing the path to the node.
	 * @return An ArrayList of DMItems that match the given path.
	 */
	public ArrayList find(ArrayList names) {
		return (ArrayList) find( getRoot(), names, 0).clone();
	}
	
	
	/**
	 * Finds all attributes matching the given path, relative to
	 *  the starting attribute provided.
	 *  
	 * @param names An ArrayList of Strings representing the path to the node.
	 * @param start The starting DMItem.
	 * @return An ArrayList of DMItems that match the given path.
	 */
	public ArrayList find(ArrayList names, DMItem start) {
		return (ArrayList) find( start, names, 0).clone();
	}
	
	
	/**
	 * Does the actual work for find.
	 *  Should be called with the DataMap root node and an index of 0. 
	 * 
	 * @param node The node to search. Use the DataMap root.
	 * @param names A list of the names to seek through in the DataMap
	 * @param index For recursion purposes. Use 0 here.
	 * @return An ArrayList of all DMItems that match the given path.
	 */
	private ArrayList find(DMItem node, ArrayList names, int index) {
	
		ArrayList kids = node.getChildren();
		
		if (names.size() == 0) {
			return kids;
		}
		
		String name = (String) names.get(index);
	
		//System.out.println(node + ": " + index);
	
		//This is the final item! Yipee!
		if (index == names.size()-1) {
		
		
			ArrayList ret = new ArrayList();
		
			for (int i=0; i<kids.size(); i++) {
				DMItem kid = (DMItem) kids.get(i);
			
				if (kid.getName().equals(name)) {
					ret.add(kid);
				} // if
			} // for
		
			return ret;
	
		//Not the final item. There's some recursing to be done.
		// I would like to point out that this would be prettier in Scheme.
		} else {
			ArrayList ret = new ArrayList();
		
			for (int i=0; i<kids.size(); i++) {
				DMItem kid = (DMItem) kids.get(i);

				if (kid.getName().equals(name) && kid.hasChildren()) {
					ret.addAll( find(kid, names, index+1));
				} // if
			} // for
		
			return ret;
		} // else
	} //ArrayList getSuggestions( ... )
	
	
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
		_ids = new HashMap();
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
		ArrayList pointers = new ArrayList();
		
		
		// Set up the root node
		_root = new DMSpecial();
		
		String name = rootNode.getAttribute("name");
		String comment = rootNode.getAttribute("comment");
		
		_root.setName(name);
		_root.setComment(comment);
		
		Integer id = new Integer(0);
		try {
			id = new Integer(rootNode.getAttribute("id"));
		} catch (NumberFormatException e) {
		}
		_root.setID(id.intValue());
		
		// put this in the HashMap
		_ids.put(id, _root);
		
		
		// Set up everything else
		NodeList kids = rootNode.getChildNodes();
		
		for (int i=0; i<kids.getLength(); i++) {
			
			if (kids.item(i).getNodeType() == Node.ELEMENT_NODE) {
				DMItem temp = parseNode((Element) kids.item(i), pointers);
				if (temp != null) {
					_root.addChild(temp);
				}
			} // if
		} // for i
		
		// Link pointer nodes into their targets
		for (int i=0; i<pointers.size(); ++i) {
			DMPointer pointer = (DMPointer) pointers.get(i);
			
			
			DMItem target = getItem(pointer.getTargetID());
			
			if (target != null) {
				pointer.setTarget( target );
			} else { //cull dead links
				remove (pointer);
			}
		}
		
	} // void parseRoot(Element rootNode)
	
	
	/**
	 * Parses the node and its children into a DMItem.
	 * 
	 * @param node The Element to parse
	 * @return A DMItem representing the Element that was parsed.
	 */
	private DMItem parseNode(Element node, ArrayList pointers) {
		DMItem ret = new DMIdentifier();
		
		String type = node.getNodeName();
		
		//*****************************************
		//           Enumeration
		if (type.equals("Enumeration")) {
			DMEnumeration temp = new DMEnumeration();
			
			NodeList enums = node.getElementsByTagName("enum");
			
			//Load the enumerations
			for (int i=0; i<enums.getLength(); i++) {
				Element en = (Element) enums.item(i);
				
				temp.getEnums().add(en.getAttribute("value"));
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
					DMItem kid = parseNode((Element) kids.item(i), pointers);
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
					DMItem kid = parseNode((Element) kids.item(i), pointers);
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
		//             TopState - shouldn't exist anymore
		} else if (type.equals("TopState")) {
			DMTopState temp = new DMTopState();
			
			//Link this back to the top state
			temp.setChildren(_root.getChildren());
			
			ret = temp;
			
		//*****************************************
		//               Pointer
		} else if (type.equals("Pointer")) {
			DMPointer temp = new DMPointer();
			
			
			temp.setTargetID( Integer.parseInt(node.getAttribute("targetID")) );
			
	
			ret = temp;
			
			pointers.add(temp);
		
		//*****************************************
		//             Bad value
		} else {
			System.out.println("Invalid Datamap entry: " + node);
			return null;
		}
		
		
		//*****************************************
		//Every node has these properties
		String name = node.getAttribute("name");
		String comment = node.getAttribute("comment");
		Integer id = new Integer(0);
		try {
			id = new Integer(node.getAttribute("id"));
		} catch (NumberFormatException e) {
			System.out.println("name: " + node.getAttribute("name"));
			System.out.println("id: " + node.getAttribute("id"));
		}
		
		ret.setName(name);
		ret.setComment(comment);
		
		int idInt = id.intValue(); 
		
		ret.setID(idInt);
		
		
		// update the current ID
		if (idInt >= _currID) 
			_currID = idInt + 1;
		
		// put this in the HashMap
		_ids.put(id, ret);
		
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
	} // static DMItem getDefault(IFolder project)
	
	
	/**
	 * Resturns the datamap associated with the given file, 
	 *  or <code>null</code> if one does not exist.
	 * 
	 * @param file The Soar source file.
	 * @return The datamap for this file.
	 */
	public static DataMap getAssociatedDatamap(IResource res) {
		return SoarPlugin.getDataMap(res);
		
		/*
		IProject proj = res.getProject();
		
		try {
			IFile dmFile = proj.getFile(new Path("datamap.xdm"));
			
			if (dmFile.exists()) {
				DataMap dm = new DataMap(dmFile);
			
				return dm;
				
			} else {
				return null;
			}
		} catch (Exception e) {
			e.printStackTrace();
			
			return null;
		} */
	}
	
	
	
	/**
	 * Returns the item in the datamap that is associated with this resource.
	 * @param res
	 * @return
	 */
	public DMItem getAssociatedVertex(IResource res) {
		if (res instanceof IContainer) {
			return getAssociatedVertex( (IContainer) res);
		} else {
			return getAssociatedVertex( res.getParent() );
		}
	}
	
	
	private DMItem getAssociatedVertex(IContainer folder) {
		if (folder instanceof IProject) return getRoot();
		
		try {
			String strID = folder.getPersistentProperty(DataMap.VERTEX_ID);
			
			Integer id = new Integer(strID);
			
			DMItem ret = getItem(id);
			
			if (ret == null) {
				return getAssociatedVertex( folder.getParent() );
			} else {
				return ret;
			}
			
		} catch (CoreException e) {
			return getRoot();
		} catch (NumberFormatException e) {
			return getAssociatedVertex( folder.getParent() );
		}
	}
	
	
	/**
	 * Gets the id of the vertex this resource is associated with.
	 * @param res
	 * @return
	 */
	public static int getAssociatedVertexID(IResource res) {
		if (res instanceof IContainer) {
			return getAssociatedVertexID( (IContainer) res);
		} else {
			return getAssociatedVertexID( res.getParent() );
		}
	}
	
	private static int getAssociatedVertexID(IContainer folder) {
		if (folder instanceof IProject) return 0;
		
		try {
			String strID = folder.getPersistentProperty(DataMap.VERTEX_ID);
			
			Integer id = new Integer(strID);
			
			return id.intValue();
			
		} catch (CoreException e) {
			return 0;
		} catch (NumberFormatException e) {
			return 0;
		}
	}


} // class
