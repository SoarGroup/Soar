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
	public void setUp() throws Exception {
	}
	
	@After
	public void tearDown() throws Exception {
		
	}
	
	@Test
	public void testLoadAll() throws Exception {
		File mapDir = new File("config/maps/eaters");
		
		for (File file : mapDir.listFiles()) {
			if (file.isFile()) {
				new EatersMap(file.getAbsolutePath(), false, .35, .85);
			}
		}
	}
	
	@Test
	public void testBasicMap() throws Exception {
		EatersMap eatersMap = new EatersMap("config/maps/eaters/tiny.txt", false, .35, .85);
		assertEquals(eatersMap.size(), 4);

		int[] xy = new int[2];
		for (xy[0] = 0; xy[0] < eatersMap.size(); xy[0]++)
			for (xy[1] = 0; xy[1] < eatersMap.size(); xy[1]++)
					assertNotNull(eatersMap.getCell(xy));
	}
}
