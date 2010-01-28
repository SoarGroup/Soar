/**
 *
 * Copyright (c) 1996-1997 Sun Microsystems, Inc.
 *
 * Use of this file and the system it is part of is constrained by the
 * file COPYRIGHT in the root directory of this system.
 *
 */

public class ASTMyID extends SimpleNode {
  private String name;

  ASTMyID(int id){
    super(id);
  }

  public void setName(String n) {
    name = n;
  }

  public String toString() {
    return "Identifier: " + name;
  }

}
