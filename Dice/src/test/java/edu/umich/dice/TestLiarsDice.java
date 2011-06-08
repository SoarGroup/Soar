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
					double eq = Predicate.eq.get(dice, sides, count);
					double ne = Predicate.ne.get(dice, sides, count);
					double gt = Predicate.gt.get(dice, sides, count);
					double le = Predicate.le.get(dice, sides, count);
					double lt = Predicate.lt.get(dice, sides, count);
					double ge = Predicate.ge.get(dice, sides, count);
				
					double a = eq + ne;
					String as = String.format("%1.5f", a);
					double b = gt + le;
					String bs = String.format("%1.5f", b);
					double c = lt + ge;
					String cs = String.format("%1.5f", c);
					
//					System.out.format("%dd%d=%d: p1:%1.5f, p2:%1.5f, " +
//							"eq%1.5f+ne%1.5f=%1.5f, " +
//							"gt%1.5f+le%1.5f=%1.5f, " +
//							"lt%1.5f+ge%1.5f=%1.5f%n", dice, sides, count, p1e, p2e, 
//							eq, ne, a, 
//							gt, le, b, 
//							lt, ge, c);

					Assert.assertEquals(as, "1.00000");
                    Assert.assertEquals(bs, "1.00000");
                    Assert.assertEquals(cs, "1.00000");
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
        Assert.assertEquals(1.0, LiarsDice.getProbabilityExact(0, 1, 0), 0);
        Assert.assertEquals(0.0, LiarsDice.getProbabilityExact(1, 1, 0), 0);
        Assert.assertEquals(0.0, LiarsDice.getProbabilityExact(0, 1, 1), 0);
        Assert.assertEquals(1.0, LiarsDice.getProbabilityExact(1, 1, 1), 0);
        Assert.assertEquals(0.0, LiarsDice.getProbabilityExact(1, 1, 2), 0);
        Assert.assertEquals(1.0, LiarsDice.getProbabilityExact(0, 2, 0), 0);
        Assert.assertEquals(0.5, LiarsDice.getProbabilityExact(1, 2, 0), 0);
        Assert.assertEquals(0.0, LiarsDice.getProbabilityExact(0, 2, 1), 0);
        Assert.assertEquals(0.5, LiarsDice.getProbabilityExact(1, 2, 1), 0);
        Assert.assertEquals(0.0, LiarsDice.getProbabilityExact(1, 2, 2), 0);

        Assert.assertEquals(1.0, LiarsDice.getProbabilityAtLeast(0, 1, 0), 0);
        Assert.assertEquals(1.0, LiarsDice.getProbabilityAtLeast(1, 1, 0), 0);
        Assert.assertEquals(0.0, LiarsDice.getProbabilityAtLeast(0, 1, 1), 0);
        Assert.assertEquals(1.0, LiarsDice.getProbabilityAtLeast(1, 1, 1), 0);
        Assert.assertEquals(0.0, LiarsDice.getProbabilityAtLeast(1, 1, 2), 0);
        Assert.assertEquals(1.0, LiarsDice.getProbabilityAtLeast(0, 2, 0), 0);
        Assert.assertEquals(1.0, LiarsDice.getProbabilityAtLeast(1, 2, 0), 0);
        Assert.assertEquals(0.0, LiarsDice.getProbabilityAtLeast(0, 2, 1), 0);
        Assert.assertEquals(0.5, LiarsDice.getProbabilityAtLeast(1, 2, 1), 0);
        Assert.assertEquals(0.0, LiarsDice.getProbabilityAtLeast(1, 2, 2), 0);

    }
    
    @Test
    public void testNonsensicalParameters()
    {
        Assert.assertEquals(0.0, LiarsDice.getProbabilityAtLeast(-1,  1,  0), 0);
        Assert.assertEquals(0.0, LiarsDice.getProbabilityAtLeast( 0, -1,  0), 0);
        Assert.assertEquals(0.0, LiarsDice.getProbabilityAtLeast( 0,  0,  0), 0);
        Assert.assertEquals(0.0, LiarsDice.getProbabilityAtLeast( 0,  1, -1), 0);
        
        Assert.assertEquals(0.0, LiarsDice.getProbabilityExact(-1,  1,  0), 0);
        Assert.assertEquals(0.0, LiarsDice.getProbabilityExact( 0, -1,  0), 0);
        Assert.assertEquals(0.0, LiarsDice.getProbabilityExact( 0,  0,  0), 0);
        Assert.assertEquals(0.0, LiarsDice.getProbabilityExact( 0,  1, -1), 0);
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
    
    @Test
    public void testZeroTolerance()
    {
        Assert.assertEquals(0.0, LiarsDice.zeroTolerance(1 - LiarsDice
                .getProbabilityAtLeast(5, 6, 0)), 0);
        Assert.assertEquals(0.0, Predicate.lt.get(5, 6, 0), 0);
    }
    
    @Test
    public void testEdgeCases()
    {
        Assert.assertEquals(0.0, Predicate.eq.get(3, 3, -1), 0);
        Assert.assertEquals(0.0, Predicate.eq.get(3, 3,  4), 0);
        
        Assert.assertEquals(1.0, Predicate.ne.get(3, 3, -1), 0);
        Assert.assertEquals(1.0, Predicate.ne.get(3, 3,  4), 0);
        
        Assert.assertEquals(1.0, Predicate.ge.get(3, 3, -1), 0);
        Assert.assertEquals(1.0, Predicate.ge.get(3, 3,  0), 0);
        Assert.assertEquals(0.0, Predicate.ge.get(3, 3,  4), 0);
        
        Assert.assertEquals(1.0, Predicate.gt.get(3, 3, -1), 0);
        Assert.assertEquals(0.0, Predicate.gt.get(3, 3,  3), 0);
        Assert.assertEquals(0.0, Predicate.gt.get(3, 3,  4), 0);
        
        Assert.assertEquals(0.0, Predicate.le.get(3, 3, -1), 0);
        Assert.assertEquals(1.0, Predicate.le.get(3, 3,  3), 0);
        Assert.assertEquals(1.0, Predicate.le.get(3, 3,  4), 0);
        
        Assert.assertEquals(0.0, Predicate.lt.get(3, 3, -1), 0);
        Assert.assertEquals(0.0, Predicate.lt.get(3, 3,  0), 0);
        Assert.assertEquals(1.0, Predicate.lt.get(3, 3,  4), 0);
    }
}
