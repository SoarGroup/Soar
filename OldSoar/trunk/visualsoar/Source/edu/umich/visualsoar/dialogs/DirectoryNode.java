
class DirectoryNode extends DefaultMutableTreeNode {
	private static DirectoryFilter directoryFilter = new DirectoryFilter();
	private boolean explored = false;
	private String name;

	/**
	* assumptions: the File Passed in must be a directory
	*/
	public DirectoryNode(File inFile) {
		setUserObject(inFile);
		if(inFile.getParentFile() == null)
			name = inFile.getPath();
		else
			name = inFile.getName();
	}

	public boolean getAllowsChildren() { return true; }
	public boolean isLeaf() { return false; }
	public File getDirectory() { return (File)getUserObject(); }
	
	public boolean isExplored() {
		return explored;
	}

	public void explore() {
		File file = getDirectory();
		File[] children = file.listFiles(directoryFilter);

		for(int i = 0; i < children.length; ++i)
			add(new DirectoryNode(children[i]));

		explored = true;
	}

	public String toString() {
		return name;
	}



}

class DirectoryFilter implements FileFilter {
	public boolean accept(File inPathName) {
		return inPathName.isDirectory();
	}
}
