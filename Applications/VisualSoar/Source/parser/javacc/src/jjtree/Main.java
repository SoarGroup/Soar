/**
 *
 * Copyright (c) 1997 Sun Microsystems, Inc.
 *
 * Use of this file and the system it is part of is constrained by the
 * file COPYRIGHT in the root directory of this system.  This file is
 * part of the internal development code for JavaCC and may not be
 * distributed.
 */

package COM.sun.labs.jjtree;

public class Main
{
  public static void main(String args[])
  {
    JJTree jjtree = new JJTree();
    int result = jjtree.main(args);
    System.exit(result);
  }
}

/*end*/
