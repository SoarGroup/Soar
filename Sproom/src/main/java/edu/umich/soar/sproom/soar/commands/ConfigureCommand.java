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
 * Configure robot.
 *
 * @author voigtjr@gmail.com
 */
public class ConfigureCommand extends OutputLinkCommand {
	private static final Log logger = LogFactory.getLog(ConfigureCommand.class);
	static final String NAME = "configure";
		
	private final Identifier wme;
	
	public ConfigureCommand(Identifier wme) {
		super(wme);
		this.wme = wme;
	}

	@Override
	protected boolean accept() {
		CommandConfig config = CommandConfig.CONFIG;
		
		{
			String temp = wme.GetParameterValue(SharedNames.LENGTH_UNITS);
			if (temp != null) {
				try {
					CommandConfig.LengthUnit lu = CommandConfig.LengthUnit.valueOf(temp.toUpperCase());
					config.setLengthUnits(lu);
					logger.debug(SharedNames.LENGTH_UNITS + ": " + lu);
				} catch (IllegalArgumentException e) {
					addStatusError("Unknown " + SharedNames.LENGTH_UNITS + " type " + temp);
					return false;
				}
			}
		}
		
		{
			String temp = wme.GetParameterValue(SharedNames.SPEED_UNITS);
			if (temp != null) {
				try {
					CommandConfig.SpeedUnit su = CommandConfig.SpeedUnit.valueOf(temp.toUpperCase());
					config.setSpeedUnits(su);
					logger.debug(SharedNames.SPEED_UNITS + ": " + su);
				} catch (IllegalArgumentException e) {
					addStatusError("Unknown " + SharedNames.SPEED_UNITS + " type " + temp);
					return false;
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
					addStatusError("Unknown " + SharedNames.ANGLE_UNITS + " type " + temp);
					return false;
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
					addStatusError("Unknown " + SharedNames.ANGLE_RESOLUTION + " type " + temp);
					return false;
				}
			}
		}
		
		{
			WMElement ptwme = wme.FindByAttribute(SharedNames.POSE_TRANSLATION, 0);
			if (ptwme != null) {
				if (!ptwme.IsIdentifier()) {
					addStatusError("Bad formatted command " + SharedNames.POSE_TRANSLATION);
					return false;
				}
				Identifier ptid = ptwme.ConvertToIdentifier();
				double[] xyz = new double[] { 0, 0, 0 };
				
				String temp = ptid.GetParameterValue(SharedNames.X);
				if (temp != null) {
					try {
						xyz[0] = Double.parseDouble(temp);
						xyz[0] = config.lengthFromView(xyz[0]);
					} catch (NumberFormatException e) {
						addStatusError("Unable to parse " + 
								SharedNames.POSE_TRANSLATION + ": " + SharedNames.X + ": " + temp);
						return false;
					}
				}
				temp = ptid.GetParameterValue(SharedNames.Y);
				if (temp != null) {
					try {
						xyz[1] = Double.parseDouble(temp);
						xyz[1] = config.lengthFromView(xyz[1]);
					} catch (NumberFormatException e) {
						addStatusError("Unable to parse " + SharedNames.POSE_TRANSLATION + 
								": " + SharedNames.Y + ": " + temp);
						return false;
					}
				}
				temp = ptid.GetParameterValue(SharedNames.Z);
				if (temp != null) {
					try {
						xyz[2] = Double.parseDouble(temp);
						xyz[2] = config.lengthFromView(xyz[2]);
					} catch (NumberFormatException e) {
						addStatusError("Unable to parse " + SharedNames.POSE_TRANSLATION + 
								": " + SharedNames.Z + ": " + temp);
						return false;
					}
				}
				config.setPoseTranslation(xyz);
				logger.debug(SharedNames.POSE_TRANSLATION + ": " + Arrays.toString(xyz));
			}
		}
		
		addStatus(CommandStatus.ACCEPTED);
		return true;
	}

	@Override
	public void update(Adaptable app) {
		addStatus(CommandStatus.COMPLETE);
	}
}
