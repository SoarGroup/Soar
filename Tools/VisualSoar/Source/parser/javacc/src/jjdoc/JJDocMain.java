/**
 *
 * Copyright (c) 1996-1997 Sun Microsystems, Inc.
 *
 * Use of this file and the system it is part of is constrained by the
 * file COPYRIGHT in the root directory of this system.
 *
 */

package COM.sun.labs.jjdoc;

import COM.sun.labs.javacc.*;

public class JJDocMain {

  static void help_message() {
    java.io.PrintWriter ostr = new java.io.PrintWriter(new java.io.OutputStreamWriter(System.out));

    ostr.println("    jjdoc option-settings - (to read from standard input)");
    ostr.println("OR");
    ostr.println("    jjdoc option-settings inputfile (to read from a file)");
    ostr.println("");
    ostr.println("WHERE");
    ostr.println("    \"option-settings\" is a sequence of settings separated by spaces.");
    ostr.println("");

    ostr.println("Each option setting must be of one of the following forms:");
    ostr.println("");
    ostr.println("    -optionname=value (e.g., -TEXT=false)");
    ostr.println("    -optionname:value (e.g., -TEXT:false)");
    ostr.println("    -optionname       (equivalent to -optionname=true.  e.g., -TEXT)");
    ostr.println("    -NOoptionname     (equivalent to -optionname=false. e.g., -NOTEXT)");
    ostr.println("");
    ostr.println("Option settings are not case-sensitive, so one can say \"-nOtExT\" instead");
    ostr.println("of \"-NOTEXT\".  Option values must be appropriate for the corresponding");
    ostr.println("option, and must be either an integer, boolean or string value.");
    ostr.println("");
    ostr.println("The string valued options are:");
    ostr.println("");
    ostr.println("    OUTPUT_FILE");
    ostr.println("");
    ostr.println("The boolean valued options are:");
    ostr.println("");
    ostr.println("    ONE_TABLE              (default true)");
    ostr.println("    TEXT                   (default false)");
    ostr.println("");

    ostr.println("");
    ostr.println("EXAMPLES:");
    ostr.println("    jjdoc -ONE_TABLE=false mygrammar.jj");
    ostr.println("    jjdoc - < mygrammar.jj");
    ostr.println("");
    ostr.println("ABOUT JJDOC:");
    ostr.println("    jjdoc is written by Rob Duncan (rob@metamata.com), based on");
    ostr.println("    earlier versions by Sriram Sankar (sriram@metamata.com) and");
    ostr.println("    Roger Hayes (roger.hayes@eng.sun.com).");
    ostr.println("    To learn more about JavaCC, please visit the JavaCC web site at:");
    ostr.println("                http://www.suntest.com/JavaCC/");
    ostr.println("    If you wish to contact us, please send email to:");
    ostr.println("                javacc-help@metamata.com");
    ostr.println("    If you wish to be added to the mailing list, please send email to:");
    ostr.println("                javacc-request@metamata.com");
    ostr.println("    To post to the mailing list, please send email to:");
    ostr.println("                javacc-interest@metamata.com");
  }

  static boolean isOption(String opt) {
    return opt.length() > 1 && opt.charAt(0) == '-';
  }

  /**
   * A main program that exercises the parser.
   */
  public static void main(String args[]) throws Exception {

    JavaCCGlobals.bannerLine("Documentation Generator", "0.1.4");

    JavaCCParser parser = null;
    if (args.length == 0) {
      System.out.println("");
      help_message();
      System.exit(1);
    } else {
      System.out.println("(type \"jjdoc\" with no arguments for help)");
    }

    JJDocInit();

    if (isOption(args[args.length-1])) {
      System.out.println("Last argument \"" + args[args.length-1] + "\" is not a filename or \"-\".  ");
      System.exit(1);
    }
    for (int arg = 0; arg < args.length-1; arg++) {
      if (!isOption(args[arg])) {
        System.out.println("Argument \"" + args[arg] + "\" must be an option setting.  ");
        System.exit(1);
      }
      Options.setCmdLineOption(args[arg]);
    }

    if (args[args.length-1].equals("-")) {
      System.out.println("Reading from standard input . . .");
      parser = new JavaCCParser(new java.io.DataInputStream(System.in));
      JJDocGlobals.input_file = "standard input";
      JJDocGlobals.output_file = "standard output";
    } else {
      System.out.println("Reading from file " + args[args.length-1] + " . . .");
      try {
        java.io.File fp = new java.io.File(args[args.length-1]);
        if (!fp.exists()) {
           System.out.println("File " + args[args.length-1] + " not found.");
           System.exit(1);
        }
        if (fp.isDirectory()) {
           System.out.println(args[args.length-1] + " is a directory. Please use a valid file name.");
           System.exit(1);
        }
	JJDocGlobals.input_file = fp.getName();
        parser = new JavaCCParser(new java.io.FileReader(args[args.length-1]));
      } catch (NullPointerException ne) { // Should never happen
      } catch (SecurityException se) {
        System.out.println("Security voilation while trying to open " + args[args.length-1]);
        System.exit(1);
      } catch (java.io.FileNotFoundException e) {
        System.out.println("File " + args[args.length-1] + " not found.");
        System.exit(1);
      }
    }
    try {

      parser.javacc_input();
      JJDoc.start();

      if (JavaCCErrors.get_error_count() == 0) {
        if (JavaCCErrors.get_warning_count() == 0) {
          System.out.println("Grammar documentation generated successfully in " + JJDocGlobals.output_file);
        } else {
          System.out.println("Grammar documentation generated with 0 errors and "
                             + JavaCCErrors.get_warning_count() + " warnings.");
        }
        System.exit(0);
      } else {
        System.out.println("Detected " + JavaCCErrors.get_error_count() + " errors and "
                           + JavaCCErrors.get_warning_count() + " warnings.");
        System.exit((JavaCCErrors.get_error_count()==0)?0:1);
      }
    } catch (COM.sun.labs.javacc.MetaParseException e) {
      System.out.println(e.toString());
      System.out.println("Detected " + JavaCCErrors.get_error_count() + " errors and "
                         + JavaCCErrors.get_warning_count() + " warnings.");
      System.exit(1);
    } catch (COM.sun.labs.javacc.ParseException e) {
      System.out.println(e.toString());
      System.out.println("Detected " + (JavaCCErrors.get_error_count()+1) + " errors and "
                         + JavaCCErrors.get_warning_count() + " warnings.");
      System.exit(1);
    }
  }

  private static void JJDocInit() {
    Options.JavaCCInit();
    Options.optionValues.put("OUTPUT_FILE", "");
    Options.optionValues.put("ONE_TABLE", Boolean.TRUE);
    Options.optionValues.put("TEXT", Boolean.FALSE);
  }

}
