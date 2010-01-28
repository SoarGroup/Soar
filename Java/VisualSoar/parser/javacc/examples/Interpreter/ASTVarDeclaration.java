/* JJT: 0.2.2 */




public class ASTVarDeclaration extends SimpleNode
                               implements SPLParserConstants {

  int type;
  String name;

  ASTVarDeclaration(int id) {
    super(id);
  }


  public void interpret()
  {
     if (type == BOOL)
        symtab.put(name, new Boolean(false));
     else
        symtab.put(name, new Integer(0));
  }
}
