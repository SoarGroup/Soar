public class NodeAttribute extends SimpleNode {
  private String name, value;

  /* This value should be unique across all nodes. */
  public static final int NODEATTRIBUTE = -2;

  public NodeAttribute(String n, String v) {
    super(NODEATTRIBUTE);
    name = n;
    if (v == null) {
      value = "#IMPLIED";
    } else {
      value = v;
    }
  }

  public String toString() {
    return "attribute: " + name + "=" + value;
  }
}
