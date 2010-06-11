package edu.umich.dice;

import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import sml.Kernel;
import sml.WMElement;
import sml.Agent.OutputEventInterface;

/**
 * Simple class that connects to a remote kernel, registers for a command,
 * listens for that command, computes dice probability, writes result to
 * input link.
 * 
 * @author Jonathan Voigt <voigtjr@gmail.com>
 *
 */
public class SoarDiceModule
{
	public static final String COMMAND_NAME = "compute-dice-probability";
	public static final String COMMAND_ID = "id";
	public static final String COMMAND_DICE = "number-of-dice";
	public static final String COMMAND_SIDES = "number-of-faces";
	public static final String COMMAND_COUNT = "count";
	public static final String COMMAND_PREDICATE = "predicate";
	public static final String COMMAND_ERROR_FEEDBACK = "error-message";
	public static final String RESULT_ROOT = "dice-probability";
	public static final String RESULT_ID = "id";
	public static final String RESULT_PROB = "probability";
	
	/**
	 * Connect and register, then return.
	 * @param host
	 * @param port
	 * @param agentName
	 */
	public SoarDiceModule(String host, int port, String agentName)
	{
		final Kernel kernel = Kernel.CreateRemoteConnection(true, host, port);
		if (kernel.HadError()) {
			throw new IllegalStateException(kernel.GetLastErrorDescription());
		}

		final Agent agent = kernel.GetAgent(agentName);
		if (agent == null) {
			throw new IllegalStateException(kernel.GetLastErrorDescription());
		}
		
		agent.AddOutputHandler(COMMAND_NAME, commandHandler, null);
		agent.SetOutputLinkChangeTracking(true);
	}
	
	/**
	 * Handles commands as they are issued. Most errors make their way back to the
	 * Soar agent via COMMAND_ERROR_FEEDBACK on command. 
	 */
	private final OutputEventInterface commandHandler = new OutputEventInterface() {
		public void outputEventHandler(Object userData, String agentName, String commandName, WMElement outputWme) 
		{
			String errorMessage = "No error.";
			try 
			{
				if (!outputWme.IsIdentifier())
				{
					throw new IllegalStateException(COMMAND_NAME + " wme is not identifier");
				}
				
				Identifier command = outputWme.ConvertToIdentifier();
				WMElement id = command.FindByAttribute(COMMAND_ID, 0);
				String diceString = command.GetParameterValue(COMMAND_DICE);
				String sidesString = command.GetParameterValue(COMMAND_SIDES);
				String countString = command.GetParameterValue(COMMAND_COUNT);
				String predString = command.GetParameterValue(COMMAND_PREDICATE);
				
				int dice = Integer.valueOf(diceString);
				int sides = Integer.valueOf(sidesString);
				LiarsDice.Predicate pred = LiarsDice.Predicate.valueOf(predString);
				int count = Integer.valueOf(countString);
				
				Identifier root = getRoot(command.GetAgent());
				String idType = id.GetValueType();
				if (idType.equals("int")) {
					IntElement idint = id.ConvertToIntElement();
					root.CreateIntWME(RESULT_ID, idint.GetValue());
				} else if (idType.equals("float")) {
					FloatElement idfloat = id.ConvertToFloatElement();
					root.CreateFloatWME(RESULT_ID, idfloat.GetValue());
				} else {
					root.CreateStringWME(RESULT_ID, id.GetValueAsString());
				}
				
				double result = pred.get(dice, sides, count);
				root.CreateFloatWME(RESULT_PROB, result);
				
				command.AddStatusComplete();
				command.GetAgent().Commit();
				return;

			} catch (IllegalStateException e) {
				errorMessage = e.getMessage();
				e.printStackTrace();
			} catch (NumberFormatException e) {
				errorMessage = e.getMessage();
				e.printStackTrace();
			} catch (IllegalArgumentException e) {
				errorMessage = e.getMessage();
				e.printStackTrace();
			} catch (NullPointerException e) {
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
	
	private Identifier getRoot(Agent agent)
	{
		WMElement rootwme = agent.GetInputLink().FindByAttribute(RESULT_ROOT, 0);
		if (rootwme == null) 
			return agent.GetInputLink().CreateIdWME(RESULT_ROOT);

		if (!rootwme.IsIdentifier()) {
			throw new IllegalStateException(RESULT_ROOT + " exists and is not an identifier");
		}
		return rootwme.ConvertToIdentifier();
	}

}
