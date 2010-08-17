package edu.umich.dice;

import java.math.BigInteger;

import org.junit.Test;
import org.junit.Assert;

import edu.umich.dice.LiarsDice.Predicate;

public class TestLiarsDice
{
	@Test
	public void testFactorial()
	{
		Assert.assertEquals(BigInteger.ONE, LiarsDice.factorial(0));
		Assert.assertEquals(BigInteger.ONE, LiarsDice.factorial(1));
		Assert.assertEquals(BigInteger.valueOf(2), LiarsDice.factorial(2));
		Assert.assertEquals(BigInteger.valueOf(6), LiarsDice.factorial(3));
		Assert.assertEquals(BigInteger.valueOf(120), LiarsDice.factorial(5));
		Assert.assertEquals(BigInteger.valueOf(362880), LiarsDice.factorial(9));
	}
	
	@Test(expected = IllegalArgumentException.class)
	public void testFactorialNegative()
	{
		LiarsDice.factorial(-1);
	}
	
	@Test
	public void testExactSanity()
	{
		String p1 = String.format("%1.5f", 15.0 / 216);
		String p2 = String.format("%1.5f", LiarsDice.getProbabilityExact(3, 6, 2));
		Assert.assertEquals(p1, p2);
	}
	
	@Test
	public void testExactBruteForce()
	{
		int[] values;
		final int MAX = 8;
		for (int sides = 3; sides <= 6; sides += 3)
		{
			for (int count = 1; count <= MAX; ++count) 
			{
				for (int dice = count; dice <= MAX; ++dice) 
				{
					values = new int[dice];
					// init
					int eCombos = 0;
					int aCombos = 0;
					for (int die = 0; die < dice; ++die) 
					{
						values[die] = 1;
					}
		
					while (values != null) 
					{
						// check
						int ones = 0;
						for (int die = 0; die < dice; ++die) 
						{
							if (values[die] == 1) 
								ones += 1;
						}
						if (ones == count)
							eCombos += 1;
						if (ones >= count)
							aCombos += 1;
						
						// increment
						if (!increment(0, values, sides))
							values = null;
					}
					
					BigInteger tp = BigInteger.valueOf(sides).pow(dice);
					
					// exact
					double p1e = eCombos / tp.doubleValue();
					double p2e = LiarsDice.getProbabilityExact(dice, sides, count);
					String p1es = String.format("%1.5f", p1e);
					String p2es = String.format("%1.5f", p2e);
					Assert.assertEquals(p1es, p2es);
					
					// at least
					double p1a = aCombos / tp.doubleValue();
					double p2a = LiarsDice.getProbabilityAtLeast(dice, sides, count);
					String p1as = String.format("%1.5f", p1a);
					String p2as = String.format("%1.5f", p2a);
					Assert.assertEquals(p1as, p2as);
					
					// predicate opposites
					double a = Predicate.eq.get(dice, sides, count) + Predicate.ne.get(dice, sides, count);
					String as = String.format("%1.5f", a);
					Assert.assertEquals(as, "1.00000");
					double b = Predicate.gt.get(dice, sides, count) + Predicate.le.get(dice, sides, count);
					String bs = String.format("%1.5f", b);
					Assert.assertEquals(bs, "1.00000");
					double c = Predicate.lt.get(dice, sides, count) + Predicate.ge.get(dice, sides, count);
					String cs = String.format("%1.5f", c);
					Assert.assertEquals(cs, "1.00000");
					
					//System.out.format("%dd%d=%d: p1: %1.5f, p2: %1.5f, a%1.5f b%1.5f c%1.5f%n", dice, sides, count, p1e, p2e, a, b, c);
				}
			}
		}
	}
	
	private boolean increment(int position, int[] values, int sides) 
	{
		if (position >= values.length)
			return false;
		
		values[position] += 1;
		if (values[position] > sides) 
		{
			if (!increment(position + 1, values, sides)) 
				return false;
			values[position] = 1;
		}
		return true;
	}

	@Test
	public void testRange()
	{
		final int MAX = 99;
		for (int sides = 3; sides <= 6; sides += 3)
		{
			for (int count = 1; count <= MAX; ++count) 
			{
				for (int dice = count; dice <= MAX; ++dice) 
				{
					LiarsDice.getProbabilityExact(dice, sides, count);
					LiarsDice.getProbabilityAtLeast(dice, sides, count);
				}
			}
		}
	}
	
    @Test
    public void testLowParameters()
    {
        Assert.assertEquals(0.0, LiarsDice.getProbabilityAtLeast(0, 1, 1), 0);
        Assert.assertEquals(1.0, LiarsDice.getProbabilityAtLeast(1, 1, 1), 0);
        Assert.assertEquals(0.5, LiarsDice.getProbabilityAtLeast(1, 2, 1), 0);
        Assert.assertEquals(1.0, LiarsDice.getProbabilityAtLeast(1, 1, 0), 0);
        Assert.assertEquals(0.0, LiarsDice.getProbabilityAtLeast(1, 1, 2), 0);
        
        Assert.assertEquals(0.0, LiarsDice.getProbabilityExact(0, 1, 1), 0);
        Assert.assertEquals(1.0, LiarsDice.getProbabilityExact(1, 1, 1), 0);
        Assert.assertEquals(0.5, LiarsDice.getProbabilityExact(1, 2, 1), 0);
        Assert.assertEquals(1.0, LiarsDice.getProbabilityExact(1, 1, 0), 0);
        Assert.assertEquals(0.0, LiarsDice.getProbabilityExact(1, 1, 2), 0);
    }
    
    @Test
    public void testNonsensicalParameters()
    {
        Assert.assertEquals(0.0, LiarsDice.getProbabilityAtLeast(-1,  1,  1), 0);
        Assert.assertEquals(0.0, LiarsDice.getProbabilityAtLeast( 1, -1,  1), 0);
        Assert.assertEquals(0.0, LiarsDice.getProbabilityAtLeast( 1,  0,  1), 0);
        Assert.assertEquals(1.0, LiarsDice.getProbabilityAtLeast( 1,  1, -1), 0);
        
        Assert.assertEquals(0.0, LiarsDice.getProbabilityExact(-1,  1,  1), 0);
        Assert.assertEquals(0.0, LiarsDice.getProbabilityExact( 1, -1,  1), 0);
        Assert.assertEquals(0.0, LiarsDice.getProbabilityExact( 1,  0,  1), 0);
        Assert.assertEquals(1.0, LiarsDice.getProbabilityExact( 1,  1, -1), 0);
    }
    
    @Test
    public void testExpected()
    {
        Assert.assertEquals(0.0, LiarsDice.expected(-1, -1), 0);
        Assert.assertEquals(0.0, LiarsDice.expected( 0, -1), 0);
        Assert.assertEquals(0.0, LiarsDice.expected(-1,  0), 0);
        Assert.assertEquals(0.0, LiarsDice.expected( 0,  0), 0);
        Assert.assertEquals(0.0, LiarsDice.expected(-1,  1), 0);
        Assert.assertEquals(0.0, LiarsDice.expected( 0,  1), 0);
        Assert.assertEquals(0.0, LiarsDice.expected( 1, -1), 0);
        Assert.assertEquals(0.0, LiarsDice.expected( 1,  0), 0);
        Assert.assertEquals(1.0, LiarsDice.expected( 1,  1), 0);
        Assert.assertEquals(0.5, LiarsDice.expected( 1,  2), 0);
        
    }
}
