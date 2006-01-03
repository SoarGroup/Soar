package package2;

public class Incr {

  static private int count = 0;

  /**
   * Returns the next integer value each time it is called.  The
   * state information is stored in the variable count.
   */
  static public int incr() {
    return count++;
  }

}
