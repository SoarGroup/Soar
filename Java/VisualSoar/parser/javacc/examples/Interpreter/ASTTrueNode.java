/* JJT: 0.2.2 */




public class ASTTrueNode extends SimpleNode {
  ASTTrueNode(int id) {
    super(id);
  }


  public void interpret()
  {
     stack[++top] = new Boolean(true);
  }

}
