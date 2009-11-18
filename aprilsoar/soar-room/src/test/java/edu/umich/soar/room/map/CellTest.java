package edu.umich.soar.room.map;

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertEquals;

import java.util.Set;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import edu.umich.soar.room.map.Cell;
import edu.umich.soar.room.map.CellObject;
import edu.umich.soar.room.map.CellObjectObserver;
import edu.umich.soar.room.map.Cells;
import edu.umich.soar.room.map.Robot;

public class CellTest {
	Cell cell;
	CellObject[] objects = new CellObject[3];
	
	@Before
	public void setUp() {
		cell = Cells.createCell(new int[] { 0, 0 });
		objects[0] = new CellObject();
		objects[1] = new CellObject();
		objects[2] = new CellObject();

		objects[0].setProperty("name", "test0");
		objects[0].setProperty("property0", "value0");
		objects[0].setProperty("property", "value");

		objects[1].setProperty("name", "test1");
		objects[1].setProperty("property1", "value1");
		objects[1].setProperty("property", "value");
		
		objects[2].setProperty("name", "test2");
		objects[2].setProperty("property2", "value2");
		objects[2].setProperty("property", "value");
	}

	@After
	public void tearDown() {
		
	}
	
	@Test
	public void testDraw() {
		
		// starts with redraw
		assertTrue(cell.isModified());
		cell.setModified(false);
		assertFalse(cell.isModified());

		cell.setModified(true);
		assertTrue(cell.isModified());
		cell.setModified(false);
		
		// no objects
		testDrawWontTrigger("nothing", "noproperty");
		cell.removeAllObjectsByProperty("nothing");
		assertFalse(cell.isModified());
		cell.removeAllObjects();
		assertFalse(cell.isModified()); 

		// wrong object
		testDrawCheckedAdd();
		testDrawWontTrigger("wrong", "wrong-property");
		cell.removeAllObjectsByProperty("nothing");
		assertFalse(cell.isModified());
		cell.removeAllObjects();
		assertTrue(cell.isModified()); 	// gets all
		cell.setModified(false);
		
		// correct object
		testDrawCheckedAdd();
		testDrawWontTrigger("test0", "property0");
		
		cell.removeAllObjectsByProperty("property0");
		assertTrue(cell.isModified());
		cell.setModified(false);

		testDrawCheckedAdd();
		cell.removeAllObjects();
		assertTrue(cell.isModified()); 	// gets all
		cell.setModified(false);
	}
	
	void testDrawCheckedAdd() {
		cell.addObject(objects[0]);
		assertTrue(cell.isModified());
		cell.setModified(false);
	}
	
	void testDrawWontTrigger(String objectName, String propertyName) {
		cell.getAllObjects();
		assertFalse(cell.isModified());
		
		cell.getAllObjectsWithProperty(propertyName);
		assertFalse(cell.isModified());
		
		cell.hasObjectWithProperty(propertyName);
		assertFalse(cell.isModified());
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
		
		Set<CellObject> removed = cell.removeAllObjects();
		assertNotNull(removed);
		assertEquals(removed.size(), 3);
		
		cell.addObject(objects[0]);
		cell.addObject(objects[1]);
		cell.addObject(objects[2]);
		cell.addObject(objects[0]);
		cell.addObject(objects[1]);
		cell.addObject(objects[2]);

		Set<CellObject> gotten = cell.getAllObjects();
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
	public void testAddPlayerNull() {
		cell.addPlayer((Robot)null);
	}
	
	@Test(expected = NullPointerException.class)
	public void testAddObjectNull() {
		cell.addObject((CellObject)null);
	}
	
	@Test(expected = NullPointerException.class)
	public void testHasObjectNull() {
		cell.hasObjectWithProperty(null);
	}
	
	@Test(expected = NullPointerException.class)
	public void testHasPlayerNull() {
		cell.hasObject(null);
	}
	
	@Test
	public void testPropertyOperations() {
		assertFalse(cell.hasObjectWithProperty("test"));
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
		
		Set<CellObject> removals0 = cell.removeAllObjectsByProperty("property0");
		assertNotNull(removals0);
		assertEquals(removals0.size(), 1);

		Set<CellObject> removals1 = cell.removeAllObjectsByProperty("property1");
		assertNotNull(removals1);
		assertEquals(removals1.size(), 1);

		Set<CellObject> removals2 = cell.removeAllObjectsByProperty("property2");
		assertNotNull(removals2);
		assertEquals(removals2.size(), 1);

		cell.addObject(objects[0]);
		cell.addObject(objects[1]);
		cell.addObject(objects[2]);
		
		Set<CellObject> removals = cell.removeAllObjectsByProperty("property");
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
		Cells.addObserver(observer);

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
