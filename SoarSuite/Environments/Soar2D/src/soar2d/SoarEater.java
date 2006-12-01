package soar2d;

import java.util.logging.*;
import sml.*;
import soar2d.world.Cell;

class SoarCell {
	Identifier me;
	StringElement content;

	Identifier north;
	Identifier south;
	Identifier east;
	Identifier west;
	
	boolean iterated = false;
}

public class SoarEater extends Eater {
	Logger logger = Soar2D.logger;
	
	private Agent agent;
	private float random;
	java.awt.Point previousLocation;
	
	private StringElement directionWME;
	private IntElement scoreWME;
	private IntElement xWME;
	private IntElement yWME;
	private FloatElement randomWME;
	private SoarCell[][] cells = new SoarCell[(Soar2D.config.kEaterVision * 2 ) + 1][(Soar2D.config.kEaterVision * 2 ) + 1];

	public SoarEater(Agent agent, int facingInt, int points, String color) {
		super(agent.GetAgentName(), facingInt, points, color);
		
		previousLocation = new java.awt.Point(-1, -1);
		
		Identifier eater = agent.CreateIdWME(agent.GetInputLink(), Names.kEaterID);
		
		directionWME = agent.CreateStringWME(eater, Names.kDirectionID, Names.kNorth);
		agent.CreateStringWME(eater, Names.kNameID, getName());
		scoreWME = agent.CreateIntWME(eater, Names.kScoreID, getPoints());
		xWME = agent.CreateIntWME(eater, Names.kxID, 0);
		yWME = agent.CreateIntWME(eater, Names.kyID, 0);
		
		for (int i = 0; i < cells.length; ++i) {
			for (int j = 0; j < cells.length; ++j) {
				cells[i][j] = new SoarCell();
			}
		}
		
		cells[Soar2D.config.kEaterVision][Soar2D.config.kEaterVision].me = agent.CreateIdWME(agent.GetInputLink(), Names.kMyLocationID);
		createView(Soar2D.config.kEaterVision, Soar2D.config.kEaterVision);
		
		random = 0;
		generateNewRandom();
		randomWME = agent.CreateFloatWME(agent.GetInputLink(), Names.kRandomID, random);
		
		agent.Commit();
	}
	
	private void createView(int x, int y) {
		if (x >= 0 && x <= (Soar2D.config.kEaterVision * 2) && y >=0 && y <= (Soar2D.config.kEaterVision * 2) && !cells[x][y].iterated) {
			cells[x][y].iterated = true;
			cells[x][y].content = agent.CreateStringWME(cells[x][y].me, Names.kContentID, CellType.EMPTY.toString());

			if (x > 0) {
				if (cells[x - 1][y].me == null)
					cells[x - 1][y].me = agent.CreateIdWME(cells[x][y].me, Names.kWest);
				else
					cells[x][y].west = agent.CreateSharedIdWME(cells[x][y].me, Names.kWest, cells[x - 1][y].me);
			}
			
			if (x < (Soar2D.config.kEaterVision * 2)) {
				if (cells[x + 1][y].me == null)
					cells[x + 1][y].me = agent.CreateIdWME(cells[x][y].me, Names.kEast);
				else
					cells[x][y].east = agent.CreateSharedIdWME(cells[x][y].me, Names.kEast, cells[x + 1][y].me);
			}
			
			if (y > 0) {
				if (cells[x][y - 1].me == null)
					cells[x][y - 1].me = agent.CreateIdWME(cells[x][y].me, Names.kNorth);
				else
					cells[x][y].north = agent.CreateSharedIdWME(cells[x][y].me, Names.kNorth, cells[x][y - 1].me);
			}
			
			if (y < (Soar2D.config.kEaterVision * 2)) {
				if (cells[x][y + 1].me == null)
					cells[x][y + 1].me = agent.CreateIdWME(cells[x][y].me, Names.kSouth);
				else
					cells[x][y].south = agent.CreateSharedIdWME(cells[x][y].me, Names.kSouth, cells[x][y + 1].me);
			}
			
			createView(x - 1,y);
			createView(x + 1,y);
			createView(x,y - 1);
			createView(x,y + 1);
		}	
	}
	
	private void generateNewRandom() {
		float newRandom;
		do {
			newRandom = Simulation.random.nextFloat();
		} while (this.random == newRandom);
		this.random = newRandom;
	}

	public void update(World world, java.awt.Point location) {
		boolean moved = (location.x != this.previousLocation.x) || (location.y != this.previousLocation.y);
		
		int xView, yView;
		int worldSize = world.getSize();
		for (int x = 0; x < cells.length; ++x) {
			xView = x - Soar2D.config.kEaterVision + location.x;
			if (xView < 0) {
				continue;
			} else if (xView >= worldSize) {
				break;
			}
			for (int y = 0; y < cells[x].length; ++y) {
				yView = y - Soar2D.config.kEaterVision + location.y;
				if (yView < 0) {
					continue;
				} else if (yView >= worldSize) {
					break;
				}
				Cell cell = world.getCell(xView, yView);
				String content = null;
				if (cell.getEater() != null) {
					content = Names.kEater;
					
				} else if (cell.getFood() != null) {
					content = cell.getFood().name();
			
				} else {
					content = cell.getType().name();
				}
				if (moved || !cells[x][y].content.GetValue().equalsIgnoreCase(content)) {
					agent.Update(cells[x][y].content, content);
				}
			}
		}

		if (scoreWME.GetValue() != getPoints()) {
			agent.Update(scoreWME, getPoints());
		}
		
		if (!directionWME.GetValue().equalsIgnoreCase(Direction.stringOf[getFacingInt()])) {
			agent.Update(directionWME, Direction.stringOf[getFacingInt()]);
		}

		if (moved) {
			agent.Update(xWME, location.x);
			agent.Update(yWME, location.y);
		}
		
		// Random
		float oldrandom = random;
		do {
			random = Simulation.random.nextFloat();
		} while (random == oldrandom);
		agent.Update(randomWME, random);
		
		agent.Commit();
	}
	
	public void reset() {
		// FIXME: init-soar
	}
	public void shutdown() {
		// FIXME: kernel.DestroyAgent()
	}
}
