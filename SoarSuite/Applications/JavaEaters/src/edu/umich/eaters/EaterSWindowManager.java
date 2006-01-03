package edu.umich.eaters;

import java.util.ArrayList;
import java.util.Iterator;

import org.eclipse.swt.widgets.Display;

import edu.umich.JavaBaseEnvironment.AgentWindow;
import edu.umich.JavaBaseEnvironment.SWindowManager;
import edu.umich.JavaBaseEnvironment.SimulationControl;
import edu.umich.JavaBaseEnvironment.SoarAgent;

/**
 * 
 * @author stokesd
 *
 * See the superclass for comments on the purpose of this class
 * Stokesd just threw this together to get rid of superclass's dependency on
 * classes in child packages
 */

public class EaterSWindowManager extends SWindowManager
{
  public EaterSWindowManager(SimulationControl sc, boolean agentWindowsOn, boolean mapWindowOn, boolean controlPanelOn, boolean disablePopups)
  {
    super(sc, agentWindowsOn, mapWindowOn, controlPanelOn, disablePopups, "Eaters", "Eaters", "seater", "Seater files (*.seater)");
  }

  /**
   * Opens the visible map of the simulation.
   */
  public void openMap()
  {
    if(myVisualMap == null)
    {
      if(mySC instanceof EaterControl)
      {
        myVisualMap = new VisMap(mySC, myDisplay);
        myVisualMap.open();
      }
    } 
    else
      myVisualMap.open();
  }
  
  public void openAgentView(SoarAgent agent)
  {
    if(!agentWindowsOn) return;
    if(agentWindows == null) agentWindows = new ArrayList();
    Iterator iter = agentWindows.iterator();
    AgentWindow aw = null;
    SoarAgent curr = null;
    
    while(iter.hasNext())
    {
      aw = (AgentWindow)iter.next();
      curr = aw.getAgent();
      if(curr.getName().equals(agent.getName()))
        break;
      else
        curr = null;
    }
    if(curr == null)
    {
      if(mySC instanceof EaterControl)
      {
        aw = new EaterWindow(myDisplay, (EaterControl)mySC, (Eater)agent);
        agentWindows.add(aw);
      }
    } 
    else
      aw.open();
  }

  public void createHumanAgent(String colorName)
  {
    if(myDisplay == null) myDisplay = new Display();
    SoarAgent sa = mySC.createHumanAgent(colorName);

    if(sa != null && sa instanceof HumanEater)
    {
      ((HumanEater)sa).initControls(myDisplay);
      openAgentView(sa);
    }
  }
}