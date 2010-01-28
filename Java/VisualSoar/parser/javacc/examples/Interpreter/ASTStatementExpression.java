/* JJT: 0.2.2 */




public class ASTStatementExpression extends SimpleNode {
  ASTStatementExpression(int id) {
    super(id);
  }


  public void interpret()
  {
     jjtGetChild(0).interpret();
     top--; //just throw away the value.
  }

}
