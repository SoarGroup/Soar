public class ProducerConsumer {

  /**
   * A single producer-consumer instance that is used by others.
   */
  public static ProducerConsumer pc = new ProducerConsumer();

  /**
   * The data structure where the tokens are stored.
   */
  private java.util.Vector queue = new java.util.Vector();

  /**
   * The producer calls this method to add a new token
   * whenever it is available.
   */
  synchronized public void addToken(Token token) {
    queue.addElement(token);
    notify();
  }

  /**
   * The consumer calls this method to get the next token
   * in the queue.  If the queue is empty, this method
   * blocks until a token becomes available.
   */
  synchronized public Token getToken() {
    if (queue.size() == 0) {
      try {
        wait();
      } catch (InterruptedException willNotHappen) {
        throw new Error();
      }
    }
    Token retval = (Token)(queue.elementAt(0));
    queue.removeElementAt(0);
    return retval;
  }

}
