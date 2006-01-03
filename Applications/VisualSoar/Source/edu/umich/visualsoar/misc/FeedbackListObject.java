package edu.umich.visualsoar.misc;
import edu.umich.visualsoar.operatorwindow.OperatorNode;
import edu.umich.visualsoar.graph.*;
import edu.umich.visualsoar.datamap.*;
import edu.umich.visualsoar.MainFrame;



/**
 * Just used to pass some things around
 * @author Brad Jones
 * @version 0.5a 5 Aug 1999
 */
public class FeedbackListObject  
{
///////////////////////////////////////////////////////////////////
// Data Members
///////////////////////////////////////////////////////////////////
    private OperatorNode node;
    private int lineNumber = -1;
    private String message;
    private boolean msgEnough = false;
    private boolean d_isError = false;
    private boolean d_isGenerated = false;
    private String assocString = null;

    // feedback list members that are used for accessing datamap info
    //   - there probably should be a whole sub class for these types
    private boolean dataMapObject = false;
    private NamedEdge edge;
    private SoarIdentifierVertex siv;
    private String dataMapName;

///////////////////////////////////////////////////////////////////
// Constructors
///////////////////////////////////////////////////////////////////
    // deny default construction
    private FeedbackListObject() {}

    /**
     * Create the List Object with the given parameters
     */
    public FeedbackListObject(OperatorNode in_node,
                              int in_ln,
                              String msg) 
    {
        node = in_node;
        lineNumber = in_ln;
        message = msg;
    }
    
    public FeedbackListObject(OperatorNode in_node,
                              int in_ln,
                              String msg,
                              String in_assocString)
    {
        node = in_node;
        lineNumber = in_ln;
        message = msg;
        assocString = in_assocString;
    }
    
    public FeedbackListObject(OperatorNode in_node,
                              int in_ln,
                              String msg,
                              boolean _msgEnough) 
    {
        this(in_node,in_ln,msg);
        msgEnough = _msgEnough;
    }
    
    public FeedbackListObject(OperatorNode in_node,
                              int in_ln,
                              String msg,
                              boolean _msgEnough,
                              boolean isError) 
    {
        this(in_node,in_ln,msg,_msgEnough);
        d_isError = isError;
    }

    public FeedbackListObject(OperatorNode in_node,
                              int in_ln,
                              String msg,
                              boolean _msgEnough,
                              boolean isError,
                              boolean isGenerated) 
    {
        this(in_node,in_ln,msg,_msgEnough);
        d_isGenerated = isGenerated;
    }

    public FeedbackListObject(NamedEdge in_edge,
                              SoarIdentifierVertex in_siv,
                              String inDataMapName,
                              String msg) 
    {
        edge = in_edge;
        siv = in_siv;
        message = msg;
        dataMapName = inDataMapName;
        dataMapObject = true;

    }
    
///////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////
    public boolean isError() 
    {
        return d_isError;
    }

    public boolean isGenerated() 
    {
        return d_isGenerated;
    }
    
    /**
     * returns the filename of the file for which the node of this
     * list object is associated
     * @return a string which is the file name of the file of which this object is associated
     */
    public String getFileName() 
    {
        return node.getFileName();
    }
    
    /**
     * returns the node for which this object is associated
     * @return a reference to the node
     */
    public OperatorNode getNode() 
    {
        return node;
    }
    
    /**
     * returns the line number for which this object is associated
     * @return an int that is positive
     */
    public int getLine() 
    {
        return lineNumber;
    }

    /**
   *  returns the SoarIdentifierVertex that denotes the datamap that
   *  this feedbackListObject refers too.
   */
    public SoarIdentifierVertex getSiv() 
    {
        if(isDataMapObject())
        return siv;
        else
        return null;
    }

    
    /**
     * returns the message for this object
     * @return a string that is additional information for the user
     */
    public String getMessage() 
    {
        return message;
    }
    
    /**
     * lets you set a new message
     * @param s a string that is the new message
     */
    public void setMessage(String s) 
    {
        message = s;
    }

    /**
   * determines whether object relates to a datamap or production
   * @return true if object relates to a datamap entry
   */
    public boolean isDataMapObject() 
    {
        return dataMapObject;
    }

    /**
   *  Creates a DataMap from the datamap information
   *  @return a datamap based on the information in this object, null if not a datamap object type
   */
    public DataMap createDataMap(SoarWorkingMemoryModel swmm) 
    {
        DataMap dm;
        if(isDataMapObject()) 
        {
            if(siv.getValue() != 0) 
            {
                dm = new DataMap(swmm, siv, dataMapName);
            }
            else 
            {
                dm = new DataMap(swmm, swmm.getTopstate(),dataMapName);
            }
            return dm;
        }
        else
        return null;
    }

    public int getDataMapId() 
    {
        if(isDataMapObject())
        return siv.getValue();
        else
        return -1;
    }

    /**
   *  Returns the NamedEdge associated with this object, null if not a datamap object
   */
    public NamedEdge getEdge() 
    {
        if(isDataMapObject()) 
        {
            return edge;
        }
        else
        return null;
    }
    
    /**
     * returns a string to represent this object, it is a combination of
     * the file, line number and the message
     * @return a string that represents this object
     */
    public String toString() 
    {
        if(!isDataMapObject()) 
        {
            if (msgEnough)
            return message;
            else
            return node.getUniqueName() + "(" + lineNumber + "): " + message;
        }
        else 
        {
            return dataMapName + ":  " + edge.toString();
        }
    }

/**
 * Displays the file associated with the node referenced by this object
 */
public void DisplayFile()
{
    if (assocString != null)
    {
        node.openRulesToString(MainFrame.getMainFrame(),
                               lineNumber,
                               assocString);
    }
    else if (lineNumber >= 0)
    {
        node.openRules(MainFrame.getMainFrame(), lineNumber);
    }
    else
    {
        node.openRules(MainFrame.getMainFrame());
    }
}

}//class FeedbackListObject
