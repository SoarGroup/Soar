package edu.umich.soar.sproom.soar;

import java.util.concurrent.atomic.AtomicBoolean;

import edu.umich.soar.FloatWme;
import edu.umich.soar.IntWme;
import edu.umich.soar.StringWme;
import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.SharedNames;
import edu.umich.soar.sproom.command.CommandConfig;
import edu.umich.soar.sproom.command.CommandConfigListener;
import edu.umich.soar.sproom.drive.Drive2;
import edu.umich.soar.sproom.drive.Drive3;
import sml.Identifier;

/**
 * Input link management for reporting current simulation configuration.
 *
 * @author voigtjr@gmail.com
 */
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
	private final IntWme rangeCount;
	private final FloatWme[] headingPid = new FloatWme[3];
	private final FloatWme[] angularPid = new FloatWme[3];
	private final FloatWme[] linearPid = new FloatWme[3];
	private final YawWme fieldOfView;
	private final FloatWme visibleTime;
	private final DistanceWme manipulationDistanceMin;
	private final DistanceWme manipulationDistanceMax;
	
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
		
		Identifier headingPidRoot = root.CreateIdWME(SharedNames.HEADING);
		headingPid[0] = FloatWme.newInstance(headingPidRoot, SharedNames.P);
		headingPid[1] = FloatWme.newInstance(headingPidRoot, SharedNames.I);
		headingPid[2] = FloatWme.newInstance(headingPidRoot, SharedNames.D);
		
		Identifier angularPidRoot = root.CreateIdWME(SharedNames.ANGULAR);
		angularPid[0] = FloatWme.newInstance(angularPidRoot, SharedNames.P);
		angularPid[1] = FloatWme.newInstance(angularPidRoot, SharedNames.I);
		angularPid[2] = FloatWme.newInstance(angularPidRoot, SharedNames.D);
		
		Identifier linearPidRoot = root.CreateIdWME(SharedNames.LINEAR);
		linearPid[0] = FloatWme.newInstance(linearPidRoot, SharedNames.P);
		linearPid[1] = FloatWme.newInstance(linearPidRoot, SharedNames.I);
		linearPid[2] = FloatWme.newInstance(linearPidRoot, SharedNames.D);
		
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
		
		rangeCount = IntWme.newInstance(root, SharedNames.RANGE_COUNT);
		fieldOfView = YawWme.newInstance(root, SharedNames.FIELD_OF_VIEW);
		visibleTime = FloatWme.newInstance(root, SharedNames.VISIBLE_TIME);
		manipulationDistanceMin = DistanceWme.newInstance(root, SharedNames.MANIPULATION_DISTANCE_MIN);
		manipulationDistanceMax = DistanceWme.newInstance(root, SharedNames.MANIPULATION_DISTANCE_MAX);
		
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
				headingPid[i].update(c.getGains(Drive3.HEADING_PID_NAME)[i]);
				angularPid[i].update(c.getGains(Drive2.ANGULAR_PID_NAME)[i]);
				linearPid[i].update(c.getGains(Drive2.LINEAR_PID_NAME)[i]);
			}
			
			limitLinVelMax.update(c.getLimitLinVelMax());
			limitLinVelMin.update(c.getLimitLinVelMin());
			limitAngVelMax.update(c.getLimitAngVelMax());
			limitAngVelMin.update(c.getLimitAngVelMin());
			
			geomLength.update(c.getGeomLength());
			geomWidth.update(c.getGeomWidth());
			geomHeight.update(c.getGeomHeight());
			geomWheelbase.update(c.getGeomWheelbase());

			rangeCount.update(c.getRangeCount());
			fieldOfView.update(c.getFieldOfView());
			visibleTime.update(c.getVisibleNanoTime() / 1000000000.0);
			manipulationDistanceMin.update(c.getManipulationDistanceMin());
			manipulationDistanceMax.update(c.getManipulationDistanceMin());
			
			// TODO: verify everything from config is here, currently missing random seed (at least)

			configChanged.set(false);
		}
	}

	@Override
	public void destroy() {
		this.root.DestroyWME();
	}
}
