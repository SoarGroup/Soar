/* JJT: 0.2.2 */




public class ASTReadStatement extends SimpleNode {
  String name;

  ASTReadStatement(int id) {
    super(id);
  }


  public void interpret()
  {
     Object o;
     byte[] b = new byte[64];
     int i;

     if ((o = symtab.get(name)) == null)
        System.err.println("Undefined variable : " + name);

     if (o instanceof Boolean)
     {
        System.out.print("Enter a value for \'" + name + "\' (boolean) : ");
        System.out.flush();
        try
        {
           i = System.in.read(b);
           symtab.put(name, new Boolean((new String(b, 0, 0, i - 1)).trim()));
        } catch(Exception e) { System.exit(1); }
     }
     else if (o instanceof Integer)
     {
        System.out.print("Enter a value for \'" + name + "\' (int) : ");
        System.out.flush();
        try
        {
           i = System.in.read(b);
           symtab.put(name, new Integer((new String(b, 0, 0, i - 1)).trim()));
        } catch(Exception e) {
           System.out.println("Exceptio : " + e.getClass().getName());
           System.exit(1);
        }
     }
  }
}
