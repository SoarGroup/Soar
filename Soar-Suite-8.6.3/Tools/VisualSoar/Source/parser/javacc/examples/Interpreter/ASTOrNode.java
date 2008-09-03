/* JJT: 0.2.2 */




public class ASTOrNode extends SimpleNode {
  ASTOrNode(int id) {
    super(id);
  }


  public void interpret()
  {
     jjtGetChild(0).interpret();

     if (((Boolean)stack[top]).booleanValue())
     {
        stack[top] = new Boolean(true);
        return;
     }

     jjtGetChild(1).interpret();
     stack[--top] = new Boolean(((Boolean)stack[top]).booleanValue() ||
                                ((Boolean)stack[top + 1]).booleanValue());
  }

}
