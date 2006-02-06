/**
 *
 * Copyright (c) 1996-1997 Sun Microsystems, Inc.
 *
 * Use of this file and the system it is part of is constrained by the
 * file COPYRIGHT in the root directory of this system.
 *
 */


public class ASTMyOtherID extends SimpleNode {
  private String name;

  ASTMyOtherID(int id) {
    super(id);
  }

  /** Accept the visitor. **/
  public Object jjtAccept(eg4Visitor visitor, Object data) {
    return visitor.visit(this, data);
  }

  public void setName(String n) {
    name = n;
  }

  public String toString() {
    return "Identifier: " + name;
  }

}
