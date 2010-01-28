package edu.umich.soar.gridmap2d.map;

import java.io.File;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import static org.junit.Assert.assertNotNull;

import edu.umich.soar.gridmap2d.map.TankSoarMap;

public class TanksoarMapTest {
	@Before
	public void setUp()  {
	}
	
	@After
	public void tearDown() {
		
	}
	
	@Test
	public void testLoadAll() {
		File mapDir = new File("config/maps/tanksoar");
		
		for (File file : mapDir.listFiles()) {
			if (file.isFile()) {
				TankSoarMap map = TankSoarMap.generateInstance(file.getAbsolutePath(), 7);
				assertNotNull(map);
			}
		}
	}
}
