package edu.umich.tanksoar;

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

public class TankSWindowManager extends SWindowManager
{
  public TankSWindowManager(SimulationControl sc, boolean agentWindowsOn, boolean mapWindowOn)
  {
     super(sc, agentWindowsOn, mapWindowOn, true, false, "TankSoar", "TankSoar", "stank", "Stank files (*.stank)");
  }

  /**
   * Opens the visible map of the simulation.
   */
  public void openMap()
  {
	 if (myVisualMap == null || !((VisTankMap)myVisualMap).isMapOpen())
	 {
		 myVisualMap = new edu.umich.tanksoar.VisTankMap(myDisplay, mySC);
		 myVisualMap.open();
	 }
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
      if(mySC instanceof TankSoarJControl)
      {
        aw = new TankView(myDisplay, (Tank)agent);
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

    if(sa != null && sa instanceof HumanTankControl)
    {
      ((HumanTankControl)sa).initControls(myDisplay);
      openAgentView(sa);
    }
  }
}
