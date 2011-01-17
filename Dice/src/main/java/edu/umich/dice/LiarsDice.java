package edu.umich.dice;

import java.math.BigInteger;

/**
 * <p>
 * Collection of utility functions to aid players of the liar's dice game.
 * 
 * @author Jonathan Voigt <voigtjr@gmail.com>
 */
public class LiarsDice
{
    /**
     * <p>
     * Zero is a very important number.
     */
    private static final double ZERO_TOLERANCE = 0.00001; 

    /**
     * <p>
     * Zero is a very important number, if abs(in) is less/equal than
     * ZERO_TOLERANCE, make it zero.
     */
    public static double zeroTolerance(double in)
    {
        return Math.abs(in) <= ZERO_TOLERANCE ? 0 : in;
    }
    
    /**
     * <p>
     * Given a number of dice with a number of sides, returns the expected
     * number of same faces.
     * 
     * @param dice
     *            Total number of dice
     * @param sides
     *            Sides per die
     * @return Expected number having the same face
     */
    public static double expected(int dice, int sides)
    {
        if (dice < 1)
            return 0;
        if (sides < 1)
            return 0;
        return (double) dice / sides;
    }

    /**
     * <p>
     * Given some dice, returns the probability that the exact amount of the one
     * same face will show.
     * 
     * For example, invoked with (dice=3, sides=6, count=2), it returns the
     * probability that exactly two of three six-sided dice are one of those six
     * faces (e.g. fours only), or (15 / 216).
     * 
     * http://en.wikipedia.org/wiki/Binomial_probability
     * 
     * @param dice
     *            Total number of dice
     * @param sides
     *            Sides per die
     * @param count
     *            Exact number with same face
     * @return 0..1
     */
    public static double getProbabilityExact(int dice, int sides, int count)
    {
        // these make no sense
        if (dice < 0)
            return 0;
        if (count < 0)
            return 0;
        if (sides < 1)
            return 0;

        // if there are no dice, probability is zero unless count is also zero
        if (dice == 0)
        {
            if (count == 0)
                return 1;
            return 0;
        }

        // if there is only one side to the dice, probability is zero unless
        // count == dice
        if (sides == 1)
        {
            if (dice == count)
                return 1;
            return 0;
        }

        if (count > dice)
            return 0;

        // dice > 0, sides > 2, count <= dice

        // n = dice, k = exact
        // c = n! / k!(n-k)!
        BigInteger x = factorial(dice - count);
        BigInteger y = factorial(count).multiply(x);
        BigInteger c = factorial(dice).divide(y);

        // p1^k = 1/(sides^k)
        BigInteger p1kd = BigInteger.valueOf(sides).pow(count);

        // (1-p1)^(n-k) = ((sides - 1)^(n-k)) / (sides^(n-k))
        BigInteger p2nkn = BigInteger.valueOf(sides - 1).pow(dice - count);
        BigInteger p2nkd = BigInteger.valueOf(sides).pow(dice - count);

        // put it together: c * (P1)^k * (1-P1)^(n-k)
        double result = c.doubleValue();
        result *= 1 / p1kd.doubleValue();
        result *= p2nkn.doubleValue() / p2nkd.doubleValue();

        return result;
    }

    /**
     * <p>
     * Given some dice, returns the probability that at least the given amount
     * of the one same face will show.
     * 
     * For example, invoked with (dice=3, sides=6, count=2), it returns the
     * probability that at least two of three six-sided dice are one of those
     * six faces (e.g. fours only), or (16 / 216). Note that this example has
     * only one more instance than getProbabilityExact because it includes the
     * case where all three dice are fours.
     * 
     * <p>
     * Note: linear performance penalty for large (dice - count).
     * 
     * @param dice
     *            Total number of dice
     * @param sides
     *            Sides per die
     * @param count
     *            At least this many with the same face
     * @return 0..1
     */
    public static double getProbabilityAtLeast(int dice, int sides, int count)
    {
        // these make no sense
        if (dice < 0)
            return 0;
        if (count < 0)
            return 0;
        if (sides < 1)
            return 0;

        double result = 0;
        for (int i = 0; count + i <= dice; ++i)
            result += getProbabilityExact(dice, sides, count + i);
        return result;
    }

    /**
     * <p>
     * Non-recursive implementation of factorial using BigInteger.
     * 
     * @param n
     *            Cannot be negative
     * @return n factorial
     */
    public static BigInteger factorial(long n)
    {
        if (n < 0)
            throw new IllegalArgumentException(
                    "n! is a sequence with integer values for nonnegative n");

        BigInteger result = BigInteger.ONE;
        for (long i = 2; i <= n; ++i)
            result = result.multiply(BigInteger.valueOf(i));
        return result;
    }

    /**
     * <p>
     * Predicates for various dice probability options, optimized for the
     * popular use cases (exact, at least, less than)
     */
    public enum Predicate
    {
        /**
         * <p>
         * equal (count is exact)
         */
        eq
        {
            public double get(int dice, int sides, int count)
            {
                return zeroTolerance(getProbabilityExact(dice, sides, count));
            }
        },
        /**
         * <p>
         * not equal (count is not exact)
         */
        ne
        {
            public double get(int dice, int sides, int count)
            {
                return zeroTolerance(1 - getProbabilityExact(dice, sides, count));
            }
        },
        /**
         * <p>
         * less than (actual faces are less than count)
         */
        lt
        {
            public double get(int dice, int sides, int count)
            {
                // Special edge case, temporary
                if (count < 1)
                    return 0;
                return zeroTolerance(1 - getProbabilityAtLeast(dice, sides, count));
            }
        },
        /**
         * <p>
         * greater than (actual faces are greater than count)
         */
        gt
        {
            public double get(int dice, int sides, int count)
            {
                return zeroTolerance(getProbabilityAtLeast(dice, sides, count)
                        - getProbabilityExact(dice, sides, count));
            }
        },
        /**
         * <p>
         * less than or equals (at most count actual faces)
         */
        le
        {
            public double get(int dice, int sides, int count)
            {
                return zeroTolerance((1 - getProbabilityAtLeast(dice, sides, count))
                        + getProbabilityExact(dice, sides, count));
            }
        },
        /**
         * <p>
         * greater than or equals (at least count actual faces)
         */
        ge
        {
            public double get(int dice, int sides, int count)
            {
                return zeroTolerance(getProbabilityAtLeast(dice, sides, count));
            }
        },
        ;

        /**
         * <p>
         * Perform the predicate, return probability of count related to
         * predicate.
         * 
         * @param dice
         *            Total number of dice
         * @param sides
         *            Sides per die
         * @param count
         *            Target face count
         * @return 0..1
         */
        abstract public double get(int dice, int sides, int count);
    }

    /**
     * <p>
     * Simple command line interface, see usage().
     * 
     * @param args
     *            see usage().
     */
    public static void main(String[] args)
    {
        try
        {
            int dice = Integer.valueOf(args[0]);
            int sides = Integer.valueOf(args[1]);
            Predicate pred = Predicate.valueOf(args[2]);
            int count = Integer.valueOf(args[3]);

            System.out.format("%dd%d%s%d: ", dice, sides, pred, count);
            System.out.println(pred.get(dice, sides, count));
            return;
        }
        catch (ArrayIndexOutOfBoundsException e)
        {
        }
        catch (NumberFormatException e)
        {
        }
        catch (IllegalArgumentException e)
        {
        }
        usage();
    }

    /**
     * <p>
     * Help for command line interface.
     */
    private static void usage()
    {
        System.out.println("Usage: DICE SIDES PREDICATE COUNT");
        System.out.println("\tDICE: Total dice, positive");
        System.out.println("\tSIDES: Sides per dice, positive");
        System.out.print("\tPREDICATE: one of ");
        for (Predicate pred : Predicate.values())
            System.out.format("%s ", pred);
        System.out.println();
        System.out.println("\tCOUNT: Target same face count 1..DICE");
        System.out.println();
    }
}
