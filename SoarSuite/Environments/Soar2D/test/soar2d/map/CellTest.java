package soar2d.map;

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertEquals;

import java.util.ArrayList;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

public class CellTest {
	Cell cell;
	CellObject[] objects = new CellObject[3];

	@Before
	public void setUp() throws Exception {
		cell = Cell.createCell(true, new int[] {0, 0});
		
		objects[0] = new CellObject("test0");
		objects[0].addProperty("property0", "value0");
		objects[0].addProperty("property", "value");
		objects[1] = new CellObject("test1");
		objects[1].addProperty("property1", "value1");
		objects[1].addProperty("property", "value");
		objects[2] = new CellObject("test2");
		objects[2].addProperty("property2", "value2");
		objects[2].addProperty("property", "value");
	}

	@After
	public void tearDown() throws Exception {
		
	}
	
	@Test
	public void testDraw() throws Exception {
		
		assertTrue(cell.checkRedraw());
		assertTrue(cell.resetRedraw());
		assertFalse(cell.checkRedraw());

		cell.forceRedraw();
		assertTrue(cell.checkRedraw());
		
		assertTrue(cell.resetRedraw());
		assertFalse(cell.checkRedraw());
		
		cell.setPlayer(null);
		assertTrue(cell.resetRedraw());
		assertFalse(cell.checkRedraw());
		
		cell.addObject(objects[0]);
		assertTrue(cell.resetRedraw());
		assertFalse(cell.checkRedraw());
		
		cell.getAllWithProperty("test");
		assertFalse(cell.resetRedraw());

		cell.getAll();
		assertFalse(cell.resetRedraw());
		
		cell.hasAnyWithProperty("test");
		assertFalse(cell.resetRedraw());
		
		cell.removeAllByProperty("test");
		assertTrue(cell.resetRedraw());
		assertFalse(cell.checkRedraw());

		cell.removeAll();
		assertTrue(cell.resetRedraw());
		assertFalse(cell.checkRedraw());
		
		cell.getObject("test");
		assertFalse(cell.resetRedraw());
		
		cell.hasObject("test");
		assertFalse(cell.resetRedraw());
		
		cell.removeObject("test");
		assertTrue(cell.resetRedraw());
		assertFalse(cell.checkRedraw());
	}

	@Test
	public void testObject() throws Exception {

		assertFalse(cell.hasObject("test"));
		assertNull(cell.getObject("test"));
		assertNull(cell.removeObject("test"));
		
		cell.addObject(objects[0]);
		assertTrue(cell.hasObject(objects[0].getName()));
		assertEquals(cell.getObject(objects[0].getName()), objects[0]);
		assertEquals(cell.removeObject(objects[0].getName()), objects[0]);
		assertNull(cell.removeObject(objects[0].getName()));
		
		cell.addObject(objects[0]);
		cell.addObject(objects[1]);
		cell.addObject(objects[2]);
		
		ArrayList<CellObject> removed = cell.removeAll();
		assertNotNull(removed);
		assertEquals(removed.size(), 3);
		
		cell.addObject(objects[0]);
		cell.addObject(objects[1]);
		cell.addObject(objects[2]);
		cell.addObject(objects[0]);
		cell.addObject(objects[1]);
		cell.addObject(objects[2]);

		ArrayList<CellObject> gotten = cell.getAll();
		assertNotNull(gotten);
		assertEquals(gotten.size(), 3);

		gotten = cell.getAll();
		assertNotNull(gotten);
		assertEquals(gotten.size(), 3);
		
		removed = cell.removeAll();
		assertNotNull(removed);
		assertEquals(removed.size(), 3);
		assertNull(cell.removeAll());
		assertNull(cell.getAll());
		
		cell.addObject(objects[0]);
		cell.addObject(objects[1]);
		cell.addObject(objects[2]);
		
		assertTrue(cell.hasObject(objects[1].getName()));
		assertEquals(cell.getObject(objects[1].getName()), objects[1]);
		assertEquals(cell.removeObject(objects[1].getName()), objects[1]);
		assertNull(cell.removeObject(objects[1].getName()));
	}
	
	@Test(expected = NullPointerException.class)
	public void testAddNull() throws Exception {
		cell.addObject(null);
	}
	
	@Test(expected = NullPointerException.class)
	public void testGetNull() throws Exception {
		cell.getObject(null);
	}
	
	@Test(expected = NullPointerException.class)
	public void testRemoveNull() throws Exception {
		cell.removeObject(null);
	}
	
	@Test
	public void testNullParams() throws Exception {
		assertNull(cell.getAllWithProperty(null));
		assertFalse(cell.hasAnyWithProperty(null));
		assertFalse(cell.hasObject(null));
		assertNull(cell.removeAllByProperty(null));
		cell.setPlayer(null);
	}

	@Test
	public void testPropertyOperations() throws Exception {
		assertFalse(cell.hasAnyWithProperty("test"));
		assertNull(cell.getAllWithProperty("test"));
		assertNull(cell.removeAllByProperty("test"));
		
		cell.addObject(objects[0]);
		cell.addObject(objects[1]);
		cell.addObject(objects[2]);
		
		assertNotNull(cell.getAllWithProperty("property"));
		assertEquals(cell.getAllWithProperty("property").size(), 3);
		assertNotNull(cell.getAll());
		assertEquals(cell.getAll().size(), 3);
		assertNotNull(cell.getAllWithProperty("property0"));
		assertEquals(cell.getAllWithProperty("property0").size(), 1);
		assertNotNull(cell.getAllWithProperty("property1"));
		assertEquals(cell.getAllWithProperty("property1").size(), 1);
		assertNotNull(cell.getAllWithProperty("property2"));
		assertEquals(cell.getAllWithProperty("property2").size(), 1);
		
		ArrayList<CellObject> removals0 = cell.removeAllByProperty("property0");
		assertNotNull(removals0);
		assertEquals(removals0.size(), 1);

		ArrayList<CellObject> removals1 = cell.removeAllByProperty("property1");
		assertNotNull(removals1);
		assertEquals(removals1.size(), 1);

		ArrayList<CellObject> removals2 = cell.removeAllByProperty("property2");
		assertNotNull(removals2);
		assertEquals(removals2.size(), 1);

		cell.addObject(objects[0]);
		cell.addObject(objects[1]);
		cell.addObject(objects[2]);
		
		ArrayList<CellObject> removals = cell.removeAllByProperty("property");
		assertNotNull(removals);
		assertEquals(removals.size(), 3);
	}
}
