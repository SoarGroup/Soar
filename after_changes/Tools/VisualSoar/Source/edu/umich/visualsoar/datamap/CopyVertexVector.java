package edu.umich.visualsoar.datamap;

import java.util.*;
import java.awt.datatransfer.*;
import java.io.IOException;
import edu.umich.visualsoar.graph.*;

/**
 * This allows verticies to be copied in a shallow manner
 * @author Jon Bauman
 */
  
public class CopyVertexVector extends Vector implements Transferable {
	public static final DataFlavor[] 	flavors = 
		{ new DataFlavor(Vector.class, 
			"Visual Soar Vertex Type and Name") };
	private static final List 			flavorList = 
		Arrays.asList(flavors);

	public class CopyVertex {
		private String		name;
		private	SoarVertex	vertex;

		public CopyVertex(String inName, SoarVertex inVertex) {
			name = inName;
			vertex = inVertex;
		}
		
		public String getName() {
			return name;
		}
		
		public SoarVertex getVertex() {
			return vertex;
		}
	}

	public CopyVertexVector(int capacity) {
		super(capacity);
	}

	public void add(String name, SoarVertex vertex) {
		add(new CopyVertex(name, vertex));
	}
	
	public String getName(int index) {
		CopyVertex v = (CopyVertex)get(index);
		return v.getName();
	}

	public SoarVertex getVertex(int index) {
		CopyVertex v = (CopyVertex)get(index);
		return v.getVertex();
	}

	public synchronized Object getTransferData(DataFlavor flavor) throws UnsupportedFlavorException, IOException {
		if (flavor.equals(flavors[0])) {
			return this;
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
