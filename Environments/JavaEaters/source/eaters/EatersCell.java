package eaters;

public class EatersCell {
	private static int foodCount = 0;
	
	public static int getFoodCount() {
		return foodCount;
	}

	private CellType type;
	private Food food = null;
	private Eater eater = null;

	public EatersCell() {
		this.type = CellType.EMPTY;
	}
	
	public void set(String typeString) {
		setType(CellType.getType(typeString));
		setFood(Food.getFood(typeString));
	}
	
	public void setType(CellType type) {
		if (type == null) {
			type = CellType.EMPTY;
			return;
		}
		if (type.equals(CellType.WALL)) {
			setFood(null);
		}
		this.type = type;
	}
	
	public boolean isWall() {
		return this.type.equals(CellType.WALL);
	}
	
	public boolean isEmpty() {
		return this.type.equals(CellType.EMPTY);
	}
	
	public CellType getType() {
		return this.type;
	}
	
	public void setEater(Eater eater) {
		this.eater = eater;
	}
	
	public Eater getEater() {
		return eater;
	}
	
	public void setFood(Food food) {
		if (this.food != null) {
			--EatersCell.foodCount;
		}
		
		if (food != null) {
			++EatersCell.foodCount;
		}
		
		this.food = food;
	}
	
	public Food getFood() {
		return food;
	}
	
	private boolean collision = false;
	
	public void setCollision(boolean setting) {
		collision = setting;
	}
	
	public boolean checkCollision() {
		return collision;
	}
	
}
