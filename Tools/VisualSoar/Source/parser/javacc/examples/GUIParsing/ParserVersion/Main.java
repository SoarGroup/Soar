public class Main {

  public static void main(String[] args) {
    new CalcGUI();
    TokenManager tm = new TokenCollector();
    CalcInputParser cp = new CalcInputParser(tm);
    while (true) {
      try {
        cp.Input();
      } catch (ParseException e) {
        CalcGUI.print("ERROR (click 0)");
      }
    }
  }

}
