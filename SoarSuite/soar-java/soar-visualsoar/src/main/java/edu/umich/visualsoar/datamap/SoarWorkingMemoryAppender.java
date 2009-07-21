package edu.umich.visualsoar.datamap;
import edu.umich.visualsoar.graph.SoarVertex;
import java.io.*;
import edu.umich.visualsoar.util.ReaderUtils;
import java.util.Vector;
public class SoarWorkingMemoryAppender {
	public static void append(SoarWorkingMemoryModel swmm,Reader fr) throws IOException, NumberFormatException {
		try {
			int prevNumber = swmm.numberOfVertices();
			int numberOfVertices = ReaderUtils.getInteger(fr);			
			for(int i = 0; i < numberOfVertices; ++i) {
				String type = ReaderUtils.getWord(fr);
				SoarVertex vertexToAdd = null;
				int id = ReaderUtils.getInteger(fr);
				if (type.equals("SOAR_ID")) {
					vertexToAdd = swmm.createNewSoarId();
				}
				else if (type.equals("ENUMERATION")) {
					int enumerationSize = ReaderUtils.getInteger(fr);
					Vector v = new Vector();
					for(int j = 0; j < enumerationSize; ++j) 
						v.add(ReaderUtils.getWord(fr));
					vertexToAdd = swmm.createNewEnumeration(v);
				}
				else if (type.equals("INTEGER_RANGE")) {
					vertexToAdd = swmm.createNewIntegerRange(ReaderUtils.getInteger(fr),ReaderUtils.getInteger(fr));
				}
				else if (type.equals("INTEGER")) {
					vertexToAdd = swmm.createNewInteger();
				}
				else if (type.equals("FLOAT_RANGE")) {
					vertexToAdd = swmm.createNewFloatRange(ReaderUtils.getFloat(fr),ReaderUtils.getFloat(fr));
				}
				else if (type.equals("FLOAT")) {
					vertexToAdd = swmm.createNewFloat();
				}
				else if (type.equals("STRING")) {
					vertexToAdd = swmm.createNewString();
				}
				else {
					System.err.println("Unknown type: please update SoarWorking Memory Reader constructor :" + type);
				}
								
			}
			int numberOfEdges = ReaderUtils.getInteger(fr);
			for(int j = 0; j < numberOfEdges; ++j) 
				swmm.addTriple(swmm.getVertexForId(ReaderUtils.getInteger(fr)+prevNumber),ReaderUtils.getWord(fr),swmm.getVertexForId(ReaderUtils.getInteger(fr)+prevNumber) );
		}
		catch(IOException ioe) {
			ioe.printStackTrace();
		}
		catch(NumberFormatException nfe) {
			nfe.printStackTrace();
			throw nfe;
		}
	}
}
