package package1;

public class Main {

  public static void main(String[] args) {
    System.out.println("Calling incr 10 times...");
    for (int i = 0; i < 10; i++) {
      System.out.println(package2.Incr.incr());
    }
  }

}
