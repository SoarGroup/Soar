package utilities;

import java.util.logging.*;

import simulation.WorldEntity;

public class RelativeDirections {
	private static Logger logger = Logger.getLogger("simulation");
	public int forward;
	public int backward;
	public int left;
	public int right;
	
	public void calculate(int facing) {
			switch (facing) {
			case WorldEntity.kNorthInt:
				forward = WorldEntity.kNorthInt;
				backward = WorldEntity.kSouthInt;
				left = WorldEntity.kWestInt;
				right = WorldEntity.kEastInt;
				break;
			case WorldEntity.kEastInt:
				forward = WorldEntity.kEastInt;
				backward = WorldEntity.kWestInt;
				left = WorldEntity.kNorthInt;
				right = WorldEntity.kSouthInt;
				break;
			case WorldEntity.kSouthInt:
				forward = WorldEntity.kSouthInt;
				backward = WorldEntity.kNorthInt;
				left = WorldEntity.kEastInt;
				right = WorldEntity.kWestInt;
				break;
			case WorldEntity.kWestInt:
				forward = WorldEntity.kWestInt;
				backward = WorldEntity.kEastInt;
				left = WorldEntity.kSouthInt;
				right = WorldEntity.kNorthInt;
				break;
		}
	}
}

