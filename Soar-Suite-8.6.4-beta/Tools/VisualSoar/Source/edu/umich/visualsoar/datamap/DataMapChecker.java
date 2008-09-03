package edu.umich.visualsoar.datamap;
import edu.umich.visualsoar.parser.TriplesExtractor;
import edu.umich.visualsoar.graph.SoarIdentifierVertex;
import edu.umich.visualsoar.operatorwindow.*;
import java.util.*;
import javax.swing.*;
import java.io.FileWriter;

/**
 * This class provides some static methods to do the checking against the
 * datamap
 * @author Brad Jones */
 
public class DataMapChecker 
{
    // There is no need to instantiate this class
    private DataMapChecker() {}
    
    // Static Member Functions

    public static void check(SoarWorkingMemoryModel dataMap,
                             SoarIdentifierVertex startVertex,
                             TriplesExtractor triplesExtractor,
                             CheckerErrorHandler ceh) 
    {
        Map varMap = DataMapMatcher.matches(dataMap,
                                            startVertex,
                                            triplesExtractor,
                                            ceh);
        if(varMap != null) 
        {
            Set keySet = varMap.keySet();
            Iterator vars = keySet.iterator();
            while(vars.hasNext()) 
            {
                String varKey = (String)vars.next();
                Set value = (Set)varMap.get(varKey);
                if(value.isEmpty()) 
                ceh.variableNotMatched(varKey); 
            }
        }
    }

    /**
     * Similar to check(), but writes comments to a log file
     */
    public static void checkLog(SoarWorkingMemoryModel dataMap,
                                SoarIdentifierVertex startVertex,
                                TriplesExtractor triplesExtractor,
                                CheckerErrorHandler ceh,
                                FileWriter fw) 
    {
        Map varMap = DataMapMatcher.matchesLog(dataMap,
                                               startVertex,
                                               triplesExtractor,
                                               ceh,
                                               fw);
        if(varMap != null) 
        {
            Set keySet = varMap.keySet();
            Iterator vars = keySet.iterator();
            while(vars.hasNext()) 
            {
                String varKey = (String)vars.next();
                Set value = (Set)varMap.get(varKey);
                if(value.isEmpty())
                ceh.variableNotMatched(varKey);
            }
        }
    }


    /*
     *  This function is responsible for checking the datamap to see if the triples in the triplesExtractor are valid.
     *  In the triples are invalid, the datamap is adjusted to match the triples.
     */
    public static void complete(SoarWorkingMemoryModel dataMap,
                                SoarIdentifierVertex startVertex,
                                TriplesExtractor triplesExtractor,
                                CheckerErrorHandler ceh,
                                OperatorNode current) 
    {
        DataMapMatcher.complete(dataMap, startVertex, triplesExtractor, ceh, current);
    }//complete()
    
}

