package edu.umich.soar.sproom.soar;

import java.util.ArrayList;
import java.util.List;

import sml.Agent;
import sml.Identifier;
import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.SharedNames;

class InputLink {
	private final Agent agent;
	private final List<InputLinkElement> elements = new ArrayList<InputLinkElement>();
	
	public static InputLink newInstance(Adaptable app) {
		return new InputLink(app);
	}

	InputLink(Adaptable app) {
		this.agent = (Agent)app.getAdapter(Agent.class);
		
		Identifier inputLink = agent.GetInputLink();
		elements.add(new TimeIL(agent.CreateIdWME(inputLink, SharedNames.TIME), app));
		elements.add(new ConfigurationIL(agent.CreateIdWME(inputLink, SharedNames.CONFIGURATION), app));
		elements.add(new SelfIL(agent.CreateIdWME(inputLink, SharedNames.SELF), app));
		elements.add(new WaypointsIL(agent.CreateIdWME(inputLink, SharedNames.WAYPOINTS), app));
		elements.add(new ReceivedMessagesIL(agent.CreateIdWME(inputLink, SharedNames.RECEIVED_MESSAGES), app));
		elements.add(new AreaDescriptionIL(agent.CreateIdWME(inputLink, SharedNames.AREA_DESCRIPTION), app));
		elements.add(new ObjectsIL(agent.CreateIdWME(inputLink, SharedNames.OBJECTS), app));
		elements.add(new LidarIL(agent.CreateIdWME(inputLink, SharedNames.LIDAR), app));
	}
	
	void update(Adaptable app) {
		for (InputLinkElement element : elements) {
			element.update(app);
		}
	}

	public void destroy() {
		for (InputLinkElement element : elements) {
			element.destroy();
		}
	}
}
