package edu.umich.visualsoar.datamap;

import edu.umich.visualsoar.graph.*;
import edu.umich.visualsoar.MainFrame;
import edu.umich.visualsoar.dialogs.*;
import edu.umich.visualsoar.util.*;
import edu.umich.visualsoar.misc.*;
import edu.umich.visualsoar.operatorwindow.*;
import edu.umich.visualsoar.parser.ParseException;
import edu.umich.visualsoar.parser.TokenMgrError;
import edu.umich.visualsoar.parser.SoarProduction;
import edu.umich.visualsoar.parser.Triple;
import javax.swing.*;
import javax.swing.tree.*;
import java.awt.*;
import java.awt.dnd.*;
import java.awt.datatransfer.*;
import java.awt.event.*;
import java.io.*;
import java.util.*;

/**
 * A class to implement the behavior of the DataMap
 * @author Jon Bauman 
 * @author Brad Jones
 */

public class DataMapTree extends JTree implements ClipboardOwner 
{

	////////////////////////////////////////
	// DataMembers
	////////////////////////////////////////
	public static Clipboard clipboard = new Clipboard("Datamap Clipboard");
	public static NamedEdge cutEdge;
	private static DataMapTree s_DataMapTree;

	//public Action cutAction = new CutAction();
	public Action copyAction = new CopyAction();
	public Action pasteAction = new PasteAction();
	public Action searchAction = new SearchAction();
	public Action validateDataMapAction = new ValidateDataMapAction();
	public Action removeInvalidAction = new RemoveInvalidAction();

	private SoarWorkingMemoryModel swmm;
	private static JPopupMenu contextMenu = new JPopupMenu();
	private static JMenuItem AddIdentifierItem = new JMenuItem("Add Identifier...");
	private static JMenuItem AddEnumerationItem = new JMenuItem("Add Enumeration...");
	private static JMenuItem AddIntegerItem = new JMenuItem("Add Integer...");
	private static JMenuItem AddFloatItem = new JMenuItem("Add Float...");
	private static JMenuItem AddStringItem = new JMenuItem("Add String...");

	private static JMenuItem SearchForItem = new JMenuItem("Search For...");
	private static JMenuItem FindUsingProdsItem = new JMenuItem("Find Productions that Create or Test this WME");
	private static JMenuItem FindTestingProdsItem = new JMenuItem("Find Productions that Test this WME");
	private static JMenuItem FindCreatingProdsItem = new JMenuItem("Find Productions that Create this WME");

	private static JMenuItem RemoveAttributeItem = new JMenuItem("Delete Attribute...");
	private static JMenuItem RenameAttributeItem = new JMenuItem("Rename Attribute...");
	private static JMenuItem EditValueItem = new JMenuItem("Edit Value(s)...");

	private static JMenuItem EditCommentItem = new JMenuItem("Add/Edit Comment...");
	private static JMenuItem RemoveCommentItem = new JMenuItem("Remove Comment");

	private static JMenuItem ValidateEntryItem = new JMenuItem("Validate Entry");
	private static JMenuItem ValidateAllItem = new JMenuItem("Validate All");

	private static JMenu ChangeTypeSubMenu = new JMenu("Change DataMap Type...");
	private static JMenuItem ChangeToIdentifierItem = new JMenuItem("to Identifier");
	private static JMenuItem ChangeToEnumerationItem = new JMenuItem("to Enumeration");
	private static JMenuItem ChangeToIntegerItem = new JMenuItem("to Integer");
	private static JMenuItem ChangeToFloatItem = new JMenuItem("to Float");
	private static JMenuItem ChangeToStringItem = new JMenuItem("to String");

	public static TreePath OriginalSelectionPath;

	static 
	{

		contextMenu.add(AddIdentifierItem);
		contextMenu.add(AddEnumerationItem);
		contextMenu.add(AddIntegerItem);
		contextMenu.add(AddFloatItem);
		contextMenu.add(AddStringItem);

		contextMenu.addSeparator();
		contextMenu.add(SearchForItem);
		contextMenu.add(FindUsingProdsItem);
		contextMenu.add(FindTestingProdsItem);
		contextMenu.add(FindCreatingProdsItem);
		contextMenu.addSeparator();

		contextMenu.add(ChangeTypeSubMenu);
		contextMenu.add(RemoveAttributeItem);
		contextMenu.add(RenameAttributeItem);
		contextMenu.add(EditValueItem);

		contextMenu.addSeparator();

		contextMenu.add(EditCommentItem);
		contextMenu.add(RemoveCommentItem);

		contextMenu.addSeparator();

		contextMenu.add(ValidateEntryItem);
		contextMenu.add(ValidateAllItem);



		ChangeTypeSubMenu.add(ChangeToIdentifierItem);
		ChangeToIdentifierItem.addActionListener(
				new ActionListener() 
				{
					public void actionPerformed(ActionEvent e) 
					{
						DataMapTree dmt = (DataMapTree)contextMenu.getInvoker();
						dmt.changeTypeTo(0);
					}
				});

		ChangeTypeSubMenu.add(ChangeToEnumerationItem);
		ChangeToEnumerationItem.addActionListener(
				new ActionListener() 
				{
					public void actionPerformed(ActionEvent e) 
					{
						DataMapTree dmt = (DataMapTree)contextMenu.getInvoker();
						dmt.changeTypeTo(1);
					}
				});

		ChangeTypeSubMenu.add(ChangeToIntegerItem);
		ChangeToIntegerItem.addActionListener(
				new ActionListener() 
				{
					public void actionPerformed(ActionEvent e) 
					{
						DataMapTree dmt = (DataMapTree)contextMenu.getInvoker();
						dmt.changeTypeTo(2);
					}
				});

		ChangeTypeSubMenu.add(ChangeToFloatItem);
		ChangeToFloatItem.addActionListener(
				new ActionListener() 
				{
					public void actionPerformed(ActionEvent e) 
					{
						DataMapTree dmt = (DataMapTree)contextMenu.getInvoker();
						dmt.changeTypeTo(3);
					}
				});

		ChangeTypeSubMenu.add(ChangeToStringItem);
		ChangeToStringItem.addActionListener(
				new ActionListener() 
				{
					public void actionPerformed(ActionEvent e) 
					{
						DataMapTree dmt = (DataMapTree)contextMenu.getInvoker();
						dmt.changeTypeTo(4);
					}
				});


		AddIdentifierItem.addActionListener(
				new ActionListener() 
				{
					public void actionPerformed(ActionEvent e) 
					{
						DataMapTree dmt = (DataMapTree)contextMenu.getInvoker();
						dmt.addIdentifier();
					}
				});

		AddEnumerationItem.addActionListener(
				new ActionListener() 
				{
					public void actionPerformed(ActionEvent e) 
					{
						DataMapTree dmt = (DataMapTree)contextMenu.getInvoker();
						dmt.addEnumeration();
					}
				}); 

		AddIntegerItem.addActionListener(
				new ActionListener() 
				{
					public void actionPerformed(ActionEvent e) 
					{
						DataMapTree dmt = (DataMapTree)contextMenu.getInvoker();
						dmt.addInteger();
					}
				}); 

		AddFloatItem.addActionListener(new ActionListener() 
		{
			public void actionPerformed(ActionEvent e) 
			{
				DataMapTree dmt = (DataMapTree)contextMenu.getInvoker();
				dmt.addFloat();
			}
		});

		AddStringItem.addActionListener(
				new ActionListener() 
				{
					public void actionPerformed(ActionEvent e) 
					{
						DataMapTree dmt = (DataMapTree)contextMenu.getInvoker();
						dmt.addString();
					}
				});

		SearchForItem.addActionListener(
				new ActionListener() 
				{
					public void actionPerformed(ActionEvent e) 
					{
						DataMapTree dmt = (DataMapTree)contextMenu.getInvoker();
						dmt.searchFor();
					}
				});


		FindUsingProdsItem.addActionListener(
				new ActionListener() 
				{
					public void actionPerformed(ActionEvent e) 
					{
						DataMapTree dmt = (DataMapTree)contextMenu.getInvoker();
						dmt.findProds(true, true);
					}
				});

		FindTestingProdsItem.addActionListener(
				new ActionListener() 
				{
					public void actionPerformed(ActionEvent e) 
					{
						DataMapTree dmt = (DataMapTree)contextMenu.getInvoker();
						dmt.findProds(true, false);
					}
				});

		FindCreatingProdsItem.addActionListener(
				new ActionListener() 
				{
					public void actionPerformed(ActionEvent e) 
					{
						DataMapTree dmt = (DataMapTree)contextMenu.getInvoker();
						dmt.findProds(false, true);
					}
				});

		RemoveAttributeItem.addActionListener(
				new ActionListener() 
				{
					public void actionPerformed(ActionEvent e) 
					{
						DataMapTree dmt = (DataMapTree)contextMenu.getInvoker();
						dmt.removeEdge();
					}
				});

		RenameAttributeItem.addActionListener(
				new ActionListener() 
				{
					public void actionPerformed(ActionEvent e) 
					{
						DataMapTree dmt = (DataMapTree)contextMenu.getInvoker();
						dmt.renameEdge();
					}
				});

		EditValueItem.addActionListener(
				new ActionListener() 
				{
					public void actionPerformed(ActionEvent e) 
					{
						DataMapTree dmt = (DataMapTree)contextMenu.getInvoker();
						dmt.editValue();
					}
				});

		EditCommentItem.addActionListener(
				new ActionListener() 
				{
					public void actionPerformed(ActionEvent e) 
					{
						DataMapTree dmt = (DataMapTree)contextMenu.getInvoker();
						dmt.editComment();
					}
				});

		RemoveCommentItem.addActionListener(
				new ActionListener() 
				{
					public void actionPerformed(ActionEvent e) 
					{
						DataMapTree dmt = (DataMapTree)contextMenu.getInvoker();
						dmt.removeComment();
					}
				});

		ValidateEntryItem.addActionListener(
				new ActionListener() 
				{
					public void actionPerformed(ActionEvent e) 
					{
						DataMapTree dmt = (DataMapTree)contextMenu.getInvoker();
						dmt.validateEntry();
					}
				});

		ValidateAllItem.addActionListener(
				new ActionListener() 
				{
					public void actionPerformed(ActionEvent e) 
					{
						DataMapTree dmt = (DataMapTree)contextMenu.getInvoker();
						dmt.validateAll();
					}
				});
	}//static


	/** Reference to the DragGestureListener for Drag and Drop operations, may be deleted in future. */
	DragGestureListener dgListener = new DMTDragGestureListener();

	/** Reference to the DropTargetListener for Drag and Drop operations, may be deleted in future. */
	DropTargetListener dtListener = new DMTDropTargetListener();

	/** Reference to the DropTarget for Drag and Drop operations, may be deleted in future. */
	private DropTarget dropTarget = new DropTarget(this,DnDConstants.ACTION_LINK | DnDConstants.ACTION_COPY_OR_MOVE,dtListener,true);


	/**
	 * The lone constructor. Creating new DataMaps and reading in saved DataMaps
	 * are the same operation, so there is only one constructor. This constructor
	 * sets some data fields as well as specifying the custom cell renderer for 
	 * italicizing links and add the mouse adapter for right-clicking.
	 * @param model the model which specifies the contents of the tree.
	 * @param inProjectWindow the main project window.
	 * @param map the Datamap which contains this tree.
	 * @see VSTreeCellRenderer
	 */
	 public DataMapTree(TreeModel model, SoarWorkingMemoryModel _swmm) 
	{
		 super(model);
		 swmm = _swmm;
		 s_DataMapTree = this;

		 getSelectionModel().setSelectionMode(TreeSelectionModel.DISCONTIGUOUS_TREE_SELECTION);

		 DragSource.getDefaultDragSource().createDefaultDragGestureRecognizer(this,DnDConstants.ACTION_LINK | DnDConstants.ACTION_COPY_OR_MOVE,dgListener);
		 setAutoscrolls(true);

		 registerKeyboardAction(
				 new ActionListener() 
				 {
					 public void actionPerformed(ActionEvent e) 
					 {
						 removeEdge();
					 }
				 },
				 KeyStroke.getKeyStroke(KeyEvent.VK_BACK_SPACE, 0),
				 WHEN_ANCESTOR_OF_FOCUSED_COMPONENT);

		 registerKeyboardAction(
				 new ActionListener() 
				 {
					 public void actionPerformed(ActionEvent e) 
					 {
						 removeEdge();
					 }
				 },
				 KeyStroke.getKeyStroke(KeyEvent.VK_DELETE, 0),
				 WHEN_ANCESTOR_OF_FOCUSED_COMPONENT);

		 registerKeyboardAction(
				 new ActionListener() 
				 {
					 public void actionPerformed(ActionEvent e) 
					 {
						 SoarVertex theVertex = ((FakeTreeNode)getSelectionPath().getLastPathComponent()).getEnumeratingVertex();
						 if (theVertex.isEditable()) 
						 {
							 editValue();
						 }
					 }
				 },
				 KeyStroke.getKeyStroke(KeyEvent.VK_ENTER, 0),
				 WHEN_ANCESTOR_OF_FOCUSED_COMPONENT);

		 addMouseListener(new MouseAdapter() 
		 {

			 // Open the rules associated with invalid generated node
			 public void mouseClicked(MouseEvent e) 
			 {
				 if (e.getClickCount() == 2) 
				 {
					 FakeTreeNode ftn = (FakeTreeNode)getSelectionPath().getLastPathComponent();
					 NamedEdge ne = ftn.getEdge();
					 if(ne.isGenerated() && (ne.getNode() != null)) 
					 {
						 (ne.getNode()).openRules(MainFrame.getMainFrame(), ne.getLine());
					 }
				 }
			 }

			 public void mousePressed(MouseEvent e) 
			 {
				 if (e.isPopupTrigger()) 
				 {
					 suggestShowContextMenu(e.getX(),e.getY());
				 }
			 }

			 public void mouseReleased(MouseEvent e) 
			 {
				 if(e.isPopupTrigger()) 
				 {
					 suggestShowContextMenu(e.getX(),e.getY());
				 }
			 }
		 });

		 displayGeneratedNodes();

	}     //  end of DataMapTree constructor


	 public static DataMapTree getDataMapTree() 
	 {
		 return s_DataMapTree;
	 }

	 public void setDataMapCursor(Cursor c) 
	 {
		 this.setCursor(c);
	 }

	 /**
	  * Checks to see if x,y is a valid location on the screen, if it is then it
	  * displays the context menu there.
	  * @param x the x coordinate of the screen
	  * @param y the y coordinate of the screen
	  */

	 public void suggestShowContextMenu(int x, int y) 
	 {
		 TreePath path =
			 getPathForLocation(x, y);
		 if (path != null) 
		 {
			 setSelectionPath(path);
			 SoarVertex theVertex = ((FakeTreeNode)getSelectionPath().getLastPathComponent()).getEnumeratingVertex();
			 FakeTreeNode ftn = (FakeTreeNode)getSelectionPath().getLastPathComponent();
			 NamedEdge ne = ftn.getEdge();

			 if(ftn.getChildCount() != 0) 
			 {
				 ChangeTypeSubMenu.setEnabled(false);
			 }
			 else 
			 {
				 ChangeTypeSubMenu.setEnabled(true);
			 }

			 if (theVertex.allowsEmanatingEdges()) 
			 {
				 AddIdentifierItem.setEnabled(true);
				 AddEnumerationItem.setEnabled(true);
				 AddIntegerItem.setEnabled(true);
				 AddFloatItem.setEnabled(true);
				 AddStringItem.setEnabled(true);
			 }
			 else
			 {
				 // can't add any edges
				 AddIdentifierItem.setEnabled(false);
				 AddEnumerationItem.setEnabled(false);
				 AddIntegerItem.setEnabled(false);
				 AddFloatItem.setEnabled(false);
				 AddStringItem.setEnabled(false);
			 }

			 if (theVertex.isEditable()) 
			 {
				 EditValueItem.setEnabled(true);
			 }
			 else
			 {
				 // uneditable item
				 EditValueItem.setEnabled(false);
			 }

			 if (ne != null) 
			 {
				 if (ne.isGenerated()) 
				 {
					 ValidateEntryItem.setEnabled(true);
					 ValidateAllItem.setEnabled(true);
				 }
				 else 
				 {
					 ValidateEntryItem.setEnabled(false);
					 ValidateAllItem.setEnabled(false);
				 }
			 }   // end of if has a named edge
			 else 
			 {
				 ValidateEntryItem.setEnabled(false);
				 ValidateAllItem.setEnabled(false);
			 }

			 contextMenu.show(this,x,y);
		 }   // end of if path isn't null


	 }

	 /**
	  * Add a Soar Identifier Attribute to the dataMap
	  * @see IdentifierDialog
	  */
	 public void addIdentifier() 
	 {
		 IdentifierDialog theDialog = new IdentifierDialog(MainFrame.getMainFrame());

		 theDialog.setVisible(true);

		 if (theDialog.wasApproved()) 
		 {
			 String attribute = theDialog.getText();

			 TreePath thePath = getSelectionPath();
			 FakeTreeNode fake = ((FakeTreeNode)thePath.getLastPathComponent());
			 SoarVertex v0 = fake.getEnumeratingVertex();
			 SoarVertex v1 = swmm.createNewSoarId();
			 swmm.addTriple(v0,attribute,v1);
			 if (fake.getChildCount() != 0) 
			 {
				 expandPath(thePath);
			 }
			 else 
			 {
				 System.err.println("I am barren");
			 }
		 }
	 }

	 /**
	  * Add a Soar Enumeration Attribute to the datamap
	  * @see EnumerationDialog
	  */
	 public void addEnumeration() 
	 {
		 EnumerationDialog theDialog = new EnumerationDialog(MainFrame.getMainFrame());

		 theDialog.setVisible(true);

		 if (theDialog.wasApproved()) 
		 {
			 String attribute = theDialog.getText();
			 Vector enumVal = theDialog.getVector();

			 TreePath thePath = getSelectionPath();
			 FakeTreeNode fake = ((FakeTreeNode)thePath.getLastPathComponent());
			 SoarVertex v0 = fake.getEnumeratingVertex();
			 SoarVertex v1 = swmm.createNewEnumeration(enumVal);
			 swmm.addTriple(v0,attribute,v1);
			 if (fake.getChildCount() != 0) 
			 {
				 expandPath(thePath);
			 }
			 else 
			 {
				 System.err.println("I am barren");
			 }
		 }
	 }

	 /**
	  * Add a Soar Integer Attribute to the datamap
	  * @see NumberDialog
	  */
	 public void addInteger() 
	 {
		 NumberDialog theDialog = new NumberDialog(MainFrame.getMainFrame(), "Integer");

		 theDialog.setVisible(true);

		 if (theDialog.wasApproved()) 
		 {
			 String attribute = theDialog.getText();
			 Number low = theDialog.getLow(), high = theDialog.getHigh();

			 TreePath thePath = getSelectionPath();
			 FakeTreeNode fake = ((FakeTreeNode)thePath.getLastPathComponent()); 
			 SoarVertex v0 = fake.getEnumeratingVertex();
			 SoarVertex v1 = null;
			 if ((low.intValue() == Integer.MIN_VALUE) && (high.intValue() == Integer.MAX_VALUE)) 
			 {
				 v1 = swmm.createNewInteger();
			 }
			 else 
			 {
				 v1 = swmm.createNewIntegerRange(low.intValue(),high.intValue());
			 }
			 swmm.addTriple(v0,attribute,v1);
			 if (fake.getChildCount() != 0) 
			 {
				 expandPath(thePath);
			 }
			 else 
			 {
				 System.err.println("I am barren");
			 }
		 }
	 }

	 /**
	  * Add a Soar Float Attribute to the datamap
	  * @see NumberDialog
	  */
	 public void addFloat() 
	 {
		 NumberDialog theDialog = new NumberDialog(MainFrame.getMainFrame(), "Float");

		 theDialog.setVisible(true);

		 if (theDialog.wasApproved()) 
		 {
			 String attribute = theDialog.getText();
			 Number low = theDialog.getLow(), high = theDialog.getHigh();

			 TreePath thePath = getSelectionPath();
			 FakeTreeNode fake = ((FakeTreeNode)thePath.getLastPathComponent()); 
			 SoarVertex v0 = fake.getEnumeratingVertex();
			 SoarVertex v1 = null;
			 if ((low.floatValue() == Float.NEGATIVE_INFINITY) && (high.floatValue() == Float.POSITIVE_INFINITY)) 
			 {
				 v1 = swmm.createNewFloat();
			 }
			 else 
			 {
				 v1 = swmm.createNewFloatRange(low.floatValue(),high.floatValue());
			 }
			 swmm.addTriple(v0,attribute,v1);
			 if (fake.getChildCount() != 0) 
			 {
				 expandPath(thePath);
			 }
			 else 
			 {
				 System.err.println("I am barren");
			 }
		 }
	 }

	 /**
	  * Add a Soar String Attribute to the Datamap
	  */
	 public void addString() 
	 {
		 IdentifierDialog theDialog = new IdentifierDialog(MainFrame.getMainFrame());

		 theDialog.setTitle("Enter String");
		 theDialog.setVisible(true);

		 if (theDialog.wasApproved()) 
		 {
			 String attribute = theDialog.getText();

			 TreePath thePath = getSelectionPath();
			 FakeTreeNode fake = ((FakeTreeNode)thePath.getLastPathComponent()); 
			 SoarVertex v0 = fake.getEnumeratingVertex();
			 SoarVertex v1 = swmm.createNewString();
			 swmm.addTriple(v0,attribute,v1);
			 if (fake.getChildCount() != 0) 
			 {
				 expandPath(thePath);
			 }
			 else 
			 {
				 System.err.println("I am barren");
			 }
		 }
	 }

	 /**
	  *  Opens the SearchDataMapDialog to search the datamap beginning at the
	  *  currently selected node.
	  *  @see SearchDataMapDialog
	  */
	 public void searchFor() 
	 {
		 TreePath thePath = getSelectionPath();
		 FakeTreeNode fake = null;
		 if(thePath != null) 
		 {
			 fake = ((FakeTreeNode)thePath.getLastPathComponent());
		 }
		 else 
		 {
			 if(DataMapTree.getDataMapTree().getModel().getRoot() instanceof FakeTreeNode)
			 {
				 fake = (FakeTreeNode)DataMapTree.getDataMapTree().getModel().getRoot();
			 }
			 else 
			 {
				 System.err.println("No node selected for search option");
			 }
		 }
		 SearchDataMapDialog searchDialog = new SearchDataMapDialog(MainFrame.getMainFrame(), this, fake);
		 searchDialog.setVisible(true);
	 }

	 /**
	  *  Finds all productions that test or create the currently selected
	  *  vertex in the tree.
	  *
	  *  @param bTest   if this boolean is set to false, this function will
	  *                 ignore matches that test the WME
	  *  @param bCreate if this boolean is set to false, this function will
	  *                 ignore matches that create the WME
	  *  
	  */
	 public void findProds(boolean bTest, boolean bCreate)
	 {
		 TreePath thePath = getSelectionPath();
		 Vector vecErrors = new Vector();

		 OperatorWindow operatorWindow = MainFrame.getMainFrame().getOperatorWindow();
		 Enumeration bfe = operatorWindow.breadthFirstEnumeration();
		 while(bfe.hasMoreElements())
		 {
			 OperatorNode opNode = (OperatorNode)bfe.nextElement();
			 Vector parsedProds = null;
			 try
			 {
				 parsedProds = opNode.parseProductions();
			 }
			 catch(ParseException pe)
			 {
				 String parseError = pe.toString();
				 int i = parseError.lastIndexOf("line ");
				 String lineNum = parseError.substring(i + 5);
				 i = lineNum.indexOf(',');
				 lineNum = "(" + lineNum.substring(0, i) + "): ";
				 String errString = opNode.getFileName() + lineNum + "Unable to search productions due to parse error";
				 vecErrors.add(errString);
			 }
			 catch(TokenMgrError tme) 
			 {
				 tme.printStackTrace();
			 }
			 catch(IOException ioe) 
			 {
				 ioe.printStackTrace();
			 }

			 if (parsedProds == null)  continue;


			 Enumeration enumProds = parsedProds.elements();
			 while(enumProds.hasMoreElements()) 
			 {
				 SoarProduction sp = (SoarProduction)enumProds.nextElement();
				 Vector vecMatches =
					 DataMapMatcher.pathMatchesProduction(thePath, sp);
				 Enumeration enumMatches = vecMatches.elements();
				 while(enumMatches.hasMoreElements())
				 {
					 Triple trip = (Triple)enumMatches.nextElement();

					 //Make sure the caller has requested this match
					 if ( ((bTest) && (trip.isCondition()))
							 || ((bCreate) && (!trip.isCondition())) )
					 {
						 vecErrors.add(new FeedbackListObject(opNode,
								 trip.getLine(),
								 sp.getName()));
					 }
				 }//while
			 }//while
		 }//while

		 if (vecErrors.size() == 0)
		 {
			 vecErrors.add("No matches found.");
		 }

		 MainFrame.getMainFrame().setFeedbackListData(vecErrors);


	 }//findProds

	 /**
	  *    Function changeTypeTo() changes the selected DataMap item to
	  *    another type with the same name.
	  *    Function takes in an integer representing which type to change to
	  * @param type  determines what to change it too, 0=identifier, 1=enumeration, 2=integer, 3=float, 4=string
	  */
	 public void changeTypeTo(int type) 
	 {
		 TreePath[]      paths = getSelectionPaths();
		 FakeTreeNode    ftn;
		 NamedEdge       ne;
		 String componentName;
		 int             numIncident = 0;
		 SoarVertex v1;


		 for (int i = 0; i < paths.length; i++) 
		 {
			 numIncident = 0;

			 ftn = (FakeTreeNode)paths[i].getLastPathComponent();
			 ne = ftn.getEdge();
			 componentName = ne.getName();

			 if (ne != null) 
			 {
				 swmm.removeTriple((SoarVertex)ne.V0(),ne.getName(),(SoarVertex)ne.V1());
				 if(type == 1) 
				 {
					 v1 = swmm.createNewEnumeration("nil");
				 }
				 else if(type == 2) 
				 {
					 v1 = swmm.createNewInteger();
				 }
				 else if(type == 3) 
				 {
					 v1 = swmm.createNewFloat();
				 }
				 else if(type == 4) 
				 {
					 v1 = swmm.createNewString();
				 }
				 else 
				 {
					 v1 = swmm.createNewSoarId();
				 }

				 swmm.addTriple((SoarVertex)ne.V0(),componentName,v1);
			 }
		 }
	 }   // end of changeTypeTo()

	 /**
	  * Removes an attribute from the datamap
	  */
	 public void removeEdge() 
	 {
		 TreePath[]      paths = getSelectionPaths();
		 FakeTreeNode    ftn;
		 NamedEdge       ne;
		 Enumeration     edges;
		 Edge            currEdge;
		 String          incidentEdges;
		 int             numIncident = 0;

		 for (int i = 0; i < paths.length; i++) 
		 {
			 numIncident = 0;

			 ftn = (FakeTreeNode)paths[i].getLastPathComponent();
			 ne = ftn.getEdge();

			 if (ne != null) 
			 {
				 switch (JOptionPane.showConfirmDialog(this, "Do you really want to delete the attribute named \"" +
						 ne.getName() +"\"?", "Confirm Delete",
						 JOptionPane.YES_NO_CANCEL_OPTION)) 
						 {
						 case JOptionPane.YES_OPTION:
							 swmm.removeTriple((SoarVertex)ne.V0(),ne.getName(),(SoarVertex)ne.V1());
							 break;
						 case JOptionPane.CANCEL_OPTION:
							 return;
						 }
			 }
			 else 
			 {
				 JOptionPane.showMessageDialog(this, "This is not an attribute", "Cannot remove", JOptionPane.ERROR_MESSAGE);
			 }
		 }
	 }   

	 public void lostOwnership(Clipboard clipboard,Transferable t) 
	 {
		 System.out.println("Dang, lost ownership");
	 }

	 /**
	  * Attempts to rename an attribute on the datamap
	  * @IdentifierDialog
	  */
	 public void renameEdge() 
	 {
		 // Get what the user clicked on
		 FakeTreeNode ftn = (FakeTreeNode)getSelectionPath().getLastPathComponent();

		 // Get the edge associated with what the user clicked on
		 NamedEdge ne = ftn.getEdge();
		 if (ne == null) 
		 {
			 JOptionPane.showMessageDialog(this, "This is not an edge", "Cannot remove", JOptionPane.ERROR_MESSAGE);
			 return;
		 }
		 IdentifierDialog namedDialog = new IdentifierDialog(MainFrame.getMainFrame());
		 namedDialog.makeVisible(ne.getName());
		 if(namedDialog.wasApproved()) 
		 {
			 String newAttributeName = namedDialog.getText();
			 swmm.removeTriple((SoarVertex)ne.V0(),ne.getName(),(SoarVertex)ne.V1());
			 swmm.addTriple((SoarVertex)ne.V0(),newAttributeName,(SoarVertex)ne.V1());   
		 }
	 }

	 /**
	  *  Selects (highlights and centers) the requested edge within the DataMapTree.
	  *  @param node the string name of the edge to be highlighted
	  *  @return true if success, false if could not find edge
	  */
	 public boolean highlightEdge(FakeTreeNode node) 
	 {
		 if(node != null) 
		 {
			 TreePath path = new TreePath(node.getTreePath().toArray());
			 scrollPathToVisible(path);
			 setSelectionPath(path);

			 return true;
		 }
		 else  
		 {
			 JOptionPane.showMessageDialog(null, "Could not find a matching wme in the datamap");
			 return false;
		 }
	 }


	 /*  I think this operation is too dangerous. It isn't undoable, so it would be better to
    only allow copying, then deleting.

    private void cut() 
{
    copy();

    FakeTreeNode ftn = (FakeTreeNode)getSelectionPath().getLastPathComponent();
    NamedEdge ne = ftn.getEdge();
    if (ne != null) 
{
    swmm.removeTriple((SoarVertex)ne.V0(),ne.getName(),(SoarVertex)ne.V1());
    }
    }
	  */
	 /**
	  * Copies a piece of the datamap to the clipboard
	  */
	 private void copy() 
	 {
		 clipboard.setContents(getCopyVerticies(), this);
	 }
	 
	 private CopyVertexVector getCopyVerticies() {
		 TreePath[]          paths = getSelectionPaths();
		 FakeTreeNode        ftn;
		 NamedEdge           e;
		 SoarVertex          v;
		 String              name;
		 CopyVertexVector    copyVerticies = new CopyVertexVector(paths.length);

		 if (paths == null) 
		 {
			 return null;
		 }

		 for (int i = 0; i < paths.length; i++) 
		 {
			 ftn = (FakeTreeNode)paths[i].getLastPathComponent();
			 v = ftn.getEnumeratingVertex();
			 e = ftn.getEdge();
			 name = e.getName(); 

			 copyVerticies.add(name, v);
		 }
		 return copyVerticies;
	 }
	 
	 private void pasteCopyVertices(CopyVertexVector data) {
		 TreePath          path = getSelectionPath();
		 FakeTreeNode        ftn;

		 if (data == null) return;
		 if (path == null) 
		 {
			 return;
		 }

		 ftn = (FakeTreeNode)path.getLastPathComponent();
		 for (int j = 0; j < data.size(); j++) 
		 {
			 SoarVertex  parent = ftn.getEnumeratingVertex();
			 SoarVertex  child = swmm.createVertexCopy((SoarVertex)data.getVertex(j));
			 swmm.addTriple(parent, (String)data.getName(j), child);
		 }
	 }
	 
	 /**
	  * Paste a portion of the datamap from the clipboard
	  */
	 private void paste() 
	 {
		 TreePath[]          paths = getSelectionPaths();
		 FakeTreeNode        ftn;
		 FakeTreeNode        fakeParent;
		 Transferable        transferable;
		 DataFlavor          dataFlavor;
		 CopyVertexVector    data;

		 if (paths == null) 
		 {
			 return;
		 }

		 transferable = clipboard.getContents(this);
		 dataFlavor = TransferableVertex.flavors[0];

		 try 
		 {
			 data = (CopyVertexVector)transferable.getTransferData(dataFlavor);

		 } catch(UnsupportedFlavorException ufe) 
		 {
			 ufe.printStackTrace();
			 return;
		 } catch(IOException ioe) 
		 {
			 ioe.printStackTrace();
			 return;
		 }

		 for (int i = 0; i < paths.length; i++) 
		 {

			 ftn = (FakeTreeNode)paths[i].getLastPathComponent();
			 if(transferable.isDataFlavorSupported(dataFlavor)) 
			 {
				 for (int j = 0; j < data.size(); j++) 
				 {
					 SoarVertex  parent = ftn.getEnumeratingVertex();
					 SoarVertex  child = swmm.createVertexCopy((SoarVertex)data.getVertex(j));
					 swmm.addTriple(parent, (String)data.getName(j), child);
				 }
			 }
		 }
	 }


	 /**
	  * Can change the values for an enumeration or number attribute
	  */
	 public void editValue() 
	 {
		 FakeTreeNode ftn = (FakeTreeNode)getSelectionPath().getLastPathComponent();
		 NamedEdge ne = ftn.getEdge();
		 if (ne == null) 
		 {
			 JOptionPane.showMessageDialog(this, "This is not an edge", "Cannot Edit Value", JOptionPane.ERROR_MESSAGE);
			 return;
		 }
		 if (((SoarVertex)ne.V1()).edit(MainFrame.getMainFrame())) 
		 {
			 swmm.removeTriple((SoarVertex)ne.V0(),ne.getName(),(SoarVertex)ne.V1());
			 swmm.addTriple((SoarVertex)ne.V0(),ne.getName(),(SoarVertex)ne.V1());
		 }
	 }


	 /**
	  *  This allows the user to add a comment to an edge on the datamap.
	  *  This comment has no bearing on working memory, only to allow the
	  *  user to comment their datamap.
	  * @see CommentDialog
	  */
	 public void editComment() 
	 {
		 FakeTreeNode ftn = (FakeTreeNode)getSelectionPath().getLastPathComponent();
		 NamedEdge ne = ftn.getEdge();
		 if(ne == null) 
		 {
			 JOptionPane.showMessageDialog(this, "This is not an edge", "Cannot Edit Value", JOptionPane.ERROR_MESSAGE);
		 }

		 CommentDialog theDialog = new CommentDialog(MainFrame.getMainFrame(), ne.getComment() );
		 theDialog.setTitle("Enter Comment");
		 theDialog.setVisible(true);

		 if (theDialog.wasApproved()) 
		 {
			 String newComment = theDialog.getText();

			 swmm.notifyListenersOfRemove(ne);
			 ne.setComment(newComment);
			 swmm.notifyListenersOfAdd(ne);
		 }
	 }

	 /**
	  * This removes a comment from an edge on the datamap.
	  */
	 public void removeComment() 
	 {
		 FakeTreeNode ftn = (FakeTreeNode)getSelectionPath().getLastPathComponent();
		 NamedEdge ne = ftn.getEdge();
		 if(ne == null) 
		 {
			 JOptionPane.showMessageDialog(this, "This is not an edge", "Cannot Edit Value", JOptionPane.ERROR_MESSAGE);
		 }
		 else 
		 {
			 switch (JOptionPane.showConfirmDialog(this, "Do you really want to delete this comment?", "Confirm Delete",
					 JOptionPane.YES_NO_CANCEL_OPTION)) 
					 {
					 case JOptionPane.YES_OPTION:
						 swmm.notifyListenersOfRemove(ne);
						 ne.setComment("");
						 swmm.notifyListenersOfAdd(ne);
						 break;
					 case JOptionPane.CANCEL_OPTION:
						 return;
					 }
		 }

	 }

	 /**
	  *  This function validates the selected node of the datamap,
	  *  meaning that it makes a single node generated by the datamap
	  *  generater a valid node and restores its original color.
	  */
	 public void validateEntry() 
	 {
		 TreePath thePath = getSelectionPath();
		 FakeTreeNode ftn = (FakeTreeNode)thePath.getLastPathComponent();
		 NamedEdge ne = ftn.getEdge();
		 ne.validate();
		 swmm.notifyListenersOfRemove(ne);
		 swmm.notifyListenersOfAdd(ne);
		 if (ftn.getChildCount() != 0)
			 expandPath(thePath);
	 } // end of validateEntry()



	 /**
	  *  Similar to validateEntry(), but this function validates not
	  *  only the selected node, but all of that nodes children.
	  */
	 public void validateAll() 
	 {
		 edu.umich.visualsoar.util.Queue queue = new QueueAsLinkedList();
		 int numberOfVertices = swmm.getNumberOfVertices();
		 boolean[] visitedVertices = new boolean[numberOfVertices];
		 for(int i = 0; i < numberOfVertices; i++)
			 visitedVertices[i] = false;
		 TreePath thePath = getSelectionPath();
		 FakeTreeNode ftn = (FakeTreeNode)thePath.getLastPathComponent();
		 NamedEdge ne = ftn.getEdge();
		 queue.enqueue(ne.V1());
		 ne.validate();
		 swmm.notifyListenersOfRemove(ne);
		 swmm.notifyListenersOfAdd(ne);

		 while(!queue.isEmpty()) 
		 {
			 SoarVertex w = (SoarVertex)queue.dequeue();
			 visitedVertices[w.getValue()] = true;
			 if(w.allowsEmanatingEdges()) 
			 {
				 Enumeration edges = swmm.emanatingEdges(w);
				 while(edges.hasMoreElements()) 
				 {
					 NamedEdge theEdge = (NamedEdge)edges.nextElement();
					 theEdge.validate();
					 if(! visitedVertices[theEdge.V1().getValue()]) 
					 {
						 visitedVertices[w.getValue()] = true;
						 queue.enqueue(theEdge.V1());
					 }   // if haven't visited this vertex, add to the queue
				 } // while looking at all of the edges of the vertex
			 }
		 }   // while queue is not empty, examine each vertex in it
	 } // end of validateAll()


	 /**
	  *  Validates the entire datamap
	  */
	 public void validateDataMap() 
	 {
		 edu.umich.visualsoar.util.Queue queue = new QueueAsLinkedList();
		 int numberOfVertices = swmm.getNumberOfVertices();
		 boolean[] visitedVertices = new boolean[numberOfVertices];
		 for(int i = 0; i < numberOfVertices; i++)
			 visitedVertices[i] = false;

		 if(this.getModel().getRoot() instanceof FakeTreeNode) 
		 {
			 FakeTreeNode root = (FakeTreeNode)getModel().getRoot();
			 queue.enqueue(root.getEnumeratingVertex());
			 while(!queue.isEmpty()) 
			 {
				 SoarVertex w = (SoarVertex)queue.dequeue();

				 visitedVertices[w.getValue()] = true;
				 if(w.allowsEmanatingEdges()) 
				 {
					 Enumeration edges = swmm.emanatingEdges(w);
					 while(edges.hasMoreElements()) 
					 {
						 NamedEdge theEdge = (NamedEdge)edges.nextElement();
						 theEdge.validate();
						 swmm.notifyListenersOfRemove(theEdge);
						 swmm.notifyListenersOfAdd(theEdge);
						 if(! visitedVertices[theEdge.V1().getValue()]) 
						 {
							 visitedVertices[w.getValue()] = true;
							 queue.enqueue(theEdge.V1());
						 }   // if haven't visited this vertex, add to the queue
					 } // while looking at all of the edges of the vertex
				 }
			 }   // while queue is not empty, examine each vertex in it
		 }
		 JOptionPane.showMessageDialog(null, "Validation of DataMap completed", "DataMap Generator", JOptionPane.INFORMATION_MESSAGE);
	 }     // end of validateDataMap()


	 /**
	  *  Deletes all invalid datamap entries from the datamap
	  */
	 public void removeInvalid() 
	 {
		 edu.umich.visualsoar.util.Queue queue = new QueueAsLinkedList();
		 int numberOfVertices = swmm.getNumberOfVertices();
		 boolean[] visitedVertices = new boolean[numberOfVertices];
		 for(int i = 0; i < numberOfVertices; i++)
			 visitedVertices[i] = false;

		 if(this.getModel().getRoot() instanceof FakeTreeNode) 
		 {
			 FakeTreeNode root = (FakeTreeNode)getModel().getRoot();
			 queue.enqueue(root.getEnumeratingVertex());

			 while(!queue.isEmpty()) 
			 {
				 SoarVertex w = (SoarVertex)queue.dequeue();
				 visitedVertices[w.getValue()] = true;
				 if(w.allowsEmanatingEdges()) 
				 {
					 Enumeration edges = swmm.emanatingEdges(w);
					 while(edges.hasMoreElements()) 
					 {
						 NamedEdge ne = (NamedEdge)edges.nextElement();
						 if(ne.isGenerated()) 
						 {
							 swmm.removeTriple((SoarVertex)ne.V0(),ne.getName(),(SoarVertex)ne.V1());
							 edges = swmm.emanatingEdges(w);
						 }
						 else 
						 {
							 // Valid node, keep it and look at children
							 if(! visitedVertices[ne.V1().getValue()]) 
							 {
								 visitedVertices[w.getValue()] = true;
								 queue.enqueue(ne.V1());
							 }  // if haven't visited this vertex, add to the queue
						 }
					 } // while looking at all of the edges of the vertex
				 }
			 }   // while queue is not empty, examine each vertex in it
		 }
		 JOptionPane.showMessageDialog(null, "Removal of all invalid datamap entries completed", "DataMap Generator", JOptionPane.INFORMATION_MESSAGE);
	 }     // end of removeInvalid()





	 /**
	  *  Searches the entire dataMap looking for any edges that were not tested
	  *  by a production and that are not in the output link.
	  *  Returns feedback list information
	  */
	 public Vector searchTestDataMap(SoarIdentifierVertex in_siv, String dataMapName) 
	 {
		 Vector errors = new Vector();
		 SoarIdentifierVertex siv = in_siv;
		 edu.umich.visualsoar.util.Queue queue = new QueueAsLinkedList();
		 int numberOfVertices = swmm.getNumberOfVertices();
		 boolean[] visitedVertices = new boolean[numberOfVertices];
		 for(int i = 0; i < numberOfVertices; i++)
			 visitedVertices[i] = false;

		 if(this.getModel().getRoot() instanceof FakeTreeNode) 
		 {
			 FakeTreeNode root = (FakeTreeNode)getModel().getRoot();
			 queue.enqueue(root.getEnumeratingVertex());
			 while(!queue.isEmpty()) 
			 {
				 SoarVertex w = (SoarVertex)queue.dequeue();

				 visitedVertices[w.getValue()] = true;
				 if(w.allowsEmanatingEdges()) 
				 {
					 Enumeration edges = swmm.emanatingEdges(w);
					 while(edges.hasMoreElements()) 
					 {
						 NamedEdge theEdge = (NamedEdge)edges.nextElement();
						 String edgeName = theEdge.getName();

						 // if the edge isn't tested and not the output-link, add to error list
						 if((!theEdge.isTested()) && !edgeName.equals("output-link") && !theEdge.getErrorNoted()
								 && !edgeName.equals("top-state") && !edgeName.equals("operator") && !edgeName.equals("input-link") && !edgeName.equals("item")
								 && !edgeName.equals("impasse") && !edgeName.equals("superstate") && !edgeName.equals("io") && !edgeName.equals("attribute")
								 && !edgeName.equals("choices") && !edgeName.equals("type") && !edgeName.equals("quiescence")) 
						 {
							 errors.add(new FeedbackListObject(theEdge, siv,  dataMapName, ", was never tested in the productions of this agent."));
							 theEdge.setErrorNoted();
						 }

						 // Do not check edges on the output-link
						 if( (!visitedVertices[theEdge.V1().getValue()]) && (!theEdge.getName().equals("output-link") )) 
						 {
							 visitedVertices[w.getValue()] = true;
							 queue.enqueue(theEdge.V1());
						 }   // if haven't visited this vertex, add to the queue
					 } // while looking at all of the edges of the vertex
				 }
			 }   // while queue is not empty, examine each vertex in it
		 }
		 return errors;
	 }     // end of searchTestDataMap()

	 /**
	  *  Searches the entire dataMap looking for any edges that were not created
	  *  by a production and that are not in the input link.
	  *  Returns feedback list information
	  */
	 public Vector searchCreateDataMap(SoarIdentifierVertex in_siv, String dataMapName) 
	 {
		 Vector errors = new Vector();
		 SoarIdentifierVertex siv = in_siv;
		 edu.umich.visualsoar.util.Queue queue = new QueueAsLinkedList();
		 int numberOfVertices = swmm.getNumberOfVertices();
		 boolean[] visitedVertices = new boolean[numberOfVertices];
		 for(int i = 0; i < numberOfVertices; i++)
			 visitedVertices[i] = false;

		 if(this.getModel().getRoot() instanceof FakeTreeNode) 
		 {
			 FakeTreeNode root = (FakeTreeNode)getModel().getRoot();
			 queue.enqueue(root.getEnumeratingVertex());
			 while(!queue.isEmpty()) 
			 {
				 SoarVertex w = (SoarVertex)queue.dequeue();

				 visitedVertices[w.getValue()] = true;
				 if(w.allowsEmanatingEdges()) 
				 {
					 Enumeration edges = swmm.emanatingEdges(w);
					 while(edges.hasMoreElements()) 
					 {
						 NamedEdge theEdge = (NamedEdge)edges.nextElement();
						 String edgeName = theEdge.getName();

						 // if the edge isn't created and not the input-link, add to error list
						 if((!theEdge.isCreated()) && !edgeName.equals("input-link") && !theEdge.getErrorNoted()
								 && !edgeName.equals("top-state") && !edgeName.equals("operator") && !edgeName.equals("output-link") && !edgeName.equals("item")
								 && !edgeName.equals("impasse") && !edgeName.equals("superstate") && !edgeName.equals("io") && !edgeName.equals("attribute")
								 && !edgeName.equals("choices") && !edgeName.equals("type") && !edgeName.equals("quiescence")) 
						 {
							 errors.add(new FeedbackListObject(theEdge, siv,  dataMapName, ", was never created by the productions of this agent."));
							 theEdge.setErrorNoted();
						 }

						 // Do not check edges on the input-link
						 if( (!visitedVertices[theEdge.V1().getValue()]) && (!theEdge.getName().equals("input-link") )) 
						 {
							 visitedVertices[w.getValue()] = true;
							 queue.enqueue(theEdge.V1());
						 }   // if haven't visited this vertex, add to the queue
					 } // while looking at all of the edges of the vertex
				 }
			 }   // while queue is not empty, examine each vertex in it
		 }
		 return errors;
	 }     // end of searchCreateDataMap()

	 /**
	  *  Searches the entire dataMap looking for any edges that were tested but not created
	  *  by a production and that are not in the input link.
	  *  Returns feedback list information
	  */
	 public Vector searchTestNoCreateDataMap(SoarIdentifierVertex in_siv, String dataMapName) 
	 {
		 Vector errors = new Vector();
		 SoarIdentifierVertex siv = in_siv;
		 edu.umich.visualsoar.util.Queue queue = new QueueAsLinkedList();
		 int numberOfVertices = swmm.getNumberOfVertices();
		 boolean[] visitedVertices = new boolean[numberOfVertices];
		 for(int i = 0; i < numberOfVertices; i++)
			 visitedVertices[i] = false;

		 if(this.getModel().getRoot() instanceof FakeTreeNode) 
		 {
			 FakeTreeNode root = (FakeTreeNode)getModel().getRoot();
			 queue.enqueue(root.getEnumeratingVertex());
			 while(!queue.isEmpty()) 
			 {
				 SoarVertex w = (SoarVertex)queue.dequeue();

				 visitedVertices[w.getValue()] = true;
				 if(w.allowsEmanatingEdges()) 
				 {
					 Enumeration edges = swmm.emanatingEdges(w);
					 while(edges.hasMoreElements()) 
					 {
						 NamedEdge theEdge = (NamedEdge)edges.nextElement();
						 String edgeName = theEdge.getName();

						 // if the edge isn't created and not the input-link, add to error list
						 if( theEdge.isTestedNoCreate() && !edgeName.equals("input-link") && !theEdge.getErrorNoted()
								 && !edgeName.equals("top-state") && !edgeName.equals("operator") && !edgeName.equals("output-link") && !edgeName.equals("item")
								 && !edgeName.equals("impasse") && !edgeName.equals("superstate") && !edgeName.equals("io") && !edgeName.equals("attribute")
								 && !edgeName.equals("choices") && !edgeName.equals("type") && !edgeName.equals("quiescence")) 
						 {
							 errors.add(new FeedbackListObject(theEdge, siv,  dataMapName, ", was tested but never created by the productions of this agent."));
							 theEdge.setErrorNoted();
						 }

						 // Do not check edges on the input-link or output-link
						 if( (!visitedVertices[theEdge.V1().getValue()]) && (!theEdge.getName().equals("input-link"))  && (!theEdge.getName().equals("output-link")) ) 
						 {
							 visitedVertices[w.getValue()] = true;
							 queue.enqueue(theEdge.V1());
						 }   // if haven't visited this vertex, add to the queue
					 } // while looking at all of the edges of the vertex
				 }
			 }   // while queue is not empty, examine each vertex in it
		 }
		 return errors;
	 }     // end of searchTestNoCreateDataMap()


	 /**
	  *  Searches the entire dataMap looking for any edges that were created but not tested
	  *  by a production and that are not in the input link.
	  *  Returns feedback list information
	  */
	 public Vector searchCreateNoTestDataMap(SoarIdentifierVertex in_siv, String dataMapName) 
	 {
		 Vector errors = new Vector();
		 SoarIdentifierVertex siv = in_siv;
		 edu.umich.visualsoar.util.Queue queue = new QueueAsLinkedList();
		 int numberOfVertices = swmm.getNumberOfVertices();
		 boolean[] visitedVertices = new boolean[numberOfVertices];
		 for(int i = 0; i < numberOfVertices; i++)
			 visitedVertices[i] = false;

		 if(this.getModel().getRoot() instanceof FakeTreeNode) 
		 {
			 FakeTreeNode root = (FakeTreeNode)getModel().getRoot();
			 queue.enqueue(root.getEnumeratingVertex());
			 while(!queue.isEmpty()) 
			 {
				 SoarVertex w = (SoarVertex)queue.dequeue();

				 visitedVertices[w.getValue()] = true;
				 if(w.allowsEmanatingEdges()) 
				 {
					 Enumeration edges = swmm.emanatingEdges(w);
					 while(edges.hasMoreElements()) 
					 {
						 NamedEdge theEdge = (NamedEdge)edges.nextElement();
						 String edgeName = theEdge.getName();

						 // if the edge is created, not tested, add to error list
						 if( theEdge.isCreatedNoTest() && !edgeName.equals("input-link") && !theEdge.getErrorNoted()
								 && !edgeName.equals("top-state") && !edgeName.equals("operator") && !edgeName.equals("output-link") && !edgeName.equals("item")
								 && !edgeName.equals("impasse") && !edgeName.equals("superstate") && !edgeName.equals("io") && !edgeName.equals("attribute")
								 && !edgeName.equals("choices") && !edgeName.equals("type") && !edgeName.equals("quiescence")) 
						 {
							 errors.add(new FeedbackListObject(theEdge, siv,  dataMapName, ", was tested but never created by the productions of this agent."));
							 theEdge.setErrorNoted();
						 }

						 // Do not check edges on the output-link
						 if( (!visitedVertices[theEdge.V1().getValue()]) && (!theEdge.getName().equals("output-link")) && (!theEdge.getName().equals("input-link")) ) 
						 {
							 visitedVertices[w.getValue()] = true;
							 queue.enqueue(theEdge.V1());
						 }   // if haven't visited this vertex, add to the queue
					 } // while looking at all of the edges of the vertex
				 }
			 }   // while queue is not empty, examine each vertex in it
		 }
		 return errors;
	 }     // end of searchCreateNoTestDataMap()

	 /**
	  *  Searches the entire dataMap looking for any edges that were not tested AND not created
	  *  by a production and that are not in the input link.
	  *  Returns feedback list information
	  */
	 public Vector searchNoTestNoCreateDataMap(SoarIdentifierVertex in_siv, String dataMapName) 
	 {
		 Vector errors = new Vector();
		 SoarIdentifierVertex siv = in_siv;
		 edu.umich.visualsoar.util.Queue queue = new QueueAsLinkedList();
		 int numberOfVertices = swmm.getNumberOfVertices();
		 boolean[] visitedVertices = new boolean[numberOfVertices];
		 for(int i = 0; i < numberOfVertices; i++)
			 visitedVertices[i] = false;

		 if(this.getModel().getRoot() instanceof FakeTreeNode) 
		 {
			 FakeTreeNode root = (FakeTreeNode)getModel().getRoot();
			 queue.enqueue(root.getEnumeratingVertex());
			 while(!queue.isEmpty()) 
			 {
				 SoarVertex w = (SoarVertex)queue.dequeue();

				 visitedVertices[w.getValue()] = true;
				 if(w.allowsEmanatingEdges()) 
				 {
					 Enumeration edges = swmm.emanatingEdges(w);
					 while(edges.hasMoreElements()) 
					 {
						 NamedEdge theEdge = (NamedEdge)edges.nextElement();
						 String edgeName = theEdge.getName();

						 // if the edge isn't created and not the input-link, add to error list
						 if( theEdge.notMentioned() && !edgeName.equals("input-link") && !theEdge.getErrorNoted()
								 && !edgeName.equals("top-state") && !edgeName.equals("operator") && !edgeName.equals("output-link") && !edgeName.equals("item")
								 && !edgeName.equals("impasse") && !edgeName.equals("superstate") && !edgeName.equals("io") && !edgeName.equals("attribute")
								 && !edgeName.equals("choices") && !edgeName.equals("type") && !edgeName.equals("quiescence")) 
						 {
							 errors.add(new FeedbackListObject(theEdge, siv,  dataMapName, ", was tested but never created by the productions of this agent."));
							 theEdge.setErrorNoted();
						 }

						 // Do not check edges on the input-link or output-link
						 if( (!visitedVertices[theEdge.V1().getValue()]) && (!theEdge.getName().equals("input-link"))  && (!theEdge.getName().equals("output-link")) ) 
						 {
							 visitedVertices[w.getValue()] = true;
							 queue.enqueue(theEdge.V1());
						 }   // if haven't visited this vertex, add to the queue
					 } // while looking at all of the edges of the vertex
				 }
			 }   // while queue is not empty, examine each vertex in it
		 }
		 return errors;
	 }     // end of searchNoTestNoCreateDataMap()



	 /**
	  *  Selects (highlights and centers) the requested edge within the datamap.
	  *  @param desiredEdge the string name of the edge
	  *  @param options a list of booleans representing what type of wme's to look for
	  *  @return the FakeTreeNode that contains the desired edge, null if could not fin
	  */
	 public FakeTreeNode findEdge(String desiredEdge, Boolean options[]) 
	 {
		 return null;

	 }


	 /**
	  *  Selects (highlights and centers) the requested edge within the datamap.
	  *  @param edge the requested NamedEdge to select
	  *  @return the FakeTreeNode that contains the desired edge,  null if could not find
	  */
	 public FakeTreeNode selectEdge(NamedEdge desiredEdge) 
	 {
		 edu.umich.visualsoar.util.Queue queue = new QueueAsLinkedList();
		 FakeTreeNode foundftn = null;
		 int numberOfVertices = swmm.getNumberOfVertices();
		 boolean[] visitedVertices = new boolean[numberOfVertices];
		 boolean edgeNotFound = true;
		 int children = 0;
		 for(int i = 0; i < numberOfVertices; i++)
			 visitedVertices[i] = false;

		 if(this.getModel().getRoot() instanceof FakeTreeNode) 
		 {
			 FakeTreeNode root = (FakeTreeNode)getModel().getRoot();
			 queue.enqueue(root);

			 while((!queue.isEmpty())  && edgeNotFound) 
			 {
				 FakeTreeNode ftn = (FakeTreeNode)queue.dequeue();
				 int rootValue = ftn.getEnumeratingVertex().getValue();
				 visitedVertices[rootValue] = true;
				 children = ftn.getChildCount();

				 // See if current FakeTreeNode's edge match desired edge
				 if( ftn.getEdge() != null) 
				 {
					 if( (ftn.getEdge()).equals(desiredEdge) ) 
					 {
						 edgeNotFound = false;
						 foundftn = ftn;
					 }
				 }

				 // Examine children of ftn
				 if((children != 0) && edgeNotFound)
				 {
					 for(int i = 0; i < children; i++) 
					 {
						 FakeTreeNode childftn = ftn.getChildAt(i);
						 int vertexValue = childftn.getEnumeratingVertex().getValue();
						 if(! visitedVertices[vertexValue]) 
						 {
							 visitedVertices[vertexValue] = true;
							 queue.enqueue(childftn);
						 }   // if never visited vertex
						 else 
						 {
							 // Check this edge since it won't be added to the queue
							 if( childftn.getEdge() != null) 
							 {
								 if( (childftn.getEdge()).equals(desiredEdge) ) 
								 {
									 edgeNotFound = false;
									 foundftn = childftn;
								 }
							 }
						 }    // end of else already visited this vertex
					 }   // for checking all of ftn's children
				 }   // if ftn has children

			 }   // while queue is not empty, examine each vertex in it
		 } // if root is a valid ftn

		 return foundftn;
	 }//selectEdge

	 /**
	  *  Opens all the paths leading to generated nodes that have not been validated
	  */
	 public void displayGeneratedNodes()
	 {
		 edu.umich.visualsoar.util.Queue queue = new QueueAsLinkedList();
		 FakeTreeNode foundftn = null;
		 int numberOfVertices = swmm.getNumberOfVertices();
		 boolean[] visitedVertices = new boolean[numberOfVertices];
		 boolean edgeNotFound = true;
		 int children = 0;
		 for(int i = 0; i < numberOfVertices; i++)
			 visitedVertices[i] = false;

		 if(this.getModel().getRoot() instanceof FakeTreeNode) 
		 {
			 FakeTreeNode root = (FakeTreeNode)getModel().getRoot();
			 queue.enqueue(root);

			 while((!queue.isEmpty())  && edgeNotFound) 
			 {
				 FakeTreeNode ftn = (FakeTreeNode)queue.dequeue();
				 int rootValue = ftn.getEnumeratingVertex().getValue();
				 visitedVertices[rootValue] = true;
				 children = ftn.getChildCount();

				 // See if current FakeTreeNode's edge match desired edge
				 NamedEdge ne = ftn.getEdge();
				 if( (ne != null) && (ne.isGenerated()) )
				 {
					 TreePath path = new TreePath(ftn.getTreePath().toArray());
					 expandPath(path);
				 }

				 // Examine children of ftn
				 for(int i = 0; i < children; i++) 
				 {
					 FakeTreeNode childftn = ftn.getChildAt(i);
					 int vertexValue = childftn.getEnumeratingVertex().getValue();
					 if(! visitedVertices[vertexValue])
					 {
						 visitedVertices[vertexValue] = true;
						 queue.enqueue(childftn);
					 }   // if never visited vertex
					 else 
					 {
						 // Check this edge since it won't be added to the queue
						 ne = childftn.getEdge();
						 if( (ne != null) && (ne.isGenerated()) )
						 {
							 TreePath path = new TreePath(childftn.getTreePath().toArray());
							 expandPath(path);
						 }
					 }    // end of else already visited this vertex
				 }   // for checking all of ftn's children
			 }   // while queue is not empty, examine each vertex in it
		 } // if root is a valid ftn

	 }//displayGeneratedNodes


	 class DMTDragGestureListener implements DragGestureListener 
	 {
		 public void dragGestureRecognized(DragGestureEvent dge) 
		 {
			 int action = dge.getDragAction();
			 TreePath path = getLeadSelectionPath();
			 if (path == null) 
			 {
				 //getToolkit().beep();
				 return;
			 }
			 if(dge.getTriggerEvent() instanceof MouseEvent) 
			 {
				 if(((MouseEvent)dge.getTriggerEvent()).isPopupTrigger())
					 return;
			 }

			 if (getSelectionCount() > 1) 
			 {
				 Vector v = new Vector(1);
				 v.add("Only one item may be dragged at a time");
				 MainFrame.getMainFrame().setFeedbackListData(v);
				 return;
			 }

			 FakeTreeNode ftn = (FakeTreeNode)path.getLastPathComponent();
			 NamedEdge e = ftn.getEdge();
			 Transferable t;
			 if (e == null)
				 t = new TransferableVertex(ftn.getEnumeratingVertex(),ftn.toString());
			 else
				 t = new TransferableVertex(ftn.getEnumeratingVertex(),e.getName(),e);
			 if (action == DnDConstants.ACTION_LINK) 
			 {
				 DragSource.getDefaultDragSource().startDrag(dge, DragSource.DefaultLinkNoDrop, t, new DMTDragSourceListener());
			 }
			 else if(action == DnDConstants.ACTION_COPY) 
			 {
				 DragSource.getDefaultDragSource().startDrag(dge, DragSource.DefaultCopyNoDrop, t, new DMTDragSourceListener());
			 }
			 else if(action == DnDConstants.ACTION_MOVE) 
			 {
				 DragSource.getDefaultDragSource().startDrag(dge, DragSource.DefaultMoveNoDrop, t, new DMTDragSourceListener());
			 }

			 OriginalSelectionPath = path;
		 }   
	 }

	 class DMTDropTargetListener implements DropTargetListener 
	 {
		 public void dragEnter(DropTargetDragEvent dtde) {
		 }
		 public void dragExit(DropTargetEvent dte) 
		 {
			 // reset cursor back to normal
			 Cursor defaultCursor = new Cursor(Cursor.DEFAULT_CURSOR);
			 DataMapTree.getDataMapTree().setCursor(defaultCursor);
		 }

		 public void dragOver(DropTargetDragEvent dtde) 
		 {

			 int action = dtde.getDropAction();
			 Point loc = dtde.getLocation();
			 int x = (int)loc.getX(), y = (int)loc.getY();
			 TreePath path = getPathForLocation(x, y);
			 if (path != null) 
			 {   
				 clearSelection();
				 setSelectionPath(path);
				 if(isDropOK(x,y,action)) 
				 {
					 if(action == DnDConstants.ACTION_LINK) 
					 {
						 Cursor cursor = DragSource.DefaultLinkDrop;
						 DataMapTree.getDataMapTree().setCursor(cursor);
						 dtde.acceptDrag(DnDConstants.ACTION_LINK);
					 }
					 else if(action == DnDConstants.ACTION_COPY) 
					 {
						 Cursor cursor = DragSource.DefaultCopyDrop;
						 DataMapTree.getDataMapTree().setCursor(cursor);
						 dtde.acceptDrag(DnDConstants.ACTION_COPY);
					 }
					 else 
					 {
						 Cursor cursor = DragSource.DefaultMoveDrop;
						 DataMapTree.getDataMapTree().setCursor(cursor);
						 dtde.acceptDrag(DnDConstants.ACTION_MOVE);
					 }
				 }   // if drop ok
				 else 
				 {
					 Cursor cursor;
					 if(action == DnDConstants.ACTION_LINK) 
					 {
						 cursor = DragSource.DefaultLinkNoDrop;
					 }
					 else if(action == DnDConstants.ACTION_COPY) 
					 {
						 cursor = DragSource.DefaultCopyNoDrop;
					 }
					 else 
					 {
						 cursor = DragSource.DefaultMoveNoDrop;
					 }
					 DataMapTree.getDataMapTree().setCursor(cursor);
					 dtde.rejectDrag();
				 }
			 }   // if path ok
			 else 
			 {
				 Cursor cursor = DragSource.DefaultCopyNoDrop;
				 DataMapTree.getDataMapTree().setCursor(cursor);
				 dtde.rejectDrag();
			 }
		 }

		 public void dropActionChanged(DropTargetDragEvent dtde) {
		 }

		 public void drop(DropTargetDropEvent e) 
		 {
			 Point loc = e.getLocation();
			 int x = (int)loc.getX(), y = (int)loc.getY();

			 int action = e.getDropAction();
			 if (isDropOK(x, y, action)) 
			 {
				 e.acceptDrop(action);
			 }
			 else 
			 {
				 e.rejectDrop();
				 return;
			 }

			 // reset cursor back to normal
			 Cursor defaultCursor = new Cursor(Cursor.DEFAULT_CURSOR);
			 DataMapTree.getDataMapTree().setCursor(defaultCursor);

			 if (action == DnDConstants.ACTION_COPY) {
				 System.out.println("dndcopy!");
				 TreePath currentPath = getPathForLocation(x, y);
				 setSelectionPath(OriginalSelectionPath);
				 CopyVertexVector copyVerticies = getCopyVerticies();
				 setSelectionPath(currentPath);
				 pasteCopyVertices(copyVerticies);
			 } else {
	
				 DataFlavor[] flavors = e.getCurrentDataFlavors();
				 DataFlavor chosen = flavors[0];
	
	
				 //Get the WME we're dropping
				 Vector data = null;
				 try 
				 {
					 data = (Vector)e.getTransferable().getTransferData(chosen);
				 } 
				 catch(Throwable t) 
				 {
					 t.printStackTrace();
					 e.dropComplete(false);
					 return;
				 }
	
				 //Get the target for the drop
				 TreePath path = getPathForLocation(x, y);
				 SoarVertex vertex = ((FakeTreeNode)path.getLastPathComponent()).getEnumeratingVertex();
	
				 //Get the thing we're dropping
				 SoarVertex dataVertex =
					 swmm.getVertexForId(((Integer)data.get(0)).intValue());
	
				 //If we are moving a node, we have to first make sure that we're
				 //not creating a loop.
				 if ((action & DnDConstants.ACTION_MOVE) != 0)
				 {
					 for(int i = 0; i < path.getPathCount(); i++)
					 {
						 SoarVertex v = ((FakeTreeNode)path.getPath()[i]).getEnumeratingVertex();
						 if (dataVertex.equals(v))
						 {
							 e.rejectDrop();
							 return;
						 }
					 }
				 }
	
				 //Perform the drop
				 swmm.addTriple(vertex,(String)data.get(1),dataVertex);
				 if(action == DnDConstants.ACTION_MOVE) 
				 {
					 NamedEdge ne = (NamedEdge)data.get(2);
					 swmm.removeTriple((SoarVertex)ne.V0(),ne.getName(),(SoarVertex)ne.V1());
				 }
				 
			 }

			 e.dropComplete(true);

		 }

		 boolean isDropOK(int x, int y, int action) 
		 {
			 TreePath path = getPathForLocation(x, y);
			 if (path == null) {
				 return false;
			 }
			 if (path.equals(OriginalSelectionPath)) {
				 return false;
			 }

			 if (action == DnDConstants.ACTION_LINK || action == DnDConstants.ACTION_MOVE || action == DnDConstants.ACTION_COPY) 
			 {
				 FakeTreeNode ftn = (FakeTreeNode)path.getLastPathComponent();
				 if (ftn.isLeaf())
				 {
					 return false;
				 }
				 return true;
			 }
			 return false;

		 }
	 }

	 class DMTDragSourceListener implements DragSourceListener 
	 {

		 public void dragEnter(DragSourceDragEvent e) 
		 {
			 //DragSourceContext context = e.getDragSourceContext();

			 //intersection of the users selected action, and the source and target actions

			 int myaction = e.getDropAction();

			 Cursor cursor;
			 if( (myaction & DnDConstants.ACTION_COPY) != 0) 
			 {
				 //context.setCursor(DragSource.DefaultCopyDrop);
				 cursor = DragSource.DefaultCopyDrop;
			 }
			 else if( (myaction & DnDConstants.ACTION_LINK) != 0) 
			 {
				 //context.setCursor(DragSource.DefaultLinkDrop);
				 cursor = DragSource.DefaultLinkDrop;
			 }
			 else if( (myaction & DnDConstants.ACTION_MOVE) != 0) 
			 {
				 //context.setCursor(DragSource.DefaultMoveDrop);
				 cursor = DragSource.DefaultMoveDrop;
			 }
			 else 
			 {
				 //context.setCursor(DragSource.DefaultMoveNoDrop);
				 cursor = DragSource.DefaultMoveNoDrop;
			 }
			 DataMapTree.getDataMapTree().setCursor(cursor);
		 }


		 public void dragOver(DragSourceDragEvent e)
		 {
			 e.getDragSourceContext().setCursor(null);
		 }


		 public void dragExit(DragSourceEvent e) {

		 }
		 public void dragDropEnd(DragSourceDropEvent e) {

		 }
		 public void dropActionChanged(DragSourceDragEvent e) {

		 }
	 }

	 /*  Too Dangerous, see cut()

    class CutAction extends AbstractAction 
{
    public void actionPerformed(ActionEvent e) 
{
    cut();
    }
    }
	  */

	 class CopyAction extends AbstractAction 
	 {
		 public void actionPerformed(ActionEvent e) 
		 {
			 copy();
		 }
	 }

	 class PasteAction extends AbstractAction 
	 {
		 public void actionPerformed(ActionEvent e) 
		 {
			 paste();
		 }
	 }

	 class SearchAction extends AbstractAction 
	 {
		 public SearchAction() 
		 {
			 super("Search DataMap");
			 setEnabled(false);
		 }

		 public void actionPerformed(ActionEvent e) 
		 {
			 TreePath thePath = getSelectionPath();
			 FakeTreeNode fake = null;
			 if(thePath != null) 
			 {
				 fake = ((FakeTreeNode)thePath.getLastPathComponent());
			 }
			 else 
			 {
				 if(DataMapTree.getDataMapTree().getModel().getRoot() instanceof FakeTreeNode)
					 fake = (FakeTreeNode)DataMapTree.getDataMapTree().getModel().getRoot();
			 }
			 SearchDataMapDialog searchDialog = new SearchDataMapDialog(MainFrame.getMainFrame(), DataMapTree.getDataMapTree(), fake);
			 searchDialog.setVisible(true);
		 }
	 }  


	 class ValidateDataMapAction extends AbstractAction 
	 {
		 public void actionPerformed(ActionEvent e) 
		 {
			 validateDataMap();
		 }
	 }

	 class RemoveInvalidAction extends AbstractAction 
	 {
		 public void actionPerformed(ActionEvent e) 
		 {
			 removeInvalid();
		 }
	 }

}

