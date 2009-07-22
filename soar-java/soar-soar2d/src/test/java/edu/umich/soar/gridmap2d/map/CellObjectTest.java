package edu.umich.soar.gridmap2d.map;

import static org.junit.Assert.assertFalse;

import org.junit.Test;

import edu.umich.soar.config.Config;
import edu.umich.soar.gridmap2d.map.CellObject;


public class CellObjectTest {
	@Test
	public void testEquals() {
		CellObject a = new CellObject(CellObjectHelper.createNewConfig("a"));
		CellObject b = new CellObject(CellObjectHelper.createNewConfig("b"));
		assertFalse(a.equals(b));

		CellObject a1 = new CellObject(CellObjectHelper.createNewConfig("a"));
		CellObject a2 = new CellObject(CellObjectHelper.createNewConfig("a"));
		assertFalse(a1.equals(a2));
	}
	
	@Test(expected = NullPointerException.class)
	public void testNullConfig() {
		Config c = null;
		new CellObject(c);
	}
	
	@Test(expected = NullPointerException.class)
	public void testNullCopyConstructor() {
		CellObject c = null;
		new CellObject(c);
	}
	
	// TODO: double check applyProperties works for multiple properties because it deletes during the walk!
}
