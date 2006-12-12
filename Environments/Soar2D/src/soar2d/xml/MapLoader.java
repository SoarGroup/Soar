package soar2d.xml;

import sml.*;
import soar2d.*;
import soar2d.world.*;

import java.util.*;

public class MapLoader {
	private Stack<String> xmlPath;
	private ArrayList<String> foods;
	
	private int size = 0;
	private Cell[][] mapCells = null;
	private CellObjectManager cellObjectManager = null;
	
	private void throwSyntax(String message) throws SyntaxException {
		throw new SyntaxException(xmlPath, message);
	}
	
	private void throwSML(String message) throws SMLException {
		throw new SMLException(xmlPath, message);
	}
	
	public int getSize() {
		return size;
	}
	
	public Cell[][] getCells() {
		return mapCells;
	}
	
	public CellObjectManager getCellObjectManager() {
		return cellObjectManager;
	}
	
	public boolean load() {
		xmlPath = new Stack<String>();
		foods = new ArrayList<String>();
		cellObjectManager = new CellObjectManager();
			
		String mapFile = Soar2D.config.map.getAbsolutePath();
		ElementXML rootTag = ElementXML.ParseXMLFromFile(mapFile);
		if (rootTag == null) {
			Soar2D.control.severeError("Error parsing file " + mapFile + "\n" + ElementXML.GetLastParseErrorDescription());
			return false;
		}
		
		try {
			if (rootTag.IsTag(Names.kTagMap)) {
				xmlPath.push(Names.kTagMap);
				map(rootTag);
				xmlPath.pop();
			} else {
				throwSyntax("unrecognized tag " + rootTag.GetTagName());
			}
		} catch (SyntaxException p) {
			this.mapCells = null;
			Soar2D.control.severeError("Error parsing file " + mapFile + "\n" + p.getMessage());
			return false;
			
		} catch (SMLException s) {
			this.mapCells = null;
			Soar2D.control.severeError("SML error during parsing: " + s.getMessage());
			return false;
			
		} finally {
			assert rootTag.GetRefCount() == 1;
			rootTag.ReleaseRefOnHandle();
			rootTag.delete();
			rootTag = null;
		}
		
		// success
		return true;
	}
	private void map(ElementXML mapTag) throws SMLException, SyntaxException {
		ElementXML mainTag = null;
		for (int mapTagIndex = 0 ; mapTagIndex < mapTag.GetNumberChildren() ; ++mapTagIndex) {

			mainTag = new ElementXML();
			if (mainTag == null) throwSML("couldn't create mainTag");

			try {
				mapTag.GetChild(mainTag, mapTagIndex);
				if (mainTag == null) throwSML("failed to get tag by index");
				
				if (mainTag.IsTag(Names.kTagCellObject)) {
					xmlPath.push(Names.kTagCellObject);
					cellObject(mainTag);
					xmlPath.pop();

				} else if (mainTag.IsTag(Names.kTagCells)) {
					xmlPath.push(Names.kTagCells);
					cells(mainTag);
					xmlPath.pop();

				} else {
					throwSyntax("unrecognized tag " + mainTag.GetTagName());
				}
			} finally {
				mainTag.delete();
				mainTag = null;
			}
		}
	}
	
	private void cellObject(ElementXML cellObjectTag) throws SMLException, SyntaxException {
		String name = null;
		boolean updatable = false;
		boolean consumable = false;

		String attribute = null;
		
		attribute = cellObjectTag.GetAttribute(Names.kParamName);
		if (attribute == null) {
			throwSyntax("cell-object must have name");
		}
		if (attribute.length() <= 0) {
			throwSyntax("cell-object name must not be zero-length");
		}
		name = attribute;
		
		attribute = cellObjectTag.GetAttribute(Names.kParamUpdatable);
		if (attribute != null) {
			updatable = Boolean.parseBoolean(attribute);
		}
		
		attribute = cellObjectTag.GetAttribute(Names.kParamConsumable);
		if (attribute != null) {
			consumable = Boolean.parseBoolean(attribute);
		}
		
		CellObject cellObjectTemplate = new CellObject(name, updatable, consumable);

		ElementXML cellObjectSubTag = null;
		for (int cellObjectSubTagIndex = 0; cellObjectSubTagIndex < cellObjectTag.GetNumberChildren(); ++cellObjectSubTagIndex) {
			
			cellObjectSubTag = new ElementXML();
			if (cellObjectSubTag == null) throwSML("couldn't create cellObjectSubTag tag");

			try {
				cellObjectTag.GetChild(cellObjectSubTag, cellObjectSubTagIndex);
				if (cellObjectSubTag == null) throwSML("failed to get tag by index");
				
				if (cellObjectSubTag.IsTag(Names.kTagProperty)) {
					xmlPath.push(Names.kTagProperty);
					property(cellObjectTemplate, cellObjectSubTag, false);
					xmlPath.pop();
		
				} else if (cellObjectSubTag.IsTag(Names.kTagApply)) {
					xmlPath.push(Names.kTagApply);
					apply(cellObjectTemplate, cellObjectSubTag);
					xmlPath.pop();
					
				} else if (cellObjectSubTag.IsTag(Names.kTagUpdate)) {
					xmlPath.push(Names.kTagUpdate);
					update(cellObjectTemplate, cellObjectSubTag);
					xmlPath.pop();
					
				} else {
					throwSyntax("unrecognized tag " + cellObjectSubTag.GetTagName());
				}
				
				
			} finally {
				cellObjectSubTag.delete();
				cellObjectSubTag = null;
			}
		}
		
		cellObjectManager.registerTemplate(name, cellObjectTemplate);
	}
	private void property(CellObject cellObjectTemplate, ElementXML propertyTag, boolean apply) throws SMLException, SyntaxException {
		String name = null;
		String value = null;

		String attribute = null;
		
		attribute = propertyTag.GetAttribute(Names.kParamName);
		if (attribute == null) {
			throwSyntax("property must have a name");
		}
		if (attribute.length() <= 0) {
			throwSyntax("property name must not be zero-length");
		}
		name = attribute;
		
		attribute = propertyTag.GetAttribute(Names.kParamValue);
		if (attribute == null) {
			throwSyntax("property must have a value");
		}
		if (attribute.length() <= 0) {
			throwSyntax("property value must not be zero-length");
		}
		value = attribute;
		
		if (apply) {
			cellObjectTemplate.addPropertyApply(name, value);
		} else {
			cellObjectTemplate.addProperty(name, value);
			if (name.equalsIgnoreCase(Names.kPropertyEdible)) {
				if (Boolean.parseBoolean(value)) {
					foods.add(cellObjectTemplate.getName());
				}
			}
		}
	}
	
	private void apply(CellObject cellObjectTemplate, ElementXML applyTag) throws SMLException, SyntaxException {
		
		ElementXML applySubTag = null;
		for (int applySubTagIndex = 0; applySubTagIndex < applyTag.GetNumberChildren(); ++applySubTagIndex) {
			
			applySubTag = new ElementXML();
			if (applySubTag == null) throwSML("couldn't create applySubTag tag");

			try {
				applyTag.GetChild(applySubTag, applySubTagIndex);
				if (applySubTag == null) throwSML("failed to get tag by index");
				
				if (applySubTag.IsTag(Names.kTagPoints)) {
					xmlPath.push(Names.kTagPoints);
					cellObjectTemplate.setPointsApply(true);
					xmlPath.pop();

				} else if (applySubTag.IsTag(Names.kTagProperty)) {
					xmlPath.push(Names.kTagProperty);
					property(cellObjectTemplate, applySubTag, true);
					xmlPath.pop();
					
				} else if (applySubTag.IsTag(Names.kTagEnergy)) {
					xmlPath.push(Names.kTagEnergy);
					energy(cellObjectTemplate, applySubTag);
					xmlPath.pop();
					
				} else if (applySubTag.IsTag(Names.kTagHealth)) {
					xmlPath.push(Names.kTagHealth);
					health(cellObjectTemplate, applySubTag);
					xmlPath.pop();
					
				} else if (applySubTag.IsTag(Names.kTagMissiles)) {
					xmlPath.push(Names.kTagMissiles);
					cellObjectTemplate.setMissilesApply(true);
					xmlPath.pop();
					
				} else {
					throwSyntax("unrecognized tag " + applySubTag.GetTagName());
				}
			} finally {
				applySubTag.delete();
				applySubTag = null;
			}
		}
	}
	
	private void energy(CellObject cellObjectTemplate, ElementXML applyTag) {
		String attribute = applyTag.GetAttribute(Names.kParamShields);
		boolean shields = false;
		if (attribute != null) {
			shields = Boolean.parseBoolean(attribute);
		}
		cellObjectTemplate.setEnergyApply(true, shields);
	}
	
	private void health(CellObject cellObjectTemplate, ElementXML applyTag) {
		String attribute = applyTag.GetAttribute(Names.kParamShields);
		boolean shields = false;
		if (attribute != null) {
			shields = Boolean.parseBoolean(attribute);
		}
		cellObjectTemplate.setHealthApply(true, shields);
	}
	
	private void update(CellObject cellObjectTemplate, ElementXML updateTag) throws SMLException, SyntaxException {
		
		ElementXML updateSubTag = null;
		for (int updateSubTagIndex = 0; updateSubTagIndex < updateTag.GetNumberChildren(); ++updateSubTagIndex) {
			
			updateSubTag = new ElementXML();
			if (updateSubTag == null) throwSML("couldn't create updateSubTag tag");

			try {
				updateTag.GetChild(updateSubTag, updateSubTagIndex);
				if (updateSubTag == null) throwSML("failed to get tag by index");
				
				if (updateSubTag.IsTag(Names.kTagDecay)) {
					xmlPath.push(Names.kTagDecay);
					cellObjectTemplate.setDecayUpdate(true);
					xmlPath.pop();
					
				} else {
					throwSyntax("unrecognized tag " + updateSubTag.GetTagName());
				}
			} finally {
				updateSubTag.delete();
				updateSubTag = null;
			}
		}
	}
	
	private void cells(ElementXML cellsTag) throws SMLException, SyntaxException {
		boolean randomWalls = false;
		boolean randomFood = false;
		
		String attribute = null;
		
		attribute = cellsTag.GetAttribute(Names.kParamWorldSize);
		if (attribute == null) {
			throwSyntax("cells tag must have a world-size parameter");
		}
		if (attribute.length() <= 0) {
			throwSyntax("world-size parameter must not be zero-length");
		}
		this.size = Integer.parseInt(attribute);
		if (size < 5) {
			throwSyntax("world-size must be 5 or greater");
		}
		
		attribute = cellsTag.GetAttribute(Names.kParamRandomWalls);
		if (attribute != null) {
			randomWalls = Boolean.parseBoolean(attribute);
		}

		attribute = cellsTag.GetAttribute(Names.kParamRandomFood);
		if (attribute != null) {
			randomFood = Boolean.parseBoolean(attribute);
		}
		
		// Generate map from XML unless both are true
		if (!randomWalls || !randomFood) {
			generateMapFromXML(cellsTag);
		}
		
		// override walls if necessary
		if (randomWalls) {
			generateRandomWalls();
		}
		
		// override food if necessary
		if (randomFood) {
			generateRandomFood();
		}
	}
	
	private void generateMapFromXML(ElementXML cellsTag) throws SMLException, SyntaxException {
		if (cellsTag.GetNumberChildren() != this.size) {
			throwSyntax("there does not seem to be the " +
					"correct amount of row tags (" + cellsTag.GetNumberChildren() +
					") for the specified map size (" + this.size +
					")");
		}
		
		this.mapCells = new Cell[this.size][];
		
		ElementXML rowTag = null;
		for (int rowTagIndex = 0; rowTagIndex < cellsTag.GetNumberChildren(); ++rowTagIndex) {
			
			rowTag = new ElementXML();
			if (rowTag == null) throwSML("couldn't create rowTag tag");

			try {
				cellsTag.GetChild(rowTag, rowTagIndex);
				if (rowTag == null) throwSML("failed to get tag by index");
				
				if (rowTag.IsTag(Names.kTagRow)) {
					
					this.mapCells[rowTagIndex] = new Cell[this.size];

					xmlPath.push(Names.kTagRow);
					row(rowTagIndex, rowTag);
					xmlPath.pop();

				} else {
					throwSyntax("unrecognized tag " + rowTag.GetTagName());
				}
			} finally {
				rowTag.delete();
				rowTag = null;
			}
		}
	}
	
	private void row(int row, ElementXML rowTag) throws SMLException, SyntaxException {
		if (rowTag.GetNumberChildren() != this.size) {
			throwSyntax("there does not seem to be the " +
					"correct amount of cell tags (" + rowTag.GetNumberChildren() +
					") for the specified map size (" + this.size +
					") in row " + row);
		}
		
		ElementXML cellTag = null;
		for (int cellTagIndex = 0; cellTagIndex < rowTag.GetNumberChildren(); ++cellTagIndex) {
			
			cellTag = new ElementXML();
			if (cellTag == null) throwSML("couldn't create cellTag tag");

			try {
				rowTag.GetChild(cellTag, cellTagIndex);
				if (cellTag == null) throwSML("failed to get tag by index");
				
				if (cellTag.IsTag(Names.kTagCell)) {
					mapCells[row][cellTagIndex] = new Cell();
					
					xmlPath.push(Names.kTagCell);
					cell(mapCells[row][cellTagIndex], cellTag);
					xmlPath.pop();

				} else {
					throwSyntax("unrecognized tag " + cellTag.GetTagName());
				}
			} finally {
				cellTag.delete();
				cellTag = null;
			}
		}
	}
	
	private void cell(Cell cell, ElementXML cellTag) throws SMLException, SyntaxException {
		ElementXML objectTag = null;
		for (int cellTagIndex = 0; cellTagIndex < cellTag.GetNumberChildren(); ++cellTagIndex) {
			
			objectTag = new ElementXML();
			if (objectTag == null) throwSML("couldn't create cellTag tag");

			try {
				cellTag.GetChild(objectTag, cellTagIndex);
				if (objectTag == null) throwSML("failed to get tag by index");
				
				if (objectTag.IsTag(Names.kTagObject)) {
					xmlPath.push(Names.kTagObject);
					object(cell, objectTag);
					xmlPath.pop();

				} else {
					throwSyntax("unrecognized tag " + objectTag.GetTagName());
				}
			} finally {
				objectTag.delete();
				objectTag = null;
			}
		}
	}
	
	private void object(Cell cell, ElementXML objectTag) throws SMLException, SyntaxException {
		String name = objectTag.GetCharacterData();
		if (!cellObjectManager.hasTemplate(name)) {
			throwSyntax("object \"" + name + "\" does not map to a cell-object");
		}
		
		CellObject cellObject = cellObjectManager.createObject(name);
		cell.addCellObject(cellObject);
	}
	

	private void generateRandomWalls() throws SyntaxException {
		if (!cellObjectManager.hasTemplate(Names.kWall)) {
			throwSyntax("tried to generate random walls with no wall type");
		}
		
		if (mapCells == null) {
			mapCells = new Cell[size][];
		}
		
		// Generate perimiter wall
		for (int row = 0; row < size; ++row) {
			if (mapCells[row] == null) {
				mapCells[row] = new Cell[size];
			}
			if (mapCells[row][0] == null) {
				mapCells[row][0] = new Cell();
			}
			addWallAndRemoveFood(mapCells[row][0]);
			if (mapCells[row][size - 1] == null) {
				mapCells[row][size - 1] = new Cell();
			}
			addWallAndRemoveFood(mapCells[row][size - 1]);
		}
		for (int col = 1; col < size - 1; ++col) {
			if (mapCells[0][col] == null) {
				mapCells[0][col] = new Cell();
			}
			addWallAndRemoveFood(mapCells[0][col]);
			if (mapCells[size - 1][col] == null) {
				mapCells[size - 1][col] = new Cell();
			}
			addWallAndRemoveFood(mapCells[size - 1][col]);
		}
		
		double probability = Soar2D.config.kLowProbability;
		for (int row = 2; row < size - 2; ++row) {
			for (int col = 2; col < size - 2; ++col) {
				if (noWallsOnCorners(row, col)) {
					if (wallOnAnySide(row, col)) {
						probability = Soar2D.config.kHigherProbability;					
					}
					if (Simulation.random.nextDouble() < probability) {
						if (mapCells[row][col] == null) {
							mapCells[row][col] = new Cell();
						}
						addWallAndRemoveFood(mapCells[row][col]);
					}
					probability = Soar2D.config.kLowProbability;
				}
			}
		}
	}
	
	private void addWallAndRemoveFood(Cell cell) {
		cell.removeAllWithProperty(Names.kPropertyEdible);
		
		if (!cell.hasObject(Names.kWall)) {
			CellObject cellObject = cellObjectManager.createObject(Names.kWall);
			cell.addCellObject(cellObject);
		}
	}
	
	private boolean noWallsOnCorners(int row, int col) {
		Cell cell = mapCells[row + 1][col + 1];
		if (cell != null && !cell.enterable()) {
			return false;
		}
		
		cell = mapCells[row - 1][col - 1];
		if (cell != null && !cell.enterable()) {
			return false;
		}
		
		cell = mapCells[row + 1][col - 1];
		if (cell != null && !cell.enterable()) {
			return false;
		}
		
		cell = mapCells[row - 1][col + 1];
		if (cell != null && !cell.enterable()) {
			return false;
		}
		return true;
	}
	
	private boolean wallOnAnySide(int row, int col) {
		Cell cell = mapCells[row + 1][col];
		if (cell != null && !cell.enterable()) {
			return true;
		}
		
		cell = mapCells[row][col + 1];
		if (cell != null && !cell.enterable()) {
			return true;
		}
		
		cell = mapCells[row - 1][col];
		if (cell != null && !cell.enterable()) {
			return true;
		}
		
		cell = mapCells[row][col - 1];
		if (cell != null && !cell.enterable()) {
			return true;
		}
		return false;
	}
	
	private void generateRandomFood() {
		for (int row = 1; row < size - 1; ++row) {
			for (int col = 1; col < size - 1; ++col) {
				if (mapCells[row][col] == null) {
					mapCells[row][col] = new Cell();
					
				}
				if (mapCells[row][col].enterable()) {
					mapCells[row][col].removeAllWithProperty(Names.kPropertyEdible);
					CellObject cellObject = cellObjectManager.createObject(foods.get(Simulation.random.nextInt(foods.size())));
					mapCells[row][col].addCellObject(cellObject);
				}
			}
		}		
	}
	

}


















