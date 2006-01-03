package edu.umich.visualsoar.datamap;
import edu.umich.visualsoar.graph.*;
import java.util.*;
import javax.swing.event.TreeModelEvent;
import javax.swing.*;

/**
 * This class takes graph nodes and cleverly (or not so cleverly) disguises
 * as tree nodes, to prevent infinite recursion, the children are loaded when
 * needed
 * @author Brad Jones
 */

public class FakeTreeNode  
{
/////////////////////////////////////////
// Data Members
/////////////////////////////////////////
    
    // a flag noting whether or not this node has been loaded
    private boolean hasLoaded = false;
    
    // a string of how this node should be represented
    private String representation;
    
    // the vertex from which emanating edges are considered children
    private SoarVertex enumeratingVertex;
    
    // a reference to the graph structure so we can extract the information as needed
    private SoarWorkingMemoryModel swmm;
    
    // the parent of this node, can be null
    private FakeTreeNode parent;
    
    // the children for this node
    private Vector children = new Vector();
    
    // the associated edge for this node, can be null
    private NamedEdge theEdge = null;
    
/////////////////////////////////////////
// Constructors
/////////////////////////////////////////
    public FakeTreeNode(SoarWorkingMemoryModel in_swmm, SoarIdentifierVertex siv, String s) 
    {
        representation = s;
        enumeratingVertex = siv;
        swmm = in_swmm;
    }

    public FakeTreeNode(SoarWorkingMemoryModel in_swmm, NamedEdge ne) 
    {

        representation = ne.toString();
        enumeratingVertex = (SoarVertex)ne.V1();
        swmm = in_swmm;
        theEdge = ne;

        if( representation.equals("operator") ) 
        {
            boolean foundName = false;
            Enumeration e = swmm.emanatingEdges(enumeratingVertex);
            NamedEdge edge = null;
            while(e.hasMoreElements() && !foundName) 
            {
                edge = (NamedEdge)e.nextElement();
                if( ((edge.getName()).equals("name"))
                    && (edge.V1() instanceof EnumerationVertex) )
                {
                    foundName = true;
                }
            } // while looking through enumeration

            if(foundName) 
            {
                EnumerationVertex ev = (EnumerationVertex) edge.V1();
                if(ev != null)
                representation = "operator " + ev.toString();
            }

        }   // end of if the current node is an operator node

        // Add any possible comments to the representation of the fake node
        if(ne.hasComment() )
        representation = representation + "          * " + ne.getComment() + " *";
    }

//////////////////////////////////////////
// Accessors
//////////////////////////////////////////
    public FakeTreeNode getChildAt(int index) 
    {
        return ((FakeTreeNode)children.get(index));
    }
    
    public int getChildCount() 
    {
        if (!hasLoaded) 
        {
            int count = 0;
            Enumeration e = swmm.emanatingEdges(enumeratingVertex);
            while(e.hasMoreElements()) 
            {
                ++count;
                NamedEdge edge = (NamedEdge)e.nextElement();
                FakeTreeNode aChild = new FakeTreeNode(swmm,edge);
                aChild.setParent(this);
                children.add(aChild);
            }   
            hasLoaded = true;
            return count;
        }
        return children.size(); 
    }

    public NamedEdge getEdge() 
    {
        return theEdge;
    }

    public SoarVertex getEnumeratingVertex() 
    {
        return enumeratingVertex;
    }

    public int getIndex(FakeTreeNode ftn) 
    {
        return children.indexOf(ftn);
    }

    public FakeTreeNode getParent() 
    {
        return parent;
    }
    
    public Vector getTreePath() 
    {
        if (parent == null) 
        {
            Vector v = new Vector();
            v.add(this);
            return v;
        }
        else 
        {
            Vector v = parent.getTreePath();
            v.add(this);
            return v;
        }
    }

    public boolean hasLoaded() 
    {
        return hasLoaded;
    }
    
    public boolean isLeaf() 
    {
        return !enumeratingVertex.allowsEmanatingEdges();
    }

    public boolean isRoot() 
    {
        return (parent == null);
    }

    public String toString() 
    {
        return representation;
    }


//////////////////////////////////////////
// Manipulators
//////////////////////////////////////////
    public TreeModelEvent add(NamedEdge ne) 
    {
        int[] indices = new int[1];
        FakeTreeNode aChild = new FakeTreeNode(swmm,ne);
        aChild.setParent(this);
        boolean found = false;
        int foundAt = 0;
        for(int i = 0; i < children.size() && !found; ++i) 
        {
            NamedEdge current = getChildAt(i).getEdge();        
            if (current.getName().compareTo(ne.getName()) >= 0) 
            {
                found = true;
                foundAt = i;
            }
        }
        if (found) 
        {
            children.add(foundAt,aChild);
            indices[0] = foundAt;
        }
        else 
        {
            children.add(aChild);
            indices[0] = children.size() - 1;
        }
        
        return new TreeModelEvent(swmm,getTreePath().toArray(),indices,children.toArray());
    }


    public void setParent(FakeTreeNode ftn) 
    {
        parent = ftn;
    }

    public TreeModelEvent remove(NamedEdge ne) 
    {
        int[] indices = new int[1];
        boolean found = false;
        int count = 0;
        Enumeration e = children.elements();
        while(!found && e.hasMoreElements()) 
        {
            FakeTreeNode currentChild = (FakeTreeNode)e.nextElement();
            if(ne.equals(currentChild.getEdge())) 
            {
                found = true;   
                indices[0] = count;
            }
            ++count;
        }
        children.removeElementAt(indices[0]);
        return new TreeModelEvent(swmm,getTreePath().toArray(),indices,children.toArray());
    }
    
    public void visitChildren(edu.umich.visualsoar.util.Visitor v) 
    {
        Enumeration e = children.elements();
        while(e.hasMoreElements()) 
        {
            FakeTreeNode currentChild = (FakeTreeNode)e.nextElement();
            v.visit(currentChild);
            currentChild.visitChildren(v);
        }
    } 
}
