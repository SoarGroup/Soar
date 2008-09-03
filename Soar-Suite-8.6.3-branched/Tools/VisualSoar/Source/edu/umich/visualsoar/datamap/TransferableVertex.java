package edu.umich.visualsoar.datamap;

import edu.umich.visualsoar.graph.Vertex;

import java.awt.datatransfer.*;
import java.io.IOException;
import java.util.*;
import edu.umich.visualsoar.graph.Edge;

/**
 * This allows vertexes to be used in drag and drop operations
 * @author Brad Jones
 */


public class TransferableVertex implements Transferable {
	public static final DataFlavor[] flavors = { new DataFlavor(Vector.class, "Visual Soar Working Memory Id, and Name") };
	private static final List flavorList = Arrays.asList(flavors);
	
	private Integer id;
	private String rep;
	private Edge edge;
	
	private TransferableVertex() {}
	
	public TransferableVertex(int i, String _rep) {
		id = new Integer(i);
		rep = new String(_rep);
	}
	
	public TransferableVertex(Vertex v, String _rep) {
		id = new Integer(v.getValue());
		rep = new String(_rep);
	}
	
	public TransferableVertex(Vertex v, String inRep, Edge e) {
		this(v,inRep);
		edge = e;
	}
	
	public synchronized Object getTransferData(DataFlavor flavor) throws UnsupportedFlavorException, IOException {
		if (flavor.equals(flavors[0])) {
			Vector v = new Vector(3);
			v.add(id);
			v.add(rep);
			v.add(edge);
			return v;
		}
		throw new UnsupportedFlavorException(flavor);
	}	

	/**
	 * @return a reference to the dataflavors
	 */
	public synchronized DataFlavor[] getTransferDataFlavors() {
		return flavors;
	}
	
	/**
	 * @param flavor the data flavor to check if it is supported 
	 * @return true if the data flavor is supported false otherwise
	 */
	public boolean isDataFlavorSupported(DataFlavor flavor) {
		return (flavorList.contains(flavor));
	}
}
