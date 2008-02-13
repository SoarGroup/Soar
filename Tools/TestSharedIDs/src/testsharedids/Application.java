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
	
	private void Test2()
	{
		System.out.println("--- Initialization");
		kernel = Kernel.CreateKernelInNewThread("SoarKernelSML");
		
		// Make sure the kernel was ok
		if (kernel.HadError())
			throw new IllegalStateException("Error initializing kernel: " + kernel.GetLastErrorDescription()) ;
		
		String version = kernel.GetSoarKernelVersion() ;
		System.out.println("--- Kernel created, version " + version) ;
		
		// Create an agent
		agent = kernel.CreateAgent("javatest") ;

		if (kernel.HadError())
			throw new IllegalStateException("Error creating agent: " + kernel.GetLastErrorDescription()) ;
		
		agent.ExecuteCommandLine("watch --wmes");

		agent.RegisterForPrintEvent(smlPrintEventId.smlEVENT_PRINT, this, null);
		
		Identifier inputLink = agent.GetInputLink();

		if (inputLink == null)
			throw new IllegalStateException("Error getting the input link") ;
		
		System.out.println("--- initial creation");

		Identifier test2map = agent.CreateIdWME(inputLink, "map");
		Identifier test2Target = agent.CreateIdWME(inputLink, "target");
		Identifier test2Cell1 = agent.CreateIdWME(test2map, "cell");
		Identifier test2cell2 = agent.CreateSharedIdWME(test2Target, "cell", test2Cell1);
		
		System.out.println("--- deletion");
		agent.DestroyWME(test2cell2);
		
		System.out.println("--- recreation");
		test2cell2 = agent.CreateSharedIdWME(test2Target, "cell", test2Cell1);
		
		System.out.println("--- step");
		kernel.RunAllAgents(1);

		System.out.println("--- shutting down");
		kernel.Shutdown();
		kernel.delete();
		kernel = null;
		agent = null;
	}
	
	private void Test()
	{
		System.out.println("--- Initialization");
		kernel = Kernel.CreateKernelInNewThread("SoarKernelSML");
		
		// Make sure the kernel was ok
		if (kernel.HadError())
			throw new IllegalStateException("Error initializing kernel: " + kernel.GetLastErrorDescription()) ;
		
		String version = kernel.GetSoarKernelVersion() ;
		System.out.println("--- Kernel created, version " + version) ;
		
		// Create an agent
		agent = kernel.CreateAgent("javatest") ;

		if (kernel.HadError())
			throw new IllegalStateException("Error creating agent: " + kernel.GetLastErrorDescription()) ;
		
		agent.ExecuteCommandLine("watch --wmes");
		
		kernel.RegisterForUpdateEvent(smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES, this, null);
		agent.RegisterForPrintEvent(smlPrintEventId.smlEVENT_PRINT, this, null);
		
		Identifier inputLink = agent.GetInputLink();

		if (inputLink == null)
			throw new IllegalStateException("Error getting the input link") ;
		
		Identifier map = agent.CreateIdWME(inputLink, "map");
		centerCell = agent.CreateIdWME(map, "center");
		agent.CreateStringWME(centerCell, "data", "center");
		eastCell = agent.CreateIdWME(map, "east");
		agent.CreateStringWME(eastCell, "data", "east");
		westCell = agent.CreateIdWME(map, "west");
		agent.CreateStringWME(westCell, "data", "west");
		
		target = agent.CreateIdWME(inputLink, "target");
		targetCell = agent.CreateSharedIdWME(target, "cell", centerCell);
		if (targetCell == null)
			throw new IllegalStateException("Error creating cell shared ID from target");
		verifyTarget("center");
		
		System.out.println("--- Agent created, input link initialized.");
		
		updateCount = 0;
		System.out.println("--- Starting Soar");
		kernel.RunAllAgents(9);
		System.out.println("--- Soar finished");

		System.out.println("--- final input link:");
		System.out.println(agent.ExecuteCommandLine("print --depth 10 --internal i2"));
		
		System.out.println("--- shutting down");
		kernel.Shutdown();
		kernel.delete();
		kernel = null;
		agent = null;
	}
	
	private void verifyTarget(String shouldBe) {
		System.out.println("=== Target verification");
		System.out.println("=== attribute:    " + targetCell.GetAttribute());
		System.out.println("=== id name:      " + targetCell.GetIdentifierName());
		System.out.println("=== num children: " + targetCell.GetNumberChildren());
		System.out.println("=== time tag:     " + targetCell.GetTimeTag());
		System.out.println("=== value:        " + targetCell.GetValueAsString());
		WMElement data = targetCell.FindByAttribute("data", 0);
		if (data == null) {
			if (shouldBe == null) {
				System.out.println("=== child data:   null");
			} else { 
				String message = "!!! child data:   null (should be " + shouldBe + ")";
				System.out.println(message);
				throw new IllegalStateException(message);
			}
		} else {
			if (data.GetValueAsString().equals(shouldBe)) {
				System.out.println("=== child data:   " + data.GetValueAsString());
			} else {
				String message = "!!! child data:   " + data.GetValueAsString() + " (should be " + shouldBe + ")";
				System.out.println(message);
				throw new IllegalStateException(message);
			}
		}
	}
	
	private int updateCount = 0;
	public void updateEventHandler(int eventID, Object data, Kernel kernel, int runFlags) {
		System.out.println("--- update " + ++updateCount + " started");

		assert eventID == smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES.swigValue();
		assert data == null;
		assert this.kernel == kernel;
		
		System.out.println("--- print --depth 10 --internal i2");
		System.out.println(agent.ExecuteCommandLine("print --depth 10 --internal i2"));
		
		// Move the target
		switch (updateCount)
		{
		case 1:
			System.out.println("--- environment moving target east");
			agent.DestroyWME(targetCell);
			targetCell = agent.CreateSharedIdWME(target, "cell", eastCell);
			verifyTarget("east");
			break;
		case 2:
			System.out.println("--- environment moving target center");
			agent.DestroyWME(targetCell);
			targetCell = agent.CreateSharedIdWME(target, "cell", centerCell);
			verifyTarget("center");
			break;
		case 3:
			System.out.println("--- environment moving target west");
			agent.DestroyWME(targetCell);
			targetCell = agent.CreateSharedIdWME(target, "cell", westCell);
			verifyTarget("west");
			break;
		case 4:
			System.out.println("--- environment moving target center");
			agent.DestroyWME(targetCell);
			targetCell = agent.CreateSharedIdWME(target, "cell", centerCell);
			verifyTarget("center");
			break;
		case 5:
			System.out.println("--- environment moving target east");
			agent.DestroyWME(targetCell);
			targetCell = agent.CreateSharedIdWME(target, "cell", eastCell);
			verifyTarget("east");
			break;
		case 6:
			System.out.println("--- environment moving target center");
			agent.DestroyWME(targetCell);
			targetCell = agent.CreateSharedIdWME(target, "cell", centerCell);
			verifyTarget("center");
			break;
		case 7:
			System.out.println("--- environment moving target west");
			agent.DestroyWME(targetCell);
			targetCell = agent.CreateSharedIdWME(target, "cell", westCell);
			verifyTarget("west");
			break;
		case 8:
			System.out.println("--- environment moving target center");
			agent.DestroyWME(targetCell);
			targetCell = agent.CreateSharedIdWME(target, "cell", centerCell);
			verifyTarget("center");
			break;
		}
		
		System.out.println("--- update finished, Soar resuming");
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
