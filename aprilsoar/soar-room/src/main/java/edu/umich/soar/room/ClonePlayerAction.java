package edu.umich.soar.room;

import java.awt.event.ActionEvent;

import edu.umich.soar.room.config.PlayerConfig;
import edu.umich.soar.room.core.Simulation;
import edu.umich.soar.room.map.Robot;

public class ClonePlayerAction extends AbstractGridmap2DAction {

	private static final long serialVersionUID = -3347861376925708892L;

	private long counter = 0;
	
	public ClonePlayerAction(ActionManager manager) {
		super(manager, "Clone Player");
	}

	@Override
	public void update() {
        setEnabled(!getApplication().getSim().isRunning());
	}

	@Override
	public void actionPerformed(ActionEvent e) {
		Robot player = Adaptables.adapt(getApplication().getSelectionManager().getSelectedObject(), Robot.class);
		if (player == null) {
			return;
		}
		
		Simulation sim = getApplication().getSim();
		
		for (PlayerConfig playerConfig : sim.getConfig().playerConfigs().values()) {
			if (playerConfig.name.equals(player.getName())) {
				PlayerConfig clonePlayerConfig = new PlayerConfig();
				String clonePlayerId = "clone" + Long.toString(counter++);

				if (playerConfig.productions != null) {
					clonePlayerConfig.productions = new String(playerConfig.productions);
				}
				
				if (playerConfig.script != null) {
					clonePlayerConfig.script = new String(playerConfig.script);
				}
				
				clonePlayerConfig.name = null;

				sim.getConfig().playerConfigs().put(clonePlayerId, clonePlayerConfig);
				sim.createPlayer(clonePlayerConfig);
				break;
			}
		}
	}
	
}
