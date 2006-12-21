package soar2d.world;

import sml.*;
import soar2d.*;
import soar2d.xml.SMLException;
import soar2d.xml.SyntaxException;

import java.util.*;

public class MapLoader {
	private Stack<String> xmlPath;
	
	GridMap map;
	
	private void throwSyntax(String message) throws SyntaxException {
		throw new SyntaxException(xmlPath, message);
	}
	
	private void throwSML(String message) throws SMLException {
		throw new SMLException(xmlPath, message);
	}
	
	public GridMap getMap() {
		return map;
	}
	
	public boolean load() {
		xmlPath = new Stack<String>();
		map = new GridMap();
			
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
			this.map.mapCells = null;
			Soar2D.control.severeError("Error parsing file " + mapFile + "\n" + p.getMessage());
			return false;
			
		} catch (SMLException s) {
			this.map.mapCells = null;
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
		
		CellObject cellObjectTemplate = new CellObject(name, updatable);

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
		
		map.cellObjectManager.registerTemplate(cellObjectTemplate);
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
					
				} else if (applySubTag.IsTag(Names.kTagRemove)) {
					xmlPath.push(Names.kTagRemove);
					cellObjectTemplate.setApplyRemove(true);
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
		String attribute = applyTag.GetAttribute(Names.kParamShieldsDown);
		boolean shieldsDown = false;
		if (attribute != null) {
			shieldsDown = Boolean.parseBoolean(attribute);
		}
		cellObjectTemplate.setHealthApply(true, shieldsDown);
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
					
				} else if (updateSubTag.IsTag(Names.kTagFlyMissile)) {
					xmlPath.push(Names.kTagFlyMissile);
					cellObjectTemplate.setFlyMissileUpdate(true);
					xmlPath.pop();
					
				} else if (updateSubTag.IsTag(Names.kTagLinger)) {
					xmlPath.push(Names.kTagLinger);
					cellObjectTemplate.setLingerUpdate(true);
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
		this.map.size = Integer.parseInt(attribute);
		if (map.size < 5) {
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
		if (cellsTag.GetNumberChildren() != this.map.size) {
			throwSyntax("there does not seem to be the " +
					"correct amount of row tags (" + cellsTag.GetNumberChildren() +
					") for the specified map size (" + this.map.size +
					")");
		}
		
		this.map.mapCells = new Cell[this.map.size][];
		
		ElementXML rowTag = null;
		for (int rowTagIndex = 0; rowTagIndex < cellsTag.GetNumberChildren(); ++rowTagIndex) {
			
			rowTag = new ElementXML();
			if (rowTag == null) throwSML("couldn't create rowTag tag");

			try {
				cellsTag.GetChild(rowTag, rowTagIndex);
				if (rowTag == null) throwSML("failed to get tag by index");
				
				if (rowTag.IsTag(Names.kTagRow)) {
					
					this.map.mapCells[rowTagIndex] = new Cell[this.map.size];

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
		if (rowTag.GetNumberChildren() != this.map.size) {
			throwSyntax("there does not seem to be the " +
					"correct amount of cell tags (" + rowTag.GetNumberChildren() +
					") for the specified map size (" + this.map.size +
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
					map.mapCells[row][cellTagIndex] = new Cell();
					
					xmlPath.push(Names.kTagCell);
					cell(new java.awt.Point(cellTagIndex, row), cellTag);
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
	
	private void cell(java.awt.Point location, ElementXML cellTag) throws SMLException, SyntaxException {
		ElementXML objectTag = null;
		boolean background = false;
		for (int cellTagIndex = 0; cellTagIndex < cellTag.GetNumberChildren(); ++cellTagIndex) {
			
			objectTag = new ElementXML();
			if (objectTag == null) throwSML("couldn't create cellTag tag");

			try {
				cellTag.GetChild(objectTag, cellTagIndex);
				if (objectTag == null) throwSML("failed to get tag by index");
				
				if (objectTag.IsTag(Names.kTagObject)) {
					xmlPath.push(Names.kTagObject);
					background = object(location, objectTag);
					xmlPath.pop();

				} else {
					throwSyntax("unrecognized tag " + objectTag.GetTagName());
				}
			} finally {
				objectTag.delete();
				objectTag = null;
			}
		}

		if (Soar2D.config.tanksoar && !background) {
			// add ground
			CellObject cellObject = map.cellObjectManager.createObject(Names.kGround);
			map.addObjectToCell(location, cellObject);
			return;
		}
	}
	
	private boolean object(java.awt.Point location, ElementXML objectTag) throws SMLException, SyntaxException {
		String name = objectTag.GetCharacterData();
		if (!map.cellObjectManager.hasTemplate(name)) {
			throwSyntax("object \"" + name + "\" does not map to a cell-object");
		}
		
		CellObject cellObject = map.cellObjectManager.createObject(name);
		boolean background = false;
		if (Soar2D.config.tanksoar) {
			if (cellObject.hasProperty(Names.kPropertyBlock) 
					|| (cellObject.getName() == Names.kGround)
					|| (cellObject.hasProperty(Names.kPropertyCharger))) {
				background = true;
			}
		}
		map.addObjectToCell(location, cellObject);
		return background;
	}
	

	private void generateRandomWalls() throws SyntaxException {
		if (!map.cellObjectManager.hasTemplatesWithProperty(Names.kPropertyBlock)) {
			throwSyntax("tried to generate random walls with no blocking types");
		}
		
		if (map.mapCells == null) {
			map.mapCells = new Cell[map.size][];
		}
		
		// Generate perimiter wall
		for (int row = 0; row < map.size; ++row) {
			if (map.mapCells[row] == null) {
				map.mapCells[row] = new Cell[map.size];
			}
			if (map.mapCells[row][0] == null) {
				map.mapCells[row][0] = new Cell();
			}
			addWallAndRemoveFood(new java.awt.Point(0, row));
			if (map.mapCells[row][map.size - 1] == null) {
				map.mapCells[row][map.size - 1] = new Cell();
			}
			addWallAndRemoveFood(new java.awt.Point(map.size - 1, row));
		}
		for (int col = 1; col < map.size - 1; ++col) {
			if (map.mapCells[0][col] == null) {
				map.mapCells[0][col] = new Cell();
			}
			addWallAndRemoveFood(new java.awt.Point(col, 0));
			if (map.mapCells[map.size - 1][col] == null) {
				map.mapCells[map.size - 1][col] = new Cell();
			}
			addWallAndRemoveFood(new java.awt.Point(col, map.size - 1));
		}
		
		double probability = Soar2D.config.kLowProbability;
		for (int row = 2; row < map.size - 2; ++row) {
			for (int col = 2; col < map.size - 2; ++col) {
				if (noWallsOnCorners(row, col)) {
					if (wallOnAnySide(row, col)) {
						probability = Soar2D.config.kHigherProbability;					
					}
					if (Simulation.random.nextDouble() < probability) {
						if (map.mapCells[row][col] == null) {
							map.mapCells[row][col] = new Cell();
						}
						addWallAndRemoveFood(new java.awt.Point(col, row));
					}
					probability = Soar2D.config.kLowProbability;
				}
			}
		}
	}
	
	private void addWallAndRemoveFood(java.awt.Point location) {
		map.removeAllWithProperty(location, Names.kPropertyEdible);
		
		ArrayList<CellObject> walls = map.getAllWithProperty(location, Names.kPropertyBlock);
		if (walls.size() <= 0) {
			map.addRandomObjectWithProperty(location, Names.kPropertyBlock);
		}
	}
	
	private boolean noWallsOnCorners(int row, int col) {
		Cell cell = map.mapCells[row + 1][col + 1];
		if (cell != null && !cell.enterable()) {
			return false;
		}
		
		cell = map.mapCells[row - 1][col - 1];
		if (cell != null && !cell.enterable()) {
			return false;
		}
		
		cell = map.mapCells[row + 1][col - 1];
		if (cell != null && !cell.enterable()) {
			return false;
		}
		
		cell = map.mapCells[row - 1][col + 1];
		if (cell != null && !cell.enterable()) {
			return false;
		}
		return true;
	}
	
	private boolean wallOnAnySide(int row, int col) {
		Cell cell = map.mapCells[row + 1][col];
		if (cell != null && !cell.enterable()) {
			return true;
		}
		
		cell = map.mapCells[row][col + 1];
		if (cell != null && !cell.enterable()) {
			return true;
		}
		
		cell = map.mapCells[row - 1][col];
		if (cell != null && !cell.enterable()) {
			return true;
		}
		
		cell = map.mapCells[row][col - 1];
		if (cell != null && !cell.enterable()) {
			return true;
		}
		return false;
	}
	
	private void generateRandomFood() throws SyntaxException {
		if (!map.cellObjectManager.hasTemplatesWithProperty(Names.kPropertyEdible)) {
			throwSyntax("tried to generate random walls with no food types");
		}
		
		for (int row = 1; row < map.size - 1; ++row) {
			for (int col = 1; col < map.size - 1; ++col) {
				if (map.mapCells[row][col] == null) {
					map.mapCells[row][col] = new Cell();
					
				}
				if (map.mapCells[row][col].enterable()) {
					java.awt.Point location = new java.awt.Point(col, row);
					map.removeAllWithProperty(location, Names.kPropertyEdible);
					map.addRandomObjectWithProperty(location, Names.kPropertyEdible);
				}
			}
		}		
	}
	

}


















