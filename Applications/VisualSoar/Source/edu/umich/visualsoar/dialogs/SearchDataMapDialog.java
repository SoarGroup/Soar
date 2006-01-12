package edu.umich.visualsoar.dialogs;

import edu.umich.visualsoar.datamap.*;
import edu.umich.visualsoar.dialogs.*;
import edu.umich.visualsoar.graph.*;
import edu.umich.visualsoar.MainFrame;
import edu.umich.visualsoar.util.*;
import edu.umich.visualsoar.misc.*;
import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.filechooser.FileFilter;
import java.awt.*;
import java.awt.dnd.*;
import java.awt.datatransfer.*;
import java.awt.event.*;
import java.io.*;
import java.lang.*;
import java.util.*;

/**
 * Dialog which searches a datamap for requested edges.
 *
 * @author Brian Harleton
 * @see DataMapTree
 * @see DataMap
 */
public class SearchDataMapDialog extends JDialog {

  SearchDataMapFindPanel findPanel = new SearchDataMapFindPanel();

  SearchDataMapOptionsPanel optionsPanel = new SearchDataMapOptionsPanel();

  SearchDataMapButtonPanel buttonPanel = new SearchDataMapButtonPanel();

  private DataMapTree dmt = null;
  private SoarWorkingMemoryModel swmm = null;
  private FakeTreeNode rootOfSearch = null;
  private String lastSearch = "";

  // Search variables
  private edu.umich.visualsoar.util.Queue queue;
  private boolean[] visitedVertices;
  private int numberOfVertices;

	/**
   *  Constructor for the SearchDataMapDialog
   *  @param  owner frame that owns this dialog window
   *  @param  tree  DataMapTree that is being searched
   *  @param  rootNode the FakeTreeNode in the datamap that is currently selected and
   *  from which the search will begin at.
	 */
	public SearchDataMapDialog(final Frame owner, DataMapTree tree, FakeTreeNode rootNode) {
		super(owner, "Search DataMap", false);

    dmt = tree;
    swmm = (MainFrame.getMainFrame()).getOperatorWindow().getDatamap();
    queue = new QueueAsLinkedList();
    rootOfSearch = rootNode;
    initializeSearch();


    setResizable(false);
    Container contentPane = getContentPane();
    GridBagLayout gridbag = new GridBagLayout();
    GridBagConstraints c = new GridBagConstraints();
    contentPane.setLayout(gridbag);

    c.gridwidth = GridBagConstraints.REMAINDER;
    c.fill = GridBagConstraints.HORIZONTAL;

    contentPane.add(findPanel, c);
    contentPane.add(optionsPanel, c);
    contentPane.add(buttonPanel, c);
    pack();
    getRootPane().setDefaultButton(buttonPanel.findNextButton);

    addWindowListener(new WindowAdapter() {
      public void windowOpened(WindowEvent we) {
        setLocationRelativeTo(owner);
        findPanel.requestFocus();
      }
    });

    buttonPanel.cancelButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        dispose();
      }
    });

    buttonPanel.findNextButton.addActionListener(new ActionListener() {
      public void actionPerformed(ActionEvent e) {
        Object[] theData = findPanel.getData();
        Boolean[] theOptions = optionsPanel.getData();

        String toFind = (String) theData[0];
        Boolean caseSensitive = (Boolean) theData[1];

        if(!toFind.equals(lastSearch)) {
          initializeSearch();
          lastSearch = toFind;
        }

        if(!caseSensitive.booleanValue()) {
          toFind = toFind.toLowerCase();
        }

        FakeTreeNode foundftn = null;
        boolean edgeNotFound = true;
        int children = 0;


        while((!queue.isEmpty())  && edgeNotFound) {
          FakeTreeNode ftn = (FakeTreeNode)queue.dequeue();
          int rootValue = ftn.getEnumeratingVertex().getValue();
          visitedVertices[rootValue] = true;
          children = ftn.getChildCount();

          // See if current FakeTreeNode's edge match desired edge
          if( ftn.getEdge() != null) {
            if( (ftn.getEdge()).getName().equals(toFind)  &&
                ((theOptions[0].booleanValue() && ftn.getEnumeratingVertex() instanceof SoarIdentifierVertex)
                    || (theOptions[1].booleanValue() && ftn.getEnumeratingVertex() instanceof EnumerationVertex)
                    || (theOptions[2].booleanValue() && ftn.getEnumeratingVertex() instanceof StringVertex)
                    || (theOptions[3].booleanValue() && ftn.getEnumeratingVertex() instanceof IntegerRangeVertex)
                    || (theOptions[4].booleanValue() && ftn.getEnumeratingVertex() instanceof FloatRangeVertex))   ) {
              edgeNotFound = false;
              foundftn = ftn;
            }
          }

          // Examine children of ftn
          if((children != 0) && edgeNotFound){
            for(int i = 0; i < children; i++) {
              FakeTreeNode childftn = ftn.getChildAt(i);
              int vertexValue = childftn.getEnumeratingVertex().getValue();
              if(! visitedVertices[vertexValue]) {
                visitedVertices[vertexValue] = true;
                queue.enqueue(childftn);
              }   // if never visited vertex
              else {
                // Check this edge since it won't be added to the queue
                if( childftn.getEdge() != null) {
                  if( (childftn.getEdge()).getName().equals(toFind)  &&
                    ((theOptions[0].booleanValue() && childftn.getEnumeratingVertex() instanceof SoarIdentifierVertex)
                      || (theOptions[1].booleanValue() && childftn.getEnumeratingVertex() instanceof EnumerationVertex)
                      || (theOptions[2].booleanValue() && childftn.getEnumeratingVertex() instanceof StringVertex)
                      || (theOptions[3].booleanValue() && childftn.getEnumeratingVertex() instanceof IntegerRangeVertex)
                      || (theOptions[4].booleanValue() && childftn.getEnumeratingVertex() instanceof FloatRangeVertex))   ) {
                    edgeNotFound = false;
                    foundftn = childftn;
                  }
                }
              }    // end of else already visited this vertex
            }   // for checking all of ftn's children
          }   // if ftn has children
          else if((children != 0) && !edgeNotFound) {
            // still add children to queue for possible continued searching
            for(int i =0; i < children; i++) {
              FakeTreeNode childftn = ftn.getChildAt(i);
              int vertexValue = childftn.getEnumeratingVertex().getValue();
              if(! visitedVertices[vertexValue]) {
                visitedVertices[vertexValue] = true;
                queue.enqueue(childftn);
              } // if never visited vertex, enqueue
            }
          } // end of if edge found, still add children to queue for continued searching
          
        }   // while queue is not empty, examine each vertex in it

        if(foundftn != null) {
          dmt.highlightEdge(foundftn);
       }
        else {
          JOptionPane.showMessageDialog(null, "No more instances found");
        }

        if(! buttonPanel.keepDialog.isSelected()) {
          dispose();
        }
      }
    });



  }

  /**
   *  Initializes the search queue for a search of the datamap
   *  called when a new search dialog is created or when the user changes the
   *  desired word
   *  @param ftn the wme from which the search is to begin
   */
  private void initializeSearch() {
    queue = new QueueAsLinkedList();
    FakeTreeNode ftn = rootOfSearch;
    numberOfVertices = swmm.getNumberOfVertices();
    lastSearch = "";
    visitedVertices = new boolean[numberOfVertices];
    for(int i = 0; i < numberOfVertices; i++)
      visitedVertices[i] = false;
    queue.enqueue(ftn);
  }

}     // end of SearchDataMapDialog class
