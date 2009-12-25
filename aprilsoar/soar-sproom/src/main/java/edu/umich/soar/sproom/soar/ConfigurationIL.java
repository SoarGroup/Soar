package edu.umich.soar.sproom.soar;

import java.util.concurrent.atomic.AtomicBoolean;

import edu.umich.soar.FloatWme;
import edu.umich.soar.StringWme;
import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.SharedNames;
import edu.umich.soar.sproom.command.CommandConfig;
import edu.umich.soar.sproom.command.CommandConfigListener;
import sml.Identifier;

public class ConfigurationIL implements InputLinkElement {
	private static final String LIMITS = "limits";
	private static final String VELOCITY = "velocity";
	private static final String LINEAR = "linear";
	private static final String MAXIMUM = "maximum";
	private static final String MINIMUM = "minimum";
	private static final String ANGULAR = "angular";
	private static final String GEOMETRY = "geometry";
	private static final String LENGTH = "length";
	private static final String WIDTH = "width";
	private static final String HEIGHT = "height";
	private static final String WHEELBASE = "wheelbase";
	
	private final CommandConfig c = CommandConfig.CONFIG;

	private final StringWme lengthUnits;
	private final StringWme angleUnits;
	private final StringWme angleResolution;
	private final FloatWme[] xyz = new FloatWme[3];
	private final FloatWme limitLinVelMax;
	private final FloatWme limitLinVelMin;
	private final FloatWme limitAngVelMax;
	private final FloatWme limitAngVelMin;
	private final FloatWme geomLength;
	private final FloatWme geomWidth;
	private final FloatWme geomHeight;
	private final FloatWme geomWheelbase;
	
	private final AtomicBoolean configChanged = new AtomicBoolean(true);
	
	public ConfigurationIL(Identifier root, Adaptable app) {
		lengthUnits = StringWme.newInstance(root, SharedNames.LENGTH_UNITS);
		angleUnits = StringWme.newInstance(root, SharedNames.ANGLE_UNITS);
		angleResolution = StringWme.newInstance(root, SharedNames.ANGLE_RESOLUTION);
		
		Identifier poseTranslation = root.CreateIdWME(SharedNames.POSE_TRANSLATION);
		xyz[0] = FloatWme.newInstance(poseTranslation, SharedNames.X);
		xyz[1] = FloatWme.newInstance(poseTranslation, SharedNames.Y);
		xyz[2] = FloatWme.newInstance(poseTranslation, SharedNames.Z);
		
		{
			Identifier limits = root.CreateIdWME(LIMITS);
			Identifier velocity = limits.CreateIdWME(VELOCITY);
			Identifier linear = velocity.CreateIdWME(LINEAR);
			limitLinVelMax = FloatWme.newInstance(linear, MAXIMUM);
			limitLinVelMin = FloatWme.newInstance(linear, MINIMUM);
			Identifier angular = velocity.CreateIdWME(ANGULAR);
			limitAngVelMax = FloatWme.newInstance(angular, MAXIMUM);
			limitAngVelMin = FloatWme.newInstance(angular, MINIMUM);
		}
		{
			Identifier geometry = root.CreateIdWME(GEOMETRY);
			geomLength = FloatWme.newInstance(geometry, LENGTH);
			geomWidth = FloatWme.newInstance(geometry, WIDTH);
			geomHeight = FloatWme.newInstance(geometry, HEIGHT);
			geomWheelbase = FloatWme.newInstance(geometry, WHEELBASE);
		}
		
		update(app);
		
		c.addListener(new CommandConfigListener() {
			@Override
			public void configChanged() {
				configChanged.set(true);
			}
		});
	}

	@Override
	public void update(Adaptable app) {
		if (configChanged.get()) {
			lengthUnits.update(c.getLengthUnits().toString().toLowerCase());
			angleUnits.update(c.getAngleUnits().toString().toLowerCase());
			angleResolution.update(c.getAngleResolution().toString().toLowerCase());

			for (int i = 0; i < xyz.length; ++i) {
				xyz[i].update(c.getPoseTranslation()[i]);
			}
			
			limitLinVelMax.update(c.getLimitLinVelMax());
			limitLinVelMin.update(c.getLimitLinVelMin());
			limitAngVelMax.update(c.getLimitAngVelMax());
			limitAngVelMin.update(c.getLimitAngVelMin());
			
			geomLength.update(c.getGeomLength());
			geomWidth.update(c.getGeomWidth());
			geomHeight.update(c.getGeomHeight());
			geomWheelbase.update(c.getGeomWheelbase());
			
			configChanged.set(false);
		}
	}
}
