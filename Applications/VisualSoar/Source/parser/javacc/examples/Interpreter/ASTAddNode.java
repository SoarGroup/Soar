/* JJT: 0.2.2 */




public class ASTAddNode extends SimpleNode {
  ASTAddNode(int id) {
    super(id);
  }


  public void interpret()
  {
     jjtGetChild(0).interpret();
     jjtGetChild(1).interpret();

     stack[--top] = new Integer(((Integer)stack[top]).intValue() +
                                ((Integer)stack[top + 1]).intValue());
  }
}
