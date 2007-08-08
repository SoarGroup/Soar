package soar2d.player.kitchen;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.logging.*;

import sml.*;
import soar2d.Direction;
import soar2d.Names;
import soar2d.Simulation;
import soar2d.Soar2D;
import soar2d.map.CellObject;
import soar2d.player.MoveInfo;
import soar2d.player.Player;
import soar2d.player.PlayerConfig;
import soar2d.world.World;

public class SoarCook extends Cook {
	private Agent agent;
	
	public SoarCook(Agent agent, PlayerConfig playerConfig) {
		super(playerConfig);

		this.agent = agent;
		
		Identifier self = agent.CreateIdWME(agent.GetInputLink(), "self");
		Identifier position = agent.CreateIdWME(self, "position");
		IntElement x = agent.CreateIntWME(position, "x", 0);
		IntElement y = agent.CreateIntWME(position, "y", 0);
		IntElement reward = agent.CreateIntWME(self, "reward", 0);
	}

	@Override
	public void moveWithObjectFailed() {
		// TODO: set move-with-object command to status:error
		
	}
}
