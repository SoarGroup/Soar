package soar2d.visuals;

import org.eclipse.swt.*;
import org.eclipse.swt.widgets.*;

import soar2d.Soar2D;
import soar2d.config.Config;
import soar2d.config.Soar2DKeys;
import soar2d.map.GridMap;
import soar2d.player.Player;

public class AgentDisplay extends Composite {
	
	public AgentDisplay(final Composite parent) {
		super(parent, SWT.NONE);
	}

	void selectPlayer(Player player) {
		assert false;
	}

	void worldChangeEvent() {
		assert false;
	}

	void updateButtons() {
		assert false;
	}

	void agentEvent() {
		assert false;
	}

	public void setMap(GridMap map) {
	}
	
	private static int clonePlayer = 0;
	void clonePlayer(String playerId) {
		Config existingPlayerConfig = Soar2D.config.getChild(Soar2DKeys.playerKey(playerId));
		
		// create id and configuration
		String clonePlayerId = "clone" + Integer.toString(++clonePlayer);
		Config clonePlayerConfig = Soar2D.config.getChild(Soar2DKeys.playerKey(clonePlayerId));

		if (existingPlayerConfig.hasKey(Soar2DKeys.players.productions)) {
			clonePlayerConfig.setString(Soar2DKeys.players.productions, existingPlayerConfig.requireString(Soar2DKeys.players.productions));
		}
		
		Soar2D.simulation.createPlayer(clonePlayerId);
	}
}
