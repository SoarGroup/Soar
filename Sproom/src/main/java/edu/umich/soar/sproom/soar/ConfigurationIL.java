package edu.umich.soar.sproom.soar;

import java.util.concurrent.atomic.AtomicBoolean;

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

	private final Identifier root;
	private final StringWme lengthUnits;
	private final StringWme angleUnits;
	private final StringWme angleResolution;
	private final StringWme speedUnits;
	private final DistanceWme[] xyz = new DistanceWme[3];
	private final SpeedWme limitLinVelMax;
	private final SpeedWme limitLinVelMin;
	private final SpeedWme limitAngVelMax;
	private final SpeedWme limitAngVelMin;
	private final DistanceWme geomLength;
	private final DistanceWme geomWidth;
	private final DistanceWme geomHeight;
	private final DistanceWme geomWheelbase;
	
	private final AtomicBoolean configChanged = new AtomicBoolean(true);
	
	public ConfigurationIL(Identifier root, Adaptable app) {
		this.root = root;
		lengthUnits = StringWme.newInstance(root, SharedNames.LENGTH_UNITS);
		angleUnits = StringWme.newInstance(root, SharedNames.ANGLE_UNITS);
		angleResolution = StringWme.newInstance(root, SharedNames.ANGLE_RESOLUTION);
		speedUnits = StringWme.newInstance(root, SharedNames.SPEED_UNITS);
		
		Identifier poseTranslation = root.CreateIdWME(SharedNames.POSE_TRANSLATION);
		xyz[0] = DistanceWme.newInstance(poseTranslation, SharedNames.X);
		xyz[1] = DistanceWme.newInstance(poseTranslation, SharedNames.Y);
		xyz[2] = DistanceWme.newInstance(poseTranslation, SharedNames.Z);
		
		{
			Identifier limits = root.CreateIdWME(LIMITS);
			Identifier velocity = limits.CreateIdWME(VELOCITY);
			Identifier linear = velocity.CreateIdWME(LINEAR);
			limitLinVelMax = SpeedWme.newInstance(linear, MAXIMUM);
			limitLinVelMin = SpeedWme.newInstance(linear, MINIMUM);
			Identifier angular = velocity.CreateIdWME(ANGULAR);
			limitAngVelMax = SpeedWme.newInstance(angular, MAXIMUM);
			limitAngVelMin = SpeedWme.newInstance(angular, MINIMUM);
		}
		{
			Identifier geometry = root.CreateIdWME(GEOMETRY);
			geomLength = DistanceWme.newInstance(geometry, LENGTH);
			geomWidth = DistanceWme.newInstance(geometry, WIDTH);
			geomHeight = DistanceWme.newInstance(geometry, HEIGHT);
			geomWheelbase = DistanceWme.newInstance(geometry, WHEELBASE);
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
			speedUnits.update(c.getSpeedUnits().toString().toLowerCase());

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

	@Override
	public void destroy() {
		this.root.DestroyWME();
	}
}
