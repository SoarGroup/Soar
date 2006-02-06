/* JJT: 0.2.2 */




public class ASTAssignment extends SimpleNode {
  ASTAssignment(int id) {
    super(id);
  }


  public void interpret()
  {
     String name;

     jjtGetChild(1).interpret();
     symtab.put(name = ((ASTId)jjtGetChild(0)).name, stack[top]);
  }
}
