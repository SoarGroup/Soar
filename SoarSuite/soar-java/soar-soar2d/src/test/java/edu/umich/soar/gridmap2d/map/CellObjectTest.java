package edu.umich.soar.gridmap2d.map;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertNotNull;

import org.junit.Before;
import org.junit.Test;

import edu.umich.soar.config.Config;
import edu.umich.soar.config.ConfigFile;
import edu.umich.soar.gridmap2d.map.CellObject;


public class CellObjectTest {
	
	Config configA;
	Config configB;
	Config configC;
	
	@Before
	public void setUp() {
		{
			ConfigFile cf = new ConfigFile();
			configA = cf.getConfig();
		}
        configA.setString("name", "a");

		{
			ConfigFile cf = new ConfigFile();
			configB = cf.getConfig();
		}
        configB.setString("name", "b");

		{
			ConfigFile cf = new ConfigFile();
			configC = cf.getConfig();
		}
        configC.setString("count", "3");
        configC.setString("weight", "4.2");
        configC.setString("tolerance", "0.00000007");
        configC.setString("visible", "true");
	}
	
	@Test
	public void testEquals() {
		CellObject a = new CellObject(configA);
		CellObject b = new CellObject(configB);
		assertFalse(a.equals(b));
		assertTrue(a.equals(a));

		CellObject a1 = new CellObject(configA);
		CellObject a2 = new CellObject(configA);
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
	
	@Test(expected = NullPointerException.class)
	public void testNullKey() {
		CellObject a = new CellObject(configA);
		a.setProperty(null, "void");
	}
	
	@Test(expected = NullPointerException.class)
	public void testNullValue() {
		CellObject a = new CellObject(configA);
		a.setProperty("void", null);
	}
	
	@Test
	public void testCopy() {
		CellObject a1 = new CellObject(configA);
		CellObject a2 = new CellObject(a1);
		assertFalse(a1.equals(a2));
	}
	
	@Test
	public void testProperties() {
		CellObject a = new CellObject(configA);
		
		assertTrue(a.hasProperty("name"));
		assertFalse(a.hasProperty("foo"));
		assertFalse(a.hasProperty("Name"));
		assertEquals(a.getProperty("name"), "a");
		assertEquals(a.getProperty("name", "b", String.class), "a");

		a.setProperty("name", "edward");
		
		assertTrue(a.hasProperty("name"));
		assertFalse(a.hasProperty("foo"));
		assertFalse(a.hasProperty("Name"));
		assertEquals(a.getProperty("name"), "edward");
		assertEquals(a.getProperty("name", "b", String.class), "edward");
		
		a.setProperty("foo", "bar");
		
		assertTrue(a.hasProperty("name"));
		assertTrue(a.hasProperty("foo"));
		assertFalse(a.hasProperty("Name"));
		assertEquals(a.getProperty("name"), "edward");
		assertEquals(a.getProperty("name", "b", String.class), "edward");
		assertEquals(a.getProperty("foo"), "bar");
		
		assertTrue(a.removeProperty("foo"));

		assertTrue(a.hasProperty("name"));
		assertFalse(a.hasProperty("foo"));
		assertNull(a.getProperty("foo"));
		assertNull(a.getProperty("foo", null, String.class));
		assertEquals(a.getProperty("foo", "baz", String.class), "baz");
		assertFalse(a.hasProperty("Name"));
		assertEquals(a.getProperty("name"), "edward");
		assertEquals(a.getProperty("name", "b", String.class), "edward");

		assertEquals(a.removeProperty("name", String.class), "edward");

		assertFalse(a.hasProperty("name"));
		assertFalse(a.hasProperty("foo"));
		assertNull(a.getProperty("name"));
		assertEquals(a.getProperty("name", "b", String.class), "b");
		
		a.setProperty("name", "edward");

		assertTrue(a.hasProperty("name"));
		assertFalse(a.hasProperty("foo"));
		assertNull(a.getProperty("foo"));
		assertNull(a.getProperty("foo", null, String.class));
		assertEquals(a.getProperty("foo", "baz", String.class), "baz");
		assertFalse(a.hasProperty("Name"));
		assertEquals(a.getProperty("name"), "edward");
		assertEquals(a.getProperty("name", "b", String.class), "edward");
	}

	@Test
	public void testApplyProperties() {
		CellObject a = new CellObject(configA);

		a.setProperty("apply.properties.windows", "xp");
		a.setProperty("apply.properties.linux", "ubuntu");
		a.setProperty("apply.properties.mac", "osx");
		
		assertTrue(a.hasProperty("apply.properties.windows"));
		assertTrue(a.hasProperty("apply.properties.linux"));
		assertTrue(a.hasProperty("apply.properties.mac"));
		
		assertFalse(a.hasProperty("windows"));
		assertFalse(a.hasProperty("linux"));
		assertFalse(a.hasProperty("mac"));
		
		a.doApplyProperties();
		
		assertFalse(a.hasProperty("apply.properties.windows"));
		assertFalse(a.hasProperty("apply.properties.linux"));
		assertFalse(a.hasProperty("apply.properties.mac"));
		
		assertTrue(a.hasProperty("windows"));
		assertTrue(a.hasProperty("linux"));
		assertTrue(a.hasProperty("mac"));
	}
	
	@Test
	public void testComplex() {
		CellObject c = new CellObject(configC);
		
		assertTrue(c.hasProperty("count"));
		assertTrue(c.hasProperty("weight"));
		assertTrue(c.hasProperty("tolerance"));
		assertTrue(c.hasProperty("visible"));
		
		assertNotNull(c.getProperty("count"));
		assertNotNull(c.getProperty("weight"));
		assertNotNull(c.getProperty("tolerance"));
		assertNotNull(c.getProperty("visible"));
		
		Integer count = c.getProperty("count", Integer.class);
		Float weight = c.getProperty("weight", Float.class);
		Double tolerance = c.getProperty("tolerance", Double.class);
		Boolean visible = c.getProperty("visible", Boolean.class);

		assertTrue(count == 3);
		assertNotNull(weight);
		assertNotNull(tolerance);
		assertTrue(visible);
		
		assertNotNull(c.getProperty("count"));
		assertNotNull(c.getProperty("weight"));
		assertNotNull(c.getProperty("tolerance"));
		assertNotNull(c.getProperty("visible"));
		
		assertNotNull(c.getProperty("count", Integer.class));
		assertNotNull(c.getProperty("weight", Float.class));
		assertNotNull(c.getProperty("tolerance", Double.class));
		assertNotNull(c.getProperty("visible", Boolean.class));
	}
}
