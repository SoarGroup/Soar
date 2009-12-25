package edu.umich.soar.sproom.soar;

import java.util.ArrayList;
import java.util.List;

import sml.Agent;
import sml.Identifier;
import edu.umich.soar.sproom.Adaptable;

class InputLink {
	private static final String TIME = "time";
	private static final String CONFIGURATION = "configuration";
	private static final String SELF = "self";
	private static final String WAYPOINTS = "waypoints";
	private static final String RECEIVED_MESSAGES = "received-messages";
	
	private final Agent agent;
	private final List<InputLinkElement> elements = new ArrayList<InputLinkElement>();
	
	public static InputLink newInstance(Adaptable app) {
		return new InputLink(app);
	}

	InputLink(Adaptable app) {
		this.agent = (Agent)app.getAdapter(Agent.class);
		
		Identifier inputLink = agent.GetInputLink();
		elements.add(new TimeIL(agent.CreateIdWME(inputLink, TIME), app));
		elements.add(new ConfigurationIL(agent.CreateIdWME(inputLink, CONFIGURATION), app));
		elements.add(new SelfIL(agent.CreateIdWME(inputLink, SELF), app));
		elements.add(new WaypointsIL(agent.CreateIdWME(inputLink, WAYPOINTS), app));
		elements.add(new ReceivedMessagesIL(agent.CreateIdWME(inputLink, RECEIVED_MESSAGES), app));
	}
	
	void update(Adaptable app) {
		for (InputLinkElement element : elements) {
			element.update(app);
		}
	}
}
