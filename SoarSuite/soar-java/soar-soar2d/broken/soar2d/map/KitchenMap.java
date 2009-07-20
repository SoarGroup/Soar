package broken.soar2d.map;

import java.util.Arrays;

import org.apache.log4j.Logger;

import soar2d.Names;

public class KitchenMap implements GridMap {
	private static Logger logger = Logger.getLogger(KitchenMap.class);

	public KitchenMap() {
	}
	
	boolean haveButter = false;
	int [] spawnButter;
	
	boolean haveSugar = false;
	int [] spawnSugar;
	
	boolean haveEggs = false;
	int [] spawnEggs;
	
	boolean haveFlour = false;
	int [] spawnFlour;
	
	boolean haveCinnamon = false;
	int [] spawnCinnamon;
	
	boolean haveMolasses = false;
	int [] spawnMolasses;
	
	public void spawnBasics() {
		if (!haveButter) {
			logger.info(Names.Info.spawningButter);
			this.addObjectToCell(spawnButter, this.cellObjectManager.createObject("butter"));
		}
		if (!haveSugar) {
			logger.info(Names.Info.spawningSugar);
			this.addObjectToCell(spawnSugar, this.cellObjectManager.createObject("sugar"));
		}
		if (!haveEggs) {
			logger.info(Names.Info.spawningEggs);
			this.addObjectToCell(spawnEggs, this.cellObjectManager.createObject("eggs"));
		}
		if (!haveFlour) {
			logger.info(Names.Info.spawningFlour);
			this.addObjectToCell(spawnFlour, this.cellObjectManager.createObject("flour"));
		}
		if (!haveCinnamon) {
			logger.info(Names.Info.spawningCinnamon);
			this.addObjectToCell(spawnCinnamon, this.cellObjectManager.createObject("cinnamon"));
		}
		if (!haveMolasses) {
			logger.info(Names.Info.spawningMolasses);
			this.addObjectToCell(spawnMolasses, this.cellObjectManager.createObject("molasses"));
		}
	}

	private void checkBasics(int [] location, CellObject object, boolean adding) {
		if (object.getName().equals("butter")) {
			assert adding ^ haveButter;
			haveButter = adding;
			if (spawnButter == null) {
				spawnButter = Arrays.copyOf(spawnButter, spawnButter.length);
			}
		}
		if (object.getName().equals("sugar")) {
			assert adding ^ haveSugar;
			haveSugar = adding;
			if (spawnSugar == null) {
				spawnSugar = Arrays.copyOf(spawnSugar, spawnSugar.length);
			}
		}
		if (object.getName().equals("eggs")) {
			assert adding ^ haveEggs;
			haveEggs = adding;
			if (spawnEggs == null) {
				spawnEggs = Arrays.copyOf(spawnEggs, spawnEggs.length);
			}
		}
		if (object.getName().equals("flour")) {
			assert adding ^ haveFlour;
			haveFlour = adding;
			if (spawnFlour == null) {
				spawnFlour = Arrays.copyOf(spawnFlour, spawnFlour.length);
			}
		}
		if (object.getName().equals("cinnamon")) {
			assert adding ^ haveCinnamon;
			haveCinnamon = adding;
			if (spawnCinnamon == null) {
				spawnCinnamon = Arrays.copyOf(spawnCinnamon, spawnCinnamon.length);
			}
		}
		if (object.getName().equals("molasses")) {
			assert adding ^ haveMolasses;
			haveMolasses = adding;
			if (spawnMolasses == null) {
				spawnMolasses = Arrays.copyOf(spawnMolasses, spawnMolasses.length);
			}
		}
	}
	
	@Override
	public boolean isAvailable(int [] location) {
		Cell cell = getCell(location);
		boolean enterable = !cell.hasAnyWithProperty(Names.kPropertyBlock);
		boolean noPlayer = cell.getPlayer() == null;
		return enterable && noPlayer;
	}

	public boolean isInBounds(int[] xy) {
		return data.cells.isInBounds(xy);
	}
	
	public boolean isCountertop(int [] location) {
		return getCell(location).getObject("countertop") != null;
	}
	
	public boolean isOven(int [] location) {
		return getCell(location).getObject("oven") != null;
	}
	@Override
	void addStateUpdate(int [] location, CellObject added) {
		super.addStateUpdate(location, added);
		// Update state we keep track of specific to game type
		checkBasics(location, added, true);
	}
	
	@Override
	void removalStateUpdate(CellObject removed) {
		super.removalStateUpdate(removed);
		checkBasics(null, removed, false);
	}

	public CellObject createObjectByName(String name) {
		return data.cellObjectManager.createObject(name);
	}
}
