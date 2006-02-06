/* JJT: 0.2.2 */




public class ASTId extends SimpleNode {

  String name;

  ASTId(int id) {
    super(id);
  }


  public void interpret()
  {
     stack[++top] = symtab.get(name);
  }

}
