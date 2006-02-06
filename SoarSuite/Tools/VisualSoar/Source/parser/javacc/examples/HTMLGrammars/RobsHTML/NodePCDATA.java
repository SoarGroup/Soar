public class NodePCDATA extends SimpleNode {
  private String pcdata;

  /* This value should be unique across all nodes. */
  public static final int NODEPCDATA = -1;

  public NodePCDATA(String data) {
    super(NODEPCDATA);
    pcdata = data;
  }

  public String toString() {
    return "PCDATA: " + pcdata;
  }
}
