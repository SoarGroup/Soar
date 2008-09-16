package soar2d.player.book;

import java.util.Iterator;

import soar2d.Soar2D;
import soar2d.player.MoveInfo;
import soar2d.player.Player;
import soar2d.player.PlayerConfig;
import soar2d.world.PlayersManager;
import soar2d.world.World;

public class Mouse extends Player {

	public Mouse(PlayerConfig playerConfig) {
		super(playerConfig);
	}

	Player target;
	double targetAngleOff;

	public void update(java.awt.Point location) {
		World world = Soar2D.simulation.world;

		super.update(location);
		
		PlayersManager players = world.getPlayers();
		if (players.numberOfPlayers() > 1) {
			Iterator<Player> playersIter = players.iterator();
			target = null;
			while (playersIter.hasNext()) {
				Player player = playersIter.next();
				if (this.equals(player) || this.getName().equals("dog")) {
					continue;
				}
				if (player.getLocationId() == this.getLocationId()) {
					double angleOff = players.angleOff(this, player);
					target = player;
					targetAngleOff = angleOff;
					break;
				}
			}
		}
	}
	
	public MoveInfo getMove() {
		MoveInfo move = new MoveInfo();
		if (target == null) {
			move.forward = true;
			return move;
		}
		
		move.rotateRelative = true;
		move.rotateRelativeYaw = this.targetAngleOff + Math.PI;
		if (Math.abs(targetAngleOff) < (Math.PI / 2)) {
			move.backward = true;
		} else {
			move.forward = true;
		}
		
		return move;
	}
	public boolean getHumanMove() {
		return true;
	}

	public void reset() {
		super.reset();
		target = null;
	}
}
