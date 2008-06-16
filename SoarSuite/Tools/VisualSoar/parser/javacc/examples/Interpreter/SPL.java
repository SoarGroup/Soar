
class SPL {

  public static void main(String args[]) {
    SPLParser parser;
    if (args.length == 1) {
      System.out.println("Stupid Programming Language Interpreter Version 0.1:  Reading from file " + args[0] + " . . .");
      try {
        parser = new SPLParser(new java.io.FileInputStream(args[0]));
      } catch (java.io.FileNotFoundException e) {
        System.out.println("Stupid Programming Language Interpreter Version 0.1:  File " + args[0] + " not found.");
        return;
      }
    } else {
      System.out.println("Stupid Programming Language Interpreter Version 0.1:  Usage :");
      System.out.println("         java SPL inputfile");
      return;
    }
    try {
      parser.CompilationUnit();
      parser.jjtree.rootNode().interpret();
    } catch (ParseException e) {
      System.out.println("Stupid Programming Language Interpreter Version 0.1:  Encountered errors during parse.");
      e.printStackTrace();
    } catch (Exception e1) {
      System.out.println("Stupid Programming Language Interpreter Version 0.1:  Encountered errors during interpretation/tree building.");
      e1.printStackTrace();
    }
  }
}
