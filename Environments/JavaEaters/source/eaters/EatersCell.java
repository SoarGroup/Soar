package eaters;

public class EatersCell {
	private static int foodCount = 0;
	
	public static int getFoodCount() {
		return foodCount;
	}

	public static void resetFoodCount() {
		foodCount = 0;
	}

	private CellType type;
	private Food food = null;
	private Eater eater = null;
	private boolean draw = true;

	public EatersCell() {
		this.type = CellType.EMPTY;
	}
	
	public void set(String typeString) {
		setType(CellType.getType(typeString));
		setFood(Food.getFood(typeString));
	}
	
	public void setType(CellType type) {
		draw = true;
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
		draw = true;
		this.eater = eater;
	}
	
	public Eater getEater() {
		return eater;
	}
	
	public void setFood(Food food) {
		draw = true;
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
		draw = true;
		collision = setting;
	}
	
	public boolean checkCollision() {
		return collision;
	}
	
	public boolean checkDraw() {
		return draw;
	}
	
	public void clearDraw() {
		draw = false;
	}
}
