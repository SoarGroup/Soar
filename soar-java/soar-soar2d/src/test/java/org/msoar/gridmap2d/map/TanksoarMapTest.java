package org.msoar.gridmap2d.map;

import java.io.File;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class TanksoarMapTest {
	@Before
	public void setUp() throws Exception {
	}
	
	@After
	public void tearDown() throws Exception {
		
	}
	
	@Test
	public void testLoadAll() throws Exception {
		File mapDir = new File("config/maps/tanksoar");
		
		for (File file : mapDir.listFiles()) {
			if (file.isFile()) {
				new TankSoarMap(file.getAbsolutePath(), 7);
			}
		}
	}
}
