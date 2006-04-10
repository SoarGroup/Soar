public class Main {

  public static void main(String[] args) {
    CalcGUI gui = new CalcGUI();
    MyLexer lexer = new MyLexer(gui);
    while (true) {
      try {
        lexer.getNextToken();
      } catch (TokenMgrError e) {
      }
    }
  }

}
