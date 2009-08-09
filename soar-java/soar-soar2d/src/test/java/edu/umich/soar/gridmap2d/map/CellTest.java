package edu.umich.soar.gridmap2d.map;

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertEquals;

import java.util.List;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import edu.umich.soar.config.Config;
import edu.umich.soar.gridmap2d.map.CellObject;
import edu.umich.soar.gridmap2d.map.CellObjectObserver;

public class CellTest {
	Cell cell;
	CellObject[] objects = new CellObject[3];
	
	@Before
	public void setUp() {
		cell = Cells.createCell();
		
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
	public void tearDown() {
		
	}
	
	@Test
	public void testDraw() {
		
		// starts with redraw
		assertTrue(cell.checkRedraw());
		assertTrue(cell.checkAndResetRedraw());
		assertFalse(cell.checkRedraw());

		cell.forceRedraw();
		assertTrue(cell.checkRedraw());
		assertTrue(cell.checkAndResetRedraw());
		
		// no objects
		testDrawWontTrigger("nothing", "noproperty");
		cell.removeAllObjectsByProperty("nothing");
		assertFalse(cell.checkAndResetRedraw());
		cell.removeAllObjects();
		assertFalse(cell.checkAndResetRedraw()); 

		// wrong object
		testDrawCheckedAdd();
		testDrawWontTrigger("wrong", "wrong-property");
		cell.removeAllObjectsByProperty("nothing");
		assertFalse(cell.checkAndResetRedraw());
		cell.removeAllObjects();
		assertTrue(cell.checkAndResetRedraw()); 	// gets all

		// correct object
		testDrawCheckedAdd();
		testDrawWontTrigger("test0", "property0");
		
		cell.removeAllObjectsByProperty("property0");
		assertTrue(cell.checkAndResetRedraw());
		
		testDrawCheckedAdd();
		cell.removeAllObjects();
		assertTrue(cell.checkAndResetRedraw()); 	// gets all
	}
	
	void testDrawCheckedAdd() {
		cell.addObject(objects[0]);
		assertTrue(cell.checkAndResetRedraw());
	}
	
	void testDrawWontTrigger(String objectName, String propertyName) {
		cell.getAllObjects();
		assertFalse(cell.checkAndResetRedraw());
		
		cell.getAllObjectsWithProperty(propertyName);
		assertFalse(cell.checkAndResetRedraw());
		
		cell.hasAnyObjectWithProperty(propertyName);
		assertFalse(cell.checkAndResetRedraw());
	}

	@Test
	public void testObject() {

		cell.addObject(objects[0]);
		assertTrue(cell.hasObject(objects[0]));
		assertTrue(cell.removeObject(objects[0]));
		assertFalse(cell.removeObject(objects[0]));
		
		cell.addObject(objects[0]);
		cell.addObject(objects[1]);
		cell.addObject(objects[2]);
		
		List<CellObject> removed = cell.removeAllObjects();
		assertNotNull(removed);
		assertEquals(removed.size(), 3);
		
		cell.addObject(objects[0]);
		cell.addObject(objects[1]);
		cell.addObject(objects[2]);
		cell.addObject(objects[0]);
		cell.addObject(objects[1]);
		cell.addObject(objects[2]);

		List<CellObject> gotten = cell.getAllObjects();
		assertNotNull(gotten);
		assertEquals(gotten.size(), 3);

		gotten = cell.getAllObjects();
		assertNotNull(gotten);
		assertEquals(gotten.size(), 3);
		
		removed = cell.removeAllObjects();
		assertNotNull(removed);
		assertEquals(removed.size(), 3);
		assertTrue(cell.removeAllObjects().isEmpty());
		assertTrue(cell.getAllObjects().isEmpty());
		
		cell.addObject(objects[0]);
		cell.addObject(objects[1]);
		cell.addObject(objects[2]);
		
		assertTrue(cell.hasObject(objects[1]));
		assertTrue(cell.removeObject(objects[1]));
		assertFalse(cell.removeObject(objects[1]));
	}
	
	@Test(expected = NullPointerException.class)
	public void testAddNull() {
		cell.addObject(null);
	}
	
	@Test
	public void testNullParams() {
		assertFalse(cell.hasAnyObjectWithProperty(null));
		assertFalse(cell.hasObject(null));
	}

	@Test
	public void testPropertyOperations() {
		assertFalse(cell.hasAnyObjectWithProperty("test"));
		assertTrue(cell.getAllObjectsWithProperty("test").isEmpty());
		assertNotNull(cell.removeAllObjectsByProperty("test"));
		
		cell.addObject(objects[0]);
		cell.addObject(objects[1]);
		cell.addObject(objects[2]);
		
		assertNotNull(cell.getAllObjectsWithProperty("property"));
		assertEquals(cell.getAllObjectsWithProperty("property").size(), 3);
		assertNotNull(cell.getAllObjects());
		assertEquals(cell.getAllObjects().size(), 3);
		assertNotNull(cell.getAllObjectsWithProperty("property0"));
		assertEquals(cell.getAllObjectsWithProperty("property0").size(), 1);
		assertNotNull(cell.getAllObjectsWithProperty("property1"));
		assertEquals(cell.getAllObjectsWithProperty("property1").size(), 1);
		assertNotNull(cell.getAllObjectsWithProperty("property2"));
		assertEquals(cell.getAllObjectsWithProperty("property2").size(), 1);
		
		List<CellObject> removals0 = cell.removeAllObjectsByProperty("property0");
		assertNotNull(removals0);
		assertEquals(removals0.size(), 1);

		List<CellObject> removals1 = cell.removeAllObjectsByProperty("property1");
		assertNotNull(removals1);
		assertEquals(removals1.size(), 1);

		List<CellObject> removals2 = cell.removeAllObjectsByProperty("property2");
		assertNotNull(removals2);
		assertEquals(removals2.size(), 1);

		cell.addObject(objects[0]);
		cell.addObject(objects[1]);
		cell.addObject(objects[2]);
		
		List<CellObject> removals = cell.removeAllObjectsByProperty("property");
		assertNotNull(removals);
		assertEquals(removals.size(), 3);
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
		
		@Override
		public void addStateUpdate(CellObject object) {
			checkParams(object);
			observerAddCalled += 1;
		}

		@Override
		public void removalStateUpdate(CellObject object) {
			checkParams(object);
			observerRemoveCalled += 1;
		}
		
		void checkParams(CellObject object) {
			assertNotNull(object);
			if (observerExpectedObject != null) {
				assertEquals(object, observerExpectedObject);
			}
		}
	}
	
	ObserverTester observer = new ObserverTester();

	@Test
	public void testObserver() {
		cell.addObserver(observer);

		observer.reset();
		observerExpectedObject = objects[0];
		cell.addObject(objects[0]);
		assertEquals(observerAddCalled, 1);
		assertEquals(observerRemoveCalled, 0);
		
		observer.reset();
		cell.hasObject(objects[0]);
		assertEquals(observerAddCalled, 0);
		assertEquals(observerRemoveCalled, 0);

		observer.reset();
		observerExpectedObject = objects[0];
		cell.removeObject(objects[0]);
		assertEquals(observerAddCalled, 0);
		assertEquals(observerRemoveCalled, 1);
		
		observer.reset();
		cell.removeObject(objects[0]);
		assertEquals(observerAddCalled, 0);
		assertEquals(observerRemoveCalled, 0);
		
		observer.reset();
		cell.addObject(objects[0]);
		cell.addObject(objects[1]);
		cell.addObject(objects[2]);
		assertEquals(observerAddCalled, 3);
		assertEquals(observerRemoveCalled, 0);
		
		observer.reset();
		cell.removeAllObjects();
		assertEquals(observerAddCalled, 0);
		assertEquals(observerRemoveCalled, 3);
		
		cell.addObject(objects[0]);
		cell.addObject(objects[1]);
		cell.addObject(objects[2]);

		observer.reset();
		cell.removeAllObjectsByProperty("property");
		assertEquals(observerAddCalled, 0);
		assertEquals(observerRemoveCalled, 3);
	}
}
