package edu.umich.soar.gridmap2d.players.scripted;

import java.util.ArrayList;
import java.util.List;


import edu.umich.soar.gridmap2d.map.EatersMap;
import edu.umich.soar.gridmap2d.players.CommandInfo;
import edu.umich.soar.gridmap2d.players.EaterCommander;


public class ScriptedEater implements EaterCommander {
	
	List<CommandInfo> commands = new ArrayList<CommandInfo>();
	int index;
	
	public ScriptedEater(List<CommandInfo> commands) {
		reset();
		this.commands = new ArrayList<CommandInfo>(commands);
	}

	public CommandInfo nextCommand() {
		if (index >= commands.size()) {
			return new CommandInfo();
		}
		return commands.get(index++);
	}

	public void reset() {
		index = 0;
	}

	public void shutdown() {
	}

	public void update(EatersMap eatersMap) {
	}
}
