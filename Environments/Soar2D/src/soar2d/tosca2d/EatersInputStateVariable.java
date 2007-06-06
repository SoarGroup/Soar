/**
 * 
 */
package soar2d.tosca2d;

/**
 * @author Doug
 *
 */

import java.awt.Point;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;

import soar2d.*;
import soar2d.player.ToscaEater;
import soar2d.world.CellObject;
import tosca.Boolean;
import tosca.Double;
import tosca.Group;
import tosca.Integer;
import tosca.RefValue;
import tosca.Value;
import tosca.Vector;

public class EatersInputStateVariable extends JavaStateVariable {
	protected Value m_Value = new Value() ;
	
	private static final int kInfoBoxID = 0;

	public EatersInputStateVariable(String name) {
		super(name) ;
	}
	
	public void Initialize() {
		// For the moment, initialize as an empty group
		Group init = new Group() ;
        Boolean b = new Boolean(false);
        init.AddNamedValue("_initialized", b);
		GetCurrentValue().TakeGroup(init) ;
	}

	private RefValue createMapCell(ToscaEater eater, World world, Point viewLocation, Point location)
	{
		int maxProperties = 20 ;
		Vector mapCell = new Vector(maxProperties) ;
		
		mapCell.Set(0,0.0) ;	// Property 0 is whether this location is valid or not
		
		if (world.getMap().isInBounds(viewLocation)) {
			mapCell.Set(0, 1.0) ;	// Property 0 is whether this location is valid or not
			
			if (!world.getMap().enterable(viewLocation)) {
				mapCell.Set(1, 1.0) ;	// Property 1 is whether it's a wall
			} else {
				// Property 3: is there a box in the cell
				// box test
				// get all boxes
				ArrayList<CellObject> boxes = world.getMap().getAllWithProperty(viewLocation, Names.kPropertyBox);
				// max one box per cell, we're not prepared to handle more
				assert boxes.size() <= 1;
				if (boxes.size() > 0) {
					// there is a box
					mapCell.Set(3, 1.0);

					// Property 4: box id
					// box id 0 is the info box (kInfoBoxID)
					// box id 1..<number of boxes> are reward boxes

					// id the box
					CellObject box = boxes.get(0);
					assert box.hasProperty(Names.kPropertyBoxID);
					int boxID = box.getIntProperty(Names.kPropertyBoxID);
					
					// box should never be negative
					assert boxID >= 0;
					mapCell.Set(4, boxID);

					// Property 5 is the symbol of the box (the target box) from the info box
					// Property 6 is the action (open-code)
					// Both of these only set if they exist

					// if we're on the info box
					if (boxID == kInfoBoxID) {
						// if the target box property exists
						if (box.hasProperty(Names.kPropertyPositiveBoxID)) {
							// set property 5 to the target
							mapCell.Set(5, box.getIntProperty(Names.kPropertyPositiveBoxID));
						} else {
							mapCell.Set(5, 0);
						}
						
						// if the action (open) code exists
						if (box.hasProperty(Names.kPropertyOpenCode)) {
							// set property 6 to the code
							mapCell.Set(6, box.getIntProperty(Names.kPropertyOpenCode));
						} else {
							mapCell.Set(6, 0);
						}
					}

				} else {
					// there is no box
					mapCell.Set(3, 0);
					mapCell.Set(4, 0);
					mapCell.Set(5, 0);
					mapCell.Set(6, 0);
				}
			}
		}
		
		return mapCell ;
	}
	
	// See comments where recentMapReset is tested.
	double cachedReward = 0;
	boolean recentMapReset = false;
	public void mapReset() {
		recentMapReset = true;
	}
	
	public void update(int time, soar2d.player.ToscaEater eater, World world, java.awt.Point location) {
		// The value is stored as a group containing some named values and
		// then a map group which contains all of the cells around this eater
		Group main = new Group() ;
		
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////// begin NEW state variable format
        Boolean b = new Boolean(true);
        main.AddNamedValue("_initialized", b);
        
        // The reward must appear on the same cycle as the map reset.
		// Unfortunately, the reward (points delta) gets reset right after
		// it is read. Therefore, we must cache the reward value and detect
		// when to repost the value on the input link.
		// recentMapReset is set to true when this case happens (the map is reset
		// and we should save the reward value)
		double reward = -1.0;
		if (recentMapReset) {
			reward = cachedReward;
			recentMapReset = false;
		} else if (eater.getEater().pointsChanged()) {
			reward = eater.getEater().getPointsDelta();
		}
		cachedReward = reward;
		main.AddNamedValue("ExternalReward", new tosca.Double(reward));
		
		main.AddNamedValue("world-count", new tosca.Integer(world.getWorldCount()));
		
		Integer possibleActions = new Integer(8);
		main.AddNamedValue("PossibleActions", possibleActions);
		
		Vector stateRep = new Vector(2);
		stateRep.Set(0, location.x);
		stateRep.Set(1, location.y);
		main.AddNamedValue("StateRepresentation", stateRep);
        
        Vector ltm = new Vector();
		Vector relevantActions = new Vector(6);
		relevantActions.Set(0, 1.0);
		relevantActions.Set(1, 2.0);
		relevantActions.Set(2, 3.0);
		relevantActions.Set(3, 4.0);
		relevantActions.Set(4, -1.0);
		relevantActions.Set(5, -1.0);
		java.awt.Point myLocation = new java.awt.Point();
		myLocation.x = location.x;
		myLocation.y = location.y;
		ArrayList<CellObject> boxes = world.getMap().getAllWithProperty(myLocation, Names.kPropertyBox);
		if (boxes.size() > 0)
		{
			CellObject box = boxes.get(0);
			assert box.hasProperty(Names.kPropertyBoxID);
			int boxID = box.getIntProperty(Names.kPropertyBoxID);
			if (boxID == kInfoBoxID)
            {
				relevantActions.Set(4, 5.0);
                if (box.hasProperty(Names.kPropertyPositiveBoxID))
                {
                    ltm.Resize(2);
                    ltm.Set(0, 0);
                    ltm.Set(1, box.getIntProperty(Names.kPropertyPositiveBoxID));
                    if (box.hasProperty(Names.kPropertyOpenCode))
                    {
                        ltm.Resize(4);
                        ltm.Set(2, 1);
                        ltm.Set(3, box.getIntProperty(Names.kPropertyOpenCode));
                    }            
                }
            }
			else if (box.hasProperty(Names.kPropertyOpenCode))
			{
				relevantActions.Set(4, 6.0);
				relevantActions.Set(5, 7.0);
			}
			else
				relevantActions.Set(4, 5.0);
		}
		main.AddNamedValue("RelevantActions", relevantActions);
        main.AddNamedValue("LongTermMemories", ltm);
		
//////////end NEW state variable format		
////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////

		
		main.AddNamedValue("x", new tosca.Integer(location.x)) ;
		main.AddNamedValue("y", new tosca.Integer(location.y)) ;
		main.AddNamedValue("facing", new tosca.Integer(eater.getEater().getFacingInt())) ;
		
		Group map = new Group() ;
		
		java.awt.Point viewLocation = new java.awt.Point();
		int visionRange = Soar2D.config.getEaterVision() ;
		for (int x = location.x - visionRange; x <= location.x + visionRange; ++x) {
			for (int y = location.y - visionRange; y <= location.y + visionRange; ++y) {
				viewLocation.x = x ;
				viewLocation.y = y ;

				RefValue mapCell = createMapCell(eater, world, viewLocation, location) ;				
				
				// We'll name the map entries "cell00" to "cell55" so we can pull them out by name if we wish.
				int indexX = x - (location.x - visionRange) ;
				int indexY = y - (location.y - visionRange) ;
				
				String name = "cell" + indexX + indexY ;
				map.AddNamedValue(name, mapCell) ;
			}
		}
		main.AddNamedValue("map", map) ;
        
		Value value = new Value(main) ;
		SetValue(value, time) ;
	}
	
	protected Value GetCurrentValue() { return m_Value ; }
}
