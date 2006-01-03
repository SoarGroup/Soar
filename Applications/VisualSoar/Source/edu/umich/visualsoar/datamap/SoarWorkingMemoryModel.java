package edu.umich.visualsoar.datamap;

import edu.umich.visualsoar.graph.*;
import edu.umich.visualsoar.parser.*;
import edu.umich.visualsoar.operatorwindow.*;
import java.util.*;
import java.io.*;
import javax.swing.*;

 /**
  * This is a model of Soar's Working Memory represented by a directed graph
  * structure
  * @author Brad Jones
  */

public class SoarWorkingMemoryModel 
{
//////////////////////////////////////////////////////////
// Data Members
//////////////////////////////////////////////////////////
    // A Directed Graph that is supposed to represent WM    
    private DirectedGraph rep = new OrderedDirectedGraphAsAdjacencyLists();
    private LinkedList listeners = new LinkedList();
    private Map properties = new TreeMap();

/////////////////////////////////////////////////////////
// Constructors
/////////////////////////////////////////////////////////

    /**
     * Creates a default model of working memory
     * @param isNew only create if model is new
     * @param name name used for the topstate
     * @see #addTriple(SoarVertex,String,SoarVertex)
     * @see #addProperty(String,SoarVertex)
     */
    public SoarWorkingMemoryModel(boolean isNew,String name) 
    {
        if (isNew) 
        {
            addProperty("TOPSTATE",createNewSoarId());
            addProperty("IO",createNewSoarId());
            addProperty("INPUTLINK",createNewSoarId());
            addProperty("OUTPUTLINK",createNewSoarId());
            addProperty("INITIALIZE-" + name, createNewSoarId());
            addTriple(getProperty("TOPSTATE"),"type",createNewEnumeration("state"));
            addTriple(getProperty("TOPSTATE"),"superstate",createNewEnumeration("nil"));
            addTriple(getProperty("TOPSTATE"),"top-state",getProperty("TOPSTATE"));
            addTriple(getProperty("TOPSTATE"),"name",createNewEnumeration(name));
            addTriple(getProperty("TOPSTATE"),"io",getProperty("IO"));
            addTriple(getProperty("TOPSTATE"),"operator",getProperty("INITIALIZE-" + name));
            addTriple(getProperty("IO"),"input-link",getProperty("INPUTLINK"));
            addTriple(getProperty("IO"),"output-link",getProperty("OUTPUTLINK"));
            addTriple(getProperty("INITIALIZE-" + name),"name",createNewEnumeration("initialize-" + name));
        }
    }

/////////////////////////////////////////////////////////
// Methods
/////////////////////////////////////////////////////////
    public void addProperty(String name,SoarVertex sv) 
    {
        properties.put(name,sv);
    }
    
    public void changeProperty(String name,SoarVertex value) 
    {
        properties.remove(name);
        properties.put(name,value);
    }
    
    public SoarVertex getProperty(String name) 
    {
        return (SoarVertex)properties.get(name);
    }
    
    public void removeProperty(String name) 
    {
        properties.remove(name);
    }
    
    public Set getEntrySet() 
    {
        return properties.entrySet();
    }
    
    /**
     * This sets the topstate, this should only be called by trusted
     * members
     */
    public void setTopstate(SoarIdentifierVertex siv) 
    {
        rep.addVertex(siv); 
        addProperty("TOPSTATE",siv);
    }
    
    /**
     * For a particular id, returns the corresponding soar vertex
     */
    public SoarVertex getVertexForId(int i) 
    {
        return (SoarVertex)rep.selectVertex(i);
    }
    
    
    
    
    /**
     * Adds a listener to working memory, to receive working memory events
     */
    public void addWorkingMemoryListener(WorkingMemoryListener l) 
    {
        listeners.add(l);
    }
    
    /**
     * removes a listener from working memory, to make it stop receiving working memory events
     */
    public void removeWorkingMemoryListener(WorkingMemoryListener l) 
    {
        listeners.remove(l);
    }
    
    /**
     * Adds a triple to working memory
     */
    public void addTriple(SoarVertex v0, String attribute, SoarVertex v1) 
    {
        if (!v0.allowsEmanatingEdges())
        throw new IllegalArgumentException("The First SoarVertex does not allow emanating edges");
        NamedEdge ne = new NamedEdge(v0,v1,attribute);
        rep.addEdge(ne);
        notifyListenersOfAdd(ne);
    }

    /**
     * Adds a triple to working memory
     * Also sets a comment to the edge that the triple creates.
     * This comment is read in by the SoarWorkingMemoryReader when the .dm file is loaded
     */
    public void addTriple(SoarVertex v0, String attribute, SoarVertex v1, int generated, String comment) 
    {
        if (!v0.allowsEmanatingEdges())
        throw new IllegalArgumentException("The First SoarVertex does not allow emanating edges");
        NamedEdge ne = new NamedEdge(v0,v1,attribute);
        if(comment.length() > 1)
        ne.setComment(comment);
        if(generated == 1)
        ne.setAsGenerated();
        rep.addEdge(ne);
        notifyListenersOfAdd(ne);
    }

    /**
     *  Adds a triple to working memory
     *  Also denotes that this triple was created by the automatic DataMap generator.
     *  This will cause the dataMap entry of this triple to show up as colored text
     *  until the entry is validated by the user as acceptable.
     */
    public void addTriple(SoarVertex v0, String attribute, SoarVertex v1, boolean generated, OperatorNode current, int lineNumber) 
    {
        if (!v0.allowsEmanatingEdges())
        throw new IllegalArgumentException("The First SoarVertex does not allow emanating edges");
        NamedEdge ne = new NamedEdge(v0,v1,attribute);
        if(generated) 
        {
            ne.setAsGenerated();
            ne.setNode(current);
            ne.setLineNumber(lineNumber);
        }
        rep.addEdge(ne);
        notifyListenersOfAdd(ne);
    }

    
    public SoarIdentifierVertex getTopstate() 
    {
        return (SoarIdentifierVertex)getProperty("TOPSTATE");
    }

    /**
     * Returns an enumeration of all the edges that are emanating/leaving from a particular vertex.
     * @see NamedEdge
     */
    public Enumeration emanatingEdges(Vertex v) 
    {
        return rep.emanatingEdges(v);
    }

    public Enumeration getIncidentEdges(Vertex v) 
    {
        return rep.incidentEdges(v);
    }

/////////////////
    public Enumeration getEdges() 
    {
        return rep.edges();
    }
//////////////

    /**
     * Returns the number of vertices contained in working memory
     * @see SoarVertex
     */
    public int numberOfVertices() 
    {
        return rep.numberOfVertices();
    }

    /**
     * Returns the total number of edges contained in working memory
     * @see NamedEdge
     */
    public int numberOfEdges() 
    {
        return rep.numberOfEdges();
    }
    
    /**
     * This is only needed by the reader, in the future this will only be package accessible
     * regular users should go through the factory create methods
     */
    public void addVertex(Vertex v) 
    {
        rep.addVertex(v);
    }
    
    public void reduce(List startVertices) 
    {
        rep.reduce(startVertices);
    }
     
    /**
     * Removes the requested triple from Working Memory
     */
    public void removeTriple(SoarVertex v0, String attribute, SoarVertex v1) 
    {
        NamedEdge ne = new NamedEdge(v0,v1,attribute);
        rep.removeEdge(ne);
        notifyListenersOfRemove(ne);
    }

    /**
     * Returns an exact copy of a SoarVertex with a new id
     */
    public SoarVertex createVertexCopy(SoarVertex orig) 
    {
        SoarVertex  cpy = orig.copy(getNextVertexId());

        rep.addVertex(cpy);
        return cpy;
    }

    /**
     * Create a new EnumerationVertex with a vector of strings that
     * represent possible values for that enumeration
     * @param s the vector of strings that represent values
     * @see EnumerationVertex
     */
    public EnumerationVertex createNewEnumeration(java.util.Vector s) 
    {
        int id = getNextVertexId();
        EnumerationVertex e = new EnumerationVertex(id,s);
        rep.addVertex(e);
        return e;
    }

    /**
     * Create a new EnumerationVertex with a string that
     * represents the only acceptable value for that enumeration
     * @param s string representing the value for that enumeration attribute
     * @see EnumerationVertex
     */
    public EnumerationVertex createNewEnumeration(String s) 
    {
        int id = getNextVertexId();
        EnumerationVertex e = new EnumerationVertex(id,s);
        rep.addVertex(e);
        return e;
    }

    /**
     * Create Integer Vertex with default (unbounded) range
     * @see IntegerRangeVertex
     */
    public IntegerRangeVertex createNewInteger() 
    {
        int id = getNextVertexId();
        IntegerRangeVertex i = new IntegerRangeVertex(id,Integer.MIN_VALUE,Integer.MAX_VALUE);
        rep.addVertex(i);
        return i;
    }

    /** Create Integer Vertex with specified range
     * @param low the minimum limit of the integer attribute
     * @param high the maximum limit of the integer attribute
     * @see IntegerRangeVertex
     */
    public IntegerRangeVertex createNewIntegerRange(int low, int high) 
    {
        int id = getNextVertexId();
        IntegerRangeVertex i = new IntegerRangeVertex(id,low,high);
        rep.addVertex(i);
        return i;
    }

    /**
     * Create a StringVertex attribute
     * @see StringVertex
     */
    public StringVertex createNewString() 
    {
        int id = getNextVertexId();
        StringVertex s = new StringVertex(id);
        rep.addVertex(s);
        return s;
    }

    /**
     * Create Float Vertex with default (unbounded) range
     * @see FloatRangeVertex
     */
    public FloatRangeVertex createNewFloat() 
    {
        int id = getNextVertexId();
        FloatRangeVertex f = new FloatRangeVertex(id,Float.NEGATIVE_INFINITY,Float.POSITIVE_INFINITY);
        rep.addVertex(f);
        return f;
    }

    /** Create Float Vertex with specified range
     * @param low the minimum limit of the float attribute
     * @param high the maximum limit of the float attribute
     * @see FloatRangeVertex
     */
    public FloatRangeVertex createNewFloatRange(float low, float high) 
    {
        int id = getNextVertexId();
        FloatRangeVertex f = new FloatRangeVertex(id,low,high);
        rep.addVertex(f);
        return f;
    }

    /**
     * Create a basic soar Identifier Vertex
     * @see SoarIdentifierVertex
     */
    public SoarIdentifierVertex createNewSoarId() 
    {
        int id = getNextVertexId();
        SoarIdentifierVertex s = new SoarIdentifierVertex(id);
        rep.addVertex(s);
        return s;
    } 

    /**
     * Called to create the Vertex and appropriate identifiers for a high-level operator
     * @param superstate the parent operator of the operator
     * @param name the name of the high-level operator
     * @return SoarIdentifierVertex that is the dataMapId for that operator
     * @see SoarIdentifierVertex
     */
    public SoarIdentifierVertex createNewStateId(SoarIdentifierVertex superstate,String name) 
    {
        SoarIdentifierVertex s = createNewSoarId();
        SoarVertex type_state = createNewEnumeration("state");
        SoarVertex nameVertex = createNewEnumeration(name);
        addTriple(s,"type",type_state);
        SoarVertex op = createNewSoarId();
        if(superstate != null)
        addTriple(s,"superstate",superstate);
        addTriple(s,"name",nameVertex);
        addTriple(s,"top-state",getTopstate());
        return s;
    }

    /**
     * Each Vertex in Working Memory has a unique ID, function returns the next
     * available Vertex ID
     */
    protected int getNextVertexId() 
    {
        return rep.numberOfVertices(); 
    }

    /**
     * Notifies the listeners to refresh the screen
     * @param ne the edge that was added
     */
    protected void notifyListenersOfAdd(NamedEdge ne) 
    {
        Iterator i = listeners.iterator();
        while(i.hasNext()) 
        {
            WorkingMemoryListener wml = (WorkingMemoryListener)i.next();
            wml.WMEAdded(new WorkingMemoryEvent(ne));
        }
    }

    /**
     * Notifies the listeners to refresh the screen
     * @param ne the edge that has been removed
     */
    protected void notifyListenersOfRemove(NamedEdge ne) 
    {
        Iterator i = listeners.iterator();
        while(i.hasNext()) 
        {
            WorkingMemoryListener wml = (WorkingMemoryListener)i.next();
            wml.WMERemoved(new WorkingMemoryEvent(ne));
        }
    }


    /**
     * Writes out information for the datamap file (.dm file)
     * .dm datamap file is in the format of:
     * number of vertices <br>
     * write out all vertices: &nbsp;&nbsp; |vertex type| &nbsp;&nbsp; |vertex id int| &nbsp;&nbsp; |number of enumerations| &nbsp;&nbsp; |enumeration strings .....| <br>
     * number of edges <br>
     * write out all edges:  &nbsp;&nbsp; | left vertex | &nbsp;&nbsp; |name of edge| &nbsp;&nbsp; |right vertex|
     * @param graphWriter the file to be written too - .dm file
     */
    public void write(Writer graphWriter) throws IOException 
    {
        // Write out the number of Vertices
        graphWriter.write("" + rep.numberOfVertices() + '\n');

        // Write out all the vertices
        Enumeration v = rep.vertices();
        while(v.hasMoreElements()) 
        {
            SoarVertex vertex = (SoarVertex)v.nextElement();
            if (vertex != null)
            vertex.write(graphWriter);
        }
    
        // Write out the number of edges
        graphWriter.write("" + rep.numberOfEdges() + '\n');
        // Write out all the edges
        Enumeration e = rep.edges();
        while(e.hasMoreElements()) 
        {
            NamedEdge edge = (NamedEdge)e.nextElement();
            edge.write(graphWriter);
        }
    }

    /**
     * Writes all of the edge comments to the writer to create the comment file
     * and whether a datamap entry is generated or not.
     * Each line represents an entry of the datamap.  Each line contains a 1 or a 0
     * that signifies if generates or not and is followed by a comment if there is one.
     */
    public void writeComments(Writer commentWriter) throws IOException 
    {
        // Write out all the edge comments
        Enumeration e = rep.edges();
        while(e.hasMoreElements()) 
        {
            NamedEdge edge = (NamedEdge)e.nextElement();
            if(edge.isGenerated()) 
            {
                commentWriter.write("1 " + edge.getComment()  + '\n');
            }
            else
            commentWriter.write("0 " + edge.getComment() + '\n');
        }
    }

    /**
     * Function finds the set of variables within a productions that matches a
     * given string
     * @param sv the SoarIdentifierVertex currently checking
     * @param sp the SoarProduction to check for matching variables
     * @param variable the string that function tries to match
     * @return a List of matches, empty list if nothing found
     */
    public List matches(SoarIdentifierVertex sv,
                        SoarProduction sp,
                        String variable) 
    {
        TriplesExtractor triplesExtractor = new TriplesExtractor(sp);
        Map matchesMap = DataMapMatcher.matches(this,sv,triplesExtractor,new DoNothingMatcherErrorHandler());
        List matches = new LinkedList();
        Set matchesSet = (Set)matchesMap.get(variable);
        if(matchesSet != null)
        matches.addAll(matchesSet);
        return matches;
    }

    /**
     * Used to determine if a soar production matches Working Memory
     * @param sv the SoarIdentifierVertex in WorkingMemory currently checking
     * @param sp the Soar Production to check
     * @return a list of errors
     * @see DefaultCheckerErrorHandler
     * @see DataMapChecker#check(SoarWorkingMemoryModel,SoarIdentifierVertex,TriplesExtractor,DefaultCheckerErrorHandler)
     */
    public List checkProduction(SoarIdentifierVertex sv, SoarProduction sp) 
    {
        TriplesExtractor triplesExtractor = new TriplesExtractor(sp);
        DefaultCheckerErrorHandler dceh = new DefaultCheckerErrorHandler(sp.getName(),sp.getStartLine());
        DataMapChecker.check(this,sv,triplesExtractor,dceh);
        return dceh.getErrors();
    }

/////////////////////////////////
    /**
     * Used to determine if a soar production matches Working Memory
     * This function will also generate a log file to keep track of production checking
     * @param sv the SoarIdentifierVertex in WorkingMemory currently checking
     * @param sp the Soar Production to check
     * @param fw the log file that is being written too - "CheckingProductions.log"
     * @return a list of errors
     * @see DefaultCheckerErrorHandler
     * @see DataMapChecker#check(SoarWorkingMemoryModel,SoarIdentifierVertex,TriplesExtractor,DefaultCheckerErrorHandler)
     */
    public List checkProductionLog(SoarIdentifierVertex sv, SoarProduction sp, FileWriter fw) 
    {
        TriplesExtractor triplesExtractor = new TriplesExtractor(sp);
        try 
        {
            fw.write("Extracted the triples for the production " + sp.getName());
            fw.write('\n');
        }
        catch(IOException ioe) 
        {
            ioe.printStackTrace();
        }
        DefaultCheckerErrorHandler dceh = new DefaultCheckerErrorHandler(sp.getName(),sp.getStartLine());
        DataMapChecker.checkLog(this,sv,triplesExtractor,dceh, fw);
        return dceh.getErrors();
    }
////////////////////////////////////////////

    /**
     * Used by the Generate Productions actions to look for holes in Working Memory
     * and fix those holes.
     * @param sv the SoarIdentifierVertex in WorkingMemory currently checking
     * @param sp the Soar Production to check
     * @param current the node being examined
     * @return a list of errors
     * @see DefaultCheckerErrorHandler
     * @see DataMapChecker#complete(SoarWorkingMemoryModel,SoarIdentifierVertex,TriplesExtractor,DefaultCheckerErrorHandler,OperatorNode)
     */
    public List checkGenerateProduction(SoarIdentifierVertex sv, SoarProduction sp, OperatorNode current) 
    {
        TriplesExtractor triplesExtractor = new TriplesExtractor(sp);
        DefaultCheckerErrorHandler dceh = new DefaultCheckerErrorHandler(sp.getName(), sp.getStartLine());
        DataMapChecker.complete(this,sv,triplesExtractor, dceh, current);
        return dceh.getErrors();
    }

    /**
     * Return all parents of a vertex
     * @param sv the source vertex
     * @return a list of all matching SoarVertex 's
     */
    public List getParents(SoarVertex sv) 
    {
        return rep.getParentVertices(this, sv);
    }

    /**
     * @return first matching SoarVertex parent
     * @see DirectedGraph#getMatchingParent(SoarWorkingMemoryModel, SoarVertex)
     */
    public SoarVertex getMatchingParent(SoarVertex sv) 
    {
        return rep.getMatchingParent(this, sv);
    }

    public void resolve() 
    {
        rep.resolve();
    }

    /**
     * Returns the number of vertices in Working Memory
     */
    public int getNumberOfVertices() 
    {
        return rep.numberOfVertices();
    }
}
