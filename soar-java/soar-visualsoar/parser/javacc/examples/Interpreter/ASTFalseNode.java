/* JJT: 0.2.2 */




public class ASTFalseNode extends SimpleNode {
  ASTFalseNode(int id) {
    super(id);
  }


  public void interpret()
  {
     stack[++top] = new Boolean(false);
  }

}
