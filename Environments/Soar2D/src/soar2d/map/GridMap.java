package soar2d.map;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;

import org.apache.log4j.Logger;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;

import soar2d.Direction;
import soar2d.Names;
import soar2d.Simulation;
import soar2d.Soar2D;
import soar2d.players.Player;
import soar2d.players.RadarCell;
import soar2d.world.TankSoarWorld;

/**
 * @author voigtjr
 *
 * houses the map and associated meta-data. used for grid worlds.
 */
public abstract class GridMap {
	private static Logger logger = Logger.getLogger(GridMap.class);

	public GridMap() {
	}
	
	private static final String kTagMap = "map";
	private static final String kTagCellObject = "cell-object";
	private static final String kTagCells = "cells";
	
	public class LoadError extends Exception {
		static final long serialVersionUID = 1;
		
		public LoadError(String message) {
			super(message);
		}
	}
	
	public void load() throws LoadError {
		// one load call per object
		assert mapCells == null;
		
		File mapFile = new File(Soar2D.config.generalConfig().map);
		if (!mapFile.exists()) {
			throw new LoadError("Map file doesn't exist: " + mapFile.getAbsolutePath());
		}
		
		try {
			SAXBuilder builder = new SAXBuilder();
			Document doc = builder.build(mapFile);
			Element root = doc.getRootElement();
			if (root == null || !root.getName().equalsIgnoreCase(kTagMap)) {
				throw new LoadError("Couldn't find map tag in map file.");
			}
			
			List<Element> children = (List<Element>)root.getChildren();
			Iterator<Element> iter = children.iterator();
			while (iter.hasNext()) {
				Element child = iter.next();
				
				if (child.getName().equalsIgnoreCase(kTagCellObject)) {
					cellObject(child);
				} else if (child.getName().equalsIgnoreCase(kTagCells)) {
					cells(child);
				} else if (child.getName().equalsIgnoreCase(kTagMetadata)) {
					setMetadata(new File(child.getTextTrim()));
				} else {
					throw new LoadError("unrecognized tag: " + child.getName());
				}
			}
			
		} catch (IOException e) {
			throw new LoadError("I/O exception: " + e.getMessage());
		} catch (JDOMException e) {
			throw new LoadError("Error during parsing: " + e.getMessage());
		} catch (IllegalStateException e) {
			throw new LoadError("Illegal state: " + e.getMessage());
		}
		
		// Build cell reference map
		int [] neighborLoc = new int [2];
		for (int x = 0; x < size; ++x) {
			for (int y = 0; y < size; ++y) {
				Cell cell = getCell(new int[] { x, y });
				for (int direction = 1; direction < 5; ++direction) {
					neighborLoc[0] = x + Direction.xDelta[direction];
					neighborLoc[1] = y + Direction.yDelta[direction];
					if (isInBounds(neighborLoc)) {
						Cell neighbor = getCell(neighborLoc);
						cell.neighbors[direction] = neighbor;
					}
				}
			}
		}
	}
	
	// metadata file
	private File metadata;
	private static final String kTagMetadata = "metadata";
	public void setMetadata(File metadata) {
		this.metadata = metadata.getAbsoluteFile();
	}
	public File getMetadata() {
		return this.metadata;
	}

	public String generateXMLString() {
		Element root = new Element(kTagMap);
		
		Iterator<CellObject> iter = this.cellObjectManager.getTemplates().iterator();
		while (iter.hasNext()) {
			CellObject template = iter.next();

			Element cellObject = new Element(kTagCellObject);
			cellObjectSave(cellObject, template);
			root.addContent(cellObject);
		}
		
		Element cells = new Element(kTagCells);
		cellsSave(cells);
		root.addContent(cells);

		if (this.getMetadata() != null) {
			root.addContent(new Element(kTagMetadata).setText(getMetadata().getPath()));
		}

		XMLOutputter out = new XMLOutputter(Format.getPrettyFormat());
		return out.outputString(root);
	}
	
	private static final String kTagProperty = "property";
	private static final String kTagApply = "apply";
	private static final String kTagUpdate = "update";
	
	private static final String kAttrName = "name";
	private static final String kAttrValue = "value";
	
	CellObjectManager cellObjectManager = new CellObjectManager(); // the types of objects on this map
	public CellObjectManager getObjectManager() {
		return cellObjectManager;
	}

	private void cellObjectSave(Element cellObject, CellObject template) {
		cellObject.setAttribute(kAttrName, template.getName());
		
		Iterator<String> iter = template.getPropertyNames().iterator();
		while (iter.hasNext()) {
			String name = iter.next();
			String value = template.getProperty(name);
			
			Element property = new Element(kTagProperty);
			property.setAttribute(kAttrName, name);
			property.setAttribute(kAttrValue, value);

			cellObject.addContent(property);
		}
		
		if (template.applyable()) {
			Element apply = new Element(kTagApply);
			applySave(apply, template);
			cellObject.addContent(apply);
		}
		
		if (template.updatable()) {
			Element update = new Element(kTagUpdate);
			updateSave(update, template);
			cellObject.addContent(update);
		}
	}
	
	private void cellObject(Element cellObject) throws LoadError {
		String name = cellObject.getAttributeValue(kAttrName);
		if (name == null || name.length() <= 0) {
			throw new LoadError("cell-object must have name");
		}
		
		CellObject template = new CellObject(name);

		List<Element> children = (List<Element>)cellObject.getChildren();
		Iterator<Element> iter = children.iterator();
		while (iter.hasNext()) {
			Element child = iter.next();
			
			if (child.getName().equalsIgnoreCase(kTagProperty)) {
				property(child, template, false);
				
			} else if (child.getName().equalsIgnoreCase(kTagApply)) {
				apply(child, template);

			} else if (child.getName().equalsIgnoreCase(kTagUpdate)) {
				update(child, template);

			} else {
				throw new LoadError("Unrecognized tag: " + child.getName());
			}
		}		
		
		cellObjectManager.registerTemplate(template);
	}
	
	private void property(Element property, CellObject template, boolean apply) throws LoadError {
		String name = property.getAttributeValue(kAttrName);
		if (name == null || name.length() <= 0) {
			throw new LoadError("property must have name");
		}
		
		String value = property.getAttributeValue(kAttrValue);
		if (value == null || value.length() <= 0) {
			throw new LoadError("property must have value");
		}
		
		if (apply) {
			template.addPropertyApply(name, value);
		} else {
			template.addProperty(name, value);
		}
	}
	
	private static final String kTagPoints = "points";
	private static final String kTagEnergy = "energy";
	private static final String kTagHealth = "health";
	private static final String kTagMissiles = "missiles";
	private static final String kTagRemove = "remove";
	private static final String kTagReward = "reward";
	private static final String kTagRewardInfo = "reward-info";
	private static final String kTagUseOpenCode = "use-open-code";
	private static final String kTagReset = "reset";
	private static final String kTagFuel = "fuel";

	private void applySave(Element apply, CellObject template) {
		Iterator<String> iter = template.propertiesApply.keySet().iterator();
		while (iter.hasNext()) {
			
			String name = iter.next();
			String value = template.getPropertyApply(name);

			Element property = new Element(kTagProperty);
			property.setAttribute(kAttrName, name);
			property.setAttribute(kAttrValue, value);
			
			apply.addContent(property);
		}
		
		if (template.pointsApply) {
			apply.addContent(new Element(kTagPoints));
		}

		if (template.energyApply) {
			apply.addContent(new Element(kTagEnergy)).setAttribute(kAttrShields, Boolean.toString(template.energyApplyShieldsUp));
		}

		if (template.healthApply) {
			apply.addContent(new Element(kTagHealth)).setAttribute(kAttrShieldsDown, Boolean.toString(template.healthApplyShieldsDown));
		}

		if (template.missilesApply) {
			apply.addContent(new Element(kTagMissiles));
		}

		if (template.removeApply) {
			apply.addContent(new Element(kTagRemove));
		}
		
		if (template.rewardApply > 0) {
			apply.addContent(new Element(kTagReward)).setAttribute(kAttrQuantity, Integer.toString(template.rewardApply));
		}
		
		if (template.rewardInfoApply) {
			if (this.openCode != 0) {
				apply.addContent(new Element(kTagRewardInfo).addContent(new Element(kTagUseOpenCode)));
			} else {
				apply.addContent(new Element(kTagRewardInfo));
			}
		}
		
		if (template.resetApply) {
			apply.addContent(new Element(kTagReset));
		}
		
		if (template.fuelApply) {
			apply.addContent(new Element(kTagFuel));
		}
	}

	private void apply(Element apply, CellObject template) throws LoadError {
		List<Element> children = (List<Element>)apply.getChildren();
		Iterator<Element> iter = children.iterator();
		while (iter.hasNext()) {
			Element child = iter.next();
			
			if (child.getName().equalsIgnoreCase(kTagProperty)) {
				property(child, template, true);
				
			} else if (child.getName().equalsIgnoreCase(kTagPoints)) {
				template.setPointsApply(true);

			} else if (child.getName().equalsIgnoreCase(kTagEnergy)) {
				energy(child, template);

			} else if (child.getName().equalsIgnoreCase(kTagHealth)) {
				health(child, template);
				
			} else if (child.getName().equalsIgnoreCase(kTagMissiles)) {
				template.setMissilesApply(true);

			} else if (child.getName().equalsIgnoreCase(kTagRemove)) {
				template.setRemoveApply(true);

			} else if (child.getName().equalsIgnoreCase(kTagRewardInfo)) {
				template.setRewardInfoApply(true);
				if (child.getChild(kTagUseOpenCode) != null) {
					openCode = Simulation.random.nextInt(kOpenCodeRange) + 1;
				}

			} else if (child.getName().equalsIgnoreCase(kTagReward)) {
				reward(child, template);

			} else if (child.getName().equalsIgnoreCase(kTagReset)) {
				template.setResetApply(true);

			} else if (child.getName().equalsIgnoreCase(kTagFuel)) {
				template.setFuelApply(true);

			} else {
				throw new LoadError("Unrecognized tag: " + child.getName());
			}
		}		
	}
	
	private static final String kAttrShields = "shields";
	
	private void energy(Element energy, CellObject template) {
		boolean shields = Boolean.parseBoolean(energy.getAttributeValue(kAttrShields, "false"));
		template.setEnergyApply(true, shields);
	}

	private static final String kAttrShieldsDown = "shields-down";

	private void health(Element health, CellObject template) {
		boolean shieldsDown = Boolean.parseBoolean(health.getAttributeValue(kAttrShieldsDown, "false"));
		template.setHealthApply(true, shieldsDown);
	}

	private static final String kAttrQuantity = "quantity";

	private void reward(Element reward, CellObject template) throws LoadError {
		String quantityString = reward.getAttributeValue(kAttrQuantity);
		if (quantityString == null) {
			throw new LoadError("No reward quantity specified.");
		}
		try {
			template.setRewardApply(Integer.parseInt(quantityString));
		} catch (NumberFormatException e) {
			throw new LoadError("Invalid reward quantity specified.");
		}
	}
	
	private static final String kTagDecay = "decay";
	private static final String kTagFlyMissile = "fly-missile";
	private static final String kTagLinger = "linger";

	private void updateSave(Element update, CellObject template) {
		if (template.decayUpdate) {
			update.addContent(new Element(kTagDecay));
		}
		
		if (template.flyMissileUpdate) {
			update.addContent(new Element(kTagFlyMissile));
		}
		
		if (template.lingerUpdate) {
			update.addContent(new Element(kTagLinger));
		}
	}

	private void update(Element update, CellObject template) throws LoadError {
		List<Element> children = (List<Element>)update.getChildren();
		Iterator<Element> iter = children.iterator();
		while (iter.hasNext()) {
			Element child = iter.next();
			
			if (child.getName().equalsIgnoreCase(kTagDecay)) {
				template.setDecayUpdate(true);
				
			} else if (child.getName().equalsIgnoreCase(kTagFlyMissile)) {
				template.setFlyMissileUpdate(true);

			} else if (child.getName().equalsIgnoreCase(kTagLinger)) {
				template.setLingerUpdate(true);

			} else {
				throw new LoadError("Unrecognized tag: " + child.getName());
			}
		}		
	}

	protected int size = 0;	// the maps are square, this is the number of row/columns
	public int getSize() {
		return this.size;
	}
	public void setSize(int size) {
		this.size = size;
	}
	
	private boolean randomWalls = false;
	public boolean getRandomWalls() {
		return randomWalls;
	}
	public void setRandomWalls(boolean randomWalls) {
		this.randomWalls = randomWalls;
	}

	private boolean randomFood = false;
	public boolean getRandomFood() {
		return randomFood;
	}
	public void setRandomFood(boolean randomFood) {
		this.randomFood = randomFood;
	}

	private Cell[][] mapCells = null;	// the cells
	Cell getCell(int [] location) {
		return mapCells[location[1]][location[0]];
	}
	
	private static final String kTagRow = "row";
	private static final String kTagCell = "cell";
	
	private static final String kAttrWorldSize = "world-size";
	private static final String kAttrRandomWalls = "random-walls";
	private static final String kAttrRandomFood = "random-food";
	
	private void cellsSave(Element cells) {
		cells.setAttribute(kAttrWorldSize, Integer.toString(this.size));
		cells.setAttribute(kAttrRandomWalls, Boolean.toString(randomWalls));
		cells.setAttribute(kAttrRandomFood, Boolean.toString(randomFood));
		
		if (randomWalls && randomFood) {
			return;
		}
		
		for(int rowIndex = 0; rowIndex < this.size; ++rowIndex) {
			Element row = new Element(kTagRow);
			
			for (int colIndex = 0; colIndex < this.size; ++colIndex) {
				Element cell = new Element(kTagCell);
				cellSave(cell, this.mapCells[rowIndex][colIndex]);
				row.addContent(cell);
			}
			cells.addContent(row);
		}
	}
	
	CellObject rewardInfoObject = null;
	static final int kOpenCodeRange = 2; // 1..kOpenCodeRange (alternatively expressed as: 0..(kOpenCodeRange - 1) + 1
	// If this is not zero, we are requiring an open code.
	int openCode = 0;
	public int getOpenCode() {
		return openCode;
	}
	int positiveRewardID = 0;
	
	public boolean mapUsesOpenCode() {
		return rewardInfoObject.hasProperty(Names.kPropertyOpenCode);
	}
	
	private void cells(Element cells) throws LoadError {
		String sizeString = cells.getAttributeValue(kAttrWorldSize);
		if (sizeString == null || sizeString.length() <= 0) {
			throw new LoadError("world-size required with cells tag");
		}
		
		try {
			this.size = Integer.parseInt(sizeString);
		} catch (NumberFormatException e) {
			throw new LoadError("error parsing world-size");
		}
		
		randomWalls = Boolean.parseBoolean(cells.getAttributeValue(kAttrRandomWalls, "false"));
		randomFood = Boolean.parseBoolean(cells.getAttributeValue(kAttrRandomFood, "false"));
		
		// Generate map from XML unless both are true
		if (!randomWalls || !randomFood) {
			if (cells.getChildren().size() != this.size) {
				throw new LoadError("there does not seem to be the " +
						"correct amount of row tags (" + cells.getChildren().size() +
						") for the specified map size (" + this.size + ")");
			}
			
			this.mapCells = new Cell[this.size][];
			
			List<Element> children = (List<Element>)cells.getChildren();
			Iterator<Element> iter = children.iterator();
			int rowIndex = 0;
			while (iter.hasNext()) {
				Element child = iter.next();

				if (!child.getName().equalsIgnoreCase(kTagRow)) {
					throw new LoadError("unrecognized tag: " + child.getName());
				}
				
				this.mapCells[rowIndex] = new Cell[this.size];
				row(child, rowIndex);
				
				rowIndex += 1;
			}
		}
		
		// override walls if necessary
		if (randomWalls) {
			generateRandomWalls();
		}
		
		// override food if necessary
		if (randomFood) {
			generateRandomFood();
		}
		
		// pick positive box
		if (rewardInfoObject != null) {
			assert positiveRewardID == 0;
			positiveRewardID = Simulation.random.nextInt(cellObjectManager.rewardObjects.size());
			positiveRewardID += 1;
			rewardInfoObject.addPropertyApply(Names.kPropertyPositiveBoxID, Integer.toString(positiveRewardID));
			
			// assigning colors like this helps us see the task happen
			// colors[0] = info box (already assigned)
			// colors[1] = positive reward box
			// colors[2..n] = negative reward boxes
			Iterator<CellObject> iter = cellObjectManager.rewardObjects.iterator();
			int negativeColor = 2;
			while (iter.hasNext()) {
				CellObject aBox = iter.next();
				if (aBox.getIntProperty(Names.kPropertyBoxID) == positiveRewardID) {
					aBox.addProperty(Names.kPropertyColor, Soar2D.simulation.kColors[1]);
				} else {
					aBox.addProperty(Names.kPropertyColor, Soar2D.simulation.kColors[negativeColor]);
					negativeColor += 1;
					assert negativeColor < Soar2D.simulation.kColors.length;
				}
			}

			// if using an open code, assign that
			if (this.openCode != 0) {
				rewardInfoObject.addPropertyApply(Names.kPropertyOpenCode, Integer.toString(openCode));
			}
		}
	}
	
	private void row(Element row, int rowIndex) throws LoadError {
		if (row.getChildren().size() != this.size) {
			throw new LoadError("there does not seem to be the " +
					"correct amount of cell tags (" + row.getChildren().size() +
					") for the specified map size (" + this.size + ")");
		}
		
		
		List<Element> children = (List<Element>)row.getChildren();
		Iterator<Element> iter = children.iterator();
		int colIndex = 0;
		while (iter.hasNext()) {
			Element child = iter.next();

			if (!child.getName().equalsIgnoreCase(kTagCell)) {
				throw new LoadError("unrecognized tag: " + child.getName());
			}
			
			this.mapCells[rowIndex][colIndex] = new Cell(colIndex, rowIndex);
			cell(child, new int [] { colIndex, rowIndex });
			
			colIndex += 1;
		}
	}
	
	protected static final String kTagObject = "object";
	
	private void cellSave(Element cell, Cell theCell) {
		for (CellObject object : theCell.getAll()) {
			cell.addContent(new Element(kTagObject).setText(object.getName()));
		}
	}
	
	protected void cell(Element cell, int [] location) throws LoadError {
		List<Element> children = (List<Element>)cell.getChildren();
		Iterator<Element> iter = children.iterator();
		while (iter.hasNext()) {
			Element child = iter.next();
			
			if (!child.getName().equalsIgnoreCase(kTagObject)) {
				throw new LoadError("unrecognized tag: " + child.getName());
			}
			
			object(child, location);
		}
	}
	
	protected boolean object(Element object, int [] location) throws LoadError {
		String name = object.getTextTrim();
		if (name.length() <= 0) {
			throw new LoadError("object doesn't have name");
		}
		
		if (!cellObjectManager.hasTemplate(name)) {
			throw new LoadError("object \"" + name + "\" does not map to a cell object");
		}
		
		CellObject cellObject = cellObjectManager.createObject(name);
		
		// false for all but tanksoar, tanksoar checks
		addObjectToCell(location, cellObject);

		if (cellObject.rewardInfoApply) {
			assert rewardInfoObject == null;
			rewardInfoObject = cellObject;
		}
		
		// return used only by tanksoarmap
		return false;
	}

	private void generateRandomWalls() throws LoadError {
		if (!cellObjectManager.hasTemplatesWithProperty(Names.kPropertyBlock)) {
			throw new LoadError("tried to generate random walls with no blocking types");
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
				mapCells[row][0] = new Cell(0, row);
			}
			addWallAndRemoveFood(new int [] { 0, row });
			if (mapCells[row][size - 1] == null) {
				mapCells[row][size - 1] = new Cell(size - 1, row);
			}
			addWallAndRemoveFood(new int [] { size - 1, row });
		}
		for (int col = 1; col < size - 1; ++col) {
			if (mapCells[0][col] == null) {
				mapCells[0][col] = new Cell(col, 0);
			}
			addWallAndRemoveFood(new int [] { col, 0 });
			if (mapCells[size - 1][col] == null) {
				mapCells[size - 1][col] = new Cell(col, size - 1);
			}
			addWallAndRemoveFood(new int [] { col, size - 1 });
		}
		
		double probability = Soar2D.config.eatersConfig().low_probability;
		for (int row = 2; row < size - 2; ++row) {
			for (int col = 2; col < size - 2; ++col) {
				if (noWallsOnCorners(row, col)) {
					if (wallOnAnySide(row, col)) {
						probability = Soar2D.config.eatersConfig().high_probability;					
					}
					if (Simulation.random.nextDouble() < probability) {
						if (mapCells[row][col] == null) {
							mapCells[row][col] = new Cell(col, row);
						}
						addWallAndRemoveFood(new int [] { col, row });
					}
					probability = Soar2D.config.eatersConfig().low_probability;
				}
			}
		}
	}
	
	private void addWallAndRemoveFood(int [] location) {
		removeAllByProperty(location, Names.kPropertyEdible);
		
		if (!hasAnyWithProperty(location, Names.kPropertyBlock)) {
			addObjectToCell(location, createRandomObjectWithProperty(Names.kPropertyBlock));
		}
	}
	
	private boolean noWallsOnCorners(int row, int col) {
		Cell cell = mapCells[row + 1][col + 1];
		if (cell != null && cell.hasAnyWithProperty(Names.kPropertyBlock)) {
			return false;
		}
		
		cell = mapCells[row - 1][col - 1];
		if (cell != null && cell.hasAnyWithProperty(Names.kPropertyBlock)) {
			return false;
		}
		
		cell = mapCells[row + 1][col - 1];
		if (cell != null && cell.hasAnyWithProperty(Names.kPropertyBlock)) {
			return false;
		}
		
		cell = mapCells[row - 1][col + 1];
		if (cell != null && cell.hasAnyWithProperty(Names.kPropertyBlock)) {
			return false;
		}
		return true;
	}
	
	private boolean wallOnAnySide(int row, int col) {
		Cell cell = mapCells[row + 1][col];
		if (cell != null && cell.hasAnyWithProperty(Names.kPropertyBlock)) {
			return true;
		}
		
		cell = mapCells[row][col + 1];
		if (cell != null && cell.hasAnyWithProperty(Names.kPropertyBlock)) {
			return true;
		}
		
		cell = mapCells[row - 1][col];
		if (cell != null && cell.hasAnyWithProperty(Names.kPropertyBlock)) {
			return true;
		}
		
		cell = mapCells[row][col - 1];
		if (cell != null && cell.hasAnyWithProperty(Names.kPropertyBlock)) {
			return true;
		}
		return false;
	}
	
	private void generateRandomFood() throws LoadError {
		if (!cellObjectManager.hasTemplatesWithProperty(Names.kPropertyEdible)) {
			throw new LoadError("tried to generate random walls with no food types");
		}
		
		for (int row = 1; row < size - 1; ++row) {
			for (int col = 1; col < size - 1; ++col) {
				if (mapCells[row][col] == null) {
					mapCells[row][col] = new Cell(col, row);
					
				}
				if (!mapCells[row][col].hasAnyWithProperty(Names.kPropertyBlock)) {
					int [] location = new int [] { col, row };
					removeAllByProperty(location, Names.kPropertyEdible);
					addObjectToCell(location, createRandomObjectWithProperty(Names.kPropertyEdible));
				}
			}
		}		
	}

	HashSet<CellObject> updatables = new HashSet<CellObject>();
	HashMap<CellObject, int []> updatablesLocations = new HashMap<CellObject, int []>();
	
	public class BookObjectInfo {
		public CellObject object;
		public int [] location;
		public double [] floatLocation;
		public int area = -1;
	}
	
	public void addObjectToCell(int [] location, CellObject object) {
		removeObject(location, object.getName());
		addStateUpdate(location, object);

		Cell cell = getCell(location);
		cell.addCellObject(object);
	}
	
	public CellObject createObjectByName(String name) {
		return cellObjectManager.createObject(name);
	}
	
	public CellObject createRandomObjectWithProperty(String property) {
		return cellObjectManager.createRandomObjectWithProperty(property);
	}

	public CellObject createRandomObjectWithProperties(String property1, String property2) {
		return cellObjectManager.createRandomObjectWithProperties(property1, property2);
	}
	
	public void updateObjects(TankSoarWorld tsWorld) {
		HashSet<CellObject> copy = new HashSet<CellObject>(updatables);
		for (CellObject cellObject : copy) {
			int [] location = updatablesLocations.get(cellObject);
			
			if (cellObject.update(location)) {
				removalStateUpdate(getCell(location).removeObject(cellObject.getName()));
			}
		}
	}

	public void setPlayer(int [] location, Player player) {
		getCell(location).setPlayer(player);
	}
	
	public int pointsCount(int [] location) {
		Cell cell = getCell(location);
		ArrayList<CellObject> list = cell.getAllWithProperty(Names.kPropertyEdible);
		if (list == null) {
			return 0;
		}
		int count = 0;
		for (CellObject object : list) {
			count += object.getIntProperty(Names.kPropertyPoints);
		}
		return count;
	}

	public abstract boolean isAvailable(int [] location);
	
	public boolean exitable(int [] location, int direction) {
		ArrayList<CellObject> wallList = getAllWithProperty(location, "block");
		if (wallList != null) {
			for (CellObject object : wallList) {
				if (direction == Direction.getInt(object.getProperty("direction"))) {
					return false;
				}
			}
		}
		return true;
	}
	
	public CellObject removeObject(int [] location, String objectName) {
		CellObject object = getCell(location).removeObject(objectName);
		if (object == null) return null;
		removalStateUpdate(object);
		return object;
	}
	
	public Player getPlayer(int [] location) {
		return getCell(location).getPlayer();
	}
	
	public boolean hasObject(int [] location, String name) {
		return getCell(location).hasObject(name);
	}
	
	public CellObject getObject(int [] location, String name) {
		return getCell(location).getObject(name);
	}
	
	public void handleIncoming() {
		// TODO: a couple of optimizations possible here
		// like marking cells that have been checked, depends on direction though
		// probably more work than it is worth as this should only be slow when there are
		// a ton of missiles flying
		
		for (CellObject missile : updatables) {
			if (!missile.hasProperty(Names.kPropertyMissile)) {
				continue;
			}
	
			Cell threatenedCell = getCell(updatablesLocations.get(missile));
			while (true) {
				int direction = missile.getIntProperty(Names.kPropertyDirection);
				threatenedCell = threatenedCell.neighbors[direction];
				
				// stops at wall
				if (threatenedCell.hasAnyWithProperty(Names.kPropertyBlock)) {
					break;
				}
				
				Player player = threatenedCell.getPlayer();
				if (player != null) {
					player.setIncoming(Direction.backwardOf[direction]);
					break;
				}
			}
		}
	}
	
	void addStateUpdate(int [] location, CellObject added) {
		assert added != null;
		if (added.updatable()) {
			updatables.add(added);
			updatablesLocations.put(added, location);
		}
	}
	
	void removalStateUpdate(CellObject removed) {
		if (removed == null) {
			return;
		}
		if (removed.updatable()) {
			updatables.remove(removed);
			updatablesLocations.remove(removed);
		}
	}
	
	public void setExplosion(int [] location) {
	}
	
	public ArrayList<CellObject> getAllWithProperty(int [] location, String name) {
		return getCell(location).getAllWithProperty(name);
	}
	
	public boolean hasAnyWithProperty(int [] location, String name) {
		return getCell(location).hasAnyWithProperty(name);
	}
	
	public void removeAll(int [] location) {
		ArrayList<CellObject> removed = getCell(location).removeAll();
		for (CellObject object : removed) {
			removalStateUpdate(object);
		}
	}
	
	public void removeAllByProperty(int [] location, String name) {
		ArrayList<CellObject> removed = getCell(location).removeAllByProperty(name);
		if (removed == null) {
			return;
		}
		for (CellObject object : removed) {
			removalStateUpdate(object);
		}
	}
	
	boolean isUnopenedBox(CellObject object) {
		if (object.hasProperty(Names.kPropertyBox)) {
			String status = object.getProperty(Names.kPropertyStatus);
			if (status == null || !status.equals(Names.kOpen)) {
				return true;
			}
		}
		return false;
	}
	
	public void shutdown() {
		mapCells = null;
		cellObjectManager = null;
		size = 0;
	}

	public int getRadar(RadarCell[][] radar, int [] location, int facing, int radarPower) {
		if (radarPower == 0) {
			return 0;
		}
		
		assert radar.length == 3;

		int distance = 0;
		
		distance = radarProbe(radar, location, facing, distance, radarPower);
		
		return distance;
	}
	
	private RadarCell getRadarCell(int [] location) {
		// TODO: cache these each frame!!
		
		Cell cell;
		RadarCell radarCell;

		cell = getCell(location);
		radarCell = new RadarCell();
		radarCell.player = cell.getPlayer();
		if (!cell.hasAnyWithProperty(Names.kPropertyBlock)) {
			for (CellObject object : cell.getAllWithProperty(Names.kPropertyMiniImage)) {
				if (object.getName().equals(Names.kEnergy)) {
					radarCell.energy = true;
				} else if (object.getName().equals(Names.kHealth)) {
					radarCell.health = true;
				} else if (object.getName().equals(Names.kMissiles)) {
					radarCell.missiles = true;
				} 
			}
		} else {
			radarCell.obstacle = true;
		}
		return radarCell;
	}
	
	private int radarProbe(RadarCell[][] radar, int [] myLocation, int facing, int distance, int maxDistance) {
		assert maxDistance < radar[1].length;
		assert distance >= 0;
		assert distance + 1 < radar[1].length;
		assert distance < maxDistance;
		assert facing > 0;
		assert facing < 5;
		
		int [] location;
		
		location = Arrays.copyOf(myLocation, myLocation.length);
		Direction.translate(location, Direction.leftOf[facing]);
		radar[0][distance] = getRadarCell(location);
		if (radar[0][distance].player != null) {
			if (distance != 0) {
				radar[0][distance].player.radarTouch(Direction.backwardOf[facing]);
			} else {
				radar[0][distance].player.radarTouch(Direction.rightOf[facing]);
			}
		}
		
		location = Arrays.copyOf(myLocation, myLocation.length);
		Direction.translate(location, Direction.rightOf[facing]);
		radar[2][distance] = getRadarCell(location);
		if (radar[2][distance].player != null) {
			if (distance != 0) {
				radar[2][distance].player.radarTouch(Direction.backwardOf[facing]);
			} else {
				radar[2][distance].player.radarTouch(Direction.leftOf[facing]);
			}
		}

		distance += 1;

		location = Arrays.copyOf(myLocation, myLocation.length);
		Direction.translate(location, facing);
		radar[1][distance] = getRadarCell(location);
		if (radar[1][distance].player != null) {
			radar[1][distance].player.radarTouch(Direction.backwardOf[facing]);
		}

		boolean enterable = radar[1][distance].obstacle == false;
		boolean noPlayer = radar[1][distance].player == null;
		
		if (enterable && noPlayer) {
			CellObject radarWaves = new CellObject("radar-" + facing);
			radarWaves.addProperty(Names.kPropertyRadarWaves, "true");
			radarWaves.addProperty(Names.kPropertyDirection, Integer.toString(facing));
			radarWaves.addProperty(Names.kPropertyLinger, "1");
			radarWaves.setLingerUpdate(true);
			//System.out.println("Adding radar waves to " + location);
			addObjectToCell(location, radarWaves);
		}

		if (distance == maxDistance) {
			return distance;
		}
		
		if (enterable && noPlayer) {
			return radarProbe(radar, location, facing, distance, maxDistance);
		}
		
		return distance;
	}

	public int getBlocked(int [] location) {
		Cell cell;
		int blocked = 0;
		
		cell = getCell(new int [] { location[0]+1, location[1] });
		if (cell.hasAnyWithProperty(Names.kPropertyBlock) || cell.getPlayer() != null) {
			blocked |= Direction.kEastIndicator;
		}
		cell = getCell(new int [] { location[0]-1, location[1] });
		if (cell.hasAnyWithProperty(Names.kPropertyBlock) || cell.getPlayer() != null) {
			blocked |= Direction.kWestIndicator;
		}
		cell = getCell(new int [] { location[0], location[1]+1 });
		if (cell.hasAnyWithProperty(Names.kPropertyBlock) || cell.getPlayer() != null) {
			blocked |= Direction.kSouthIndicator;
		}
		cell = getCell(new int [] { location[0], location[1]-1 });
		if (cell.hasAnyWithProperty(Names.kPropertyBlock) || cell.getPlayer() != null) {
			blocked |= Direction.kNorthIndicator;
		}
		return blocked;
	}
	
	public boolean isBlocked(int [] location) {
		Cell cell = getCell(location);
		return cell.hasAnyWithProperty(Names.kPropertyBlock) || cell.getPlayer() != null;
	}
	
	public int getSoundNear(int [] location) {
		if (Soar2D.simulation.world.getPlayers().numberOfPlayers() < 2) {
			return 0;
		}
		
		// Set all cells unexplored.
		for(Cell[] cols : mapCells) {
			for (Cell cell : cols) {
				cell.explored = false;
			}
		}
		
		LinkedList<Cell> searchList = new LinkedList<Cell>();
		{
			Cell start = getCell(location);
			start.explored = true;
			start.distance = 0;
			start.parent = null;
			searchList.addLast(start);
		}
		
		int finalDirection = 0;
		
		while (searchList.size() > 0) {
			Cell parentCell = searchList.getFirst();
			searchList.removeFirst();

			if (logger.isTraceEnabled()) {
				logger.trace("Sound: new parent " + parentCell);
			}
			
			// subtract 1 because we add one later (exploring neighbors)
			if (parentCell.distance >= Soar2D.config.tanksoarConfig().max_sound_distance) {
				if (logger.isTraceEnabled()) {
					logger.trace("Sound: parent distance " + parentCell.distance + " is too far");
				}
				continue;
			}

			// Explore cell.
			for (int i = 1; i < 5; ++i) {
				Cell neighbor = parentCell.neighbors[i];
				if (neighbor == null) {
					continue;
				}

				if (neighbor.explored) {
					continue;
				}

				if (logger.isTraceEnabled()) {
					logger.trace("Sound: exploring " + neighbor);
				}
				neighbor.explored = true;
				
				if (neighbor.hasAnyWithProperty(Names.kPropertyBlock)) {
					logger.trace("Sound: blocked");
					continue;
				}
							
				neighbor.distance = parentCell.distance + 1;
				
				if (logger.isTraceEnabled()) {
					logger.trace("Sound: distance " + neighbor.distance);
				}
				
				Player targetPlayer = neighbor.getPlayer();
				if ((targetPlayer != null) && Soar2D.simulation.world.recentlyMovedOrRotated(targetPlayer)) {
					if (logger.isTraceEnabled()) {
						logger.trace("Sound: found recently moved player " + targetPlayer.getName());
					}
					
					// found a sound! walk home
					// I'm its parent, so see if I'm the top here
					while(parentCell.parent != null) {
						// the new cell becomes me
						neighbor = parentCell;
						
						// I become my parent
						parentCell = parentCell.parent;
					}

					// Find direction to new sound
					for (finalDirection = 1; i < 5; ++finalDirection) {
						if (neighbor == parentCell.neighbors[finalDirection]) {
							break;
						}
					}
					
					// shouldn't happen
					if (finalDirection < 5) {
						if (logger.isTraceEnabled()) {
							logger.trace("Sound: done, originated from " + Direction.stringOf[finalDirection]);
						}
					} else {
						// didn't find direction to new sound
						logger.trace("Sound: error: didn't find direction to sound!");
						assert false;
						finalDirection = 0;
					}
					
				}
				
				// end condition: this is not 0 if we found someone
				if (finalDirection != 0) {
					break;
				}

				neighbor.parent = parentCell;
				
				// add the new cell to the search list
				searchList.addLast(neighbor);
			}
			
			// end condition: this is not 0 if we found someone
			if (finalDirection != 0) {
				break;
			}
		}
		return finalDirection;
	}

	public boolean isInBounds(int [] location) {
		return (location[0] >= 0) && (location[1] >= 0) && (location[0] < getSize()) && (location[1] < getSize());
	}

	public String toString() {
		String output = "";
		for (int y = 0; y < mapCells.length; ++y) {
			for (int x = 0; x < mapCells[y].length; ++x) {
				String cellString = y + "," + x + ":\n";
				Cell cell = mapCells[y][x];
				Iterator<CellObject> iter = cell.getAll().iterator();
				while (iter.hasNext()) {
					CellObject object = iter.next();
					cellString += "\t" + object.getName() + ": ";
					
					Iterator<String> propIter = object.properties.keySet().iterator();
					while (propIter.hasNext()) {
						String key = propIter.next();
						cellString += key + ":" + object.properties.get(key) + ", ";
					}
					cellString += "\n";
				}
				output += cellString;
			}
		}
		return output;
	}
	
	public ArrayList<int []> getAvailableLocationsSlow() {
		ArrayList<int []> availableLocations = new ArrayList<int []>();
		for (int x = 0; x < getSize(); ++x) {
			for (int y = 0; y < getSize(); ++ y) {
				int [] potentialLocation = new int [] { x, y };
				if (isAvailable(potentialLocation)) {
					availableLocations.add(potentialLocation);
				}
			}
		}
		return availableLocations;
	}

	// Assumes walls on borders!
	public int [] getAvailableLocationAmortized() {
		int [] potentialLocation = new int [2];;
		
		// Loop in case there are no free spots, the 100 is totally arbitrary
		for (int counter = 0; counter < 100; ++counter) {
			potentialLocation[0] = Simulation.random.nextInt(size - 2) + 1;
			potentialLocation[1] = Simulation.random.nextInt(size - 2) + 1;
			
			if (isAvailable(potentialLocation)) {
				return potentialLocation;
			}
		}
		ArrayList<int []> locations = getAvailableLocationsSlow();
		if (locations.size() == 0) {
			return null;
		}
		return locations.get(Simulation.random.nextInt(locations.size()));
	}
	
	public boolean resetRedraw(int[] location) {
		return getCell(location).resetRedraw();
	}
	public boolean checkRedraw(int[] location) {
		return getCell(location).checkRedraw();
	}
	public void forceRedraw(int[] location) {
		getCell(location).forceRedraw();
	}
}
