package edu.umich.soar.gridmap2d.map;

import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertEquals;

import java.io.File;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import edu.umich.soar.gridmap2d.map.EatersMap;

public class EatersMapTest {
	@Before
	public void setUp() {
	}
	
	@After
	public void tearDown() {
		
	}
	
	@Test
	public void testLoadAll() {
		File mapDir = new File("config/maps/eaters");
		
		for (File file : mapDir.listFiles()) {
			if (file.isFile()) {
				EatersMap map = EatersMap.generateInstance(file.getAbsolutePath(), false, .35, .85);
				assertNotNull(map);
			}
		}
	}
	
	@Test
	public void testBasicMap() {
		EatersMap eatersMap = EatersMap.generateInstance("config/maps/eaters/tiny.txt", false, .35, .85);
		assertNotNull(eatersMap);
		assertEquals(eatersMap.size(), 4);
	}
}
