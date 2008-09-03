/* JJT: 0.2.2 */




public class ASTWriteStatement extends SimpleNode {
  String name;

  ASTWriteStatement(int id) {
    super(id);
  }


  public void interpret()
  {
     Object o;
     byte[] b = new byte[64];

     if ((o = symtab.get(name)) == null)
        System.err.println("Undefined variable : " + name);

     System.out.println("Value of " + name + " : " + symtab.get(name));
  }

}
