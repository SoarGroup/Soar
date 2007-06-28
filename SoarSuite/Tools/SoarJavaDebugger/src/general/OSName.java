package general;

public class OSName {
	// 
	// os.name values
	//Digital Unix
	//FreeBSD
	//HP UX
	//Irix
	//Linux
	//Mac OS
	//MPE/iX
	//Netware 4.11
	//OS/2
	//Solaris
	//Windows 2000
	//Windows 95
	//Windows 98
	//Windows NT
	//Windows XP
	public static boolean isWindows()
	{
		String osName = System.getProperty("os.name");
		return osName.toLowerCase().startsWith("windows") ;
	}
	
	public static boolean isMacOS()
	{
		String osName = System.getProperty("os.name");
		return osName.toLowerCase().startsWith("mac") ;
	}

	public static boolean isLinuxAndNoOtherUnix()
	{
		String osName = System.getProperty("os.name");
		return osName.toLowerCase().startsWith("linux") ;
	}

	// i.e. is Linux or a Unix flavor, but not Mac OS X flavor of Unix
	public static boolean isNotWindowsAndNotMac()
	{
		return !isWindows() && !isMacOS() ;
	}
	
	public static String kSystemLineSeparator = System.getProperty("line.separator") ;
}
