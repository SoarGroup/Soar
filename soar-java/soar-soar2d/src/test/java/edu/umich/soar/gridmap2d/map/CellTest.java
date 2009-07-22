package edu.umich.soar.gridmap2d.map;

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertEquals;

import java.util.Arrays;
import java.util.List;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import edu.umich.soar.config.Config;
import edu.umich.soar.gridmap2d.map.Cell;
import edu.umich.soar.gridmap2d.map.CellObject;
import edu.umich.soar.gridmap2d.map.CellObjectObserver;


public class CellTest {
	Cell cell;
	CellObject[] objects = new CellObject[3];
	int[] xyInitial = new int[] {0, 0};
	
	@Before
	public void setUp() throws Exception {
		cell = Cell.createCell(xyInitial);
		
		Config config;

		config = CellObjectHelper.createNewConfig("test0");
		config.setString("property0", "value0");
		config.setString("property", "value");
		objects[0] = new CellObject(config);

		config = CellObjectHelper.createNewConfig("test1");
		config.setString("property1", "value1");
		config.setString("property", "value");
		objects[1] = new CellObject(config);
		
		config = CellObjectHelper.createNewConfig("test2");
		config.setString("property2", "value2");
		config.setString("property", "value");
		objects[2] = new CellObject(config);
	}

	@After
	public void tearDown() throws Exception {
		
	}
	
	@Test
	public void testDraw() throws Exception {
		
		// starts with redraw
		assertTrue(cell.checkRedraw());
		assertTrue(cell.checkAndResetRedraw());
		assertFalse(cell.checkRedraw());

		cell.forceRedraw();
		assertTrue(cell.checkRedraw());
		assertTrue(cell.checkAndResetRedraw());
		
		cell.setPlayer(null);
		assertTrue(cell.checkAndResetRedraw());
		
		// no objects
		testDrawWontTrigger("nothing", "noproperty");
		cell.removeAllByProperty("nothing");
		assertFalse(cell.checkAndResetRedraw());
		cell.removeAll();
		assertFalse(cell.checkAndResetRedraw()); 
		cell.removeObject("nothing");
		assertFalse(cell.checkAndResetRedraw());	// nothing was removed

		// wrong object
		testDrawCheckedAdd();
		testDrawWontTrigger("wrong", "wrong-property");
		cell.removeAllByProperty("nothing");
		assertFalse(cell.checkAndResetRedraw());
		cell.removeObject("nothing");
		assertFalse(cell.checkAndResetRedraw());
		cell.removeAll();
		assertTrue(cell.checkAndResetRedraw()); 	// gets all

		// correct object
		testDrawCheckedAdd();
		testDrawWontTrigger("test0", "property0");
		
		cell.removeAllByProperty("property0");
		assertTrue(cell.checkAndResetRedraw());
		
		testDrawCheckedAdd();
		cell.removeObject("test0");
		assertTrue(cell.checkAndResetRedraw());
		
		testDrawCheckedAdd();
		cell.removeAll();
		assertTrue(cell.checkAndResetRedraw()); 	// gets all
	}
	
	void testDrawCheckedAdd() throws Exception {
		cell.addObject(objects[0]);
		assertTrue(cell.checkAndResetRedraw());
	}
	
	void testDrawWontTrigger(String objectName, String propertyName) {
		cell.getAll();
		assertFalse(cell.checkAndResetRedraw());
		
		cell.getAllWithProperty(propertyName);
		assertFalse(cell.checkAndResetRedraw());
		
		cell.hasAnyWithProperty(propertyName);
		assertFalse(cell.checkAndResetRedraw());
		
		cell.getObject(objectName);
		assertFalse(cell.checkAndResetRedraw());
		
		cell.hasObject(objectName);
		assertFalse(cell.checkAndResetRedraw());
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
		
		List<CellObject> removed = cell.removeAll();
		assertNotNull(removed);
		assertEquals(removed.size(), 3);
		
		cell.addObject(objects[0]);
		cell.addObject(objects[1]);
		cell.addObject(objects[2]);
		cell.addObject(objects[0]);
		cell.addObject(objects[1]);
		cell.addObject(objects[2]);

		List<CellObject> gotten = cell.getAll();
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
	
	@Test(expected = NullPointerException.class)
	public void testCreateNull() throws Exception {
		cell = Cell.createCell(null);
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
		
		List<CellObject> removals0 = cell.removeAllByProperty("property0");
		assertNotNull(removals0);
		assertEquals(removals0.size(), 1);

		List<CellObject> removals1 = cell.removeAllByProperty("property1");
		assertNotNull(removals1);
		assertEquals(removals1.size(), 1);

		List<CellObject> removals2 = cell.removeAllByProperty("property2");
		assertNotNull(removals2);
		assertEquals(removals2.size(), 1);

		cell.addObject(objects[0]);
		cell.addObject(objects[1]);
		cell.addObject(objects[2]);
		
		List<CellObject> removals = cell.removeAllByProperty("property");
		assertNotNull(removals);
		assertEquals(removals.size(), 3);
	}
	
	@Test
	public void testLocation() throws Exception {
		cell.getLocation()[0] += 1;
		cell.getLocation()[1] += 1;
		assertTrue(Arrays.equals(cell.getLocation(), xyInitial));
	}

	int observerAddCalled;
	int observerRemoveCalled;
	CellObject observerExpectedObject;
	
	class ObserverTester implements CellObjectObserver {
		public void reset() {
			observerAddCalled = 0;
			observerRemoveCalled = 0;
			observerExpectedObject = null;
		}
		public void addStateUpdate(int[] xy, CellObject object) {
			checkParams(xy, object);
			observerAddCalled += 1;
		}

		public void removalStateUpdate(int[] xy, CellObject object) {
			checkParams(xy, object);
			observerRemoveCalled += 1;
		}
		
		void checkParams(int[] xy, CellObject object) {
			assertNotNull(xy);
			assertTrue(Arrays.equals(xy, xyInitial));

			assertNotNull(object);
			if (observerExpectedObject != null) {
				assertEquals(object, observerExpectedObject);
			}
		}
	}
	
	ObserverTester observer = new ObserverTester();

	@Test
	public void testObserver() throws Exception {
		cell.addObserver(observer);

		observer.reset();
		cell.removeObject("test");
		assertEquals(observerAddCalled, 0);
		assertEquals(observerRemoveCalled, 0);
		
		observer.reset();
		observerExpectedObject = objects[0];
		cell.addObject(objects[0]);
		assertEquals(observerAddCalled, 1);
		assertEquals(observerRemoveCalled, 0);
		
		observer.reset();
		cell.hasObject(objects[0].getName());
		assertEquals(observerAddCalled, 0);
		assertEquals(observerRemoveCalled, 0);

		observer.reset();
		cell.getObject(objects[0].getName());
		assertEquals(observerAddCalled, 0);
		assertEquals(observerRemoveCalled, 0);
		
		observer.reset();
		observerExpectedObject = objects[0];
		cell.removeObject(objects[0].getName());
		assertEquals(observerAddCalled, 0);
		assertEquals(observerRemoveCalled, 1);
		
		observer.reset();
		cell.removeObject(objects[0].getName());
		assertEquals(observerAddCalled, 0);
		assertEquals(observerRemoveCalled, 0);
		
		observer.reset();
		cell.addObject(objects[0]);
		cell.addObject(objects[1]);
		cell.addObject(objects[2]);
		assertEquals(observerAddCalled, 3);
		assertEquals(observerRemoveCalled, 0);
		
		observer.reset();
		cell.removeAll();
		assertEquals(observerAddCalled, 0);
		assertEquals(observerRemoveCalled, 3);
		
		cell.addObject(objects[0]);
		cell.addObject(objects[1]);
		cell.addObject(objects[2]);

		observer.reset();
		cell.removeAllByProperty("property");
		assertEquals(observerAddCalled, 0);
		assertEquals(observerRemoveCalled, 3);
	}
}
