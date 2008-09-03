package edu.umich.visualsoar.datamap;

import edu.umich.visualsoar.graph.*;
import edu.umich.visualsoar.MainFrame;
import edu.umich.visualsoar.graph.SoarIdentifierVertex;
import edu.umich.visualsoar.graph.NamedEdge;
import edu.umich.visualsoar.misc.*;
import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import java.beans.*;
import java.util.Vector;

/**
 * This class is the internal frame in which the DataMap resides
 * @author Brad Jones
 * @see DataMapTree
 */

public class DataMap extends CustomInternalFrame 
{
	DataMapTree dataMapTree;
    int id;

////////////////////////////////////////
// Constructors
////////////////////////////////////////


	// Deny Default Construction
	private DataMap() {}

    /**
     * Create a DataMap in an internal frame
     * @param swmm Working Memory - SoarWorkingMemoryModel
     * @param siv the vertex that is the root of datamap
     * @param title the name of the datamap window, generally the name of the selected operator node
     * @see SoarWMTreeModelWrapper
     * @see DataMapTree
     */
	public DataMap(SoarWorkingMemoryModel swmm, SoarIdentifierVertex siv,String title) 
    {
		super("Datamap " + title,true,true,true,true);
        setType(DATAMAP);
		setBounds(0,0,250,100);
        id = siv.getValue();

        // Retile the internal frames after closing a window
        setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);
        addInternalFrameListener(
            new InternalFrameAdapter() 
            {
                public void internalFrameClosing(InternalFrameEvent e) 
                    {

                        MainFrame mf = MainFrame.getMainFrame();
                        mf.getDesktopPane().dmRemove(id);
                        dispose();

                        if(Preferences.getInstance().isAutoTilingEnabled())
                        {
                            mf.getDesktopPane().performTileAction();
                        }
                        mf.selectNewInternalFrame();
                    }
            });

        TreeModel soarTreeModel = new SoarWMTreeModelWrapper(swmm,siv,title);
        soarTreeModel.addTreeModelListener(new DataMapListenerModel());
		dataMapTree = new DataMapTree(soarTreeModel,swmm);
		getContentPane().add(new JScrollPane(dataMapTree));

        dataMapTree.setCellRenderer(new DataMapTreeRenderer());
		
		JMenuBar menuBar = new JMenuBar();
		JMenu editMenu = new JMenu("Edit");
        JMenu validateMenu = new JMenu("Validation");

/*		Too Dangerous, see DataMapTree.java

		JMenuItem cutItem = new JMenuItem("Cut");		
		editMenu.add(cutItem);
		cutItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_X,Event.CTRL_MASK));
		cutItem.addActionListener(dataMapTree.cutAction);
*/
		
		JMenuItem copyItem = new JMenuItem("Copy");
		editMenu.add(copyItem);
		copyItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_C,Event.CTRL_MASK));
		copyItem.addActionListener(dataMapTree.copyAction);
		
		
		JMenuItem pasteItem = new JMenuItem("Paste");
		editMenu.add(pasteItem);
		pasteItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_V,Event.CTRL_MASK));
		pasteItem.addActionListener(dataMapTree.pasteAction);


        JMenuItem searchItem = new JMenuItem("Search DataMap");
        editMenu.add(searchItem);
        searchItem.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_F, Event.CTRL_MASK));
        searchItem.addActionListener(dataMapTree.searchAction);

        JMenuItem validateItem = new JMenuItem("Validate DataMap");
        validateMenu.add(validateItem);
        validateItem.addActionListener(dataMapTree.validateDataMapAction);

        JMenuItem removeItem = new JMenuItem("Remove Non-Validated");
        validateMenu.add(removeItem);
        removeItem.addActionListener(dataMapTree.removeInvalidAction);

		menuBar.add(editMenu);
        menuBar.add(validateMenu);
		
		setJMenuBar(menuBar);
	}

    public int getId()
    {
        return id;
    }

    public Vector searchTestDataMap(SoarIdentifierVertex in_siv, String dataMapName) 
    {
        return dataMapTree.searchTestDataMap(in_siv, dataMapName);
    }

    public Vector searchCreateDataMap(SoarIdentifierVertex in_siv, String dataMapName) 
    {
        return dataMapTree.searchCreateDataMap(in_siv, dataMapName);
    }

    public Vector searchTestNoCreateDataMap(SoarIdentifierVertex in_siv, String dataMapName) 
    {
        return dataMapTree.searchTestNoCreateDataMap(in_siv, dataMapName);
    }

    public Vector searchCreateNoTestDataMap(SoarIdentifierVertex in_siv, String dataMapName) 
    {
        return dataMapTree.searchCreateNoTestDataMap(in_siv, dataMapName);
    }

    public Vector searchNoTestNoCreateDataMap(SoarIdentifierVertex in_siv, String dataMapName) 
    {
        return dataMapTree.searchNoTestNoCreateDataMap(in_siv, dataMapName);
    }

    public void displayGeneratedNodes() 
    {
        dataMapTree.displayGeneratedNodes();
    }

    /**
     *  Selects (highlights and centers) the requested edge within the datamap.
     *  @param edge the requested NamedEdge to select
     *  @return true if success, false if could not find edge
     */
    public boolean selectEdge(NamedEdge edge) 
    {
        FakeTreeNode node = dataMapTree.selectEdge(edge);

        if(node != null) 
        {
            TreePath path = new TreePath(node.getTreePath().toArray());
            dataMapTree.scrollPathToVisible(path);
            dataMapTree.setSelectionPath(path);

            return true;
        }
        else  
        {
            JOptionPane.showMessageDialog(null, "Could not find a matching FakeTreeNode in the datamap");
            return false;
        }
    }

    /**
	 * This class is meant to catch if the user closes an internal frame without saving
	 * the file, it prompts them to save, or discard the file or cancel the close
	 */
	class CloseListener implements VetoableChangeListener 
    {
		public void vetoableChange(PropertyChangeEvent e) throws PropertyVetoException 
        {
		}
    }

    /**
     *  This class customizes the look of the the DataMap Tree.
     *  It is responsible for changing the color of the text of nodes
     *  generated by the datamap generator.
     */
    private class DataMapTreeRenderer extends DefaultTreeCellRenderer 
    {
        public DataMapTreeRenderer() 
        {
        }

        public Component getTreeCellRendererComponent(
            JTree tree,
            Object value,
            boolean sel,
            boolean expanded,
            boolean leaf,
            int row,
            boolean hasFocus) 
        {

            super.getTreeCellRendererComponent(
                tree, value, sel,
                expanded, leaf, row,
                hasFocus);


            if (isGeneratedNode(value)) 
            {
                setForeground(Color.green.darker().darker());
            }

/// The following were just for testing purposes /////      
/*
  if (isUnTested(value)) 
  {
  setForeground(Color.magenta.darker());
  }
*/
/*
  if (isNotMentioned(value)) 
  {
  setForeground(Color.red.darker());
  }

  if(isTEST(value)) 
  {
  setForeground(Color.blue.darker());
  }
*/

            return this;
        }

        protected boolean isGeneratedNode(Object value) 
        {
            if(value instanceof FakeTreeNode) 
            {
                FakeTreeNode node = (FakeTreeNode) value;
                NamedEdge ne = node.getEdge();
                if(ne != null) 
                {
                    if(ne.isGenerated())  
                    {
                        return true;
                    }
                    else 
                    {
                        return false;
                    }
                }
            }
            return false;
        }   // end of isGeneratedNode() member function

        protected boolean isNotMentioned(Object value) 
        {
            if(value instanceof FakeTreeNode) 
            {
                FakeTreeNode node = (FakeTreeNode) value;
                NamedEdge ne = node.getEdge();
                if(ne != null) 
                {
                    if(ne.notMentioned())  
                    {
                        return true;
                    }
                    else 
                    {
                        return false;
                    }
                }
            }
            return false;
        }   // end of isNotMentioned() member function

        protected boolean isUnTested(Object value) 
        {
            if(value instanceof FakeTreeNode) 
            {
                FakeTreeNode node = (FakeTreeNode) value;
                NamedEdge ne = node.getEdge();
                if(ne != null) 
                {
                    if(!ne.isTested())  
                    {
                        return true;
                    }
                    else 
                    {
                        return false;
                    }
                }
            }
            return false;
        }   // end of isNotMentioned() member function


        protected boolean isTEST(Object value) 
        {
            if(value instanceof FakeTreeNode) 
            {
                FakeTreeNode node = (FakeTreeNode) value;
                NamedEdge ne = node.getEdge();
                if(ne != null) 
                {
                    if(ne.getTestedStatus() == 1)  
                    {
                        return true;
                    }
                    else 
                    {
                        return false;
                    }
                }
            }
            return false;
        }   // end of isNotMentioned() member function
    
    }   // end of DataMapRenderer class

    private class DataMapListenerModel implements TreeModelListener 
    {
        public void treeNodesChanged(TreeModelEvent e) 
        {

        }
        public void treeNodesInserted(TreeModelEvent e) 
        {
            // Make sure to expand path to display created node
            dataMapTree.expandPath(e.getTreePath());



        }
        public void treeNodesRemoved(TreeModelEvent e) 
        {

        }
        public void treeStructureChanged(TreeModelEvent e) 
        {
        }

    }
} // end of DataMap class

