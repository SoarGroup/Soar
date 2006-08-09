package eaters;

import simulation.Cell;

public class EatersCell extends Cell {

	static final String kWallID = "wall";
	static final String kEmptyID = "empty";
	static final String kEaterID = "eater";
	
	private static final int kWallInt = 0;
	private static final int kEmptyInt = 1;
	private static final int kEaterInt = 2;
	private static final int kReservedIDs = 3;		
	
	private Eater m_Eater;
	static int foodCount = 0;
	static int scoreCount = 0;
	
	public EatersCell() {
		m_Type = kEmptyInt;
	}
	
	public EatersCell(int foodIndex) {
		m_Type = kEmptyInt;
		setFood(foodIndex);
	}

	public boolean setType(String name) {
		if (name.equalsIgnoreCase(kWallID)) {
			m_Type = kWallInt;
		} else if (name.equalsIgnoreCase(kEmptyID)) {
			m_Type = kEmptyInt;			
		} else {	
			int index = Food.getFoodIndexByName(name);
			if (index == -1) {
				return false;
			}
			m_Type = index + kReservedIDs;
			scoreCount += Food.food[index].getValue();
			if (Food.food[index].getValue() > 0) {
				++foodCount;					
			}
		}
		return true;
	}
	
	public boolean isWall() {
		return m_Type == kWallInt;
	}
	
	public void setWall() {
		assert m_Eater == null;
		m_Type = kWallInt;
	}
	
	public boolean isEmpty() {
		return m_Type == kEmptyInt;
	}
	
	public boolean isEater() {
		return (m_Type == kEaterInt) || (m_Eater != null);
	}
	
	public boolean isFood() {
		return m_Type >= kReservedIDs;
	}
	
	public boolean removeEater() {
		if (!isEater()) {
			return false;
		}
		m_Redraw = true;
		if (m_Type == kEaterInt) {
			m_Type = kEmptyInt;
		}
		m_Eater = null;
		return true;
	}
	
	public Food setFood(Food newFood) {
		Food oldFood = null;
		if (isFood()) {
			oldFood = removeFood();
		}
		m_Type = Food.getFoodIndexByName(newFood.getName()) + kReservedIDs;
		if (m_Type == -1) {
			m_Type = kEmptyInt;
		} else {
			scoreCount += newFood.getValue();
			if (newFood.getValue() > 0) {
				++foodCount;					
			}
		}
		return oldFood;
	}
	
	void setFood(int foodIndex) {
		if (isFood()) {
			removeFood();
		}
		m_Type = foodIndex + kReservedIDs;
		scoreCount += Food.food[foodIndex].getValue();
		if (Food.food[foodIndex].getValue() > 0) {
			++foodCount;
		}
	}
	
	public Food setEater(Eater eater) {
		m_Redraw = true;
		Food f = null;
		if (isFood()) {
			f = removeFood();
		}
		m_Type = kEaterInt;
		m_Eater = eater;
		return f;
	}
	
	public Eater getEater() {
		return m_Eater;
	}
	
	public String getName() {
		switch (m_Type) {
		case kWallInt:
			return kWallID;
		case kEmptyInt:
			return kEmptyID;
		case kEaterInt:
			return kEaterID;
		default:
			break;
		}
		
		// TODO: risking null exception here
		return getFood().getName();
	}
	
	public Food getFood() {
		if ((m_Type - kReservedIDs) < 0) return null;
		return Food.food[(m_Type - kReservedIDs)];
	}
	
	public Food removeFood() {
		if (isFood()) {
			m_Redraw = true;
			Food f = getFood();
			if (m_Eater == null) {
				m_Type = kEmptyInt;
			} else {
				m_Type = kEaterInt;
			}
			scoreCount -= f.getValue();
			if (f.getValue() > 0) {
				--foodCount;
			}
			return f;
		}
		return null;
	}	
}
