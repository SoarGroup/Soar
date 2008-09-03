package edu.umich.visualsoar.operatorwindow;

import java.awt.datatransfer.*;
import java.io.IOException;
import java.util.*;

class TransferableOperatorNodeLink implements Transferable {
	public static final DataFlavor[] flavors = { new DataFlavor(Integer.class, "An Id for the Operator Node") };
	private static final List flavorList = Arrays.asList(flavors);

	private Integer soarOperatorNodeId;

	private TransferableOperatorNodeLink() {}
	
	public TransferableOperatorNodeLink(Integer inSoarOperatorNodeId) {
		soarOperatorNodeId = inSoarOperatorNodeId;
	}
	
	public synchronized Object getTransferData(DataFlavor flavor) throws UnsupportedFlavorException, IOException {
		if (flavor.equals(flavors[0])) {
			return soarOperatorNodeId;
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
