package edu.umich.soar.examples;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.atomic.AtomicBoolean;

import sml.Agent;
import sml.Identifier;
import sml.Kernel;
import sml.smlUpdateEventId;
import sml.Kernel.UpdateEventInterface;
import edu.umich.soar.ProductionUtils;

/**
 * A simple, asynchronous, interactive environment example. A loop is started
 * that accepts input from standard in. The input is simply parsed and, unless
 * it is a special command, it is asynchronously passed on to the Soar input
 * link. If it is a special command, then the environment acts on the command.
 * 
 * <p>
 * The special commands include start (or run) to start running Soar, stop to
 * stop running Soar, and quit to exit the program.
 * 
 * <p>
 * The Soar interface reports any non-special commands received on the command
 * line to a messages WME on the input-link. Each message has a unique id. The
 * agent is expected to issue commands on the output-link.
 * 
 * <p>
 * One of these Soar commands is print, and it simply prints the contents of the
 * "content" parameter. The other command is clear which tells the Soar
 * interface to remove any existing messages from the input-link.
 * 
 * <p>
 * The agent included with this registers a few canned responses with itself
 * (responding to "hello" and "jump") and then uses these canned responses if
 * their triggers come in. If it doesn't have a canned response then it simply
 * echos the message as a command on the output link.
 * 
 * <p>
 * Since the agent doesn't have a response for clear, issuing this means it gets
 * passed as a command on the output link which then, in turn, causes the
 * environment to clear the input link.
 * 
 * <p>
 * Illustrative run:
 * 
 * <pre>
 * > start 
 * > Soar interface: Starting Soar. 
 * > hello 
 * > Soar interface: Agent said: Greetings Professor Falken. 
 * > jump 
 * > Soar interface: Agent said: How high? 
 * > xyzzy 
 * > Soar interface: Unknown command received: xyzzy 
 * > clear 
 * > Soar interface: Messages cleared. 
 * > stop 
 * > Soar interface: Stopping Soar. 
 * > quit
 * </pre>
 * 
 * <p>
 * Note that the Soar Java Debugger can connect to this environment as soon as
 * the environment is started. Issue the -remote switch when launching the
 * debugger. Avoid starting and stopping Soar from within the debugger when
 * doing this, however, since this environment is not written to handle that
 * correctly in all cases.
 * 
 * <p>
 * This example would normally be implemented using multiple files rather than
 * static inner classes but it is done like this in order to contain it in one
 * file. These classes could be moved out and things should work the same.
 * 
 * @author voigtjr
 */
public class SimpleAsyncEnv
{

    /*
     * When Soar issues a print, it makes a call to this interface which must be
     * registered beforehand.
     */
    public interface PrintListener
    {
        public void printEvent(String message);
    }

    /*
     * Cleans up the related code.
     */
    public static final PrintListener nullListener = new PrintListener()
    {
        public void printEvent(String message)
        {
        }
    };

    public static class Soar implements Runnable
    {
        /*
         * These are two valid commands that can be expected from the agent.
         */
        private final String PRINT = "print";

        private final String CLEAR = "clear";

        private final Kernel kernel;

        private final Agent agent;

        /*
         * We cache the root of the messages WME on the input-link so that we
         * can quickly add additional messages as they come in.
         */
        private final Identifier messagesRoot;

        /*
         * We hold on to each message's WME as we add it so that we can easily
         * remove them when a clear command is issued, without having to remove
         * and re-add the root messages WME.
         */
        private final List<Identifier> messages = new ArrayList<Identifier>();

        /*
         * Here is the shared state that we must synchronize between the
         * environment and the Soar interface. These are messages that have been
         * entered in to standard in but not yet added to the input-link.
         */
        private final BlockingQueue<String> lines = new LinkedBlockingQueue<String>();

        /*
         * This variable is used to gracefully ask Soar to stop executing.
         */
        private final AtomicBoolean stopSoar = new AtomicBoolean(true);

        /*
         * Output from the Soar interface gets sent to this print listener so
         * that it may be clearly distinguished from other print calls.
         */
        private PrintListener pl = nullListener;

        public Soar()
        {
            kernel = Kernel.CreateKernelInNewThread();
            /*
             * As long as the execution environment is set up correctly and Java
             * can find the Java_smlClientInterface library this will return a
             * kernel object even if there is an error. In the event of an
             * error, the kernel object will have diagnostic information in it
             * for retrieval. If the execution environment isn't set up
             * correctly, however, a runtime exception will be thrown here when
             * it cannot load the shared library.
             */
            if (kernel.HadError())
            {
                System.err.println("Error creating kernel: "
                        + kernel.GetLastErrorDescription());
                System.exit(1);
            }

            agent = kernel.CreateAgent("soar");
            /*
             * The agent, however, does return null on error. Use kernel to get
             * diagnostic information.
             */
            if (agent == null)
            {
                System.err.println("Error creating agent: "
                        + kernel.GetLastErrorDescription());
                System.exit(1);
            }

            /*
             * Load the productions from the jar file. Most of the time
             * agent.LoadProductions() is used instead of this to load
             * productions.
             */
            try
            {
                String result = ProductionUtils.sourceAgentFromJar(agent,
                        "/simpleasyncenv.soar");
                if (agent.GetLastCommandLineResult() == false)
                {
                    System.err.println(result);
                    System.exit(1);
                }
            }
            catch (IOException ex)
            {
                ex.printStackTrace();
                System.exit(1);
            }

            /*
             * Create and cache the root messages WME on the input-link so that
             * we can quickly add messages later.
             */
            messagesRoot = agent.GetInputLink().CreateIdWME("messages");

            /*
             * Register for update event that fires after our agent passes its
             * output phase. Our update handler will post new messages on the
             * input-link and read any commands off of the output link.
             */
            kernel.RegisterForUpdateEvent(
                    smlUpdateEventId.smlEVENT_AFTER_ALL_OUTPUT_PHASES,
                    new UpdateEventInterface()
                    {
                        @Override
                        public void updateEventHandler(int eventID,
                                Object data, Kernel kernel, int runFlags)
                        {
                            /*
                             * Pull each line out of the queue and post it on
                             * the input-link.
                             */
                            for (String line = lines.poll(); line != null; line = lines
                                    .poll())
                            {
                                /*
                                 * Each message has its own message WME off of
                                 * the messages root.
                                 */
                                Identifier message = messagesRoot
                                        .CreateIdWME("message");
                                /*
                                 * On each message WME there are two attribute
                                 * value pairs: one is the integer id number of
                                 * the message and the other is its string
                                 * content.
                                 */
                                message.CreateIntWME("id", messages.size());
                                message.CreateStringWME("content", line);
                                /*
                                 * Store the identifier for easy clearing later.
                                 */
                                messages.add(message);
                            }

                            /*
                             * Iterate through the commands on the output link.
                             */
                            for (int index = 0; index < agent
                                    .GetNumberCommands(); ++index)
                            {
                                /*
                                 * Get the command by index. Note: avoid storing
                                 * this identifier because the agent created it
                                 * and may delete it at any time. If you need to
                                 * store it across decision cycles, you'll need
                                 * to do a lot more work to make sure it is
                                 * valid before attempting to use it in future
                                 * updates.
                                 */
                                Identifier command = agent.GetCommand(index);

                                /*
                                 * This is the attribute of the command
                                 * identifier's WME.
                                 */
                                String name = command.GetCommandName();

                                if (name.equals(PRINT))
                                {
                                    /*
                                     * Send print commands directly to the
                                     * listener and add status complete.
                                     */
                                    pl
                                            .printEvent("Agent said: "
                                                    + command
                                                            .GetParameterValue("content"));
                                    command.AddStatusComplete();

                                }
                                else if (name.equals(CLEAR))
                                {
                                    /*
                                     * Iterate through stored message WMEs and
                                     * delete them. Deleting a WME deletes all
                                     * of its children (in this case the id and
                                     * content WMEs).
                                     */
                                    for (Identifier message : messages)
                                    {
                                        message.DestroyWME();
                                    }

                                    /*
                                     * Clear our cache now that it has been
                                     * invalidated.
                                     */
                                    messages.clear();

                                    /*
                                     * Issue feedback and mark status complete.
                                     */
                                    pl.printEvent("Messages cleared.");
                                    command.AddStatusComplete();

                                }
                                else
                                {
                                    /*
                                     * Issue error feedback message and mark
                                     * status error..
                                     */
                                    pl.printEvent("Unknown command received: "
                                            + name);
                                    command.AddStatusError();
                                }
                            }

                            /*
                             * This marks any commands on the output-link as
                             * seen so they will not be encountered via
                             * GetCommand on future updates if they are still on
                             * the output-link then.
                             */
                            agent.ClearOutputLinkChanges();

                            /*
                             * Finally, check to see if we have been asked to
                             * stop and issue a stop if so.
                             */
                            if (stopSoar.get())
                            {
                                kernel.StopAllAgents();
                            }
                        }
                    }, null);
            /*
             * That final null parameter above is for user data. Anything passed
             * there will appear in the updateEventHandler's Object data
             * parameter.
             */
        }

        public void setPrintListener(PrintListener pl)
        {
            if (pl == null)
            {
                this.pl = nullListener;
            }
            else
            {
                this.pl = pl;
            }
        }

        public void run()
        {
            pl.printEvent("Starting Soar.");

            /*
             * Reset the request to stop just before we start up.
             */
            stopSoar.set(false);
            /*
             * This run call blocks, hopefully you're in a separate thread or
             * things will hang here.
             */
            kernel.RunAllAgentsForever();
            pl.printEvent("Stopping Soar.");
        }

        public void stop()
        {
            /*
             * Politely ask the agent to stop itself during its next update
             * event.
             */
            stopSoar.set(true);
        }

        public void put(String line)
        {
            /*
             * Queue the line for addition to the messages input-link. The queue
             * handles synchronization issues.
             */
            try
            {
                lines.put(line);
            }
            catch (InterruptedException e)
            {
                e.printStackTrace();
            }
        }

        public void shutdown()
        {
            /*
             * In case things are running, make a half-hearted attempt to stop
             * them first. This is a hack. Instead, you should be registering
             * for kernel events that tell you when Soar starts and stops so
             * that you know when you need to stop Soar and when it actually
             * stops. See kernel.RegisterForSystemEvent()
             */
            stop();
            try
            {
                Thread.sleep(500);
            }
            catch (InterruptedException ignored)
            {

            }
            /*
             * This will remove any agents and close the listener thread that
             * listens for things like remote debugger connections.
             */
            kernel.Shutdown();
        }
    }

    /*
     * This class is what the user will be interacting with when running this
     * simple environment. It listens for console input, parses it very naively
     * for some basic commands, and dumps everything else to the Soar interface
     * so that it can hand them off to the agent.
     * 
     * The constructor blocks as it executes until quit is entered at the
     * prompt.
     */
    public static class SimpleConsole
    {
        /*
         * We keep a reference to the Soar interface.
         */
        private final Soar soar;

        /*
         * Various string constants.
         */
        private final String PROMPT = "> ";

        private final String START = "start";

        private final String RUN = "run";

        private final String STOP = "stop";

        private final String QUIT = "quit";

        /*
         * Wrap standard input so that it is a bit easier to use.
         */
        private final BufferedReader input = new BufferedReader(
                new InputStreamReader(System.in));

        /*
         * Create an executor service to run Soar in since it blocks.
         */
        private final ExecutorService exec = Executors
                .newSingleThreadExecutor();

        public SimpleConsole(Soar soar)
        {
            this.soar = soar;
            /*
             * Tell the Soar interface we would like to receive its messages.
             */
            this.soar.setPrintListener(new PrintListener()
            {
                @Override
                public void printEvent(String message)
                {
                    /*
                     * Clearly distinguish output from the agent. In a GUI this
                     * would go to its own text box. Reprint the prompt since it
                     * is likely clobbered.
                     */
                    System.out.print(String.format("%nSoar interface: %s%n%s",
                            message, PROMPT));
                }
            });

            /*
             * Start the input loop. Separated in to its own function call for
             * clarity.
             */
            run();
        }

        /*
         * This is the main command loop. The input isn't fully sanitized so
         * don't bang on it too hard.
         * 
         * If it encounters start or run it executes Soar in a new thread. Stop
         * makes it request Soar to stop, and quit breaks out eventually
         * shutting down Soar and the executor service and closing the program.
         * 
         * Anything else just gets put to Soar so that it will show up as a
         * message.
         */
        private void run()
        {
            try
            {
                while (!Thread.interrupted())
                {
                    System.out.print(PROMPT);

                    String line = input.readLine();
                    if (line == null || line.isEmpty())
                    {
                        continue;
                    }

                    if (line.equalsIgnoreCase(START)
                            || line.equalsIgnoreCase(RUN))
                    {
                        /*
                         * Start Soar in its own thread. Soar implements
                         * Runnable which means it will execute the run()
                         * method.
                         * 
                         * Note that multiple successive calls to this will
                         * cause the run calls to queue up in the executor
                         * service. Calling start twice then stop will cause
                         * soar to stop (when stop is issued) and then
                         * immediately start up again as the second start gets
                         * processed. All of this can be prevented by
                         * registering for system start and stop events with the
                         * kernel. See kernel.RegisterForSystemEvent()
                         */
                        exec.execute(soar);

                    }
                    else if (line.equalsIgnoreCase(STOP))
                    {
                        /*
                         * Politely ask Soar to stop itself asynchronously. Does
                         * not block.
                         */
                        soar.stop();
                    }
                    else if (line.equalsIgnoreCase(QUIT))
                    {
                        /*
                         * We're done, goto finally.
                         */
                        break;
                    }
                    else
                    {
                        /*
                         * Everything else gets added to the Soar input link.
                         */
                        soar.put(line);
                    }
                }
            }
            catch (IOException e)
            {
                /*
                 * Thrown by readLine.
                 */
                e.printStackTrace();
            }
            finally
            {
                /*
                 * Shutdown the Soar interface and the executor service.
                 */
                soar.shutdown();
                exec.shutdown();
            }
        }
    }

    public static void main(String[] args)
    {
        /*
         * Create the Soar interface and hand it to the simple console.
         */
        Soar soar = new Soar();
        new SimpleConsole(soar); // blocks until quit
    }
}
