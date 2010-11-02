package edu.umich.dice;

import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import sml.Kernel;
import sml.WMElement;
import sml.Agent.OutputEventInterface;
import sml.Kernel.SystemEventInterface;

/**
 * <p>
 * Simple class that connects to a remote kernel, registers for a command,
 * listens for that command, computes dice probability, writes result to input
 * link.
 * 
 * @author Jonathan Voigt <voigtjr@gmail.com>
 * 
 */
public class SoarDiceModule
{
    private static final String HOST_DEFAULT = "localhost";

    private static final int PORT_DEFAULT = Kernel.kDefaultSMLPort;

    private static final String AGENT_DEFAULT = "soar1";

    /**
     * <p>
     * Attach this module to a running Soar agent.
     * 
     * @param args
     *            see usage()
     */
    public static void main(String[] args)
    {
        String host = HOST_DEFAULT;
        int port = PORT_DEFAULT;
        String agent = AGENT_DEFAULT;

        if (args.length >= 4)
        {
            System.out.println(usage());
            System.exit(1);
        }

        try
        {
            if (args.length >= 1)
                host = args[0];
            if (args.length >= 2)
                port = Integer.parseInt(args[1]);
            if (args.length == 3)
                agent = args[2];
        }
        catch (NumberFormatException e)
        {
            e.printStackTrace();
            System.out.println(usage());
            System.exit(1);
        }

        new SoarDiceModule(host, port, agent);
    }

    private static String usage()
    {
        StringBuilder usage = new StringBuilder("Usage: [HOST [PORT] [AGENT_NAME]]]\n");
        usage.append("\tHOST: Hostname of the instance of Soar to connect to. Default: ");
        usage.append(HOST_DEFAULT);
        usage.append("\n");
        usage.append("\tPORT: Port of the instance of Soar to connect to. Default: ");
        usage.append(PORT_DEFAULT);
        usage.append("\n");
        usage.append("\tAGENT_NAME: Agent name to connect to. Default: ");
        usage.append(AGENT_DEFAULT);
        usage.append("\n");
        return usage.toString();
    }

    /**
     * <p>
     * Attribute of the wme supplied by the agent to command this module.
     */
    public static final String COMMAND_NAME = "compute-dice-probability";

    /**
     * <p>
     * Attribute of the wme supplied by the agent with an agent-defined id.
     */
    public static final String COMMAND_ID = "id";

    /**
     * <p>
     * Attribute of the wme supplied by the agent indicating the number of dice
     * parameter.
     */
    public static final String COMMAND_DICE = "number-of-dice";

    /**
     * <p>
     * Attribute of the wme supplied by the agent indicating the number of faces
     * parameter.
     */
    public static final String COMMAND_SIDES = "number-of-faces";

    /**
     * <p>
     * Attribute of the wme supplied by the agent indicating the count
     * parameter.
     */
    public static final String COMMAND_COUNT = "count";

    /**
     * <p>
     * Attribute of the wme supplied by the agent indicating the predicate
     * parameter.
     */
    public static final String COMMAND_PREDICATE = "predicate";

    /**
     * <p>
     * Attribute of the wme placed on the command wme holding the error message
     * in the event of status error.
     */
    public static final String COMMAND_ERROR_FEEDBACK = "error-message";

    /**
     * <p>
     * Attribute of the wme serving as the parent for the results of commands.
     */
    public static final String RESULT_ROOT = "dice-probability";

    /**
     * <p>
     * Attribute of the wme with a copy of the id supplied by the agent in the
     * command.
     */
    public static final String RESULT_ID = "id";

    /**
     * <p>
     * Attribute of the wme with the result float 0..1
     */
    public static final String RESULT_PROB = "probability";

    private boolean stop = false;

    /**
     * <p>
     * Connect and register, then return.
     * 
     * @param host
     *            Machine that Soar is running on.
     * @param port
     *            Port that Soar is listening on (commonly 12121).
     * @param agentName
     *            The agent name to connect to.
     */
    public SoarDiceModule(String host, int port, String agentName)
    {
        final Kernel kernel = Kernel.CreateRemoteConnection(true, host, port);
        if (kernel.HadError())
            throw new IllegalStateException(kernel.GetLastErrorDescription());

        final Agent agent = kernel.GetAgent(agentName);
        if (agent == null)
            throw new IllegalStateException(kernel.GetLastErrorDescription());

        agent.AddOutputHandler(COMMAND_NAME, commandHandler, null);
        agent.SetOutputLinkChangeTracking(true);

        kernel.RegisterForSystemEvent(
                sml.smlSystemEventId.smlEVENT_BEFORE_SHUTDOWN, shutdownHandler,
                null);

        System.out.println("Connected, running.");
        while (!stop)
        {
            try
            {
                Thread.sleep(1000);
            }
            catch (InterruptedException e)
            {
                break;
            }
        }
        System.out.println("Shutting down.");
        System.exit(0);
    }

    private final SystemEventInterface shutdownHandler = new SystemEventInterface()
    {
        public void systemEventHandler(int eventID, Object data, Kernel kernel)
        {
            stop = true;
        }
    };

    /**
     * <p>
     * Handles commands as they are issued. Most errors make their way back to
     * the Soar agent via COMMAND_ERROR_FEEDBACK on command.
     */
    private final OutputEventInterface commandHandler = new OutputEventInterface()
    {
        public void outputEventHandler(Object userData, String agentName,
                String commandName, WMElement outputWme)
        {
            String errorMessage = "No error.";
            try
            {
                handleCommand(outputWme);
                return;
            }
            catch (SoarDiceException e)
            {
                errorMessage = e.getMessage();
                e.printStackTrace();
            }

            // report errors to Soar
            Identifier command = outputWme.ConvertToIdentifier();
            if (command == null)
                return;
            command.CreateStringWME(COMMAND_ERROR_FEEDBACK, errorMessage);
            command.GetAgent().Commit();
        }
    };

    private void handleCommand(WMElement outputWme) throws SoarDiceException
    {
        if (!outputWme.IsIdentifier())
            throw new SoarDiceException(COMMAND_NAME + " wme is not identifier");

        Identifier command = outputWme.ConvertToIdentifier();
        WMElement id = command.FindByAttribute(COMMAND_ID, 0);
        if (id == null)
            throw new SoarDiceException(COMMAND_NAME + " does not have "
                    + COMMAND_ID);

        String diceString = command.GetParameterValue(COMMAND_DICE);
        if (diceString == null)
            throw new SoarDiceException(id.GetValueAsString()
                    + " does not have " + COMMAND_DICE);

        String sidesString = command.GetParameterValue(COMMAND_SIDES);
        if (sidesString == null)
            throw new SoarDiceException(id.GetValueAsString()
                    + " does not have " + COMMAND_SIDES);

        String countString = command.GetParameterValue(COMMAND_COUNT);
        if (countString == null)
            throw new SoarDiceException(id.GetValueAsString()
                    + " does not have " + COMMAND_COUNT);

        String predString = command.GetParameterValue(COMMAND_PREDICATE);
        if (predString == null)
            throw new SoarDiceException(id.GetValueAsString()
                    + " does not have " + COMMAND_PREDICATE);

        try
        {
            int dice = Integer.valueOf(diceString);
            int sides = Integer.valueOf(sidesString);
            int count = Integer.valueOf(countString);

            LiarsDice.Predicate pred = LiarsDice.Predicate.valueOf(predString);

            Identifier root = getRoot(command.GetAgent());
            String idType = id.GetValueType();
            if (idType.equals("int"))
            {
                IntElement idint = id.ConvertToIntElement();
                root.CreateIntWME(RESULT_ID, idint.GetValue());
            }
            else if (idType.equals("float"))
            {
                FloatElement idfloat = id.ConvertToFloatElement();
                root.CreateFloatWME(RESULT_ID, idfloat.GetValue());
            }
            else
                root.CreateStringWME(RESULT_ID, id.GetValueAsString());

            double result = pred.get(dice, sides, count);
            root.CreateFloatWME(RESULT_PROB, result);

            command.AddStatusComplete();
            command.GetAgent().Commit();
            return;
        }
        catch (NumberFormatException e)
        {
            throw new SoarDiceException(
                    "Error parsing integer parameter dice, sides, or count");
        }
        catch (IllegalArgumentException e)
        {
            StringBuilder sb = new StringBuilder("Unknown predicate: ");
            sb.append(command.GetParameterValue(COMMAND_PREDICATE));
            sb.append(", possibilities are:");
            for (LiarsDice.Predicate p : LiarsDice.Predicate.values())
                sb.append(" ").append(p);
            throw new SoarDiceException(sb.toString());
        }
    }

    /**
     * <p>
     * Returns the root identifier on the passed agent's input link where to
     * place result structures for commands.
     * 
     * @param agent
     *            The current agent.
     * @return Parent wme to place result structures.
     * @throws SoarDiceException
     */
    private Identifier getRoot(Agent agent) throws SoarDiceException
    {
        WMElement rootwme = agent.GetInputLink()
                .FindByAttribute(RESULT_ROOT, 0);
        if (rootwme == null)
            return agent.GetInputLink().CreateIdWME(RESULT_ROOT);

        if (!rootwme.IsIdentifier())
            throw new SoarDiceException(RESULT_ROOT
                    + " exists on input-link and is not an identifier");

        return rootwme.ConvertToIdentifier();
    }

}
