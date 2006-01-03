/**
 *
 * Copyright (c) 1996-1997 Sun Microsystems, Inc.
 *
 * Use of this file and the system it is part of is constrained by the
 * file COPYRIGHT in the root directory of this system.
 *
 */

/* This is an example of how the Visitor pattern might be used to
   implement the dumping code that comes with SimpleNode.  It's a bit
   long-winded, but it does illustrate a couple of the main points.

   1) the visitor can maintain state between the nodes that it visits
   (for example the current indentation level).

   2) if you don't implement a jjtAccept() method for a subclass of
   SimpleNode, then SimpleNode's acceptor will get called.

   3) the utility method childrenAccept() can be useful when
   implementing preorder or postorder tree walks.

   Err, that's it. */

public class eg4DumpVisitor implements eg4Visitor
{
  private int indent = 0;

  private String indentString() {
    StringBuffer sb = new StringBuffer();
    for (int i = 0; i < indent; ++i) {
      sb.append(" ");
    }
    return sb.toString();
  }

  public Object visit(SimpleNode node, Object data) {
    System.out.println(indentString() + node +
		       ": acceptor not unimplemented in subclass?");
    ++indent;
    data = node.childrenAccept(this, data);
    --indent;
    return data;
  }

  public Object visit(ASTStart node, Object data) {
    System.out.println(indentString() + node);
    ++indent;
    data = node.childrenAccept(this, data);
    --indent;
    return data;
  }

  public Object visit(ASTAdd node, Object data) {
    System.out.println(indentString() + node);
    ++indent;
    data = node.childrenAccept(this, data);
    --indent;
    return data;
  }

  public Object visit(ASTMult node, Object data) {
    System.out.println(indentString() + node);
    ++indent;
    data = node.childrenAccept(this, data);
    --indent;
    return data;
  }

  public Object visit(ASTMyOtherID node, Object data) {
    System.out.println(indentString() + node);
    ++indent;
    data = node.childrenAccept(this, data);
    --indent;
    return data;
  }

  public Object visit(ASTInteger node, Object data) {
    System.out.println(indentString() + node);
    ++indent;
    data = node.childrenAccept(this, data);
    --indent;
    return data;
  }
}

/*end*/
