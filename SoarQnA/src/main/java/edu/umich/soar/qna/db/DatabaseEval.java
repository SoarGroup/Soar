package edu.umich.soar.qna.db;

import java.io.File;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.CountDownLatch;

import sml.Agent;
import sml.Kernel;
import edu.umich.soar.qna.AsynchronousSMLModule;
import edu.umich.soar.qna.DataSourceManager;
import edu.umich.soar.qna.SMLModule;
import edu.umich.soar.qna.SynchronousSMLModule;

public class DatabaseEval {
	
	private static final String dbFileName = "temp.db";
	private static final String dbDriver = "org.sqlite.JDBC";
	private static final String dbURLPrefix = "jdbc:sqlite:";
	
	private static final String qnaUID = "db";
	private static final String qnaDriver = "edu.umich.soar.qna.db.DatabaseDataSourceDriver";

	private static void usage() {
		System.out.println("Usage: " + DatabaseEval.class.getSimpleName() + " <set size> <trials> [asynchronous: any value]");
	}
	
	public static void main(String[] args) {
		
		try {
			if ((args.length!=2) && (args.length!=3)) {
				usage();
				System.exit(1);
			}
			
			Integer setSize = Integer.parseInt(args[0]);
			Integer trials = Integer.parseInt(args[1]);
			
			boolean synch = (args.length == 2);
			
			// create database
			File r = new File(dbFileName);
			Connection conn = null;
			if (!r.exists()) {
				System.err.println("Creating database:");
				
				Class.forName(dbDriver).newInstance();
				conn = DriverManager.getConnection((dbURLPrefix + dbFileName), "", "");
				
				System.err.println(" create table");
				{
					PreparedStatement statement = conn.prepareStatement("CREATE TABLE data (k INTEGER, v INTEGER)");
					statement.execute();
				}
				
				System.err.println(" create index");
				{
					PreparedStatement statement = conn.prepareStatement("CREATE INDEX data_index ON data (k,v)");
					statement.execute();
				}
				
				System.err.print(" populate table");
				{
					PreparedStatement begin = conn.prepareStatement("BEGIN");
					begin.execute();
					
					PreparedStatement statement = conn.prepareStatement("INSERT INTO data (k,v) VALUES (?,?)");
					
					for (int k=1; k<=10; k++) {
						for (int v=1; v<=setSize; v++) {
							statement.setInt(1, k);
							statement.setInt(2, v);
							statement.executeUpdate();
						}
						
						System.err.print(".");
					}
					
					PreparedStatement commit = conn.prepareStatement("COMMIT");
					commit.execute();
				}
				System.err.println("");
				System.err.println("");
				
				// keep things clean
				conn.close();
				r.deleteOnExit();
			} else {
				System.err.println("Please move existing file: " + r);
				System.exit(1);
			}
			
			System.err.println("Creating Soar Agent:");
			final Kernel kernel;
			final Agent agent;
			{
				kernel = Kernel.CreateKernelInCurrentThread(Kernel.GetDefaultLibraryName(),true,0);
				
				System.err.println(" qna");
				{
					agent = kernel.CreateAgent("qna");
					agent.ExecuteCommandLine("timers -d");
					agent.ExecuteCommandLine("waitsnc -e");
					
					// initialize
					agent.ExecuteCommandLine("sp {propose*initialize-eval (state <s> ^superstate nil -^name) --> (<s> ^operator <o> +) (<o> ^name initialize-eval)}");
					agent.ExecuteCommandLine("sp {apply*initialize-eval (state <s> ^operator <op> ^io <io>) (<io> ^input-link.qna-registry.<src>.query <qry> ^output-link <out>) (<op> ^name initialize-eval) --> (<s> ^name eval ^counter 1) (<out> ^qna-query <q>) (<q> ^source <src> ^query <qry> ^parameters <ps> ^results incremental) (<ps> ^1 1)}");
					
					// next
					agent.ExecuteCommandLine("sp {propose*next (state <s> ^name eval ^counter <c> ^io.output-link.qna-query.result <r>) (<r> ^features.v <c> ^next pending) --> (<s> ^operator <op> + =) (<op> ^name next)}");
					agent.ExecuteCommandLine("sp {apply*next (state <s> ^operator <op> ^counter <c> ^io.output-link <out>) (<op> ^name next) (<out> ^qna-query.id <q-id>) --> (<s> ^counter <c> - (+ <c> 1)) (<out> ^qna-next.query <q-id>)}");
					agent.ExecuteCommandLine("sp {apply*next-clean (state <s> ^operator <op> ^io.output-link <out>) (<op> ^name next) (<out> ^qna-next <q>) (<q> ^status) --> (<out> ^qna-next <q> -)}");
					
					// finish
					agent.ExecuteCommandLine("sp {propose*finish (state <s> ^name eval ^counter <c> ^io.output-link.qna-query.result <r>) (<r> ^features.v <c> ^next nil) --> (<s> ^operator <op>) (<op> ^name finish)}");
					agent.ExecuteCommandLine("sp {apply*finish (state <s> ^operator <op> ^counter <c>) (<op> ^name finish) --> (<s> ^done t ^counter <c> -)}");
					agent.ExecuteCommandLine("sp {apply*finish*clean (state <s> ^operator <op> ^io.output-link <out>) (<out> ^<cmd-name> <cmd>) (<op> ^name finish) --> (<out> ^<cmd-name> <cmd> -)}");
					
					// done
					agent.ExecuteCommandLine("sp {propose*done (state <s> ^name eval ^done t) --> (<s> ^operator <op> + =) (<op> ^name done)}");
					agent.ExecuteCommandLine("sp {apply*done (state <s> ^operator <op>) (<op> ^name done) --> (halt)}");
				}
			}
			System.err.println("");
			
			
			System.err.println("Setting up QnA ("+( (synch)?("synchronous"):("asynchronous") )+"):");
			DataSourceManager man = new DataSourceManager();
			CountDownLatch doneSignal = new CountDownLatch(1);
			SMLModule qna = null;
			{
				System.err.println(" connection parameters");
				{
					Map<String, String> connectionParameters = new HashMap<String, String>();
					connectionParameters.put("driver", dbDriver);
					connectionParameters.put("url", (dbURLPrefix+dbFileName));
					connectionParameters.put("username", "");
					connectionParameters.put("password", "");
					
					man.addDataSource(qnaUID, qnaDriver, connectionParameters);
				}
				
				System.err.println(" query");
				{
					man.registerQuery(qnaUID, "nxt", "SELECT v FROM data WHERE k=? ORDER BY v ASC");
				}
				
				System.err.println(" initialize");
				{
					if (synch) {
						qna = new SynchronousSMLModule(kernel, agent, man, doneSignal);
					} else {
						qna = new AsynchronousSMLModule(kernel, agent, man, doneSignal);
					}
				}
			}
			System.err.println("");
			
			System.err.println("Trials:");
			{
				long timeMsec;
				
				for (int t=1; t<=trials; t++) {
					System.err.print(" "+t);
					
					timeMsec = System.currentTimeMillis();
					agent.ExecuteCommandLine("run");
					timeMsec = System.currentTimeMillis() - timeMsec;
					
					// data collection
					System.out.print("size="+setSize);
					System.out.print(" trial="+t);
					System.out.print(" dcs="+agent.GetDecisionCycleCounter());
					System.out.print(" msec="+timeMsec);
					System.out.println();
					
					agent.ExecuteCommandLine("init");
					
					System.err.println("");
				}
			}
			System.err.println("");
			
			System.err.println("Shutting down:");
			{
				System.err.println(" qna");
				{
					qna.close();
					man.close();
				}
				
				System.err.println(" soar");
				{
					kernel.Shutdown();
				}
			}
			
		} catch (Exception e) {
			System.out.println(e);
			e.printStackTrace();
		}
	}
}
