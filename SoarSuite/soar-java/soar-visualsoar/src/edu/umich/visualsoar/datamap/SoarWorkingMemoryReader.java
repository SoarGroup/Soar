package edu.umich.visualsoar.datamap;

import edu.umich.visualsoar.graph.*;
import edu.umich.visualsoar.util.ReaderUtils;
import java.util.Vector;
import java.io.*;
import javax.swing.*;

/**
 * This class has one function read, which reads a representation of soar working memory
 * from a file and re-creates the DataMap
 * @author Brad Jones
 */
 
public class SoarWorkingMemoryReader {
	/**
		This function reads a description of soars working memory
		from a file and re-creates the datamap
		
		@param swmm the working memory model that the description should be read into
		@param fr the Reader that the description should be read from
    @param cr the Reader that datamap comments should be read from (if exists)
		
		@throws IOException if something goes wrong
	*/
	public static void read(SoarWorkingMemoryModel swmm,Reader fr, Reader cr) throws IOException {
		try {
			// Get the number of vertices from the file
			int numberOfVertices = ReaderUtils.getInteger(fr);


			// Get the root node
			String rootType = ReaderUtils.getWord(fr);
			SoarIdentifierVertex topstate = null;

			if (rootType.equals("SOAR_ID"))
				topstate = new SoarIdentifierVertex(ReaderUtils.getInteger(fr));
			else
				System.err.println("Root type must be Soar id");
				
			swmm.setTopstate(topstate);


			// Get the rest of the vertices
			for(int i = 1; i < numberOfVertices; ++i) {
				String type = ReaderUtils.getWord(fr);
				SoarVertex vertexToAdd = null;
				int id = ReaderUtils.getInteger(fr);
				if (type.equals("SOAR_ID")) {
					vertexToAdd = new SoarIdentifierVertex(id);
				}
				else if (type.equals("ENUMERATION")) {
					int enumerationSize = ReaderUtils.getInteger(fr);
					Vector v = new Vector();
					for(int j = 0; j < enumerationSize; ++j) 
						v.add(ReaderUtils.getWord(fr));
					vertexToAdd = new EnumerationVertex(id,v);
				}
				else if (type.equals("INTEGER_RANGE")) {
					vertexToAdd = new IntegerRangeVertex(id,ReaderUtils.getInteger(fr),ReaderUtils.getInteger(fr));
				}
				else if (type.equals("INTEGER")) {
					vertexToAdd = new IntegerRangeVertex(id,Integer.MIN_VALUE,Integer.MAX_VALUE);
				}
				else if (type.equals("FLOAT_RANGE")) {
					vertexToAdd = new FloatRangeVertex(id,ReaderUtils.getFloat(fr),ReaderUtils.getFloat(fr));
				}
				else if (type.equals("FLOAT")) {
					vertexToAdd = new FloatRangeVertex(id,Float.NEGATIVE_INFINITY,Float.POSITIVE_INFINITY);
				}
				else if (type.equals("STRING")) {
					vertexToAdd = new StringVertex(id);
				}
				else {
					System.err.println("Unknown type: please update SoarWorking Memory Reader constructor :" + type);
				}
				swmm.addVertex(vertexToAdd);				
			}


			// Get the number edges
			int numberOfEdges = ReaderUtils.getInteger(fr);

      
      // Check to see if a Comment file existed
      if(cr != null) {
        // Read in the edges and connect them and also read in the comment file
        for(int j = 0; j < numberOfEdges; ++j) {
          swmm.addTriple(swmm.getVertexForId(ReaderUtils.getInteger(fr)), ReaderUtils.getWord(fr), swmm.getVertexForId(ReaderUtils.getInteger(fr)), ReaderUtils.getInteger(cr), ReaderUtils.getLine(cr) );
        }

      }
      else {
  			// Read in the edges and connect them
  			for(int j = 0; j < numberOfEdges; ++j)   {
  			 	swmm.addTriple(swmm.getVertexForId(ReaderUtils.getInteger(fr)), ReaderUtils.getWord(fr), swmm.getVertexForId(ReaderUtils.getInteger(fr)) );
        }
      }
		}
		catch(IOException ioe) {
			ioe.printStackTrace();
			throw ioe;
		}
		catch(NumberFormatException nfe) {
			nfe.printStackTrace();
			throw nfe;
		}
	}
}
