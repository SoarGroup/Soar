package edu.umich.soar.qna;

import java.io.File;
import java.util.Map.Entry;
import java.util.concurrent.CountDownLatch;

import org.ini4j.Wini;
import org.ini4j.spi.EscapeTool;

public class SoarQnA {
	
	private static void addSources(String fileName, DataSourceManager man) {
		try {
			Wini ini = new Wini(new File(fileName));
			EscapeTool esc = EscapeTool.getInstance();
			
			if (ini.containsKey("sources")) {
				for (Entry<String, String> s : ini.get("sources").entrySet()) {
					System.out.println("Source '" + s.getKey() + "': " + ((man.addDataSource(s.getKey(), esc.unquote(s.getValue())))?("success"):("failure")));
				}
			}
			
		} catch (Exception e) {
			e.printStackTrace();
			System.exit(0);
		}
	}
	
	private static void usage() {
		System.out.println( "java " + SoarQnA.class.getSimpleName() + " <host> <port> <agent name> <configuration file>" );
	}
	
	public static void main(String[] args) {
		
		if (args.length != 4) {
			usage();
			System.exit(0);
		}
		
		DataSourceManager man = new DataSourceManager();
		addSources(args[3], man);
		
		CountDownLatch doneSignal = new CountDownLatch(1);
		QnASMLModule SoarQnA = new QnASMLModule(args[0], Integer.parseInt(args[1]), args[2], man, doneSignal);
		
		try {
			doneSignal.await();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		
		System.out.println("Shutting down");

		SoarQnA.close();
		man.close();
	}
}
