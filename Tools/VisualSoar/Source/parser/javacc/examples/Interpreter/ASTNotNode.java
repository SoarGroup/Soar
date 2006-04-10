/* JJT: 0.2.2 */




public class ASTNotNode extends SimpleNode {
  ASTNotNode(int id) {
    super(id);
  }


  public void interpret()
  {
     jjtGetChild(0).interpret();
     stack[top] = new Boolean(!((Boolean)stack[top]).booleanValue());
  }
}
