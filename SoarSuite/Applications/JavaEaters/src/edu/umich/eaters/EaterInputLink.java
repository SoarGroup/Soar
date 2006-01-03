/* File: EaterInputLink.java
 * Date: Jul 06, 2005
 */
package edu.umich.eaters;

import sml.*;

/**
 * A simple class to communicate between the Agent's input link and the Eater in the sim
 * 
 * @author Alex Roper
 */
public class EaterInputLink {
	
	private class MapSquare {
		Identifier me = null;
		StringElement content = null;

		Identifier north = null;
		Identifier south = null;
		Identifier east = null;
		Identifier west = null;
		
		boolean iterated = false;
		
		public MapSquare() {}			
	}
	
	Identifier iRoot = null;
	  Identifier eater = null;
	    StringElement direction = null;
	    StringElement name = null;
	    IntElement score = null;
	    IntElement X = null;
	    IntElement Y = null;
	
	MapSquare[][] fiveByFive;//[2][2] is my-location.
	
	Agent agent;
	
	public EaterInputLink(Agent agentIn)
	{
		fiveByFive = new MapSquare[5][5];
		for (int a = 0;a < 5;a++)
		{
			for (int b = 0;b < 5;b++)
			{
				fiveByFive[a][b] = new MapSquare();
			}
		}
		agent = agentIn;
		genStructure();
	}
	
	private void genStructure()
	{
		if (iRoot == null)
			iRoot = agent.GetInputLink();
		if (eater == null)
			eater = agent.CreateIdWME(iRoot,"eater");
		if (fiveByFive[2][2].me == null)
		{
			fiveByFive[2][2].me = agent.CreateIdWME(iRoot,"my-location");
			createSoarLocationMap(2,2);
		}
		
		agent.Commit();
	}
	
	public void update(EaterInputInfo iInfo)
	{
		//Update children of ^eater
		//TODO: actually put direction on iLink
		if (direction == null)
			direction = agent.CreateStringWME(eater,"direction","north");
		else if (!direction.GetValue().equals("north"))
			agent.Update(direction,"north");
		
		if (name == null)
			name = agent.CreateStringWME(eater,"name",iInfo.eater.name);
		else if (!name.GetValue().equals(iInfo.eater.name))
			agent.Update(name,iInfo.eater.name);
		
		if (score == null)
			score = agent.CreateIntWME(eater,"score",iInfo.eater.score);
		else if (score.GetValue() != iInfo.eater.score)
			agent.Update(score,iInfo.eater.score);
		
		if (X == null)
			X = agent.CreateIntWME(eater,"x",iInfo.eater.x);
		else if (X.GetValue() != iInfo.eater.x)
			agent.Update(X,iInfo.eater.x);
		
		if (Y == null)
			Y = agent.CreateIntWME(eater,"y",iInfo.eater.y);
		else if (Y.GetValue() != iInfo.eater.y)
			agent.Update(Y,iInfo.eater.y);
		
		// This commented out code would look to see if the map has changed, and if so recreate it
		//We want the map recreated every turn in any case, so just set dirty to true.
		boolean dirty = true;
		//This implementation could interfere with advanced eaters such as mappers and planners
		//FIXME: use correct input link updates so planning & mapping would work
		/*
		//Determine if any part of the map has changed
		boolean dirty = false;
		for (int x = 0;x < 5 && !dirty;x++)
		{
			for (int y = 0;y < 5 && !dirty;y++)
			{
				if (fiveByFive[x][y].content == null)
					dirty = true;
				else if (iInfo.visibleSquares[x][y] instanceof BonusFood && !fiveByFive[x][y].content.GetValue().equals("bonusfood"))
					dirty = true;
				else if (iInfo.visibleSquares[x][y] instanceof NormalFood && !fiveByFive[x][y].content.GetValue().equals("normalfood"))
					dirty = true;
				else if (iInfo.visibleSquares[x][y] instanceof Eater && !fiveByFive[x][y].content.GetValue().equals("eater"))
					dirty = true;
				else if (iInfo.visibleSquares[x][y] instanceof EatersEmpty && !fiveByFive[x][y].content.GetValue().equals("empty"))
					dirty = true;
				else if (iInfo.visibleSquares[x][y] instanceof Wall && !fiveByFive[x][y].content.GetValue().equals("wall"))
					dirty = true;
			}
		}
		*/
		
		if (dirty)
		{
			for (int x = 0;x < 5;x++)
			{
				for (int y = 0;y < 5;y++)
				{
					String cont = "";
					if (iInfo.visibleSquares[x][y] instanceof BonusFood)
						cont = "bonusfood";
					else if (iInfo.visibleSquares[x][y] instanceof NormalFood)
						cont = "normalfood";
					else if (iInfo.visibleSquares[x][y] instanceof Eater)
						cont = "eater";
					else if (iInfo.visibleSquares[x][y] instanceof EatersEmpty)
						cont = "empty";
					else if (iInfo.visibleSquares[x][y] instanceof EatersWall)
						cont = "wall";
					
					if (fiveByFive[x][y].content == null)
						fiveByFive[x][y].content = agent.CreateStringWME(fiveByFive[x][y].me,"content",cont);
					//If any part of the map changes, update it ALL even if a given square has not changed
					else
						agent.Update(fiveByFive[x][y].content,cont);
				}
			}
		}
		
		agent.Commit();
		
	}
	/**
	 * Recursive routine to update and create the map structure of the eater.
	 * Creates links if square doesn't exist, otherwise creates
	 * 
	 * @param x X coordinate
	 * @param y Y coordinate
	 */
	private void createSoarLocationMap(int x,int y)
	{
		if (x >= 0 && x <= 4 && y >=0 && y <= 4 && !fiveByFive[x][y].iterated)
		{
			fiveByFive[x][y].iterated = true;
			if (x > 0)
			{
				if (fiveByFive[x - 1][y].me == null)
					fiveByFive[x - 1][y].me = agent.CreateIdWME(fiveByFive[x][y].me,"west");
				else
					fiveByFive[x][y].west = agent.CreateSharedIdWME(fiveByFive[x][y].me,"west",fiveByFive[x - 1][y].me);
			}
			
			if (x < 4)
			{
				if (fiveByFive[x + 1][y].me == null)
					fiveByFive[x + 1][y].me = agent.CreateIdWME(fiveByFive[x][y].me,"east");
				else
					fiveByFive[x][y].east = agent.CreateSharedIdWME(fiveByFive[x][y].me,"east",fiveByFive[x + 1][y].me);
			}
			
			if (y > 0)
			{
				if (fiveByFive[x][y - 1].me == null)
					fiveByFive[x][y - 1].me = agent.CreateIdWME(fiveByFive[x][y].me,"north");
				else
					fiveByFive[x][y].north = agent.CreateSharedIdWME(fiveByFive[x][y].me,"north",fiveByFive[x][y - 1].me);
			}
			
			if (y < 4)
			{
				if (fiveByFive[x][y + 1].me == null)
					fiveByFive[x][y + 1].me = agent.CreateIdWME(fiveByFive[x][y].me,"south");
				else
					fiveByFive[x][y].south = agent.CreateSharedIdWME(fiveByFive[x][y].me,"south",fiveByFive[x][y + 1].me);
			}
			
			createSoarLocationMap(x - 1,y);
			createSoarLocationMap(x + 1,y);
			createSoarLocationMap(x,y - 1);
			createSoarLocationMap(x,y + 1);
		}
	}	
}
