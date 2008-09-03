package VTransformer;

public class Main
{
  public static void main(String args[]) {
    System.err.println("Reading from standard input...");
    JavaParser p = new JavaParser(System.in);
    try {
      ASTCompilationUnit cu = p.CompilationUnit();
      JavaParserVisitor visitor = new AddAcceptVisitor(System.out);
      cu.jjtAccept(visitor, null);
      System.err.println("Thank you.");
    } catch (Exception e) {
      System.err.println("Oops.");
      System.err.println(e.getMessage());
      e.printStackTrace();
    }
  }
}

/*end*/
