
/**
 * A subclass of CalcInputTokenManager so that we can do better error reporting
 * via the GUI object.
 */
class MyLexer extends CalcInputParserTokenManager {

   /**
    * We redefined the lexical error reporting function so that it beeps
    * and displays a messgae thru the GUI.
    */
   protected void LexicalError() {
     CalcGUI.Error("ERROR (click 0)");
     ReInit(gui.getCollector(), OPERAND);
     result = 0.0;
   }

   public MyLexer(CalcGUI guiObj)
   {
      super(guiObj.getCollector(), OPERAND);
      gui = guiObj;
   }
}
