/**
 * 
 */
package edu.umich.soar.sproom.soar.commands;

import java.util.Arrays;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.SharedNames;
import edu.umich.soar.sproom.command.CommandConfig;

import sml.Identifier;
import sml.WMElement;

/**
 * @author voigtjr
 *
 * Configure robot.
 */
public class ConfigureCommand extends OutputLinkCommand {
	private static final Log logger = LogFactory.getLog(ConfigureCommand.class);
	static final String NAME = "configure";
		
	private final Identifier wme;
	private boolean complete = false;
	
	ConfigureCommand(Identifier wme) {
		super(Integer.valueOf(wme.GetTimeTag()));
		this.wme = wme;
	}

	@Override
	public String getName() {
		return NAME;
	}
	
	@Override
	public OutputLinkCommand accept() {
		CommandConfig config = CommandConfig.CONFIG;
		
		{
			String temp = wme.GetParameterValue(SharedNames.LENGTH_UNITS);
			if (temp != null) {
				try {
					CommandConfig.LengthUnit lu = CommandConfig.LengthUnit.valueOf(temp.toUpperCase());
					config.setLengthUnits(lu);
					logger.debug(SharedNames.LENGTH_UNITS + ": " + lu);
				} catch (IllegalArgumentException e) {
					return new InvalidCommand(wme, "Unknown " + SharedNames.LENGTH_UNITS + " type " + temp);
				}
			}
		}
		
		{
			String temp = wme.GetParameterValue(SharedNames.ANGLE_UNITS);
			if (temp != null) {
				try {
					CommandConfig.AngleUnit au = CommandConfig.AngleUnit.valueOf(temp.toUpperCase());
					config.setAngleUnits(au);
					logger.debug(SharedNames.ANGLE_UNITS + ": " + au);
				} catch (IllegalArgumentException e) {
					return new InvalidCommand(wme, "Unknown " + SharedNames.ANGLE_UNITS + " type " + temp);
				}
			}
		}
		
		{
			String temp = wme.GetParameterValue(SharedNames.ANGLE_RESOLUTION);
			if (temp != null) {
				try {
					CommandConfig.AngleResolution ar = CommandConfig.AngleResolution.valueOf(temp.toUpperCase());
					config.setAngleResolution(ar);
					logger.debug(SharedNames.ANGLE_RESOLUTION + ": " + ar);
				} catch (IllegalArgumentException e) {
					return new InvalidCommand(wme, "Unknown " + SharedNames.ANGLE_RESOLUTION + " type " + temp);
				}
			}
		}
		
		{
			WMElement ptwme = wme.FindByAttribute(SharedNames.POSE_TRANSLATION, 0);
			if (ptwme != null) {
				if (!ptwme.IsIdentifier()) {
					return new InvalidCommand(wme, "Bad formatted command " + SharedNames.POSE_TRANSLATION);
				}
				Identifier ptid = ptwme.ConvertToIdentifier();
				double[] xyz = new double[] { 0, 0, 0 };
				
				String temp = ptid.GetParameterValue(SharedNames.X);
				if (temp != null) {
					try {
						xyz[0] = Double.parseDouble(temp);
						xyz[0] = config.lengthFromView(xyz[0]);
					} catch (NumberFormatException e) {
						return new InvalidCommand(wme, "Unable to parse " + 
								SharedNames.POSE_TRANSLATION + ": " + SharedNames.X + ": " + temp);
					}
				}
				temp = ptid.GetParameterValue(SharedNames.Y);
				if (temp != null) {
					try {
						xyz[1] = Double.parseDouble(temp);
						xyz[1] = config.lengthFromView(xyz[1]);
					} catch (NumberFormatException e) {
						return new InvalidCommand(wme, "Unable to parse " + SharedNames.POSE_TRANSLATION + 
								": " + SharedNames.Y + ": " + temp);
					}
				}
				temp = ptid.GetParameterValue(SharedNames.Z);
				if (temp != null) {
					try {
						xyz[2] = Double.parseDouble(temp);
						xyz[2] = config.lengthFromView(xyz[2]);
					} catch (NumberFormatException e) {
						return new InvalidCommand(wme, "Unable to parse " + SharedNames.POSE_TRANSLATION + 
								": " + SharedNames.Z + ": " + temp);
					}
				}
				config.setPoseTranslation(xyz);
				logger.debug(SharedNames.POSE_TRANSLATION + ": " + Arrays.toString(xyz));
			}
		}
		
		CommandStatus.accepted.addStatus(wme);
		return this;
	}

	@Override
	public void update(Adaptable app) {
		if (!complete) {
			CommandStatus.complete.addStatus(wme);
			complete = true;
		}
	}
}
