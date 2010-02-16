package edu.umich.soar;

import java.io.IOException;

import sml.Agent;
import sml.Identifier;
import sml.Kernel;
import sml.smlUpdateEventId;
import sml.Kernel.UpdateEventInterface;

/**
 * @author voigtjr
 * 
 *         Very simple environment that adds a WME to the agent's input link
 *         with attribute "hello" and value "world" and then stops when any
 *         command from the agent is received.
 */
public class HelloWorld {
    private Kernel kernel;
    private Agent agent;

    public HelloWorld() {
	System.out.println("Creating Kernel...");
	kernel = Kernel.CreateKernelInNewThread();

	System.out.println("Creating Agent...");
	agent = kernel.CreateAgent("soar");

	System.out.println("Loading productions...");
	try {
	    ProductionUtils.sourceAgentFromJar(agent, "/helloworld.soar");
	} catch (IOException ex) {
	    ex.printStackTrace();
	    System.exit(1);
	}

	System.out.println("Creating input link...");
	agent.GetInputLink().CreateStringWME("hello", "world");

	System.out.println("Registering for update event...");
	kernel.RegisterForUpdateEvent(
		smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES,
		new UpdateEventInterface() {
		    @Override
		    public void updateEventHandler(int eventID, Object data,
			    Kernel kernel, int runFlags) {
			System.out.println("Update event...");
			for (int index = 0; index < agent.GetNumberCommands(); ++index) {
			    Identifier command = agent.GetCommand(index);

			    String name = command.GetCommandName();
			    System.out.println("Received command: " + name);
			    if (name.equals("stop")) {
				kernel.StopAllAgents();
				command.AddStatusComplete();
			    }
			}
			System.out.println("Update event complete.");
		    }
		}, null);

	System.out.println("Running agents...");
	kernel.RunAllAgents(5);

	System.out.println("Shutting down...");
	kernel.DestroyAgent(agent);
	agent = null;
	kernel.Shutdown();
	kernel = null;

	System.out.println("Done.");
    }

    public static void main(String[] args) {
	new HelloWorld();
    }

}
