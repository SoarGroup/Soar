/* JJT: 0.2.2 */

import java.io.*;

public class ASTSpecialBlock extends SimpleNode {
  ASTSpecialBlock(int id) {
    super(id);
  }


// Manually inserted code begins here

  public void process (PrintWriter ostr) {
    Token t = begin; // t corresponds to the "{" of the special block.
    t.image = "{ try {";
    while (t != end) {
      print(t, ostr);
      t = t.next;
    }
    // t now corresponds to the last "}" of the special block.
    t.image = "} }";
    print(t, ostr);
  }

}
