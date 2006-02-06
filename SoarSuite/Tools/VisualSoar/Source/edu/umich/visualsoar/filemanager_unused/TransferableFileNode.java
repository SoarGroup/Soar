package edu.umich.visualsoar.filemanager;

import java.awt.datatransfer.*;
import java.util.*;

public class TransferableFileNode implements Transferable {
	public static final DataFlavor[] flavors = { new DataFlavor(FileNode.class,"Visual Soar FileNode") };
	private static final List flavorList = Arrays.asList(flavors);
	
	private FileNode fileNode;
	
	private TransferableFileNode() {}
	
	public TransferableFileNode(FileNode inFileNode) {
		fileNode = inFileNode;
	}
	
	public synchronized Object getTransferData(DataFlavor flavor) throws java.io.IOException, UnsupportedFlavorException {
		if(flavor.equals(flavors[0])) {
			return fileNode;
		}
		throw new UnsupportedFlavorException(flavor);
	}
	
	public synchronized DataFlavor[] getTransferDataFlavors() {
		return flavors;
	}
	
	public boolean isDataFlavorSupported(DataFlavor flavor) {
		return flavorList.contains(flavor);
	}
}
