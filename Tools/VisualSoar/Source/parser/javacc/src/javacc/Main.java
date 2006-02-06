/**
 *
 * Copyright (c) 1996 Sun Microsystems, Inc.
 *
 * Use of this file and the system it is part of is constrained by the
 * file COPYRIGHT in the root directory of this system.
 *
 */

package COM.sun.labs.javacc;

public class Main {

  static void help_message() {
    System.out.println("Usage:");
    System.out.println("    javacc option-settings inputfile");
    System.out.println("");
    System.out.println("\"option-settings\" is a sequence of settings separated by spaces.");
    System.out.println("Each option setting must be of one of the following forms:");
    System.out.println("");
    System.out.println("    -optionname=value (e.g., -STATIC=false)");
    System.out.println("    -optionname:value (e.g., -STATIC:false)");
    System.out.println("    -optionname       (equivalent to -optionname=true.  e.g., -STATIC)");
    System.out.println("    -NOoptionname     (equivalent to -optionname=false. e.g., -NOSTATIC)");
    System.out.println("");
    System.out.println("Option settings are not case-sensitive, so one can say \"-nOsTaTiC\" instead");
    System.out.println("of \"-NOSTATIC\".  Option values must be appropriate for the corresponding");
    System.out.println("option, and must be either an integer, a boolean, or a string value.");
    System.out.println("");
    System.out.println("The integer valued options are:");
    System.out.println("");
    System.out.println("    LOOKAHEAD              (default 1)");
    System.out.println("    CHOICE_AMBIGUITY_CHECK (default 2)");
    System.out.println("    OTHER_AMBIGUITY_CHECK  (default 1)");
    System.out.println("");
    System.out.println("The boolean valued options are:");
    System.out.println("");
    System.out.println("    STATIC                 (default true)");
    System.out.println("    DEBUG_PARSER           (default false)");
    System.out.println("    DEBUG_LOOKAHEAD        (default false)");
    System.out.println("    DEBUG_TOKEN_MANAGER    (default false)");
    System.out.println("    OPTIMIZE_TOKEN_MANAGER (default true)");
    System.out.println("    ERROR_REPORTING        (default true)");
    System.out.println("    JAVA_UNICODE_ESCAPE    (default false)");
    System.out.println("    UNICODE_INPUT          (default false)");
    System.out.println("    IGNORE_CASE            (default false)");
    System.out.println("    COMMON_TOKEN_ACTION    (default false)");
    System.out.println("    USER_TOKEN_MANAGER     (default false)");
    System.out.println("    USER_CHAR_STREAM       (default false)");
    System.out.println("    BUILD_PARSER           (default true)");
    System.out.println("    BUILD_TOKEN_MANAGER    (default true)");
    System.out.println("    SANITY_CHECK           (default true)");
    System.out.println("    FORCE_LA_CHECK         (default false)");
    System.out.println("    CACHE_TOKENS           (default false)");
    System.out.println("");
    System.out.println("The string valued options are:");
    System.out.println("");
    System.out.println("    OUTPUT_DIRECTORY       (default Current Directory)");
    System.out.println("");
    System.out.println("EXAMPLE:");
    System.out.println("    javacc -STATIC=false -LOOKAHEAD:2 -debug_parser mygrammar.jj");
    System.out.println("");
    System.out.println("ABOUT JavaCC:");
    System.out.println("    JavaCC is a parser generator for Java built by Sriram Sankar");
    System.out.println("    (sriram@metamata.com), Sreenivasa Viswanadha (sreeni@metamata.com),");
    System.out.println("    and Rob Duncan (rob@metamata.com).");
    System.out.println("");
    System.out.println("    To learn more about JavaCC, please visit the JavaCC web site at:");
    System.out.println("                http://www.sun.com/suntest/JavaCC/");
    System.out.println("    If you wish to contact us, please send email to:");
    System.out.println("                javacc-support@metamata.com");
    System.out.println("    If you wish to be added to the mailing list, please send email to:");
    System.out.println("                javacc-interest-request@metamata.com");
    System.out.println("    To post to the mailing list, please send email to:");
    System.out.println("                javacc-interest@metamata.com");
    System.out.println("");
  }

  static boolean isOption(String opt) {
    return opt.length() > 1 && opt.charAt(0) == '-';
  }

  /**
   * A main program that exercises the parser.
   */
  public static void main(String args[]) throws Exception {
    int errorcode = mainProgram(args);
    System.exit(errorcode);
  }

  /**
   * The method to call to exercise the parser from other Java programs.
   * It returns an error code.  See how the main program above uses
   * this method.
   */
  public static int mainProgram(String args[]) throws Exception {

    // Initialize all static state
    reInitAll();

    JavaCCGlobals.bannerLine("Parser Generator", "");

    Options.JavaCCInit();
    JavaCCParser parser = null;
    if (args.length == 0) {
      System.out.println("");
      help_message();
      return 1;
    } else {
      System.out.println("(type \"javacc\" with no arguments for help)");
    }

    if (isOption(args[args.length-1])) {
      System.out.println("Last argument \"" + args[args.length-1] + "\" is not a filename.");
      return 1;
    }
    for (int arg = 0; arg < args.length-1; arg++) {
      if (!isOption(args[arg])) {
        System.out.println("Argument \"" + args[arg] + "\" must be an option setting.");
        return 1;
      }
      Options.setCmdLineOption(args[arg]);
    }

    try {
      java.io.File fp = new java.io.File(args[args.length-1]);
      if (!fp.exists()) {
         System.out.println("File " + args[args.length-1] + " not found.");
         return 1;
      }
      if (fp.isDirectory()) {
         System.out.println(args[args.length-1] + " is a directory. Please use a valid file name.");
         return 1;
      }
      parser = new JavaCCParser(new java.io.FileReader(args[args.length-1]));
    } catch (NullPointerException ne) { // Should never happen
    } catch (SecurityException se) {
      System.out.println("Security voilation while trying to open " + args[args.length-1]);
      return 1;
    } catch (java.io.FileNotFoundException e) {
      System.out.println("File " + args[args.length-1] + " not found.");
      return 1;
    }

    try {
      System.out.println("Reading from file " + args[args.length-1] + " . . .");
      JavaCCGlobals.fileName = JavaCCGlobals.origFileName = args[args.length-1];
      JavaCCGlobals.jjtreeGenerated = JavaCCGlobals.isGeneratedBy("JJTree", args[args.length-1]);
      JavaCCGlobals.jjcovGenerated = JavaCCGlobals.isGeneratedBy("JJCov", args[args.length-1]);
      JavaCCGlobals.toolNames = JavaCCGlobals.getToolNames(args[args.length-1]);
      parser.javacc_input();
      JavaCCGlobals.setOutputDir(); // Set the output directory
      Semanticize.start();
      ParseGen.start();
      LexGen.start();
      OtherFilesGen.start();

      if ((JavaCCErrors.get_error_count() == 0) && (Options.B("BUILD_PARSER") || Options.B("BUILD_TOKEN_MANAGER"))) {
        if (JavaCCErrors.get_warning_count() == 0) {
          System.out.println("Parser generated successfully.");
        } else {
          System.out.println("Parser generated with 0 errors and "
                             + JavaCCErrors.get_warning_count() + " warnings.");
        }
        return 0;
      } else {
        System.out.println("Detected " + JavaCCErrors.get_error_count() + " errors and "
                           + JavaCCErrors.get_warning_count() + " warnings.");
        return (JavaCCErrors.get_error_count()==0)?0:1;
      }
    } catch (MetaParseException e) {
      System.out.println("Detected " + JavaCCErrors.get_error_count() + " errors and "
                         + JavaCCErrors.get_warning_count() + " warnings.");
      return 1;
    } catch (ParseException e) {
      System.out.println(e.toString());
      System.out.println("Detected " + (JavaCCErrors.get_error_count()+1) + " errors and "
                         + JavaCCErrors.get_warning_count() + " warnings.");
      return 1;
    }
  }

   private static void reInitAll()
   {
      COM.sun.labs.javacc.Expansion.reInit();
      COM.sun.labs.javacc.JavaCCErrors.reInit();
      COM.sun.labs.javacc.JavaCCGlobals.reInit();
      COM.sun.labs.javacc.Options.reInit();
      COM.sun.labs.javacc.JavaCCParserInternals.reInit();
      COM.sun.labs.javacc.RStringLiteral.reInit();
      COM.sun.labs.javacc.JavaFiles.reInit();
      COM.sun.labs.javacc.LexGen.reInit();
      COM.sun.labs.javacc.NfaState.reInit();
      COM.sun.labs.javacc.MatchInfo.reInit();
      COM.sun.labs.javacc.LookaheadWalk.reInit();
      COM.sun.labs.javacc.Semanticize.reInit();
      COM.sun.labs.javacc.ParseGen.reInit();
      COM.sun.labs.javacc.OtherFilesGen.reInit();
      COM.sun.labs.javacc.ParseEngine.reInit();
   }

}
