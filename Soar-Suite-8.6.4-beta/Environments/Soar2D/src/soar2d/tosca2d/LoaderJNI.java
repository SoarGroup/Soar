
// NOTE: If you move this loaderJNI class to a new package
// you'll also need to change the name of the JNI method in the C++ code
// to include the new package name.
package soar2d.tosca2d;

public class LoaderJNI {
	public LoaderJNI(String libraryName) {
	    try {
	        System.loadLibrary(libraryName);
	    } catch (UnsatisfiedLinkError e) {
	      System.err.println("Native code library failed to load. \n" + e);
	      throw e ;
	    }
	    System.out.println("Loaded \n" + libraryName);		
	}
	public final native long Librarian_GetExistingLibrarian();

}
