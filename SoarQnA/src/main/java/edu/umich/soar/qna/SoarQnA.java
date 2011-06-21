package edu.umich.soar.qna;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Map.Entry;
import java.util.concurrent.CountDownLatch;

import org.ini4j.Wini;
import org.ini4j.spi.EscapeTool;

import sml.Agent;
import sml.Kernel;

public class SoarQnA {

	private static void addSources(String fileName, DataSourceManager man) {
		try {
			Wini ini = new Wini(new File(fileName));
			EscapeTool esc = EscapeTool.getInstance();

			if (ini.containsKey("sources")) {
				for (Entry<String, String> s : ini.get("sources").entrySet()) {
					System.out
							.println("Source '"
									+ s.getKey()
									+ "': "
									+ ((man.addDataSource(s.getKey(), esc
											.unquote(s.getValue()))) ? ("success")
											: ("failure")));
				}
			}

		} catch (Exception e) {
			e.printStackTrace();
			System.exit(0);
		}
	}

	private static void usage() {
		System.out.println("java " + SoarQnA.class.getSimpleName()
				+ " <host> <port> <agent name> <configuration file> [asynchronous: any value]");
	}

	public static void main(String[] args) {
		boolean installed = false;
		for (String s : resources) {
			install(s);
			installed = true;
		}
		if (installed) {
			System.out
					.println("Installed resources from jar to working directory "
							+ System.getProperty("user.dir"));
		}

		if ((args.length != 4) && (args.length != 5)) {
			usage();
			System.exit(0);
		}

		DataSourceManager man = new DataSourceManager();
		addSources(args[3], man);
		
		final Kernel kernel = Kernel.CreateRemoteConnection(true, args[0], Integer.parseInt(args[1]));
		if (kernel.HadError()) {
			throw new IllegalStateException(kernel.GetLastErrorDescription());
		}
		
		final Agent agent = kernel.GetAgent(args[2]);
		if (agent == null) {
			throw new IllegalStateException(kernel.GetLastErrorDescription());
		}
		
		final boolean synch = (args.length == 4);

		CountDownLatch doneSignal = new CountDownLatch(1);
		
		SMLModule SoarQnA = null;
		System.out.print("Operating mode: ");
		if (synch) {
			SoarQnA = new SynchronousSMLModule(kernel, agent, man, doneSignal);
			System.out.println("synchronous");
		} else {
			SoarQnA = new AsynchronousSMLModule(kernel, agent, man, doneSignal);
			System.out.println("asynchronous");
		}

		try {
			doneSignal.await();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}

		System.out.println("Shutting down");

		SoarQnA.close();
		man.close();
	}

	private static void install(String file) {
		try {
			File r = new File(file);
			if (r.exists())
				return;

			String jarpath = "/" + file;
			InputStream is = SoarQnA.class.getResourceAsStream(jarpath);
			if (is == null) {
				System.err.println("Resource not found: " + jarpath);
				return;
			}

			// Create the new file on disk
			File parent = r.getParentFile();
			if (parent != null) {
				if (!parent.exists())
					parent.mkdirs();
			}
			FileOutputStream os = new FileOutputStream(r);

			// Copy the file onto disk
			byte bytes[] = new byte[2048];
			int read;
			while (true) {
				read = is.read(bytes);

				// EOF
				if (read == -1)
					break;

				os.write(bytes, 0, read);
			}

			is.close();
			os.close();
		} catch (IOException e) {
			System.err.println(file + ": " + e.getMessage());
		}
	}

	// in SoarSuite/SoarQnA/src/main/resources
	// find soar-qna -type f | grep -v ".svn" | sed 's/.*/"&",/'
	private static final String[] resources = { "soar-qna/dice.ini",
			"soar-qna/math.ini", "soar-qna/qna-test/_firstload.soar",
			"soar-qna/qna-test/abs/abs-check.soar",
			"soar-qna/qna-test/abs/abs-init.soar",
			"soar-qna/qna-test/abs/abs-query.soar",
			"soar-qna/qna-test/abs/abs_source.soar",
			"soar-qna/qna-test/abs/elaborations.soar",
			"soar-qna/qna-test/abs.soar",
			"soar-qna/qna-test/addition/addition-check.soar",
			"soar-qna/qna-test/addition/addition-init.soar",
			"soar-qna/qna-test/addition/addition-query.soar",
			"soar-qna/qna-test/addition/addition_source.soar",
			"soar-qna/qna-test/addition/elaborations.soar",
			"soar-qna/qna-test/addition.soar",
			"soar-qna/qna-test/all/all_source.soar",
			"soar-qna/qna-test/all_vars/all_vars-check.soar",
			"soar-qna/qna-test/all_vars/all_vars-init.soar",
			"soar-qna/qna-test/all_vars/all_vars-query.soar",
			"soar-qna/qna-test/all_vars/all_vars_source.soar",
			"soar-qna/qna-test/all_vars/elaborations.soar",
			"soar-qna/qna-test/all_vars.soar", "soar-qna/qna-test/clean.soar",
			"soar-qna/qna-test/comment.dm",
			"soar-qna/qna-test/div/div-check.soar",
			"soar-qna/qna-test/div/div-init.soar",
			"soar-qna/qna-test/div/div-query.soar",
			"soar-qna/qna-test/div/div_source.soar",
			"soar-qna/qna-test/div/elaborations.soar",
			"soar-qna/qna-test/div.soar",
			"soar-qna/qna-test/division/division-check.soar",
			"soar-qna/qna-test/division/division-init.soar",
			"soar-qna/qna-test/division/division-query.soar",
			"soar-qna/qna-test/division/division_source.soar",
			"soar-qna/qna-test/division/elaborations.soar",
			"soar-qna/qna-test/division.soar", "soar-qna/qna-test/done.soar",
			"soar-qna/qna-test/elaborations/_all.soar",
			"soar-qna/qna-test/elaborations/elaborations_source.soar",
			"soar-qna/qna-test/elaborations/top-state.soar",
			"soar-qna/qna-test/float/elaborations.soar",
			"soar-qna/qna-test/float/float-check.soar",
			"soar-qna/qna-test/float/float-init.soar",
			"soar-qna/qna-test/float/float-query.soar",
			"soar-qna/qna-test/float/float_source.soar",
			"soar-qna/qna-test/float.soar",
			"soar-qna/qna-test/inc_vars/elaborations.soar",
			"soar-qna/qna-test/inc_vars/inc_vars-check.soar",
			"soar-qna/qna-test/inc_vars/inc_vars-query.soar",
			"soar-qna/qna-test/inc_vars/inc_vars_source.soar",
			"soar-qna/qna-test/inc_vars.soar",
			"soar-qna/qna-test/initialize-qna-test.soar",
			"soar-qna/qna-test/int/elaborations.soar",
			"soar-qna/qna-test/int/int-check.soar",
			"soar-qna/qna-test/int/int-init.soar",
			"soar-qna/qna-test/int/int-query.soar",
			"soar-qna/qna-test/int/int_source.soar",
			"soar-qna/qna-test/int.soar",
			"soar-qna/qna-test/mod/elaborations.soar",
			"soar-qna/qna-test/mod/mod-check.soar",
			"soar-qna/qna-test/mod/mod-init.soar",
			"soar-qna/qna-test/mod/mod-query.soar",
			"soar-qna/qna-test/mod/mod_source.soar",
			"soar-qna/qna-test/mod.soar",
			"soar-qna/qna-test/multiplication/elaborations.soar",
			"soar-qna/qna-test/multiplication/multiplication-check.soar",
			"soar-qna/qna-test/multiplication/multiplication-init.soar",
			"soar-qna/qna-test/multiplication/multiplication-query.soar",
			"soar-qna/qna-test/multiplication/multiplication_source.soar",
			"soar-qna/qna-test/multiplication.soar",
			"soar-qna/qna-test/negation/elaborations.soar",
			"soar-qna/qna-test/negation/negation-check.soar",
			"soar-qna/qna-test/negation/negation-init.soar",
			"soar-qna/qna-test/negation/negation-query.soar",
			"soar-qna/qna-test/negation/negation_source.soar",
			"soar-qna/qna-test/negation.soar",
			"soar-qna/qna-test/one_var/elaborations.soar",
			"soar-qna/qna-test/one_var/one_var-check.soar",
			"soar-qna/qna-test/one_var/one_var-init.soar",
			"soar-qna/qna-test/one_var/one_var-query.soar",
			"soar-qna/qna-test/one_var/one_var_source.soar",
			"soar-qna/qna-test/one_var.soar", "soar-qna/qna-test/qna-test.dm",
			"soar-qna/qna-test/qna-test_source.soar",
			"soar-qna/qna-test/reciprocation/elaborations.soar",
			"soar-qna/qna-test/reciprocation/reciprocation-check.soar",
			"soar-qna/qna-test/reciprocation/reciprocation-init.soar",
			"soar-qna/qna-test/reciprocation/reciprocation-query.soar",
			"soar-qna/qna-test/reciprocation/reciprocation_source.soar",
			"soar-qna/qna-test/reciprocation.soar",
			"soar-qna/qna-test/sleep/sleep-check.soar",
			"soar-qna/qna-test/sleep/sleep-init.soar",
			"soar-qna/qna-test/sleep/sleep-query.soar",
			"soar-qna/qna-test/sleep/sleep_source.soar",
			"soar-qna/qna-test/sleep/elaborations.soar",
			"soar-qna/qna-test/sleep.soar",
			"soar-qna/qna-test/sqrt/elaborations.soar",
			"soar-qna/qna-test/sqrt/sqrt-check.soar",
			"soar-qna/qna-test/sqrt/sqrt-init.soar",
			"soar-qna/qna-test/sqrt/sqrt-query.soar",
			"soar-qna/qna-test/sqrt/sqrt_source.soar",
			"soar-qna/qna-test/sqrt.soar",
			"soar-qna/qna-test/subtraction/elaborations.soar",
			"soar-qna/qna-test/subtraction/subtraction-check.soar",
			"soar-qna/qna-test/subtraction/subtraction-init.soar",
			"soar-qna/qna-test/subtraction/subtraction-query.soar",
			"soar-qna/qna-test/subtraction/subtraction_source.soar",
			"soar-qna/qna-test/subtraction.soar", "soar-qna/qna-test.soar",
			"soar-qna/qna-test.vsa", "soar-qna/qna.ini",
			"soar-qna/qna_proposal.pdf", "soar-qna/readme.txt",
			"soar-qna/test.db", "soar-qna/test_db.ini",
			"soar-qna/qna-writer.ini", "soar-qna/db-writer.ini",
			"soar-qna/writer.vsa","soar-qna/writer.soar",
			"soar-qna/writer/_firstload.soar","soar-qna/writer/add-done.soar",
			"soar-qna/writer/add.soar","soar-qna/writer/all/all_source.soar",
			"soar-qna/writer/begin-done.soar","soar-qna/writer/comment.dm",
			"soar-qna/writer/commit-done.soar","soar-qna/writer/delete-done.soar",
			"soar-qna/writer/done.soar","soar-qna/writer/elaborations/_all.soar",
			"soar-qna/writer/elaborations/elaborations_source.soar",
			"soar-qna/writer/elaborations/top-state.soar",
			"soar-qna/writer/finish.soar","soar-qna/writer/index-done.soar",
			"soar-qna/writer/initialize-writer.soar","soar-qna/writer/rollback-done.soar",
			"soar-qna/writer/table-done.soar","soar-qna/writer/writer.dm",
			"soar-qna/writer/writer_source.soar",
			};
}
