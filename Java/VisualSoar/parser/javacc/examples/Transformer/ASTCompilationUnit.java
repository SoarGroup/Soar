/* JJT: 0.2.2 */

import java.io.*;

public class ASTCompilationUnit extends SimpleNode {
  ASTCompilationUnit(int id) {
    super(id);
  }


// Manually inserted code begins here

  public void process (PrintWriter ostr) {
    Token t = begin;
    ASTSpecialBlock bnode;
    for (int i = 0; i < jjtGetNumChildren(); i++) {
      bnode = (ASTSpecialBlock)jjtGetChild(i);
      do {
        print(t, ostr);
        t = t.next;
      } while (t != bnode.begin);
      bnode.process(ostr);
      t = bnode.end.next;
    }
    while (t != null) {
      print(t, ostr);
      t = t.next;
    }
  }

}
