import java.io.*;
import java.util.*;

public class Globals {

  // The mappings from old id's to new id's.
  static Hashtable mappings = new Hashtable();

  // A table of targets of all known mappings.
  static Hashtable mapTargets = new Hashtable();

  // These id's may not be changed.
  static Hashtable noChangeIds = new Hashtable();

  // These id's should be used for mappings.
  static Hashtable useIds = new Hashtable();

  // The location of the input and output directories.
  static File inpDir, outDir;

  // Set to true by Java parser if class has a main program.
  static boolean mainExists;

  // Returns the map of old to obfuscated id.  If map does not
  // exist, it is created.
  static String map(String str) {
    Object obj = mappings.get(str);
    if (obj != null) {
      return (String)obj;
    }
    if (useIds.isEmpty()) {
      String newId = "O0" + counter++;
      mappings.put(str, newId);
      return newId;
    } else {
      obj = useIds.keys().nextElement();
      useIds.remove(obj);
      String newId = (String)obj;
      mappings.put(str, newId);
      return newId;
    }
  }

  // A counter used to generate new identifiers
  static int counter = 0;

}
