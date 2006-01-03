/* JJT: 0.2.2 */




public class ASTBlock extends SimpleNode {
  ASTBlock(int id) {
    super(id);
  }


  public void interpret()
  {
     int i, k = jjtGetNumChildren();

     for (i = 0; i < k; i++)
        jjtGetChild(i).interpret();
 
  }

}
