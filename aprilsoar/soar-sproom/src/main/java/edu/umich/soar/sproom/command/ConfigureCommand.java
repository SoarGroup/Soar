/**
 * 
 */
package edu.umich.soar.sproom.command;

import java.util.Arrays;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import edu.umich.soar.sproom.Adaptable;

import lcmtypes.pose_t;

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
	
	private static final String LENGTH_UNITS = "length-units";
	private static final String ANGLE_UNITS = "angle-units";
	private static final String ANGLE_RESOLUTION = "angle-resolution";
	private static final String POSE_TRANSLATION = "pose_translation";
	private static final String X = "x";
	private static final String Y = "y";
	private static final String Z = "z";
	
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
			String temp = wme.GetParameterValue(LENGTH_UNITS);
			if (temp != null) {
				try {
					CommandConfig.LengthUnit lu = CommandConfig.LengthUnit.valueOf(temp.toUpperCase());
					config.setLengthUnits(lu);
					logger.debug(LENGTH_UNITS + ": " + lu);
				} catch (IllegalArgumentException e) {
					return new InvalidCommand(wme, "Unknown " + LENGTH_UNITS + " type " + temp);
				}
			}
		}
		
		{
			String temp = wme.GetParameterValue(ANGLE_UNITS);
			if (temp != null) {
				try {
					CommandConfig.AngleUnit au = CommandConfig.AngleUnit.valueOf(temp.toUpperCase());
					config.setAngleUnits(au);
					logger.debug(ANGLE_UNITS + ": " + au);
				} catch (IllegalArgumentException e) {
					return new InvalidCommand(wme, "Unknown " + ANGLE_UNITS + " type " + temp);
				}
			}
		}
		
		{
			String temp = wme.GetParameterValue(ANGLE_RESOLUTION);
			if (temp != null) {
				try {
					CommandConfig.AngleResolution ar = CommandConfig.AngleResolution.valueOf(temp.toUpperCase());
					config.setAngleResolution(ar);
					logger.debug(ANGLE_RESOLUTION + ": " + ar);
				} catch (IllegalArgumentException e) {
					return new InvalidCommand(wme, "Unknown " + ANGLE_RESOLUTION + " type " + temp);
				}
			}
		}
		
		{
			WMElement ptwme = wme.FindByAttribute(POSE_TRANSLATION, 0);
			if (ptwme != null) {
				if (!ptwme.IsIdentifier()) {
					return new InvalidCommand(wme, "Bad formatted command " + POSE_TRANSLATION);
				}
				Identifier ptid = ptwme.ConvertToIdentifier();
				double[] xyz = new double[] { 0, 0, 0 };
				
				String temp = ptid.GetParameterValue(X);
				if (temp != null) {
					try {
						xyz[0] = Double.parseDouble(temp);
						xyz[0] = config.lengthFromView(xyz[0]);
					} catch (NumberFormatException e) {
						return new InvalidCommand(wme, "Unable to parse " + POSE_TRANSLATION + ": " + X + ": " + temp);
					}
				}
				temp = ptid.GetParameterValue(Y);
				if (temp != null) {
					try {
						xyz[1] = Double.parseDouble(temp);
						xyz[1] = config.lengthFromView(xyz[1]);
					} catch (NumberFormatException e) {
						return new InvalidCommand(wme, "Unable to parse " + POSE_TRANSLATION + ": " + Y + ": " + temp);
					}
				}
				temp = ptid.GetParameterValue(Z);
				if (temp != null) {
					try {
						xyz[2] = Double.parseDouble(temp);
						xyz[2] = config.lengthFromView(xyz[2]);
					} catch (NumberFormatException e) {
						return new InvalidCommand(wme, "Unable to parse " + POSE_TRANSLATION + ": " + Z + ": " + temp);
					}
				}
				config.setPoseTranslation(xyz);
				logger.debug(POSE_TRANSLATION + ": " + Arrays.toString(xyz));
			}
		}
		
		CommandStatus.accepted.addStatus(wme);
		return this;
	}

	@Override
	public void update(pose_t pose, Adaptable app) {
		if (!complete) {
			CommandStatus.complete.addStatus(wme);
			complete = true;
		}
	}
}
