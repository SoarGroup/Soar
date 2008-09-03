/* JJT: 0.2.2 */




public class ASTIfStatement extends SimpleNode {

  ASTIfStatement(int id) {
    super(id);
  }


  public void interpret()
  {
     jjtGetChild(0).interpret();

     if (((Boolean)stack[top--]).booleanValue())
        jjtGetChild(1).interpret();
     else if (jjtGetNumChildren() == 3)
        jjtGetChild(2).interpret();
  }

}
