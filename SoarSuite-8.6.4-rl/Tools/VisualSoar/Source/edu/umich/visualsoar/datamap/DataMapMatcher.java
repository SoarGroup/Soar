package edu.umich.visualsoar.datamap;
import edu.umich.visualsoar.parser.*;
import edu.umich.visualsoar.graph.*;
import edu.umich.visualsoar.util.EnumerationIteratorWrapper;
import edu.umich.visualsoar.operatorwindow.*;
import javax.swing.*;
import javax.swing.tree.*;
import java.util.*;
import java.io.*;


/**
 * This class provides some static methods to do the matching against the
 * datamap
 * @author Brad Jones */

public class DataMapMatcher 
{
    // There is no need to instantiate this class
    private DataMapMatcher() {}

    // Static Member Functions
    /**
     * Checks to see if a production has a matching data structure in the
     * datamap
     * @param dataMap Soar Working Memory
     * @param startVertex the state in working memory that is being examined
     * @param triplesExtractor all the triples that were in a production
     * @param meh the structure that holds the errors when they are found */
    public static Map matches(SoarWorkingMemoryModel dataMap,
                              SoarIdentifierVertex startVertex,
                              TriplesExtractor triplesExtractor,
                              MatcherErrorHandler meh) 
    {
        Map varMap = new HashMap();
        Iterator iter = triplesExtractor.variables();
        while(iter.hasNext()) 
        {
            varMap.put(((Pair)iter.next()).getString(),new HashSet());
        }
        
        // Take care of the first Variable
        // Make sure there are the right number of state variables
        int result = triplesExtractor.getStateVariableCount();
        if(result == 0) 
        {
            meh.noStateVariable();
            return null;
        }
        else if(triplesExtractor.getStateVariableCount() > 1) 
        {
            meh.tooManyStateVariables();
            return null;
        }

        Pair stateVar = triplesExtractor.stateVariable();
        Set stateSet = (Set)varMap.get(stateVar.getString());
        stateSet.add(startVertex);
        
        Enumeration e = new EnumerationIteratorWrapper(triplesExtractor.triples());
        while(e.hasMoreElements()) 
        {
            Triple currentTriple = (Triple)e.nextElement();
            if ( (currentTriple.getAttribute().getString().equals("operator"))
                 && (TripleUtils.isFloat(currentTriple.getValue().getString())
                     ||  TripleUtils.isInteger(currentTriple.getValue().getString()) ) ) 
            {
                continue;
            }
            if (!addConstraint(dataMap,currentTriple,varMap)) 
            {
                meh.badConstraint(currentTriple);
            }
        }
        return varMap;
    }


    /**
     * Similar to matches(), but writes comments to a log file
     */
    public static Map matchesLog(SoarWorkingMemoryModel dataMap,
                                 SoarIdentifierVertex startVertex,
                                 TriplesExtractor triplesExtractor,
                                 MatcherErrorHandler meh,
                                 FileWriter log) 
    {
        Map varMap = new HashMap();
        Iterator iter = triplesExtractor.variables();
        while(iter.hasNext()) 
        {
            varMap.put(((Pair)iter.next()).getString(),new HashSet());
        }

        // Take care of the first Variable
        // Make sure there are the right number of state variables
        int result = triplesExtractor.getStateVariableCount();
        if(result == 0) 
        {
            meh.noStateVariable();
            return null;
        }
        else if(triplesExtractor.getStateVariableCount() > 1) 
        {
            meh.tooManyStateVariables();
            return null;
        }

        Pair stateVar = triplesExtractor.stateVariable();
        Set stateSet = (Set)varMap.get(stateVar.getString());
        stateSet.add(startVertex);
        
        Enumeration e = new EnumerationIteratorWrapper(triplesExtractor.triples());
        while(e.hasMoreElements()) 
        {
            Triple currentTriple = (Triple)e.nextElement();

            try 
            {
                log.write("currentTriple's attribute is:  " + currentTriple.getAttribute().getString() );
                log.write('\n');
            }
            catch(IOException ioe) 
            {
                ioe.printStackTrace();
            }


            if ( (currentTriple.getAttribute().getString()).equals("operator")) 
            {
                try 
                {
                    log.write("triple has attribute called operator");
                    log.write('\n');
                }
                catch(IOException ioe) 
                {
                    ioe.printStackTrace();
                }

                if( TripleUtils.isFloat(currentTriple.getValue().getString()) ||   TripleUtils.isInteger(currentTriple.getValue().getString()) ) 
                {
                    try 
                    {
                        log.write("Ignoring the triple " + currentTriple.toString());
                        log.write('\n');
                    }
                    catch(IOException ioe) 
                    {
                        ioe.printStackTrace();
                    }
                    continue;
                } // if value is integer or float
            } // if attribute is "operator"

            try 
            {
                log.write("Examining the triple " + currentTriple.toString());
                log.write('\n');
            }
            catch(IOException ioe) 
            {
                ioe.printStackTrace();
            }

            if (!addConstraintLog(dataMap,currentTriple,varMap, log)) 
            {
                try 
                {
                    log.write("Could not match a constraint for the triple " + currentTriple.toString());
                    log.write('\n');
                }
                catch(IOException ioe) 
                {
                    ioe.printStackTrace();
                }
                meh.badConstraint(currentTriple);
            }
        }
        return varMap;
    }


    /*
     * This function checks to see if the triplesExtractor triples match the
     * datamap correctly.  If not, the datamap is then corrected to match the
     * triple.
     */
    public static void complete(SoarWorkingMemoryModel dataMap,
                                SoarIdentifierVertex startVertex,
                                TriplesExtractor triplesExtractor,
                                MatcherErrorHandler meh,
                                OperatorNode current)
    {

        boolean notFound = true;

        Map varMap = new HashMap();
        Iterator iter = triplesExtractor.variables();
        while(iter.hasNext()) 
        {
            varMap.put(((Pair)iter.next()).getString(),new HashSet());
        }


        Pair stateVar = triplesExtractor.stateVariable();
        Set stateSet = (Set)varMap.get(stateVar.getString());
        stateSet.add(startVertex);


        Enumeration e = new EnumerationIteratorWrapper(triplesExtractor.triples());
        while(e.hasMoreElements()) 
        {
            notFound = true;
            Triple currentTriple = (Triple)e.nextElement();
            if ( (currentTriple.getAttribute().getString().equals("operator"))  && (TripleUtils.isFloat(currentTriple.getValue().getString()) ||   TripleUtils.isInteger(currentTriple.getValue().getString()) ) ) 
            {
                continue;
            }

            // Error in DataMap, generate new structure to fix this error
            if (!addConstraint(dataMap,currentTriple,varMap))
            {

                // Ignore case if attribute is a 'variable' (<' '>)
                if( TripleUtils.isVariable(currentTriple.getAttribute().getString()) )
                return;

                Set varSet = (Set)varMap.get(currentTriple.getVariable().getString());


                // for every possible start
                Enumeration ed = new EnumerationIteratorWrapper(varSet.iterator());
                while(ed.hasMoreElements()) 
                {
                    notFound = true;
                    NamedEdge attributeEdge = null;
                    Object o = ed.nextElement();
                    SoarVertex currentSV = null;


                    // If parent Vertex is not a SoarIdentifierVertex, need to
                    // create one
                    if(!(o instanceof SoarIdentifierVertex)) 
                    {
                        if(o instanceof SoarVertex) 
                        {

                            SoarVertex matchingVertex =
                                dataMap.getMatchingParent((SoarVertex)o);
                            if(matchingVertex != null) 
                            {
                                currentSV = matchingVertex;
                            }
                            else 
                            {
                                java.util.List matchingVertices =
                                    dataMap.getParents((SoarVertex)o);
                                Iterator z = matchingVertices.iterator();
                                String parentName = "";
                                SoarVertex parentVertex = null;

                                while(z.hasNext()) 
                                {
                                    Object oz = z.next();
                                    if(oz instanceof SoarIdentifierVertex) 
                                    {
                                        parentVertex = (SoarVertex)oz;
                                        // Get the name of the vertex to create
                                        Enumeration parentEdges =
                                            dataMap.emanatingEdges(parentVertex);
                                        while(parentEdges.hasMoreElements()) 
                                        {
                                            NamedEdge parentEdge =
                                                (NamedEdge)parentEdges.nextElement();
                                            if(parentEdge.V1().equals((SoarVertex)o))
                                            parentName = parentEdge.getName();
                                        }
                                    }
                                }//while finding all the possible parent vertices

                                if(parentName != "") 
                                {
                                    SoarVertex v1 = dataMap.createNewSoarId();
                                    dataMap.addTriple(parentVertex,
                                                      parentName,
                                                      v1,
                                                      true,
                                                      current,
                                                      currentTriple.getLine());
                                    meh.generatedIdentifier(currentTriple,
                                                            parentName);
                                    currentSV = v1;
                                }
                            }     // end of else create a new SoarIdentifierVertex
                        }
                    }   // end of if parent vertex wasn't a SoarIdentifierVertex
                    else 
                    {
                        currentSV = (SoarVertex)o;
                    }


                    /*  If attribute is 'name'
                     *    1.)  See if name is in reference to the name of a operator.
                     *    2.)  See if that name and value already exists.
                     *    3.)  See if there is an empty operator folder that can be used.
                     *    4.)  If operator, name doesn't already exist and no empty folders,
                     *          create a new operator identifier folder for the name attribute
                     *          to be put.
                     */
                    if( currentTriple.getAttribute().getString().equals("name") && currentSV != null && (dataMap.getTopstate().getValue() != currentSV.getValue()) ) 
                    {
                        // create new identifier 'operator' at higher level

                        // Get the parent vertices
                        java.util.List parentVertices =
                            dataMap.getParents((SoarVertex)o);
                        Iterator x = parentVertices.iterator();
                        SoarVertex parentVertex = null;
                        while(x.hasNext()) 
                        {
                            Object ox = x.next();
                            if(ox instanceof SoarIdentifierVertex) 
                            {
                                parentVertex = (SoarVertex)ox;
                            }
                        }
                        
                        // make sure found the parent
                        if(parentVertex == null)
                        {    
                            continue;
                        }

                        //Keeps track of if the name attribute is attached to an
                        //operator identifier
                        boolean operatorType = false;
                        
                        //Check to see if name and value already exist somewhere
                        boolean alreadyHere = false;
                        
                        //Keep track of if there are any operator folders with
                        //no name identifier attached
                        boolean emptyFolder = false;

                        // Check to see if this name attribute is part of an
                        // operator
                        Enumeration fatherEdges = dataMap.emanatingEdges(parentVertex);
                        while(fatherEdges.hasMoreElements() && !operatorType) 
                        {
                            NamedEdge edge = (NamedEdge)fatherEdges.nextElement();

                            if((edge.V1().getValue() == currentSV.getValue()) && edge.getName().equals("operator")) 
                            {
                                operatorType = true;
                            }
                        }     // end of looking to see if the name attribute's parent is an operator identifier


                        // If it is an operator type,  might need to create new operator identifier
                        if(operatorType) 
                        {
                            // Check to see if an identifier 'operator' with same name exists already
                            Enumeration parentEdges = dataMap.emanatingEdges(parentVertex);
                            while(parentEdges.hasMoreElements() && !alreadyHere) 
                            {

                                NamedEdge edge = (NamedEdge)parentEdges.nextElement();
                                if(edge.getName().equals("operator")) 
                                {
                                    Enumeration operatorEdges = dataMap.emanatingEdges(edge.V1());
                                    emptyFolder = true;    // keeps track if there is already operator identifier with no name attached

                                    while(operatorEdges.hasMoreElements() && !alreadyHere) 
                                    {
                                        NamedEdge operatorEdge = (NamedEdge)operatorEdges.nextElement();
                                        if(operatorEdge.getName().equals("name")) 
                                        {
                                            emptyFolder = false;
                                            if(operatorEdge.V1() instanceof EnumerationVertex) 
                                            {
                                                EnumerationVertex nameVertex = (EnumerationVertex)operatorEdge.V1();
                                                Iterator i = nameVertex.getEnumeration();
                                                while(i.hasNext() && !alreadyHere) 
                                                {
                                                    String ns = (String)i.next();
                                                    if(ns.equals(currentTriple.getValue().getString())) 
                                                    {
                                                        alreadyHere = true;
                                                        currentSV = null;
                                                    }
                                                }
                                            }     // end of if EnumerationVertex found
                                        }     // end of if found edge called 'name'
                                    }
                                    // If empty operator folder found, use it
                                    if(emptyFolder)
                                    currentSV = (SoarVertex)edge.V1();
                                }   // end of checking operator identifier children for identical name
                            }   // while checking parent edges for soarIdentifierVertexs 'operator'




                            /* create new identifier 'operator' at higher level if not already one
                             * with same name or an operator identifier without a name
                             */
                            if(parentVertex != null && !alreadyHere && !emptyFolder) 
                            {
                                // Create a new SoarIdentifierVertex with name 'operator'
                                SoarVertex v1 = dataMap.createNewSoarId();
                                dataMap.addTriple(parentVertex, "operator", v1, true, current, currentTriple.getLine());
                                meh.generatedIdentifier(currentTriple, "operator");
                                // Set new 'operator' soar identifier as the parent vertex
                                currentSV = v1;
                            }
                        }   // end of if attribute name is part of an Operator
                    }     // end of if Attribute == 'name'

                    if(currentSV == null) {   // Couldn't find the parent, give up
                        continue;
                    }


                    // Case UNKNOWN VERTEX   [use when 'value' is a variable or unknown at this time]
                    // Possibly create as SoarIdentifier, then change later if needed
                    if( TripleUtils.isVariable(currentTriple.getValue().getString()) )  
                    {
                        // First check to see if somehow this was already created
                        boolean alreadyThere = false;
                        Enumeration dEdges = dataMap.emanatingEdges(currentSV);
                        while(dEdges.hasMoreElements()) 
                        {
                            NamedEdge dEdge = (NamedEdge)dEdges.nextElement();
                            if(dEdge.getName().equals(currentTriple.getAttribute().getString()) && (dEdge.V1() instanceof SoarIdentifierVertex))
                            alreadyThere = true;
                        }
                        if(!alreadyThere) 
                        {
                            SoarVertex v1 = dataMap.createNewSoarId();
                            dataMap.addTriple(currentSV, currentTriple.getAttribute().getString(), v1, true, current, currentTriple.getLine());
                            meh.generatedIdentifier(currentTriple, currentTriple.getAttribute().getString());
                        }
                    }

                    // INTEGER VERTEX
                    else if(TripleUtils.isInteger(currentTriple.getValue().getString() )) 
                    {
                        SoarVertex v1 = dataMap.createNewInteger();
                        dataMap.addTriple(currentSV, currentTriple.getAttribute().getString(), v1, true, current, currentTriple.getLine());
                        meh.generatedInteger(currentTriple, currentTriple.getAttribute().getString());
                    }
                    // FLOAT VERTEX
                    else if(TripleUtils.isFloat(currentTriple.getValue().getString())) 
                    {
                        SoarVertex v1 = dataMap.createNewFloat();
                        dataMap.addTriple(currentSV, currentTriple.getAttribute().getString(), v1, true, current, currentTriple.getLine());
                        meh.generatedFloat(currentTriple, currentTriple.getAttribute().getString());
                    }
                    // Case ENUMERATED VERTEX
                    else 
                    {
                        // Get all the edges from the start
                        // ATTRIBUTE Search for the matching attribute
                        Enumeration edges = dataMap.emanatingEdges(currentSV);
                        while(edges.hasMoreElements() && notFound) 
                        {
                            NamedEdge currentEdge = (NamedEdge)edges.nextElement();
                            if(currentEdge.getName().equals(currentTriple.getAttribute().getString())) 
                            {
                                notFound = false;
                                attributeEdge = currentEdge;  // remember this edge
                            }
                        }    // end of while going through edges of currentSV
                        // If attribute is not found, add it
                        if(notFound) 
                        {
                            Vector v1Vector = new Vector();
                            v1Vector.add(currentTriple.getValue().getString());
                            SoarVertex v1 = dataMap.createNewEnumeration(v1Vector);
                            dataMap.addTriple(currentSV, currentTriple.getAttribute().getString(), v1, true, current, currentTriple.getLine());
                            meh.generatedEnumeration(currentTriple, currentTriple.getAttribute().getString());
                        } // end of if not found

                        // VALUE Attribute was already there.  Check for value on attribute, if not there, add it
                        else 
                        {
                            // if attributeEdge is already a SoarIdentifierVertex, remove it and make it an EnumerationVertex
                            if(attributeEdge.V1() instanceof SoarIdentifierVertex) 
                            {
                                dataMap.removeTriple((SoarVertex)attributeEdge.V0(), currentTriple.getAttribute().getString(), (SoarVertex)attributeEdge.V1());
                                Vector v1Vector = new Vector();
                                v1Vector.add(currentTriple.getValue().getString());
                                SoarVertex v1 = dataMap.createNewEnumeration(v1Vector);
                                dataMap.addTriple(currentSV, currentTriple.getAttribute().getString(), v1, true, current, currentTriple.getLine());
                                meh.generatedAddToEnumeration(currentTriple, currentTriple.getAttribute().getString(), currentTriple.getValue().getString());
                            }
                            // AttributeEdge is already an EnumerationVertex, now see if value is there
                            else if(attributeEdge.V1() instanceof EnumerationVertex) 
                            {
                                boolean valueFound = false;
                                Vector edgeStrings = new Vector();
                                SoarVertex attributeVertex = (SoarVertex)attributeEdge.V1();
                                Enumeration attributeEdges = dataMap.emanatingEdges(attributeVertex);
                                while(attributeEdges.hasMoreElements()) 
                                {
                                    NamedEdge currentEdge = (NamedEdge)attributeEdges.nextElement();
                                    edgeStrings.add(currentEdge.getName());
                                    if( currentEdge.getName().equals(currentTriple.getValue().getString())) 
                                    {
                                        valueFound = true;
                                    }
                                } // end of looking through the enumerated edges of the attribute

                                if(!valueFound) 
                                {
                                    edgeStrings.add(currentTriple.getValue().getString()); // add value to string list
                                    EnumerationVertex enumV = (EnumerationVertex) attributeEdge.V1();
                                    enumV.add(currentTriple.getValue().getString());
                                    dataMap.removeTriple((SoarVertex)attributeEdge.V0(), attributeEdge.getName(), (SoarVertex)attributeEdge.V1());
                                    dataMap.addTriple((SoarVertex)attributeEdge.V0(), attributeEdge.getName(), enumV, true, current, currentTriple.getLine());
                                    meh.generatedAddToEnumeration(currentTriple, currentTriple.getAttribute().getString(), currentTriple.getValue().getString());
                                }    // end of if value not found on enumeration, then add it
                            } // end of adding value to attribute
                            // AttributeEdge is neither an EnumerationVertex or a SoarIdentifier, don't even try to handle this
                            else 
                            {
                                return;
                            }
                        }   // end of attribute already there, make sure value is there

                    }  // end of create enumerated vertex
                }
            }    // end of if addConstraint() fails
        }   // while going through the elements

    } // end of complete()



    private static boolean addConstraint(SoarWorkingMemoryModel dataMap,
                                         Triple triple,
                                         Map match) 
    {
        Set varSet = (Set)match.get(triple.getVariable().getString());
        boolean matched = false;
        // for every possible start
        Enumeration e = new EnumerationIteratorWrapper(varSet.iterator());
        while(e.hasMoreElements()) 
        {
            Object o = e.nextElement();

            // In case they try to use a attribute variable as 
            // soar identifier
            if(!(o instanceof SoarVertex))
            {
                continue;
            }
            SoarVertex currentSV = (SoarVertex)o;

            // Get all the edges from the start
            Enumeration edges = dataMap.emanatingEdges(currentSV);
            while(edges.hasMoreElements()) 
            {
                NamedEdge currentEdge = (NamedEdge)edges.nextElement();
                if (currentEdge.satisfies(triple)) 
                {
                    // Used for the Datamap Searches for untested/uncreated elements
                    if(triple.isCondition())
                    currentEdge.tested();
                    else
                    currentEdge.created();

                    if (!matched)
                    matched = true;
                    if (TripleUtils.isVariable(triple.getAttribute().getString())) 
                    {
                        Set attrSet = (Set)match.get(triple.getAttribute().getString());
                        attrSet.add(currentEdge.getName());
                    }   
                    if (TripleUtils.isVariable(triple.getValue().getString())) 
                    {
                        Set valSet = (Set)match.get(triple.getValue().getString());
                        valSet.add(currentEdge.V1());
                    }
                }
            }
        }
        return matched;
    }



    private static boolean addConstraintLog(SoarWorkingMemoryModel dataMap,Triple triple, Map match, FileWriter log) 
    {
        Set varSet = (Set)match.get(triple.getVariable().getString());
        boolean matched = false;
        // for every possible start
        Enumeration e = new EnumerationIteratorWrapper(varSet.iterator());
        while(e.hasMoreElements()) 
        {
            Object o = e.nextElement();

            // In case they try to use a attribute variable as 
            // soar identifier
            if(!(o instanceof SoarVertex))
            continue;
            SoarVertex currentSV = (SoarVertex)o;
            
            // Get all the edges from the start
            Enumeration edges = dataMap.emanatingEdges(currentSV);
            while(edges.hasMoreElements()) 
            {
                NamedEdge currentEdge = (NamedEdge)edges.nextElement();
                if (currentEdge.satisfies(triple)) 
                {

                    if(triple.isCondition()) 
                    {
                        currentEdge.tested();
                        try 
                        {
                            log.write("edge:  " + currentEdge.getName() + "  was marked as tested! ");
                            log.write('\n');
                        }
                        catch(IOException ioe) 
                        {
                            ioe.printStackTrace();
                        }
                    }
                    else 
                    {
                        currentEdge.created();
                        try 
                        {
                            log.write("edge:  " + currentEdge.getName() + "  was marked as created! ");
                            log.write('\n');
                        }
                        catch(IOException ioe) 
                        {
                            ioe.printStackTrace();
                        }
                    }

                    if (!matched)
                    matched = true;
                    if (TripleUtils.isVariable(triple.getAttribute().getString())) 
                    {
                        Set attrSet = (Set)match.get(triple.getAttribute().getString());
                        attrSet.add(currentEdge.getName());
                    }   
                    if (TripleUtils.isVariable(triple.getValue().getString())) 
                    {
                        Set valSet = (Set)match.get(triple.getValue().getString());
                        valSet.add(currentEdge.V1());
                    }
                }
            }
        }
        return matched;
    }

    /*
     *  Creates a SoarIdentifier Vertex with the same name at the same
     *  place in the datamap.  If sv is already a SoarIdentifierVertex
     *  or a new SoarIdentifierVertex cannot be created, returns null.
     *  Otherwise, function returns created vertex
     *  This function is called during datamap generation when a SoarIdentifier
     *  Vertex is requested, but there already exists a non-Identifier SoarVertex
     *  at that part of the datamap with the same name.
     */
    private static SoarVertex createParentIdentifier(SoarVertex sv) 
    {
        return null;
    }


    /*
     * This function scans a list of triples for all the triples whose
     * variable (id) is a state variable.  All such triples are
     * returned in a second vector.
     *
     * @param te the triples from the production to match
     * 
     * @see pmpHelper
     */
    private static Vector findStateTriples(TriplesExtractor te)
    {
        Iterator iterTriples = te.triples();
        Vector vecStateTriples = new Vector();
        Triple trip = null;

        //Find the ones that have state
        while(iterTriples.hasNext())
        {
            trip = (Triple)iterTriples.next();
                
            if (trip.hasState())
            {
                vecStateTriples.add(trip);
            }
        }//while

        //Find the ones that that use a state variable as their id
        iterTriples = te.triples();
        while(iterTriples.hasNext())
        {
            trip = (Triple)iterTriples.next();
            if (vecStateTriples.contains(trip)) continue;
            
            Enumeration enumStateTriples = vecStateTriples.elements();
            while(enumStateTriples.hasMoreElements())
            {
                Triple trip2 = (Triple)enumStateTriples.nextElement();
                String s1 = trip.getVariable().getString();
                String s2 = trip2.getVariable().getString();

                if (s1.equals(s2))
                {
                    vecStateTriples.add(trip);
                    break;
                }
            }//while
        }//while

        return vecStateTriples;
        
    }//findStateTriples
    
    /*
     * This *recursive* helper function is used to actually find a path
     * match for pathMatchesProduction()
     *
     * @param vecEdges vector of all the edges to be matched in
     *                  the order that they are to be matched.
     * @param nEdgePos starting position in vecEdges (the original caller
     *             should pass in zero)
     * @param te the triples from the production to match
     * @param vecUsedTriples the triples that have already been
     *                       matched (and cannot be reused)
     * @param vecStateTriples the triples that are headed by a state
     *                        variable.  If this parameter is null
     *                        them pmphelper generates it automatically.
     * @param id the id of any triple that matches the current named
     *           edge must equals() this one.  If this is null then
     *           the id must be a state variable.
     * @param vecMatches return value for pathMatchesProduction (see below)
     * 
     * @see pathMatchesProduction
     */
    private static void pmpHelper(Vector vecEdges,
                                  int nEdgePos,
                                  TriplesExtractor te,
                                  Vector vecUsedTriples,
                                  Vector vecStateTriples,
                                  Pair id,
                                  Vector vecMatches)
    {
        //Trivial case  %%%Is this needed?
        if (vecEdges.size() <= nEdgePos)
        {
            return;
        }
        
        Iterator iterTriples = null;
        Triple trip = null;
        
        //Find all the triples headed by a state variable
        if (vecStateTriples == null)
        {
            vecStateTriples = findStateTriples(te);
        }//if
        
        iterTriples = te.triples();
        NamedEdge ne = (NamedEdge)vecEdges.get(nEdgePos);
        while(iterTriples.hasNext())
        {
            trip = (Triple)iterTriples.next();
            if ( (ne.satisfies(trip))
                 && (!vecUsedTriples.contains(trip)) )
            {
                if ( ((id == null) && (vecStateTriples.contains(trip)))
                     || ((id != null) && (id.equals(trip.getVariable()))) )
                {
                    if (vecEdges.size() == nEdgePos + 1)
                    {
                        if (!vecMatches.contains(trip))
                        {
                            vecMatches.add(trip);
                        }
                    }
                    else
                    {
                        vecUsedTriples.add(trip);
                        pmpHelper(vecEdges,
                                  nEdgePos + 1,
                                  te,
                                  vecUsedTriples,
                                  vecStateTriples,
                                  trip.getValue(),
                                  vecMatches);
                        vecUsedTriples.remove(trip);
                    }//else
                }//if
            }//if
        }//while
        
    }//pmpHelper
                                  
                                  
    
    /*
     *  Determines whether a given datamap TreePath matches a given
     *  SoarProduction.  This function returns a vector of all the the
     *  Triples that match the last node in the path for each complete
     *  match that is found.  For example, if the path is:
     *          "<s> ^foo.bar.baz.qux <q>"
     *  then this function will return all the Triples in SoarProduction
     *  that match "<0> ^qux <q>" *and* are part of unique set of
     *  Triples that matches the entire path.  Note that this means
     *  that this function can be fooled in unusual circumstances.  If a
     *  production looks like the following then it will generate a false match
     *  because all the WMEs are present and they are in the right order:
     *      sp {tricky
     *         (state <s> ^foo.bar <s>
     *                    ^baz.qux <qux>)
     *      -->
     *         etc...
     *      
     *  If the production does not satisfy the path then the vector returned
     *  will be empty.
     *
     * @param thePath the path to match
     * @param SoarProduction the production to match
     *
     * @see pmphelper
     */
    public static Vector pathMatchesProduction(TreePath thePath,
                                               SoarProduction sp)
    {
        
        Vector vecEdges = new Vector();
        for (int i = 0; i < thePath.getPathCount(); i++) 
        {
            FakeTreeNode ftn = (FakeTreeNode)thePath.getPathComponent(i);
            NamedEdge ne = ftn.getEdge();
            if (ne != null)
            {
                vecEdges.add(ne);
            }
        }//for

        TriplesExtractor te = new TriplesExtractor(sp);
        Vector vecMatches = new Vector();
        
        pmpHelper(vecEdges, 0, te, new Vector(), null, null, vecMatches);

        return vecMatches;
        
    }//pathMatchesProduction

    
}//class DataMapMatcher

