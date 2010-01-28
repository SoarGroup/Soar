public class TokenCollector implements TokenManager {

  public Token getNextToken() {
    return ProducerConsumer.pc.getToken();
  }

}
