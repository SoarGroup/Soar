package soar2d.player.book;

import sml.Identifier;
import sml.IntElement;
import sml.StringElement;
import soar2d.world.World;

class MessageInputLink {
	SoarRobot robot;
	Identifier parent;
	StringElement from, message;
	IntElement cycle;

	int cycleCreated;

	MessageInputLink(SoarRobot robot, Identifier parent) {
		this.robot = robot;
		this.parent = parent;
	}
	
	void initialize(String from, String message, World world) {
		this.from = robot.agent.CreateStringWME(parent, "from", from);
		this.message = robot.agent.CreateStringWME(parent, "message", message);
		this.cycleCreated = world.getWorldCount();
		this.cycle = robot.agent.CreateIntWME(parent, "cycle", this.cycleCreated); 
	}
}

