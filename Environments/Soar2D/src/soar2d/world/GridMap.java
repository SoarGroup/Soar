package soar2d.world;

import java.awt.Point;
import java.awt.geom.Point2D;
import java.io.IOException;
import java.util.*;
import java.util.logging.Logger;

import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;
import org.jdom.output.Format;
import org.jdom.output.XMLOutputter;

import soar2d.*;
import soar2d.Configuration.SimType;
import soar2d.player.*;

/**
 * @author voigtjr
 *
 * houses the map and associated meta-data. used for grid worlds.
 */
public class GridMap {
	public static final Logger logger = Logger.getLogger("soar2d");

	private Configuration config;
	
	public GridMap(Configuration config) {
		this.config = config;
	}
	
	public GridMap(GridMap in) {
		this.size = in.size;
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
		
		if (config == null) {
			throw new LoadError("Configuration not set");
		}
		
		if (!config.getMap().exists()) {
			throw new LoadError("Map file doesn't exist: " + config.getMap().getAbsolutePath());
		}
		
		try {
			SAXBuilder builder = new SAXBuilder();
			Document doc = builder.build(config.getMap());
			Element root = doc.getRootElement();
			if (root == null || !root.getName().equalsIgnoreCase(kTagMap)) {
				throw new LoadError("Couldn't find map tag in map file.");
			}
			
			List children = root.getChildren();
			Iterator iter = children.iterator();
			while (iter.hasNext()) {
				Element child = (Element)iter.next();
				
				if (child.getName().equalsIgnoreCase(kTagCellObject)) {
					cellObject(child);
				} else if (child.getName().equalsIgnoreCase(kTagCells)) {
					cells(child);
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

		List children = cellObject.getChildren();
		Iterator iter = children.iterator();
		while (iter.hasNext()) {
			Element child = (Element)iter.next();
			
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
	}

	private void apply(Element apply, CellObject template) throws LoadError {
		List children = apply.getChildren();
		Iterator iter = children.iterator();
		while (iter.hasNext()) {
			Element child = (Element)iter.next();
			
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
		List children = update.getChildren();
		Iterator iter = children.iterator();
		while (iter.hasNext()) {
			Element child = (Element)iter.next();
			
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

	private int size = 0;	// the maps are square, this is the number of row/columns
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
	Cell getCell(java.awt.Point location) {
		if (location == null) return null;
		assert location.x >= 0;
		assert location.y >= 0;
		assert location.x < size;
		assert location.y < size;
		return mapCells[location.y][location.x];
	}
	
	Cell getCell(int x, int y) {
		assert x >= 0;
		assert y >= 0;
		assert x < size;
		assert y < size;
		return mapCells[y][x];
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
			
			List children = cells.getChildren();
			Iterator iter = children.iterator();
			int rowIndex = 0;
			while (iter.hasNext()) {
				Element child = (Element)iter.next();

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
		
		
		List children = row.getChildren();
		Iterator iter = children.iterator();
		int colIndex = 0;
		while (iter.hasNext()) {
			Element child = (Element)iter.next();

			if (!child.getName().equalsIgnoreCase(kTagCell)) {
				throw new LoadError("unrecognized tag: " + child.getName());
			}
			
			this.mapCells[rowIndex][colIndex] = new Cell();
			cell(child, new java.awt.Point(colIndex, rowIndex));
			
			colIndex += 1;
		}
	}
	
	private static final String kTagObject = "object";
	
	private void cellSave(Element cell, Cell theCell) {
		Iterator<String> iter = theCell.cellObjects.keySet().iterator();
		while (iter.hasNext()) {
			cell.addContent(new Element(kTagObject).setText(iter.next()));
		}
	}
	
	private void cell(Element cell, java.awt.Point location) throws LoadError {
		boolean background = false;
		
		List children = cell.getChildren();
		Iterator iter = children.iterator();
		while (iter.hasNext()) {
			Element child = (Element)iter.next();
			
			if (!child.getName().equalsIgnoreCase(kTagObject)) {
				throw new LoadError("unrecognized tag: " + child.getName());
			}
			
			background = object(child, location);
		}
		
		if (config.getType() == SimType.kTankSoar && !background) {
			// add ground
			CellObject cellObject = cellObjectManager.createObject(Names.kGround);
			addObjectToCell(location, cellObject);
			return;
		}
	}
	
	private boolean object(Element object, java.awt.Point location) throws LoadError {
		String name = object.getTextTrim();
		if (name.length() <= 0) {
			throw new LoadError("object doesn't have name");
		}
		
		if (!cellObjectManager.hasTemplate(name)) {
			throw new LoadError("object \"" + name + "\" does not map to a cell object");
		}
		
		CellObject cellObject = cellObjectManager.createObject(name);
		boolean background = false;
		if (config.getType() == SimType.kTankSoar) {
			if (cellObject.hasProperty(Names.kPropertyBlock) 
					|| (cellObject.getName() == Names.kGround)
					|| (cellObject.hasProperty(Names.kPropertyCharger))) {
				background = true;
			}
		}
		addObjectToCell(location, cellObject);

		if (cellObject.rewardInfoApply) {
			assert rewardInfoObject == null;
			rewardInfoObject = cellObject;
		}
		
		return background;
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
				mapCells[row][0] = new Cell();
			}
			addWallAndRemoveFood(new java.awt.Point(0, row));
			if (mapCells[row][size - 1] == null) {
				mapCells[row][size - 1] = new Cell();
			}
			addWallAndRemoveFood(new java.awt.Point(size - 1, row));
		}
		for (int col = 1; col < size - 1; ++col) {
			if (mapCells[0][col] == null) {
				mapCells[0][col] = new Cell();
			}
			addWallAndRemoveFood(new java.awt.Point(col, 0));
			if (mapCells[size - 1][col] == null) {
				mapCells[size - 1][col] = new Cell();
			}
			addWallAndRemoveFood(new java.awt.Point(col, size - 1));
		}
		
		double probability = config.getLowProbability();
		for (int row = 2; row < size - 2; ++row) {
			for (int col = 2; col < size - 2; ++col) {
				if (noWallsOnCorners(row, col)) {
					if (wallOnAnySide(row, col)) {
						probability = config.getHighProbability();					
					}
					if (Simulation.random.nextDouble() < probability) {
						if (mapCells[row][col] == null) {
							mapCells[row][col] = new Cell();
						}
						addWallAndRemoveFood(new java.awt.Point(col, row));
					}
					probability = config.getLowProbability();
				}
			}
		}
	}
	
	private void addWallAndRemoveFood(java.awt.Point location) {
		removeAllWithProperty(location, Names.kPropertyEdible);
		
		ArrayList<CellObject> walls = getAllWithProperty(location, Names.kPropertyBlock);
		if (walls.size() <= 0) {
			addRandomObjectWithProperty(location, Names.kPropertyBlock);
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
	
	private void generateRandomFood() throws LoadError {
		if (!cellObjectManager.hasTemplatesWithProperty(Names.kPropertyEdible)) {
			throw new LoadError("tried to generate random walls with no food types");
		}
		
		for (int row = 1; row < size - 1; ++row) {
			for (int col = 1; col < size - 1; ++col) {
				if (mapCells[row][col] == null) {
					mapCells[row][col] = new Cell();
					
				}
				if (mapCells[row][col].enterable()) {
					java.awt.Point location = new java.awt.Point(col, row);
					removeAllWithProperty(location, Names.kPropertyEdible);
					addRandomObjectWithProperty(location, Names.kPropertyEdible);
				}
			}
		}		
	}

	private int scoreCount = 0;
	public int getScoreCount() {
		return scoreCount;
	}
	
	private int foodCount = 0;
	public int getFoodCount() {
		return foodCount;
	}
	
	private int missilePacks = 0;	// returns the number of missile packs on the map
	public int numberMissilePacks() {
		return missilePacks;
	}
	
	private boolean health = false;	// true if there is a health charger
	public boolean hasHealthCharger() {
		return health;
	}
	
	private boolean energy = false;	// true if there is an energy charger
	public boolean hasEnergyCharger() {
		return energy;
	}
	
	HashSet<CellObject> unopenedBoxes = new HashSet<CellObject>();
	public int getUnopenedBoxCount() {
		return unopenedBoxes.size();
	}
	
	HashSet<CellObject> updatables = new HashSet<CellObject>();
	HashMap<CellObject, java.awt.Point> updatablesLocations = new HashMap<CellObject, java.awt.Point>();
	
	public class BookObjectInfo {
		public CellObject object;
		public Point location;
		public Point2D.Double floatLocation;
	}
	HashSet<CellObject> bookObjects = new HashSet<CellObject>();
	HashMap<Integer, BookObjectInfo> bookObjectInfo = new HashMap<Integer, BookObjectInfo>();
	public HashSet<CellObject> getBookObjects() {
		return bookObjects;
	}
	public BookObjectInfo getBookObjectInfo(Integer id) {
		return bookObjectInfo.get(id);
	}
	public boolean isBookObject(CellObject co) {
		if (co.name.equals("mblock")) {
			return true;
		}
		return false;
	}
	
	public void addObjectToCell(java.awt.Point location, CellObject object) {
		Cell cell = getCell(location);
		if (cell.hasObject(object.getName())) {
			CellObject old = cell.removeObject(object.getName());
			assert old != null;
			updatables.remove(old);
			updatablesLocations.remove(old);
			if (isBookObject(old)) {
				bookObjects.remove(old);
				bookObjectInfo.remove(old.getId());
			}
			removalStateUpdate(old);
		}
		if (object.updatable()) {
			updatables.add(object);
			updatablesLocations.put(object, location);
		}
		if (isBookObject(object)) {
			bookObjects.add(object);
			BookObjectInfo info = new BookObjectInfo();
			info.location = new Point(location);
			info.floatLocation = new Point2D.Double();
			info.floatLocation.x = info.location.x * Soar2D.config.getBookCellSize();
			info.floatLocation.y = info.location.y * Soar2D.config.getBookCellSize();
			info.floatLocation.x += Soar2D.config.getBookCellSize() / 2.0;
			info.floatLocation.y += Soar2D.config.getBookCellSize() / 2.0;
			info.object = object;
			bookObjectInfo.put(object.getId(), info);
		}
		if (config.getTerminalUnopenedBoxes()) {
			if (isUnopenedBox(object)) {
				unopenedBoxes.add(object);
			}
		}
		
		// Update state we keep track of specific to game type
		switch (config.getType()) {
		case kTankSoar:
			if (object.hasProperty(Names.kPropertyCharger)) {
				if (!health && object.hasProperty(Names.kPropertyHealth)) {
					health = true;
				}
				if (!energy && object.hasProperty(Names.kPropertyEnergy)) {
					energy = true;
				}
			}
			if (object.hasProperty(Names.kPropertyMissiles)) {
				missilePacks += 1;
			}
			break;
		case kEaters:
			if (object.hasProperty(Names.kPropertyEdible)) {
				foodCount += 1;
			}
			if (object.hasProperty(Names.kPropertyPoints)) {
				scoreCount += object.getIntProperty(Names.kPropertyPoints);
			}
			break;
			
		case kBook:
			break;
		}
		cell.addCellObject(object);
		setRedraw(cell);
	}
	
	public boolean addRandomObjectWithProperty(java.awt.Point location, String property) {
		CellObject object = cellObjectManager.createRandomObjectWithProperty(property);
		if (object == null) {
			return false;
		}
		addObjectToCell(location, object);
		return true;
	}
	
	public boolean addObjectByName(java.awt.Point location, String name) {
		CellObject object = cellObjectManager.createObject(name);
		if (object == null) {
			return false;
		}

		addObjectToCell(location, object);
		return true;
	}

	public CellObject createRandomObjectWithProperty(String property) {
		return cellObjectManager.createRandomObjectWithProperty(property);
	}

	public boolean addRandomObjectWithProperties(java.awt.Point location, String property1, String property2) {
		CellObject object = cellObjectManager.createRandomObjectWithProperties(property1, property2);
		if (object == null) {
			return false;
		}
		addObjectToCell(location, object);
		return true;
	}
	
	public void updateObjects(World world) {
		if (!updatables.isEmpty()) {
			Iterator<CellObject> iter = updatables.iterator();
			
			ArrayList<java.awt.Point> explosions = new ArrayList<java.awt.Point>();
			while (iter.hasNext()) {
				CellObject cellObject = iter.next();
				java.awt.Point location = updatablesLocations.get(cellObject);
				assert location != null;
				int previousScore = 0;
				if (config.getType() == SimType.kEaters) {
					if (cellObject.hasProperty(Names.kPropertyPoints)) {
						previousScore = cellObject.getIntProperty(Names.kPropertyPoints);
					}
				}
				if (cellObject.update(world, location)) {
					Cell cell = getCell(location);
					
					cellObject = cell.removeObject(cellObject.getName());
					assert cellObject != null;
					
					setRedraw(cell);
					
					// if it is not tanksoar or if the cell is not a missle or if shouldRemoveMissile returns true
					if ((config.getType() != SimType.kTankSoar) || !cellObject.hasProperty(Names.kPropertyMissile) 
							|| shouldRemoveMissile(world, location, cell, cellObject)) {
						
						// we need an explosion if it was a tanksoar missile
						if ((config.getType() == SimType.kTankSoar) && cellObject.hasProperty(Names.kPropertyMissile)) {
							explosions.add(location);
						}
						iter.remove();
						updatablesLocations.remove(cellObject);
						removalStateUpdate(cellObject);
					}
				}
				if (config.getType() == SimType.kEaters) {
					if (cellObject.hasProperty(Names.kPropertyPoints)) {
						scoreCount += cellObject.getIntProperty(Names.kPropertyPoints) - previousScore;
					}
				}
			}
			
			Iterator<java.awt.Point> explosion = explosions.iterator();
			while (explosion.hasNext()) {
				setExplosion(explosion.next());
			}
		}
		
		if (config.getTerminalUnopenedBoxes()) {
			Iterator<CellObject> iter = unopenedBoxes.iterator();
			while (iter.hasNext()) {
				CellObject box = iter.next();
				if (!isUnopenedBox(box)) {
					iter.remove();
				}
			}
		}
	}
	
	public void setPlayer(java.awt.Point location, Player player) {
		Cell cell = getCell(location);
		cell.setPlayer(player);
		setRedraw(cell);
	}
	
	public int pointsCount(java.awt.Point location) {
		Cell cell = getCell(location);
		ArrayList list = cell.getAllWithProperty(Names.kPropertyEdible);
		Iterator iter = list.iterator();
		int count = 0;
		while (iter.hasNext()) {
			count += ((CellObject)iter.next()).getIntProperty(Names.kPropertyPoints);
		}
		return count;
	}
	
	public boolean isAvailable(java.awt.Point location) {
		Cell cell = getCell(location);
		boolean enterable = cell.enterable();
		boolean noPlayer = cell.getPlayer() == null;
		boolean noMissilePack = cell.getAllWithProperty(Names.kPropertyMissiles).size() <= 0;
		boolean noCharger = cell.getAllWithProperty(Names.kPropertyCharger).size() <= 0;
		return enterable && noPlayer && noMissilePack && noCharger;
	}
	
	public boolean enterable(java.awt.Point location) {
		Cell cell = getCell(location);
		return cell.enterable();
	}
	
	public CellObject removeObject(java.awt.Point location, String objectName) {
		Cell cell = getCell(location);
		setRedraw(cell);
		CellObject object = cell.removeObject(objectName);
		if (object == null) {
			return null;
		}
		
		if (object.updatable()) {
			updatables.remove(object);
			updatablesLocations.remove(object);
		}
		if (isBookObject(object)) {
			bookObjects.remove(object);
			bookObjectInfo.remove(object.getId());
		}
		removalStateUpdate(object);
		
		return object;
	}
	
	public Player getPlayer(java.awt.Point location) {
		if (location == null) return null;
		Cell cell = getCell(location);
		return cell.getPlayer();
	}
	
	public boolean hasObject(java.awt.Point location, String name) {
		if (location == null) return false;
		Cell cell = getCell(location);
		return cell.hasObject(name);
	}
	
	public CellObject getObject(java.awt.Point location, String name) {
		if (location == null) return null;
		Cell cell = getCell(location);
		return cell.getObject(name);
	}
	
	private boolean shouldRemoveMissile(World world, java.awt.Point location, Cell cell, CellObject missile) {
		// instead of removing missiles, move them

		// what direction is it going
		int missileDir = missile.getIntProperty(Names.kPropertyDirection);
		
		while (true) {
			// move it
			Direction.translate(location, missileDir);
			
			// check destination
			cell = getCell(location);
			
			if (!cell.enterable()) {
				// missile is destroyed
				return true;
			}
			
			Player player = cell.getPlayer();
			
			if (player != null) {
				// missile is destroyed
				world.missileHit(player, location, missile);
				return true;
			}
	
			// missile didn't hit anything
			
			// if the missile is not in phase 2, return
			if (missile.getIntProperty(Names.kPropertyFlyPhase) != 2) {
				cell.addCellObject(missile);
				updatablesLocations.put(missile, location);
				return false;
			}
			
			// we are in phase 2, call update again, this will move us out of phase 2 to phase 3
			missile.update(world, location);
		}
	}
		
	public void handleIncoming() {
		// TODO: a couple of optimizations possible here
		// like marking cells that have been checked, depends on direction though
		// probably more work than it is worth as this should only be slow when there are
		// a ton of missiles flying
		
		Iterator<CellObject> iter = updatables.iterator();
		while (iter.hasNext()) {
			CellObject missile = iter.next();
			if (!missile.hasProperty(Names.kPropertyMissile)) {
				continue;
			}
	
			java.awt.Point threatenedLocation = new java.awt.Point(updatablesLocations.get(missile));
			while (true) {
				int direction = missile.getIntProperty(Names.kPropertyDirection);
				Direction.translate(threatenedLocation, direction);
				if (!enterable(threatenedLocation)) {
					break;
				}
				Player player = getPlayer(threatenedLocation);
				if (player != null) {
					player.setIncoming(Direction.backwardOf[direction]);
					break;
				}
			}
		}
	}
	
	private void removalStateUpdate(CellObject object) {
		switch (config.getType()) {
		case kTankSoar:
			if (object.hasProperty(Names.kPropertyCharger)) {
				if (health && object.hasProperty(Names.kPropertyHealth)) {
					health = false;
				}
				if (energy && object.hasProperty(Names.kPropertyEnergy)) {
					energy = false;
				}
			}
			if (object.hasProperty(Names.kPropertyMissiles)) {
				missilePacks -= 1;
			}
			break;
		case kEaters:
			if (object.hasProperty(Names.kPropertyEdible)) {
				foodCount -= 1;
			}
			if (object.hasProperty(Names.kPropertyPoints)) {
				scoreCount -= object.getIntProperty(Names.kPropertyPoints);
			}
			break;
		}
		if (config.getTerminalUnopenedBoxes()) {
			if (isUnopenedBox(object)) {
				unopenedBoxes.remove(object);
			}
		}
	}
	
	public ArrayList<CellObject> getAllWithProperty(java.awt.Point location, String name) {
		Cell cell = getCell(location);
		return cell.getAllWithProperty(name);
	}
	
	public void removeAll(java.awt.Point location) {
		removeAllWithProperty(location, null);
		
	}
	
	public void removeAllWithProperty(java.awt.Point location, String name) {
		Cell cell = getCell(location);
		
		cell.iter = cell.cellObjects.values().iterator();
		CellObject cellObject;
		while (cell.iter.hasNext()) {
			cellObject = cell.iter.next();
			if (name == null || cellObject.hasProperty(name)) {
				if (cellObject.updatable()) {
					updatables.remove(cellObject);
					updatablesLocations.remove(cellObject);
				}
				if (isBookObject(cellObject)) {
					bookObjects.remove(cellObject);
					bookObjectInfo.remove(cellObject.getId());
				}
				cell.iter.remove();
				removalStateUpdate(cellObject);
			}
		}
	}
	
	private boolean isUnopenedBox(CellObject object) {
		if (object.hasProperty(Names.kPropertyBox)) {
			String status = object.getProperty(Names.kPropertyStatus);
			if (status == null || !status.equals(Names.kOpen)) {
				return true;
			}
		}
		return false;
	}
	
	private void setRedraw(Cell cell) {
		cell.addCellObject(new CellObject(Names.kRedraw));
	}
	
	public void setExplosion(java.awt.Point location) {
		CellObject explosion = null;
		switch (config.getType()) {
		case kTankSoar:
			explosion = cellObjectManager.createObject(Names.kExplosion);
			break;
			
		case kEaters:
			explosion = new CellObject(Names.kExplosion);
			explosion.addProperty(Names.kPropertyLinger, "2");
			explosion.setLingerUpdate(true);
			break;
		}
		addObjectToCell(location, explosion);
	}
	
	public void shutdown() {
		mapCells = null;
		cellObjectManager = null;
		size = 0;
	}

	public int getRadar(RadarCell[][] radar, java.awt.Point location, int facing, int radarPower) {
		if (radarPower == 0) {
			return 0;
		}
		
		assert radar.length == 3;

		int distance = 0;
		
		distance = radarProbe(radar, location, facing, distance, radarPower);
		
		return distance;
	}
	
	private RadarCell getRadarCell(java.awt.Point location) {
		// TODO: cache these each frame!!
		
		Cell cell;
		Iterator<CellObject> iter;
		RadarCell radarCell;

		cell = getCell(location);
		radarCell = new RadarCell();
		radarCell.player = cell.getPlayer();
		if (cell.enterable()) {
			iter = cell.getAllWithProperty(Names.kPropertyMiniImage).iterator();
			while (iter.hasNext()) {
				CellObject object = iter.next();
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
	
	private int radarProbe(RadarCell[][] radar, java.awt.Point myLocation, int facing, int distance, int maxDistance) {
		assert maxDistance < radar[1].length;
		assert distance >= 0;
		assert distance + 1 < radar[1].length;
		assert distance < maxDistance;
		assert facing > 0;
		assert facing < 5;
		
		java.awt.Point location;
		
		location = new java.awt.Point(myLocation);
		Direction.translate(location, Direction.leftOf[facing]);
		radar[0][distance] = getRadarCell(location);
		if (radar[0][distance].player != null) {
			if (distance != 0) {
				radar[0][distance].player.radarTouch(Direction.backwardOf[facing]);
			} else {
				radar[0][distance].player.radarTouch(Direction.rightOf[facing]);
			}
		}
		
		location = new java.awt.Point(myLocation);
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

		location = new java.awt.Point(myLocation);
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

	public int getBlocked(Point location) {
		Cell cell;
		int blocked = 0;
		
		cell = getCell(location.x+1, location.y);
		if (!cell.enterable() || cell.getPlayer() != null) {
			blocked |= Direction.kEastIndicator;
		}
		cell = getCell(location.x-1, location.y);
		if (!cell.enterable() || cell.getPlayer() != null) {
			blocked |= Direction.kWestIndicator;
		}
		cell = getCell(location.x, location.y+1);
		if (!cell.enterable() || cell.getPlayer() != null) {
			blocked |= Direction.kSouthIndicator;
		}
		cell = getCell(location.x, location.y-1);
		if (!cell.enterable() || cell.getPlayer() != null) {
			blocked |= Direction.kNorthIndicator;
		}
		return blocked;
	}
	
	public int getSoundNear(java.awt.Point location) {
		if (Soar2D.simulation.world.getPlayers().size() < 2) {
			return 0;
		}
		
		// Set all cells unexplored.
		for(int y = 1; y < mapCells.length - 1; ++y) {
			for (int x = 1; x < mapCells.length - 1; ++x) {
				mapCells[y][x].distance = -1;
			}
		}
		
		LinkedList<java.awt.Point> searchList = new LinkedList<java.awt.Point>();
		searchList.addLast(new java.awt.Point(location));
		int distance = 0;
		getCell(location).distance = distance;
		getCell(location).parent = null;

		int relativeDirection = -1;
		int newCellX = 0;
		int newCellY = 0;
		java.awt.Point parentLocation;
		Cell parentCell;
		Cell newCell;

		while (searchList.size() > 0) {
			parentLocation = searchList.getFirst();
			searchList.removeFirst();
			parentCell = getCell(parentLocation);
			distance = parentCell.distance;
			if (distance >= config.getMaxSmellDistance()) {
				//System.out.println(parentCell + " too far");
				continue;
			}

			// Explore cell.
			for (int i = 1; i < 5; ++i) {
				newCellX = parentLocation.x;
				newCellY = parentLocation.y;
				newCellX += Direction.xDelta[i];
				newCellY += Direction.yDelta[i];

				if (!isInBounds(newCellX, newCellY)) {
					continue;
				}

				newCell = getCell(newCellX, newCellY);
				if (!newCell.enterable()) {
					//System.out.println(parentCell + " not enterable");
					continue;
				}
							
				if (newCell.distance >= 0) {
					//System.out.println(parentCell + " already explored");
					continue;
				}
				newCell.distance = distance + 1;
				
				Player targetPlayer = newCell.getPlayer();
				if ((targetPlayer != null) && Soar2D.simulation.world.recentlyMovedOrRotated(targetPlayer)) {
					// I'm its parent, so see if I'm the top here
					while(parentCell.parent != null) {
						// the new cell becomes me
						newCellX = parentLocation.x;
						newCellY = parentLocation.y;
						
						// I become my parent
						parentLocation = getCell(parentLocation).parent;
						parentCell = getCell(parentLocation);
					}
					// location is now the top of the list, compare
					// to find the direction to the new cell
					if (newCellX < parentLocation.x) {
						relativeDirection = Direction.kWestInt;
					} else if (newCellX > parentLocation.x) {
						relativeDirection = Direction.kEastInt;
					} else if (newCellY < parentLocation.y) {
						relativeDirection = Direction.kNorthInt;
					} else if (newCellY > parentLocation.y) {
						relativeDirection = Direction.kSouthInt;
					} else {
						assert false;
						relativeDirection = 0;
					}
					break;
				}
				
				if (relativeDirection != -1) {
					break;
				}
				
				// add me as the new cell's parent				
				newCell.parent = parentLocation;
				// add the new cell to the search list
				searchList.addLast(new java.awt.Point(newCellX, newCellY));
			}
			
			if (relativeDirection != -1) {
				break;
			}
		}
		
		if (relativeDirection == -1) {
			relativeDirection = 0;
		}
		return relativeDirection;
	}

	public boolean isInBounds(int x, int y) {
		return (x >= 0) && (y >= 0) && (x < getSize()) && (y < getSize());
	}

	public boolean isInBounds(Point location) {
		return isInBounds(location.x, location.y);
	}

	private int roomCount = 0;
	private int gatewayCount = 0;
	private int wallCount = 0;
	
	public class Barrier {
		public int id = -1;
		public boolean gateway = false;
		
		public java.awt.Point left;
		public java.awt.Point right;
		
		public int direction;

		private Point2D.Double centerpoint;
		public Point2D.Double centerpoint() {
			//  IMPORTANT! Assumes left/right won't change
			if (centerpoint != null) {
				return centerpoint;
			}
			
			int m = 0;
			int n = 0;
			centerpoint = new Point2D.Double(0,0);
			
			switch (direction) {
			case Direction.kNorthInt:
			case Direction.kSouthInt:
				// horizontal
				m = left.x;
				n = right.x;
				centerpoint.y = left.y * Soar2D.config.getBookCellSize();
				break;
			case Direction.kEastInt:
			case Direction.kWestInt:
				// vertical
				m = left.y;
				n = right.y;
				centerpoint.x = left.x * Soar2D.config.getBookCellSize();
				break;
			}
			
			int numberOfBlocks = n - m;
			java.awt.Point upperLeft = left;
			// take abs, also note that if negative then we need to calculate center from the upper-left block
			// which this case is the "right"
			if (numberOfBlocks < 0) {
				numberOfBlocks *= -1;
				upperLeft = right;
			}
			numberOfBlocks += 1; // endpoints 0,0 and 0,3 represent 4 blocks
			
			if (left.x == right.x) {
				// vertical
				// add half to y
				centerpoint.y = upperLeft.y * Soar2D.config.getBookCellSize();
				centerpoint.y += (numberOfBlocks / 2.0) * Soar2D.config.getBookCellSize();
				
				// if west, we gotta add a cell size to x
				if (direction == Direction.kWestInt) {
					centerpoint.x += Soar2D.config.getBookCellSize();
				}
				
			} else {
				// horizontal
				// add half to x
				centerpoint.x = upperLeft.x * Soar2D.config.getBookCellSize();
				centerpoint.x += (numberOfBlocks / 2.0) * Soar2D.config.getBookCellSize();

				// if north, we gotta add a cell size to y
				if (direction == Direction.kNorthInt) {
					centerpoint.y += Soar2D.config.getBookCellSize();
				}
			}
			return centerpoint;
		}
		
		public String toString() {
			String output = new String(Integer.toString(id));
			output += " (" + Direction.stringOf[direction] + ")";
			Point2D.Double center = centerpoint();
			output += " (" + Integer.toString(left.x) + "," + Integer.toString(left.y) + ")-(" 
					+ Double.toString(center.x) + "," + Double.toString(center.y) + ")-(" 
					+ Integer.toString(right.x) + "," + Integer.toString(right.y) + ")";
			if (gateway) {
				output += " (gateway)";
			}
			return output;
		}
	}
	
	// Mapping of room id to the list of the barriers surrounding that room
	private HashMap<Integer, ArrayList<Barrier> > roomBarrierMap = new HashMap<Integer, ArrayList<Barrier> >();
	public ArrayList<Barrier> getRoomBarrierList(int roomID) {
		return roomBarrierMap.get(roomID);
	}

	// Mapping of gateway id to the list of the ids of rooms it connects
	private HashMap<Integer, ArrayList<Integer> > gatewayDestinationMap = new HashMap<Integer, ArrayList<Integer> >();
	public ArrayList<Integer> getGatewayDestinationList(int gatewayID) {
		return gatewayDestinationMap.get(gatewayID);
	}
	
	public boolean generateRoomStructure() {
		// Start in upper-left corner
		// if cell is enterable, flood fill to find boundaries of room
		// Go from left to right, then to the start of the next line
		LinkedList<java.awt.Point> floodQueue = new LinkedList<java.awt.Point>();
		HashSet<java.awt.Point> explored = new HashSet<java.awt.Point>((this.size-2)*2);
		
		// this is where we will store gateway barriers for conversion to rooms 
		// in the second phase of map structure generation. 
		// this will contain duplicates since two different gateways can 
		// be represented by the same squares
		ArrayList<Barrier> gatewayBarriers = new ArrayList<Barrier>();

		java.awt.Point location = new java.awt.Point();
		for (location.y = 1; location.y < (this.size - 1); ++location.y) {
			for (location.x = 1; location.x < (this.size - 1); ++location.x) {
				if (explored.contains(location)) {
					continue;
				}
				explored.add(location);
				
				Cell cell = getCell(location);
				if (!cell.enterable() || cell.hasObject(Names.kPropertyGateway)) {
					continue;
				}
				
				assert cell.getObject(Names.kRoomID) == null;

				// cell is enterable, we have a room
				int roomNumber = roomCount + gatewayCount + wallCount;
				roomCount += 1;
				
				CellObject roomObject = cellObjectManager.createObject(Names.kRoomID);
				roomObject.addProperty(Names.kPropertyNumber, Integer.toString(roomNumber));

				HashSet<java.awt.Point> floodExplored = new HashSet<java.awt.Point>((this.size-2)*2);
				floodExplored.add(location);
				cell.addCellObject(roomObject);
				//System.out.print("Room " + roomNumber + ": (" + location.x + "," + location.y + ") ");
				
				// add the surrounding cells to the queue 
				floodQueue.add(new java.awt.Point(location.x+1,location.y));
				floodQueue.add(new java.awt.Point(location.x,location.y+1));
				floodQueue.add(new java.awt.Point(location.x-1,location.y));
				floodQueue.add(new java.awt.Point(location.x,location.y-1));
				
				// flood and mark all room cells and save walls
				HashSet<java.awt.Point> walls = new HashSet<java.awt.Point>();
				while(floodQueue.size() > 0) {
					java.awt.Point floodLocation = floodQueue.removeFirst();

					if (floodExplored.contains(floodLocation)) {
						continue;
					}
					floodExplored.add(floodLocation);
					
					cell = getCell(floodLocation);
					if (!cell.enterable() || cell.hasObject(Names.kPropertyGateway)) {
						walls.add(floodLocation);
						continue;
					}

					explored.add(floodLocation);

					cell.addCellObject(new CellObject(roomObject));
					//System.out.print("(" + floodLocation.x + "," + floodLocation.y + ") ");

					// add the four surrounding cells to the queue
					floodQueue.add(new java.awt.Point(floodLocation.x+1,floodLocation.y));
					floodQueue.add(new java.awt.Point(floodLocation.x-1,floodLocation.y));
					floodQueue.add(new java.awt.Point(floodLocation.x,floodLocation.y+1));
					floodQueue.add(new java.awt.Point(floodLocation.x,floodLocation.y-1));
				}
				
				// figure out walls going clockwise starting with the wall north of the first square in the room
				int direction = Direction.kEastInt;
				java.awt.Point startingWall = new java.awt.Point(location.x, location.y-1);
				java.awt.Point next = new java.awt.Point(startingWall);
				
				// Keep track of the current barrier information. When the wall ends, simply add it to the the room
				// barrier map.
				Barrier currentBarrier = null;
				
				// Keep track of barrier information
				ArrayList<Barrier> barrierList = new ArrayList<Barrier>();
				
				// I probably should have commented this more when I wrote it.
				// The comments have been inserted after the initial writing so they may be slightly wrong.
				
				while (true) {
					
					// This is used to figure out how to turn corners when walking along walls.
					// Also used with figuring out barrier endpoints.
					java.awt.Point previous = new java.awt.Point(next);
					
					// next is actually the location we're examining now
					cell = getCell(next);
					
					// used to detect turns
					java.awt.Point rightOfNext = null;
					
					// Get the wall and gateway objects. The can't both exist.
					CellObject gatewayObject = cell.getObject(Names.kGatewayID);
					CellObject wallObject = cell.getObject(Names.kWallID);
					
					// One must exist, but not both
					assert ((gatewayObject == null) && (wallObject != null)) || ((gatewayObject != null) && (wallObject == null));

					if (gatewayObject != null) {
						if (currentBarrier != null) {
							// If we were just walking a wall, end and add the barrier
							if (currentBarrier.gateway == false) {
								barrierList.add(currentBarrier);
								currentBarrier = null;
							}
						}
						
						// At this point, the currentBarrier, if it exists, is the gateway we're walking
						if (currentBarrier == null) {
							
							// getting here means we're starting a new section of gateway
							
							// create a barrier
							currentBarrier = new Barrier();
							currentBarrier.gateway = true;
							currentBarrier.left = new java.awt.Point(next);
							currentBarrier.direction = Direction.leftOf[direction];
							
							// create a new gateway id
							currentBarrier.id = roomCount + gatewayCount + wallCount;
							gatewayCount += 1;

							//System.out.println();
							//System.out.print("  Gateway " + currentBarrier.id + ": ");
							
							// add the current room to the gateway destination list
							ArrayList<Integer> gatewayDestinations = gatewayDestinationMap.get(new Integer(currentBarrier.id));
							if (gatewayDestinations == null) {
								gatewayDestinations = new ArrayList<Integer>();
							}
							gatewayDestinations.add(new Integer(roomNumber));
							gatewayDestinationMap.put(new Integer(currentBarrier.id), gatewayDestinations);
						}

						// is are noted by the direction of the wall
						gatewayObject.addProperty(Direction.stringOf[currentBarrier.direction], Integer.toString(currentBarrier.id));
						
						// tell the gateway it should be drawn
						gatewayObject.addProperty(Names.kPropertyGatewayRender, Names.kTrue);

					} else if (wallObject != null) /*redundant*/ {
					
						if (currentBarrier != null) {
							// If we were just walking a gateway, end and add the barrier
							if (currentBarrier.gateway) {
								// keep track of gateway barriers
								gatewayBarriers.add(currentBarrier);
								barrierList.add(currentBarrier);
								currentBarrier = null;
							}
						}
						
						// At this point, the currentBarrier, if it exists, is the wall we're walking
						if (currentBarrier == null) {
							
							// getting here means we're starting a new section of wall
							currentBarrier = new Barrier();
							currentBarrier.left = new java.awt.Point(next);
							currentBarrier.direction = Direction.leftOf[direction];
							
							// create a new id
							currentBarrier.id = roomCount + gatewayCount + wallCount;
							wallCount += 1;
							
							//System.out.println();
							//System.out.print("  Wall " + currentBarrier.id + ": (" + currentBarrier.direction + "): ");
						}
						
						// is are noted by the direction of the wall
						wallObject.addProperty(Direction.stringOf[currentBarrier.direction], Integer.toString(currentBarrier.id));

					} else {
						// the world is ending, check the asserts
						assert false;
					}
					
					//System.out.print("(" + next.x + "," + next.y + ") ");

					// since current barrier is the gateway we're walking, update it's endpoint before we translate it
					currentBarrier.right = new java.awt.Point(next);
					
					// walk to the next section of wall
					Direction.translate(next, direction);
					
					// we get the right of next here because if there is a next and
					// a wall to the right of it, that means next is a gateway but there is
					// a wall in the way so that gateway doesn't technically border our room,
					// so, the gateway ends and we continue on the next segment of wall.
					rightOfNext = new java.awt.Point(next);
					Direction.translate(rightOfNext, Direction.rightOf[direction]);

					// if there isn't a next, we're done anyway.
					// continue if we're moving on with this section of wall, or fall
					// through to terminate the wall.
					if (walls.contains(next) && !walls.contains(rightOfNext)) {
						continue;
					}

					// gateway or wall stops here
					//System.out.print("(turn)");

					// and the barrier to the list
					
					if (currentBarrier.gateway) {
						// keep track of gateway barriers
						gatewayBarriers.add(currentBarrier);
					}
					
					barrierList.add(currentBarrier);
					currentBarrier = null;
					
					// when going clockwise, when we turn right, we go to the cell adjacent to "next",
					// when we turn left, we go to the cell adjacent to "previous"
					//
					//  going east
					// +---+---+---+  Direction travelling along wall: east
					// |   | L |   |  W = wall we've seen
					// +---+---+---+  P = "previous"
					// | W | P | N |  N = "next" (we just tested this and it isn't a wall)
					// +---+---+---+  L = possible next, indicates left turn (P included on the new wall)
					// |   |   | R |  R = possible next, indicates right turn
					// +---+---+---+    = irrelevant cell
					
					//  going west    going north   going south
					// +---+---+---+ +---+---+---+ +---+---+---+
					// | R |   |   | |   | N | R | |   | W |   |
					// +---+---+---+ +---+---+---+ +---+---+---+
					// | N | P | W | | L | P |   | |   | P | L |
					// +---+---+---+ +---+---+---+ +---+---+---+
					// |   | L |   | |   | W |   | | R | N |   |
					// +---+---+---+ +---+---+---+ +---+---+---+
					
					// try turning right first
					Direction.translate(next, Direction.rightOf[direction]);
					if (walls.contains(next)) {
						// right worked
						direction = Direction.rightOf[direction];

					} else {
						// try turning left next
						next = new java.awt.Point(previous);
						Direction.translate(next, Direction.leftOf[direction]);
						
						if (walls.contains(next)) {
							// left worked
							direction = Direction.leftOf[direction];
							
							// need to stay on previous because it is included on the new wall
							next = previous;
							// the remove will silently fail and that's ok
							
						} else {
							// single length wall (perform "left" turn)
							direction = Direction.leftOf[direction];

							// need to stay on previous because it is included on the new wall
							next = previous;
							// the remove will silently fail and that's ok
						}
					}
					
					// See if our turn leads us home
					if (next.equals(startingWall)) {
						break;
					}
				}
				//System.out.println();
				
				// Generate centerpoints and store room information
				generateCenterpoints(roomNumber, barrierList);
				this.roomBarrierMap.put(roomNumber, barrierList);
			}
		}
		
		// convert all the gateways to areas
		gatewaysToAreasStep(gatewayBarriers);
		
		// print gateway information
		Iterator<Integer> gatewayKeyIter = gatewayDestinationMap.keySet().iterator();
		while (gatewayKeyIter.hasNext()) {
			Integer gatewayId = gatewayKeyIter.next();
			String toList = "";
			Iterator<Integer> gatewayDestIter = gatewayDestinationMap.get(gatewayId).iterator();
			while (gatewayDestIter.hasNext()) {
				toList += gatewayDestIter.next() + " ";
			}
			System.out.println("Gateway " + gatewayId + ": " + toList);
		}
		
		return true;
	}

	private void addDestinationToGateway(int roomNumber, int gatewayId) {
		ArrayList<Integer> gatewayDestinations = gatewayDestinationMap.get(new Integer(gatewayId));
		assert gatewayDestinations != null;
		gatewayDestinations.add(new Integer(roomNumber));
		gatewayDestinationMap.put(new Integer(gatewayId), gatewayDestinations);
	}
	
	/**
	 * Create a wall for the new rooms created by the initial gateways.
	 * 
	 * @param endPoint The square representing the side of the room butting up to this wall
	 * @param direction The direction to go to reach the wall
	 * @param barrierList The barrier list for our new room
	 */
	private void doNewWall(java.awt.Point endPoint, int direction, ArrayList<Barrier> barrierList) {
		Barrier currentBarrier = new Barrier();
		currentBarrier.left = new java.awt.Point(endPoint);
		currentBarrier.left.x += Direction.xDelta[direction];
		currentBarrier.left.y += Direction.yDelta[direction];

		// this is a wall and is only size 1
		currentBarrier.right = new java.awt.Point(currentBarrier.left);
		
		// get a new wall id
		currentBarrier.id = roomCount + gatewayCount + wallCount;
		wallCount += 1;
		// the direction is the direction we just traveled to get to the wall
		currentBarrier.direction = direction;
		
		// get the wall object
		Cell cell = getCell(currentBarrier.left);
		CellObject wallObject = cell.getObject(Names.kWallID);
		
		// walls don't share ids, they are noted by the direction of the wall
		wallObject.addProperty(Direction.stringOf[currentBarrier.direction], Integer.toString(currentBarrier.id));

		// store the barrier
		barrierList.add(currentBarrier);
	}
	
	/**
	 * Creates a gateway for the new rooms created by the initial gateways.
	 * 
	 * @param startPoint Travelling clockwise around the room, this is the end of the room adjacent to the start gateway we're creating
	 * @param endPoint This is the end of the room adjacent to the end of the gateway.
	 * @param direction This is the side of the room the gateway is on, or (alternatively), what direction we have to turn to face the gateway.
	 * @param walkDirection This is the direction we go to walk down the gateway (parallel to the room)
	 * @param roomNumber This is the id number of the room we're in
	 * @param barrierList This is the list of barriers for the room we're in
	 */
	private void doNewGateway(java.awt.Point startPoint, java.awt.Point endPoint, int direction, int walkDirection, int roomNumber, ArrayList<Barrier> barrierList) {
		// next is the gateway to the left of our left endpoint
		Barrier currentBarrier = new Barrier();
		currentBarrier.gateway = true;
		currentBarrier.left = new java.awt.Point(startPoint);
		currentBarrier.left.x += Direction.xDelta[direction];
		currentBarrier.left.y += Direction.yDelta[direction];

		// get a new gateway id
		currentBarrier.id = roomCount + gatewayCount + wallCount;
		gatewayCount += 1;
		// the direction is the direction we just traveled to get to the gateway
		currentBarrier.direction = direction;

		// this is a gateway of unknown size
		currentBarrier.right = new java.awt.Point(currentBarrier.left);
		
		// we know it ends left of the right endpoint
		java.awt.Point endOfGateway = new java.awt.Point(endPoint);
		endOfGateway.x += Direction.xDelta[direction];
		endOfGateway.y += Direction.yDelta[direction];
		
		while (true) {
			// create the gateway object
			CellObject gatewayObject = cellObjectManager.createObject(Names.kGatewayID);

			// gateway don't share ids, they are noted by the direction of the gateway
			gatewayObject.addProperty(Direction.stringOf[currentBarrier.direction], Integer.toString(currentBarrier.id));

			// put the object in the cell
			Cell cell = getCell(currentBarrier.right);
			cell.addCellObject(gatewayObject);
			
			// record the destinations which is the new room and the room the gateway is sitting on
			// add the current room to the gateway destination list
			ArrayList<Integer> gatewayDestinations = new ArrayList<Integer>();
			gatewayDestinations.add(new Integer(roomNumber));
			gatewayDestinations.add(new Integer(cell.getObject(Names.kRoomID).getIntProperty(Names.kPropertyNumber)));
			gatewayDestinationMap.put(new Integer(currentBarrier.id), gatewayDestinations);
			
			if (currentBarrier.right.equals(endOfGateway)) {
				break;
			}
			
			// increment and loop
			currentBarrier.right.x += Direction.xDelta[walkDirection];
			currentBarrier.right.y += Direction.yDelta[walkDirection];
		}

		// store the barrier
		barrierList.add(currentBarrier);
	}	

	
	private void gatewaysToAreasStep(ArrayList<Barrier> gatewayBarriers) {
		// make the gateway also a room
		// add new room to current gateway destination list
		// create new barrier list for this new room: 2 walls and 2 gateways
		// update gateway destination list for new gateways
		
		Iterator<Barrier> iter = gatewayBarriers.iterator();
		while (iter.hasNext()) {
			Barrier gatewayBarrier = iter.next();
			
			// duplicates exist in this list, check to see if we're already a room
			{
				Cell cell = getCell(gatewayBarrier.left);
				if (cell.hasObject(Names.kRoomID)) {
					// we have already processed this room, just need to add the room id
					// to the gateway's destination list
					addDestinationToGateway(cell.getObject(Names.kRoomID).getIntProperty(Names.kPropertyNumber), gatewayBarrier.id);
					continue;
				}
			}
			
			// get a new room id
			int roomNumber = roomCount + gatewayCount + wallCount;
			roomCount += 1;
			
			// add new id to current gateway destination list
			addDestinationToGateway(roomNumber, gatewayBarrier.id);
			
			CellObject theNewRoomObject = cellObjectManager.createObject(Names.kRoomID);
			theNewRoomObject.addProperty(Names.kPropertyNumber, Integer.toString(roomNumber));
			{
				Cell cell = getCell(gatewayBarrier.left);
				cell.addCellObject(theNewRoomObject);
			}

			int incrementDirection = -1;
			if (gatewayBarrier.left.x == gatewayBarrier.right.x) {
				// vertical gateway
				if (gatewayBarrier.left.y < gatewayBarrier.right.y) {
					// increasing to right, south
					incrementDirection = Direction.kSouthInt;

				} else if (gatewayBarrier.left.y > gatewayBarrier.right.y) {
					// decreasing to right, north
					incrementDirection = Direction.kNorthInt;
				}
			} else {
				// horizontal gateway
				if (gatewayBarrier.left.x < gatewayBarrier.right.x) {
					// increasing to right, east
					incrementDirection = Direction.kEastInt;

				} else if (gatewayBarrier.left.x > gatewayBarrier.right.x) {
					// decreasing to right, west
					incrementDirection = Direction.kWestInt;
				}
			}
			
			if (incrementDirection == -1) {
				// TODO: special case, single size gateway, we don't handle this yet
				assert false;
			}
			
			// we need to walk to the right and assing the room id to everyone
			java.awt.Point current = new java.awt.Point(gatewayBarrier.left);
			while (current.equals(gatewayBarrier.right) == false) {
				current.x += Direction.xDelta[incrementDirection];
				current.y += Direction.yDelta[incrementDirection];

				Cell cell = getCell(current);
				cell.addCellObject(new CellObject(theNewRoomObject));
			}

			// now we need to round up the four barriers
			ArrayList<Barrier> barrierList = new ArrayList<Barrier>();
			
			////////////////////
			// we can start by walking the wrong direction off the left endpoint
			doNewWall(gatewayBarrier.left, Direction.backwardOf[incrementDirection], barrierList);
			////////////////////
			
			////////////////////
			// then to the left of our left endpoint, and walk down the gateway to the left of the right endpoint
			doNewGateway(gatewayBarrier.left, gatewayBarrier.right, Direction.leftOf[incrementDirection], incrementDirection, roomNumber, barrierList);
			////////////////////
			
			////////////////////
			// next is just off the right endpoint
			doNewWall(gatewayBarrier.right, incrementDirection, barrierList);
			////////////////////

			////////////////////
			// then to the right of our right endpoint, and walk backwards down the gateway to the right of the left endpoint
			doNewGateway(gatewayBarrier.right, gatewayBarrier.left, Direction.rightOf[incrementDirection], Direction.backwardOf[incrementDirection], roomNumber, barrierList);
			////////////////////

			// Generate centerpoints and store room information
			generateCenterpoints(roomNumber, barrierList);
			this.roomBarrierMap.put(roomNumber, barrierList);
		}
	}
	
	public void generateCenterpoints(int roomNumber, ArrayList<Barrier> barrierList) {
		System.out.println("Room " + roomNumber + ":");
		Iterator<Barrier> iter = barrierList.iterator();
		while (iter.hasNext()) {
			Barrier barrier = iter.next();
			
			// generate centerpoint
			barrier.centerpoint();

			System.out.println(barrier);
		}
	}
	
	public CellObject getInObject(Point location) {
		if (!this.isInBounds(location)) {
			return null;
		}
		
		CellObject cellObject = getObject(location, Names.kRoomID);
		if (cellObject == null) {
			cellObject = getObject(location, Names.kGatewayID);
			if (cellObject == null) {
				return null;
			}
		}
		
		return cellObject;
	}
	
	public String toString() {
		String output = "";
		for (int y = 0; y < mapCells.length; ++y) {
			for (int x = 0; x < mapCells[y].length; ++x) {
				String cellString = y + "," + x + ":\n";
				Cell cell = mapCells[y][x];
				Iterator<CellObject> iter = cell.cellObjects.values().iterator();
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
}
