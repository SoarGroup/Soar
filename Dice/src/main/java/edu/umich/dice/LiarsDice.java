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
    private static final String DICE_NEGATIVE = "Dice count is negative";
    private static final String SIDES_NONPOSITIVE = "Dice need more than one side";
    private static final String COUNT_NEGATIVE = "Count is negative";

    /**
     * <p>
     * Given a number of dice with a number of sides, returns the expected
     * number of same faces. If either argument is negative, returns 0.
     * 
     * @param dice
     *            Total number of dice
     * @param sides
     *            Sides per die
     * @return Expected number having the same face
     */
    public static double expected(int dice, int sides)
    {
        if (dice < 0)
        {
            warn(DICE_NEGATIVE, dice, sides);
            return 0;
        } 
        else if (dice < 1)
            return 0;
        
        if (sides < 1)
        {
            warn(SIDES_NONPOSITIVE, dice, sides);
            return 0;
        }

        return (double) dice / sides;
    }

    /**
     * <p>
     * Given some dice, returns the probability that the exact amount of the
     * same face will show.
     * 
     * http://en.wikipedia.org/wiki/Binomial_probability
     * 
     * @param dice
     *            Total number of dice, must be positive
     * @param sides
     *            Sides per die, must be positive
     * @param count
     *            Exact number with same face
     * @return 0..1
     */
    public static double getProbabilityExact(int dice, int sides, int count)
    {
        double result = sanity(dice, sides, count);
        if (result >= 0)
            return result;

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
        result = c.doubleValue();
        result *= 1 / p1kd.doubleValue();
        result *= p2nkn.doubleValue() / p2nkd.doubleValue();

        return result;
    }

    /**
     * <p>
     * Given some dice, returns the probability that at least the given amount
     * of the same face will show.
     * 
     * <p>
     * Note: linear performance penalty for large (dice - count).
     * 
     * @param dice
     *            Total number of dice, must be positive
     * @param sides
     *            Sides per die, must be positive
     * @param count
     *            At least this many with the same face
     * @return 0..1
     */
    public static double getProbabilityAtLeast(int dice, int sides, int count)
    {
        double result = sanity(dice, sides, count);
        if (result >= 0)
            return result;

        result = 0;
        for (int i = 0; count + i <= dice; ++i)
            result += getProbabilityExact(dice, sides, count + i);
        return result;
    }

    /**
     * <p>
     * Deal with insane parameters, warn to stderr if invalid parameters and
     * always return valid probability.
     * 
     * @param dice
     *            greater than zero
     * @param sides
     *            greater than zero
     * @param count
     *            between 1 and dice
     * @return Probability (zero or one) if parameters are nonsensical, negative
     *         if the parameters are valid.
     */
    private static double sanity(int dice, int sides, int count)
    {
        if (dice < 0)
        {
            warn(DICE_NEGATIVE, dice, sides, count);
            return 0;
        }
        else if (dice < 1)
            return 0;

        if (sides < 1)
        {
            warn(SIDES_NONPOSITIVE, dice, sides, count);
            return 0;
        }

        if (count < 0)
        {
            warn(COUNT_NEGATIVE, dice, sides, count);
            return 1.0;
        }
        else if (count < 1)
            return 1.0;

        if (count > dice)
            return 0;

        return -1.0;
    }

    private static void warn(String prefix, int dice, int sides)
    {
        System.err.format(
                "%s: dice: %d, sides: %d%n", prefix, dice,
                sides);
    }

    private static void warn(String prefix, int dice, int sides, int count)
    {
        System.err.format(
                "%s: dice: %d, sides: %d, count: %d%n", prefix, dice,
                sides, count);
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
                return getProbabilityExact(dice, sides, count);
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
                return 1 - getProbabilityExact(dice, sides, count);
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
                return 1 - getProbabilityAtLeast(dice, sides, count);
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
                return getProbabilityAtLeast(dice, sides, count)
                        - getProbabilityExact(dice, sides, count);
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
                return (1 - getProbabilityAtLeast(dice, sides, count))
                        + getProbabilityExact(dice, sides, count);
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
                return getProbabilityAtLeast(dice, sides, count);
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
