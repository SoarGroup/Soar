package edu.umich.soar.sproom.soar;

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import lcmtypes.pose_t;
import sml.Identifier;
import edu.umich.soar.IntWme;
import edu.umich.soar.StringWme;
import edu.umich.soar.sproom.Adaptable;
import edu.umich.soar.sproom.SharedNames;
import edu.umich.soar.sproom.command.MapMetadata;
import edu.umich.soar.sproom.command.Pose;
import edu.umich.soar.sproom.command.MapMetadata.Area;
import edu.umich.soar.sproom.command.MapMetadata.Gateway;
import edu.umich.soar.sproom.command.MapMetadata.Walls.WallDir;

public class AreaDescriptionIL implements InputLinkElement {
	private static final Log logger = LogFactory.getLog(AreaDescriptionIL.class);

	private Area lastArea;
	private final IntWme idwme;
	private final StringWme	typewme;
	private final List<PointDataIL> pointDataList = new ArrayList<PointDataIL>();
	private final List<Identifier> oldGatewaysWalls = new ArrayList<Identifier>();
	private final Identifier root;
	
	public AreaDescriptionIL(Identifier root, Adaptable app) {
		this.root = root;
		
		idwme = IntWme.newInstance(root, SharedNames.ID);
		typewme = StringWme.newInstance(root, SharedNames.TYPE);
	}

	@Override
	public void update(Adaptable app) {
		MapMetadata metadata = (MapMetadata)app.getAdapter(MapMetadata.class);
		Pose poseClass = (Pose)app.getAdapter(Pose.class);

		pose_t pose = poseClass.getPose();
		if (pose == null) {
			return;
		}
		
		Area area = metadata.getArea(pose.pos);
		
		if (area != null) {
			metadata.publish(area);
			
			if (!area.equals(lastArea)) {
				logger.debug("new area data: " + area);
				lastArea = area;
				
				// new area data
				for (Identifier old : oldGatewaysWalls) {
					old.DestroyWME();
				}
				oldGatewaysWalls.clear();
				pointDataList.clear();
				
				idwme.update(area.getId());
				typewme.update(area.isDoor() ? SharedNames.DOOR : SharedNames.ROOM);
				
				// gateways
				for (int i = 0; i < area.getGateways().size(); ++i) {
					Gateway gateway = area.getGateways().get(i);
					String dir = area.getDirs().get(i).toString().toLowerCase();
					
					Identifier gatewaywme = root.CreateIdWME(SharedNames.GATEWAY);
					StringWme.newInstance(gatewaywme, SharedNames.DIRECTION, dir.toString().toLowerCase());

					IntWme.newInstance(gatewaywme, SharedNames.ID, gateway.getId());
					for (Area to : gateway.getTo()) {
						IntWme.newInstance(gatewaywme, SharedNames.TO, to.getId());
					}
					PointDataIL pointData = new PointDataIL(gatewaywme, gateway.getPos());
					pointDataList.add(pointData);
					oldGatewaysWalls.add(gatewaywme);
				}
				
				// walls
				for (WallDir dir : WallDir.values()) {
					Identifier wallwme = root.CreateIdWME(SharedNames.WALL);
					StringWme.newInstance(wallwme, SharedNames.DIRECTION, dir.toString().toLowerCase());
					
					PointDataIL pointData = new PointDataIL(wallwme, area.getWalls().getPos(dir));
					pointDataList.add(pointData);
					oldGatewaysWalls.add(wallwme);
				}
			}
		}
		
		for (PointDataIL pointData : pointDataList) {
			pointData.update(app);
		}
	}
	
	@Override
	public void destroy() {
		this.root.DestroyWME();
	}

}
