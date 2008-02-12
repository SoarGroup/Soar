package testsharedids;

/********************************************************************************************
*
* Application.java
* 
* Description:	
* 
* Created on 	Feb 8, 2008
* @author 		Jonathan Voigt
********************************************************************************************/

import sml.* ;

/************************************************************************
 * 
 * Test app for shared ID weirdness
 * 
 ************************************************************************/
public class Application implements Kernel.UpdateEventInterface, Agent.PrintEventInterface
{
	private Kernel kernel ;
	private Agent agent;
	
	private Identifier westCell;
	private Identifier centerCell;
	private Identifier eastCell;

	private Identifier target;
	private Identifier targetCell;

	private void Test()
	{
		kernel = Kernel.CreateKernelInNewThread("SoarKernelSML");
		
		// Make sure the kernel was ok
		if (kernel.HadError())
			throw new IllegalStateException("Error initializing kernel: " + kernel.GetLastErrorDescription()) ;
		
		String version = kernel.GetSoarKernelVersion() ;
		System.out.println("Soar version " + version) ;
		
		// Create an agent
		agent = kernel.CreateAgent("javatest") ;

		if (kernel.HadError())
			throw new IllegalStateException("Error creating agent: " + kernel.GetLastErrorDescription()) ;
		
		agent.ExecuteCommandLine("watch --wmes");
		
		// This is what most environments do
		kernel.SetAutoCommit(false);
		
		kernel.RegisterForUpdateEvent(smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, this, null);
		agent.RegisterForPrintEvent(smlPrintEventId.smlEVENT_PRINT, this, null);
		
		Identifier inputLink = agent.GetInputLink();

		if (inputLink == null)
			throw new IllegalStateException("Error getting the input link") ;
		
		Identifier map = agent.CreateIdWME(inputLink, "map");
		centerCell = agent.CreateIdWME(map, "cell");
		eastCell = agent.CreateIdWME(map, "east");
		westCell = agent.CreateIdWME(map, "west");
		
		target = agent.CreateIdWME(inputLink, "target");
		targetCell = agent.CreateSharedIdWME(target, "cell", centerCell);
		if (targetCell == null)
			throw new IllegalStateException("Error creating cell shared ID from target");
		
		agent.Commit();
		
		System.out.println("Agent created, input link initialized.");
		
		updateCount = 0;
		kernel.RunAllAgents(9);
		
		System.out.println("----------------");
		System.out.println("final:");
		System.out.println(agent.ExecuteCommandLine("print --depth 10 --internal i2"));
		System.out.println("----------------");
		
		kernel.Shutdown();
		kernel.delete();
		kernel = null;
		agent = null;
	}
	
	private int updateCount = 0;
	public void updateEventHandler(int eventID, Object data, Kernel kernel, int runFlags) {
		assert eventID == smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES.swigValue();
		assert data == null;
		assert this.kernel == kernel;
		
		System.out.println("----------------");
		System.out.println("update: " + ++updateCount);
		System.out.println(agent.ExecuteCommandLine("print --depth 10 --internal i2"));
		
		// Move the target
		switch (updateCount)
		{
		case 1:
			System.out.println("update: moving east");
			agent.DestroyWME(targetCell);
			agent.Commit();
			targetCell = agent.CreateSharedIdWME(target, "cell", eastCell);
			break;
		case 2:
			System.out.println("update: moving center");
			agent.DestroyWME(targetCell);
			agent.Commit();
			targetCell = agent.CreateSharedIdWME(target, "cell", centerCell);
			break;
		case 3:
			System.out.println("update: moving west");
			agent.DestroyWME(targetCell);
			agent.Commit();
			targetCell = agent.CreateSharedIdWME(target, "cell", westCell);
			break;
		case 4:
			System.out.println("update: moving center");
			agent.DestroyWME(targetCell);
			agent.Commit();
			targetCell = agent.CreateSharedIdWME(target, "cell", centerCell);
			break;
		case 5:
			System.out.println("update: moving east");
			agent.DestroyWME(targetCell);
			agent.Commit();
			targetCell = agent.CreateSharedIdWME(target, "cell", eastCell);
			break;
		case 6:
			System.out.println("update: moving center");
			agent.DestroyWME(targetCell);
			agent.Commit();
			targetCell = agent.CreateSharedIdWME(target, "cell", centerCell);
			break;
		case 7:
			System.out.println("update: moving west");
			agent.DestroyWME(targetCell);
			agent.Commit();
			targetCell = agent.CreateSharedIdWME(target, "cell", westCell);
			break;
		case 8:
			System.out.println("update: moving center");
			agent.DestroyWME(targetCell);
			agent.Commit();
			targetCell = agent.CreateSharedIdWME(target, "cell", centerCell);
			break;
		}
		
		System.out.println("----------------");
		agent.Commit();
	}

	public void printEventHandler(int eventID, Object data, Agent agent, String message) {
		assert eventID == smlPrintEventId.smlEVENT_PRINT.swigValue();
		assert data == null;
		assert this.agent == agent;

		System.out.println(message);
	}

	public Application()
	{
		boolean success = true ;
		String  msg = "" ;
		
		try
		{
			Test() ;
			
			msg = "Test succeeded";
		}
		catch (Throwable t)
		{
			success = false ;
			msg = t.toString() ;
		}
		finally
		{
			if (success)
				System.out.println(msg);
			else
				System.err.println(msg);
			System.exit(success ? 0 : 1);
		}
	}
	
	public static void main(String[] args)
	{
		new Application();
	}
}
