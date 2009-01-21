package soar2d.visuals;

import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.Composite;

import soar2d.Soar2D;
import soar2d.config.PlayerConfig;
import soar2d.map.GridMap;
import soar2d.players.Player;

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
		PlayerConfig existingPlayerConfig = Soar2D.config.playerConfigs().get(playerId);
		
		// create id and configuration
		String clonePlayerId = "clone" + Integer.toString(++clonePlayer);
		PlayerConfig clonePlayerConfig = new PlayerConfig();

		if (existingPlayerConfig.productions != null) {
			clonePlayerConfig.productions = new String(existingPlayerConfig.productions);
		}
		
		Soar2D.config.playerConfigs().put(clonePlayerId, clonePlayerConfig);
		Soar2D.simulation.createPlayer(clonePlayerId, clonePlayerConfig);
	}
}
