package utilities;

import simulation.WorldEntity;

public class RelativeDirections {
	public int forward;
	public int backward;
	public int left;
	public int right;
	public int xIncrement;
	public int yIncrement;
	
	public void calculate(int facing) {
			switch (facing) {
			case WorldEntity.kNorthInt:
				forward = WorldEntity.kNorthInt;
				backward = WorldEntity.kSouthInt;
				left = WorldEntity.kWestInt;
				right = WorldEntity.kEastInt;
				xIncrement = 0;
				yIncrement = -1;
				break;
			case WorldEntity.kEastInt:
				forward = WorldEntity.kEastInt;
				backward = WorldEntity.kWestInt;
				left = WorldEntity.kNorthInt;
				right = WorldEntity.kSouthInt;
				xIncrement = 1;
				yIncrement = 0;
				break;
			case WorldEntity.kSouthInt:
				forward = WorldEntity.kSouthInt;
				backward = WorldEntity.kNorthInt;
				left = WorldEntity.kEastInt;
				right = WorldEntity.kWestInt;
				xIncrement = 0;
				yIncrement = 1;
				break;
			case WorldEntity.kWestInt:
				forward = WorldEntity.kWestInt;
				backward = WorldEntity.kEastInt;
				left = WorldEntity.kSouthInt;
				right = WorldEntity.kNorthInt;
				xIncrement = -1;
				yIncrement = 0;
				break;
		}
	}
}

