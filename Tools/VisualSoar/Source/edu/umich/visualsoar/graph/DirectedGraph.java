package edu.umich.visualsoar.graph;
import edu.umich.visualsoar.util.*;
import edu.umich.visualsoar.datamap.*;
import java.util.*;
import javax.swing.*;

/**
 * This class is a graph class that provides an interface for different kinds of graphs
 * it is based on Object-Oriented Design patterns in C++, later converted to Java
 * @author Brad Jones
 */
public abstract class DirectedGraph extends Graph {
///////////////////////////////////////////////////////////////////
// Methods
///////////////////////////////////////////////////////////////////
	public boolean isConnected() {
		for(int v = 0; v < numberOfVertices; ++v) {
			CountingVisitor visitor = new CountingVisitor();
			depthFirstTraversal(new PreOrder(visitor), selectVertex(v));
			if (visitor.count() != numberOfVertices)
				return false;
		}
		return true;
	}
	
	public boolean isCyclic() {
		CountingVisitor visitor = new CountingVisitor();
		topologicalOrderTraversal(visitor);
		return (visitor.count() != numberOfVertices);
	}
	
	public void topologicalOrderTraversal(Visitor visitor) {
		int[] inDegree = new int[numberOfVertices];
		for(int v = 0; v < numberOfVertices; ++v)
			inDegree[v] = 0;
		Enumeration e = edges();
		while(e.hasMoreElements()) {
			Edge edge = (Edge)e.nextElement();
			++inDegree[edge.V1().getValue()];
		}

		edu.umich.visualsoar.util.Queue queue = new QueueAsLinkedList();
		for(int v = 0; v < numberOfVertices; ++v)
			if (inDegree[v] == 0)
				queue.enqueue(selectVertex(v));
		
		while (!queue.isEmpty() && !visitor.isDone()) {
			Vertex vertex = (Vertex)queue.dequeue();
			visitor.visit(vertex);
			Enumeration q = emanatingEdges(vertex);
			while(q.hasMoreElements()) {
				Edge edge = (Edge)q.nextElement();
				Vertex to = edge.V1();
				if (--inDegree[to.getValue()] == 0)
					queue.enqueue(to);
			}
		}
	}

  /**
   *  Function uses a Breadth-first traversal to search
   *  through all of the vertices to find all vertices
   *  that connect to the SoarVertex parameter sv.
   *  Returns an enumeration of those vertices.
   */
  public List getParentVertices(SoarWorkingMemoryModel swmm, SoarVertex sv) {
    boolean[] visitedVertices = new boolean[numberOfVertices];
    for(int i = 1; i < numberOfVertices; i++)
      visitedVertices[i] = false;
    visitedVertices[0] = true;
    edu.umich.visualsoar.util.Queue queue = new QueueAsLinkedList();
    List foundVertices = new LinkedList();
    queue.enqueue(selectVertex(0));

    while(!queue.isEmpty()) {
      SoarVertex w = (SoarVertex)queue.dequeue();
      visitedVertices[w.getValue()] = true;

      if(w.allowsEmanatingEdges()) {
        Enumeration edges = swmm.emanatingEdges(w);
        while(edges.hasMoreElements()) {
          NamedEdge edge = (NamedEdge)edges.nextElement();
          if(! visitedVertices[edge.V1().getValue()]) {
            if(edge.V1().equals(sv)) {
              foundVertices.add(w);
            }
            visitedVertices[w.getValue()] = true;
            queue.enqueue(edge.V1());
          }     // if haven't visited this vertex yet, then check if match and add to queue

        }   // while edges have more elements
      }
    }    // while queue is not empty
    return foundVertices;
  }


  /**
   *  Similar to getParentVertices(), but this looks for a
   *  SoarIdentifierVertex that was recently created.
   */
  public SoarVertex getMatchingParent(SoarWorkingMemoryModel swmm, SoarVertex sv) {
    boolean[] visitedVertices = new boolean[numberOfVertices];
    for(int i = 1; i < numberOfVertices; i++)
      visitedVertices[i] = false;
    visitedVertices[0] = true;
    edu.umich.visualsoar.util.Queue queue = new QueueAsLinkedList();
    queue.enqueue(selectVertex(0));

    while(!queue.isEmpty()) {
      SoarVertex w = (SoarVertex)queue.dequeue();
      visitedVertices[w.getValue()] = true;
      if(w.allowsEmanatingEdges()) {
        Enumeration edges = swmm.emanatingEdges(w);
        while(edges.hasMoreElements()) {
          NamedEdge edge = (NamedEdge)edges.nextElement();
          if(! visitedVertices[edge.V1().getValue()]) {
            // Now find the edge that shares the same name, but is of type SoarIdentifierVertex
            if(edge.V1().equals(sv)) {
              Enumeration foundEdges = swmm.emanatingEdges(w);
              while(foundEdges.hasMoreElements()) {
                NamedEdge foundEdge = (NamedEdge)foundEdges.nextElement();
                if(edge.getName().equals(foundEdge.getName()) && (foundEdge.V1() instanceof SoarIdentifierVertex)) {
                  return ((SoarVertex)foundEdge.V1());
                }
              }
            }
            visitedVertices[w.getValue()] = true;
            queue.enqueue(edge.V1());
          }     // if haven't visited this vertex yet, then check if match and add to queue
        }   // while edges have more elements
      }
    }    // while queue is not empty
    return null;


  }

	
	/**
	 * This function finds all vertices that are unreachable from a state
	 * and adds them to a list of holes so that they can be recycled for later
	 * use
	 */
	 public abstract void reduce(List listOfStartVertices);
	 public abstract void resolve();
}
