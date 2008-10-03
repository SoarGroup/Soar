package splintersoar;

import orc.util.*;

public class GamePadManager implements OverrideInterface {

	GamePad gp = new GamePad();
	
	@Override
	public double getLeft() {
		return gp.getAxis( 1 ) * -1;
	}

	@Override
	public double getRight() {
		return gp.getAxis( 3 ) * -1;
	}
}
