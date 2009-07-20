/* JJT: 0.2.2 */




public class ASTBitwiseOrNode extends SimpleNode {
  ASTBitwiseOrNode(int id) {
    super(id);
  }


  public void interpret()
  {
     jjtGetChild(0).interpret();
     jjtGetChild(1).interpret();

     if (stack[top] instanceof Boolean)
        stack[--top] = new Boolean(((Boolean)stack[top]).booleanValue() |
                                ((Boolean)stack[ + 1]).booleanValue());
     else if (stack[top] instanceof Integer)
        stack[--top] = new Integer(((Integer)stack[top]).intValue() |
                                ((Integer)stack[ + 1]).intValue());
  }
}
