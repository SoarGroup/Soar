package edu.umich.soar.gridmap2d.config;

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertEquals;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.junit.Before;
import org.junit.Test;

import edu.umich.soar.config.Config;
import edu.umich.soar.config.ConfigFile;

public class ConfigTest {
	class ConfigTestPair {
		ConfigTestPair(String key, String value) {
			this.key = key;
			this.value = value;
		}
		String key;
		String value;
	}
	
	final String largeTest = "test/org/msoar/gridmap2d/config/test.cnf";
	List<ConfigTestPair> largeTestData = new ArrayList<ConfigTestPair>();

	@Before
	public void setUp() throws Exception {
		largeTestData.add(new ConfigTestPair("hello", "[world]"));
		largeTestData.add(new ConfigTestPair("block.inside", "[true]"));
		largeTestData.add(new ConfigTestPair("block.outside", "[false]"));
		largeTestData.add(new ConfigTestPair("block", "[also with values]"));
		largeTestData.add(new ConfigTestPair("parent.child1.one", "[1]"));
		largeTestData.add(new ConfigTestPair("parent.child1.two", "[2]"));
		largeTestData.add(new ConfigTestPair("parent.child2.one", "[11]"));
		largeTestData.add(new ConfigTestPair("parent.child2.two", "[22]"));
		largeTestData.add(new ConfigTestPair("quotes", "[quotes]"));
		largeTestData.add(new ConfigTestPair("noquotes", "[noquotes]"));
		largeTestData.add(new ConfigTestPair("quotesspace", "[quotes space]"));
		largeTestData.add(new ConfigTestPair("noquotesspace", "[noquotesspace]"));
		largeTestData.add(new ConfigTestPair("single", "[data]"));
		largeTestData.add(new ConfigTestPair("single_element_array", "[data]"));
		largeTestData.add(new ConfigTestPair("trailing_comma_ok", "[data]"));
		largeTestData.add(new ConfigTestPair("two_element_array", "[data, banks]"));
		largeTestData.add(new ConfigTestPair("two_element_array_with_trailer", "[data, banks]"));
		largeTestData.add(new ConfigTestPair("array_no_quotes", "[data, banks]"));
		largeTestData.add(new ConfigTestPair("array_spaces_no_quotes", "[databanks, startrek]"));
		largeTestData.add(new ConfigTestPair("array_spaces_no_quotes_with_trailer", "[databanks, startrek]"));
		largeTestData.add(new ConfigTestPair("double_noquotes", "[1.23]"));
		largeTestData.add(new ConfigTestPair("double_quotes", "[1.23]"));
		largeTestData.add(new ConfigTestPair("split_across_lines", "[testing]"));
		largeTestData.add(new ConfigTestPair("array_line_split", "[hey, you, guys]"));
		largeTestData.add(new ConfigTestPair("array_line_split_noquotes", "[hey, you, guys]"));
		largeTestData.add(new ConfigTestPair("crazy.spacing", "[databanks]"));
		largeTestData.add(new ConfigTestPair("key1", "[value1]"));
		largeTestData.add(new ConfigTestPair("key2", "[value2]"));
		largeTestData.add(new ConfigTestPair("key3", "[value3.1, value3.2, value3.3]"));
		largeTestData.add(new ConfigTestPair("key4", "[value4.1, value4.2]"));
		largeTestData.add(new ConfigTestPair("key5", "[value5.1, value5.2]"));
		largeTestData.add(new ConfigTestPair("key6.subkey6.1", "[value6.1]"));
		largeTestData.add(new ConfigTestPair("key6.subkey6.2", "[value6.2]"));
		largeTestData.add(new ConfigTestPair("dashes-ok", "[yes]"));
		//largeTestData.add(new ConfigTestPair("null-array", "[]"));
		//largeTestData.add(new ConfigTestPair("null-value", null));
		//largeTestData.add(new ConfigTestPair("null-array-values", "[,,,]"));
		//largeTestData.add(new ConfigTestPair("nested_not_square", "[]"));
	}

	@Test
	public void testConfig() throws Exception {
		Config cf = new Config(new ConfigFile(largeTest));

		for (ConfigTestPair pair : largeTestData) {
			System.out.println(pair.key + ": " + Arrays.toString(cf.getStrings(pair.key)));
			if (pair.value != null) {
				assertEquals(pair.value, Arrays.toString(cf.getStrings(pair.key)));
			} else {
				assertTrue(cf.hasKey(pair.key));
			}
		}
	}
	
	@Test
	public void testDoubleChild() throws Exception {
		Config cf = new Config(new ConfigFile(largeTest));

		Config grandparent = cf.getChild("grandparent");
		Config parent = grandparent.getChild("parent");
		assertEquals("indeed", parent.getString("child"));
	}
	
	@Test
	public void testPropertyRemoval() throws Exception {
		Config cf = new Config(new ConfigFile(largeTest));

		assertTrue(cf.hasKey("hello"));
		cf.removeKey("hello");
		assertFalse(cf.hasKey("hello"));
	}

	@Test
	public void testGetKeys() throws Exception {
		Config cf = new Config(new ConfigFile(largeTest));

		String[] keys = cf.getKeys();
		Arrays.sort(keys);

		for (ConfigTestPair pair : largeTestData) {
			assertTrue(Arrays.binarySearch(keys, pair.key) >= 0);
		}
		
		Config grandparent = cf.getChild("grandparent");
		Config parent = grandparent.getChild("parent");

		for (String key : parent.getKeys()) {
			assertTrue(key.equals("child"));
		}
	}
}
