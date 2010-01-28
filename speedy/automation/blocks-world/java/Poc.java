
import java.io.*;
import sml.*;

public class Poc {
	
	public static void main( String[] args ) throws Exception {
		
		if ( args.length < 1 )
		{
			System.out.println( "java Poc " + "<soar dir> [key1=val1 key2=val2 ...]" + "\n" );
			return;
		}
		
		// get pid
		int pid;
		{
			Process p = Runtime.getRuntime().exec( "./pid.sh" );			
			p.waitFor();
			pid = Integer.parseInt( new BufferedReader( new InputStreamReader( p.getInputStream() ) ).readLine() );
		}
		
		// prep
		Kernel pKernel;
		Agent pAgent;
		{
			pKernel = Kernel.CreateKernelInNewThread();
			pAgent = pKernel.CreateAgent( "headless" );
			
			pAgent.LoadProductions( args[0] + "/SoarLibrary/Demos/blocks-world/blocks-world.soar" );
			
			// no monitors, success
			pAgent.ExecuteCommandLine( "excise blocks-world*elaborate*state*success" );
			pAgent.ExecuteCommandLine( "excise blocks-world*monitor*world-state" );
			pAgent.ExecuteCommandLine( "excise blocks-world*monitor*operator-application*move-block" );
			
			// watch 0, seed, timers
			pAgent.ExecuteCommandLine( "watch 0" );
			pAgent.ExecuteCommandLine( "srand 55512" );
			pAgent.ExecuteCommandLine( "timers --off" );
		}
		
		for ( int i=0; i<10; i++ )
		{
			// perform the task
			{
				pAgent.ExecuteCommandLine( "d " + 10000 );
			}
			
			// report
			String result;
			{
				Process p = Runtime.getRuntime().exec( "php report.php" + " " + pid );
				p.waitFor();
				
				System.out.print( new BufferedReader( new InputStreamReader( p.getInputStream() ) ).readLine() );
			}
			
			// soar stats
			{
				ClientAnalyzedXML response = new ClientAnalyzedXML();
				pAgent.ExecuteCommandLineXML( "stats", response );
				
				System.out.print( " decisions=" + response.GetArgInt( sml_Names.getKParamStatsCycleCountDecision(), 0 ) );
			}
			
			// extra params
			{
				for ( int j=1; j<args.length; j++ )
				{
					System.out.print( " " + args[ j ] );
				}
			}
			
			System.out.println();
		}
		
		// clean
		{
			pKernel.Shutdown();
		}
	}
	
}