/* JJT: 0.2.2 */




public class ASTBitwiseComplNode extends SimpleNode {
  ASTBitwiseComplNode(int id) {
    super(id);
  }


  public void interpret()
  {
     jjtGetChild(0).interpret();

     stack[top] = new Integer(~((Integer)stack[top]).intValue());
  }
}
