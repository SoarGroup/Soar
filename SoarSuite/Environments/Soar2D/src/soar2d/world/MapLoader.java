package soar2d.world;

import sml.*;
import soar2d.*;

import java.util.*;

class MapLoaderException extends Throwable {
	static final long serialVersionUID = 1;
	private String message;
	
	public MapLoaderException(String message) {
		this.message = "Path: ";
		while (MapLoader.xmlPath.size() > 0) {
			this.message += "<" + (String)MapLoader.xmlPath.pop() + ">";
		}
		this.message += "\nElementXML: " + ElementXML.GetLastParseErrorDescription() + "\n";
		this.message += message;
	}
	
	public String getMessage() {
		return message;
	}
}

class ParseException extends MapLoaderException {
	static final long serialVersionUID = 2;
	public ParseException(String message) {
		super(message);
	}
}

class SMLException extends MapLoaderException {
	static final long serialVersionUID = 3;
	public SMLException(String message) {
		super(message);
	}
}

public class MapLoader {
	static Stack xmlPath;
	private ArrayList foods;
	
	private int size = 0;
	private Cell[][] mapCells = null;
	
	public int getSize() {
		return size;
	}
	
	public Cell[][] getCells() {
		return mapCells;
	}
	
	public boolean load(String mapFile) {
		xmlPath = new Stack();
		
		ElementXML rootTag = ElementXML.ParseXMLFromFile(mapFile);
		if (rootTag == null) {
			Soar2D.control.severeError("Error parsing file: " + ElementXML.GetLastParseErrorDescription());
			return false;
		}
		
		try {
			if (rootTag.IsTag(Names.kTagMap)) {
				xmlPath.push(Names.kTagMap);
				parseMap(rootTag);
				xmlPath.pop();
			} else {
				throw new ParseException("unrecognized tag " + rootTag.GetTagName());
			}
		} catch (ParseException p) {
			this.mapCells = null;
			Soar2D.control.severeError("Error parsing file: " + p.getMessage());
			return false;
			
		} catch (SMLException s) {
			this.mapCells = null;
			Soar2D.control.severeError("SML error during parsing: " + s.getMessage());
			return false;
			
		} finally {
			// BADBAD: call garbage collector because something is lame and leaks
			// and this is the only thing that fixes it
			assert rootTag.GetRefCount() == 1;
			rootTag.ReleaseRefOnHandle();
			rootTag = null;
			System.gc();
		}
		
		// success
		return true;
	}
	private void parseMap(ElementXML mapTag) throws SMLException, ParseException {
		ElementXML mainTag = null;
		for (int mapTagIndex = 0 ; mapTagIndex < mapTag.GetNumberChildren() ; ++mapTagIndex) {

			mainTag = new ElementXML();
			if (mainTag == null) throw new SMLException("couldn't create mainTag");

			try {
				mapTag.GetChild(mainTag, mapTagIndex);
				if (mainTag == null) throw new SMLException("failed to get tag by index");
				
				if (mainTag.IsTag(Names.kTagCellObject)) {
					xmlPath.push(Names.kTagCellObject);
					parseCellObject(mainTag);
					xmlPath.pop();

				} else if (mainTag.IsTag(Names.kTagCells)) {
					xmlPath.push(Names.kTagCellObject);
					parseCells(mainTag);
					xmlPath.pop();
					
				} else {
					throw new ParseException("unrecognized tag " + mainTag.GetTagName());
				}
			} finally {
				mainTag.delete();
				mainTag = null;
			}
		}
	}
	
	private void parseCellObject(ElementXML cellObjectTag) throws SMLException, ParseException {
		String name = null;
		boolean updatable = false;
		boolean consumable = false;

		String attribute = null;
		
		attribute = cellObjectTag.GetAttribute(Names.kParamName);
		if (attribute == null) {
			throw new ParseException("cell-object must have name");
		}
		if (attribute.length() <= 0) {
			throw new ParseException("cell-object name must not be zero-length");
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
			if (cellObjectSubTag == null) throw new SMLException("couldn't create cellObjectSubTag tag");

			try {
				cellObjectTag.GetChild(cellObjectSubTag, cellObjectSubTagIndex);
				if (cellObjectSubTag == null) throw new SMLException("failed to get tag by index");
				
				if (cellObjectSubTag.IsTag(Names.kTagProperty)) {
					xmlPath.push(Names.kTagProperty);
					parseProperty(cellObjectTemplate, cellObjectSubTag);
					xmlPath.pop();
		
				} else if (cellObjectSubTag.IsTag(Names.kTagApply)) {
					xmlPath.push(Names.kTagApply);
					parseApply(cellObjectTemplate, cellObjectSubTag);
					xmlPath.pop();
					
				} else {
					throw new ParseException("unrecognized tag " + cellObjectSubTag.GetTagName());
				}
				
				
			} finally {
				cellObjectSubTag.delete();
				cellObjectSubTag = null;
			}
		}
	}
	
	private void parseProperty(CellObject cellObjectTemplate, ElementXML propertyTag) throws SMLException, ParseException {
		String name = null;
		String value = null;

		String attribute = null;
		
		attribute = propertyTag.GetAttribute(Names.kParamName);
		if (attribute == null) {
			throw new ParseException("property must have a name");
		}
		if (attribute.length() <= 0) {
			throw new ParseException("property name must not be zero-length");
		}
		name = attribute;
		
		attribute = propertyTag.GetAttribute(Names.kParamValue);
		if (attribute == null) {
			throw new ParseException("property must have a value");
		}
		if (attribute.length() <= 0) {
			throw new ParseException("property value must not be zero-length");
		}
		value = attribute;
		
		cellObjectTemplate.addProperty(name, value);
		if (name.equalsIgnoreCase(Names.kPropertyEdible)) {
			if (Boolean.parseBoolean(value)) {
				foods.add(cellObjectTemplate.getName());
			}
		}
	}
	
	private void parseApply(CellObject cellObjectTemplate, ElementXML applyTag) throws SMLException, ParseException {
		
		ElementXML applySubTag = null;
		for (int applySubTagIndex = 0; applySubTagIndex < applyTag.GetNumberChildren(); ++applySubTagIndex) {
			
			applySubTag = new ElementXML();
			if (applySubTag == null) throw new SMLException("couldn't create applySubTag tag");

			try {
				applyTag.GetChild(applySubTag, applySubTagIndex);
				if (applySubTag == null) throw new SMLException("failed to get tag by index");
				
				if (applySubTag.IsTag(Names.kTagPoints)) {
					xmlPath.push(Names.kTagPoints);
					cellObjectTemplate.setPointsApply();
					xmlPath.pop();
		
				} else if (applySubTag.IsTag(Names.kTagProperty)) {
					xmlPath.push(Names.kTagProperty);
					throw new ParseException("tag not implemented");
					//xmlPath.pop();
					
				} else if (applySubTag.IsTag(Names.kTagEnergy)) {
					xmlPath.push(Names.kTagEnergy);
					throw new ParseException("tag not implemented");
					//xmlPath.pop();
					
				} else if (applySubTag.IsTag(Names.kTagHealth)) {
					xmlPath.push(Names.kTagHealth);
					throw new ParseException("tag not implemented");
					//xmlPath.pop();
					
				} else if (applySubTag.IsTag(Names.kTagMissiles)) {
					xmlPath.push(Names.kTagMissiles);
					throw new ParseException("tag not implemented");
					//xmlPath.pop();
					
				} else {
					throw new ParseException("unrecognized tag " + applySubTag.GetTagName());
				}
			} finally {
				applySubTag.delete();
				applySubTag = null;
			}
		}
	}
	
	private void parseCells(ElementXML cellsTag) throws SMLException, ParseException {
		boolean randomWalls = false;
		boolean randomFood = false;
		
		String attribute = null;
		
		attribute = cellsTag.GetAttribute(Names.kParamWorldSize);
		if (attribute == null) {
			throw new ParseException("cells tag must have a world-size parameter");
		}
		if (attribute.length() <= 0) {
			throw new ParseException("world-size parameter must not be zero-length");
		}
		this.size = Integer.parseInt(attribute);
		if (size < 5) {
			throw new ParseException("world-size must be 5 or greater");
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
	
	private void generateMapFromXML(ElementXML cellsTag) throws SMLException, ParseException {
		if (cellsTag.GetNumberChildren() != this.size) {
			throw new ParseException("there does not seem to be the " +
					"correct amount of row tags (" + cellsTag.GetNumberChildren() +
					") for the specified map size (" + this.size +
					")");
		}
		
		this.mapCells = new Cell[this.size][];
		
		ElementXML rowTag = null;
		for (int rowTagIndex = 0; rowTagIndex < cellsTag.GetNumberChildren(); ++rowTagIndex) {
			
			rowTag = new ElementXML();
			if (rowTag == null) throw new SMLException("couldn't create rowTag tag");

			try {
				cellsTag.GetChild(rowTag, rowTagIndex);
				if (rowTag == null) throw new SMLException("failed to get tag by index");
				
				if (rowTag.IsTag(Names.kTagRow)) {
					
					this.mapCells[rowTagIndex] = new Cell[this.size];

					xmlPath.push(Names.kTagRow);
					parseRow(rowTagIndex, rowTag);
					xmlPath.pop();

				} else {
					throw new ParseException("unrecognized tag " + rowTag.GetTagName());
				}
			} finally {
				rowTag.delete();
				rowTag = null;
			}
		}
	}
	
	private void parseRow(int row, ElementXML rowTag) throws SMLException, ParseException {
		if (rowTag.GetNumberChildren() != this.size) {
			throw new ParseException("there does not seem to be the " +
					"correct amount of cell tags (" + rowTag.GetNumberChildren() +
					") for the specified map size (" + this.size +
					") in row " + row);
		}
		
		ElementXML cellTag = null;
		for (int cellTagIndex = 0; cellTagIndex < rowTag.GetNumberChildren(); ++cellTagIndex) {
			
			cellTag = new ElementXML();
			if (cellTag == null) throw new SMLException("couldn't create cellTag tag");

			try {
				rowTag.GetChild(cellTag, cellTagIndex);
				if (cellTag == null) throw new SMLException("failed to get tag by index");
				
				if (cellTag.IsTag(Names.kTagCell)) {
					mapCells[row][cellTagIndex] = new Cell();
					
					xmlPath.push(Names.kTagCell);
					parseCell(mapCells[row][cellTagIndex], cellTag);
					xmlPath.pop();

				} else {
					throw new ParseException("unrecognized tag " + cellTag.GetTagName());
				}
			} finally {
				cellTag.delete();
				cellTag = null;
			}
		}
	}
	
	private void parseCell(Cell cell, ElementXML cellTag) throws SMLException, ParseException {
		ElementXML objectTag = null;
		for (int cellTagIndex = 0; cellTagIndex < cellTag.GetNumberChildren(); ++cellTagIndex) {
			
			objectTag = new ElementXML();
			if (objectTag == null) throw new SMLException("couldn't create cellTag tag");

			try {
				cellTag.GetChild(objectTag, cellTagIndex);
				if (objectTag == null) throw new SMLException("failed to get tag by index");
				
				if (objectTag.IsTag(Names.kTagObject)) {
					xmlPath.push(Names.kTagObject);
					parseObject(cell, objectTag);
					xmlPath.pop();

				} else {
					throw new ParseException("unrecognized tag " + objectTag.GetTagName());
				}
			} finally {
				objectTag.delete();
				objectTag = null;
			}
		}
	}
	
	private void parseObject(Cell cell, ElementXML objectTag) throws SMLException, ParseException {
		String name = objectTag.GetCharacterData();
		if (!CellObject.hasTemplate(name)) {
			throw new ParseException("object \"" + name + "\" does not map to a cell-object");
		}
		
		cell.addCellObject(CellObject.createObject(name));
	}

	private void generateRandomWalls() throws ParseException {
		if (!CellObject.hasTemplate(Names.kWall)) {
			throw new ParseException("tried to generate random walls with no wall type");
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
			mapCells[row][0].addCellObject(CellObject.createObject(Names.kWall));
			if (mapCells[row][size - 1] == null) {
				mapCells[row][size - 1] = new Cell();
			}
			mapCells[row][size - 1].addCellObject(CellObject.createObject(Names.kWall));
		}
		for (int col = 1; col < size - 1; ++col) {
			if (mapCells[0][col] == null) {
				mapCells[0][col] = new Cell();
			}
			mapCells[0][col].addCellObject(CellObject.createObject(Names.kWall));
			if (mapCells[size - 1][col] == null) {
				mapCells[size - 1][col] = new Cell();
			}
			mapCells[size - 1][col].addCellObject(CellObject.createObject(Names.kWall));
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
						mapCells[row][col].addCellObject(CellObject.createObject(Names.kWall));
					}
					probability = Soar2D.config.kLowProbability;
				}
			}
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
					mapCells[row][col].addCellObject(CellObject.createObject((String)foods.get(Simulation.random.nextInt(foods.size()))));
				}
			}
		}		
	}
	

}


















