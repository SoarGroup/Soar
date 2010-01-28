package edu.umich.soar.room;

import static org.junit.Assert.assertTrue;

import java.util.Arrays;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;

import edu.umich.soar.room.core.Direction;

public class DirectionTest {

	@Before
	public void setUp() {
		
	}

	@After
	public void tearDown() {
		
	}
	
	@Test
	public void testRelatives() {
		assertTrue(Direction.NORTH == Direction.SOUTH.backward());
		assertTrue(Direction.EAST == Direction.WEST.backward());
		assertTrue(Direction.SOUTH == Direction.NORTH.backward());
		assertTrue(Direction.WEST == Direction.EAST.backward());
		
		assertTrue(Direction.NORTH == Direction.EAST.left());
		assertTrue(Direction.EAST == Direction.SOUTH.left());
		assertTrue(Direction.SOUTH == Direction.WEST.left());
		assertTrue(Direction.WEST == Direction.NORTH.left());
		
		assertTrue(Direction.NORTH == Direction.WEST.right());
		assertTrue(Direction.EAST == Direction.NORTH.right());
		assertTrue(Direction.SOUTH == Direction.EAST.right());
		assertTrue(Direction.WEST == Direction.SOUTH.right());
		
	}
	
	@Test
	public void testTranslation() {
		int[] point = new int[] { 5, 5 };

		point = Direction.translate(point, Direction.NONE);
		assertTrue(Arrays.equals(point, new int[] {5, 5}));

		point = Direction.translate(point, Direction.NORTH);
		assertTrue(Arrays.equals(point, new int[] {5, 4}));

		point = Direction.translate(point, Direction.EAST);
		assertTrue(Arrays.equals(point, new int[] {6, 4}));

		point = Direction.translate(point, Direction.SOUTH);
		assertTrue(Arrays.equals(point, new int[] {6, 5}));

		point = Direction.translate(point, Direction.WEST);
		assertTrue(Arrays.equals(point, new int[] {5, 5}));

		int[] target;
		target = Direction.translate(point, Direction.NONE, new int[2]);
		assertTrue(point != target);
		assertTrue(Arrays.equals(point, new int[] {5, 5}));
		assertTrue(Arrays.equals(target, new int[] {5, 5}));

		target = Direction.translate(point, Direction.SOUTH, new int[2]);
		assertTrue(Arrays.equals(point, new int[] {5, 5}));
		assertTrue(Arrays.equals(target, new int[] {5, 6}));

		target = Direction.translate(point, Direction.WEST, new int[2]);
		assertTrue(Arrays.equals(point, new int[] {5, 5}));
		assertTrue(Arrays.equals(target, new int[] {4, 5}));

		target = Direction.translate(point, Direction.NORTH, new int[2]);
		assertTrue(Arrays.equals(point, new int[] {5, 5}));
		assertTrue(Arrays.equals(target, new int[] {5, 4}));

		target = Direction.translate(point, Direction.EAST, new int[2]);
		assertTrue(Arrays.equals(point, new int[] {5, 5}));
		assertTrue(Arrays.equals(target, new int[] {6, 5}));
	}

	@Test
	public void testNoneIs0() {
		assertTrue(Direction.values()[0] == Direction.NONE);
		assertTrue(Direction.values()[1] != Direction.NONE);
		assertTrue(Direction.values()[2] != Direction.NONE);
		assertTrue(Direction.values()[3] != Direction.NONE);
		assertTrue(Direction.values()[4] != Direction.NONE);
	}
	
	@Test
	public void testIdParse() {
		for(Direction dir : Direction.values()) {
			assertTrue(dir == Direction.parse(dir.id()));
		}
	}
}
