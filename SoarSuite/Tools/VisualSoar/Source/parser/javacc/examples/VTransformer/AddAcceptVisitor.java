package VTransformer;

import java.io.PrintWriter;

public class AddAcceptVisitor extends UnparseVisitor
{

  public AddAcceptVisitor(PrintWriter o)
  {
    super(o);
  }


  public Object visit(ASTClassBodyDeclaration node, Object data)
  {
    /* Are we the first child of our parent? */
    if (node == node.jjtGetParent().jjtGetChild(0)) {

      /** Attempt to make the new code match the indentation of the
          node. */
      StringBuffer pre = new StringBuffer("");
      for (int i = 1; i < node.getFirstToken().beginColumn; ++i) {
	pre.append(" ");
      }

      out.println(pre + "");
      out.println(pre + "/** Accept the visitor. **/");
      out.println(pre + "public Object jjtAccept(JavaParserVisitor visitor, Object data) {");
      out.println(pre + "  return visitor.visit(this, data);");
      out.println(pre + "}");
    }
    return super.visit(node, data);
  }

}

/*end*/
