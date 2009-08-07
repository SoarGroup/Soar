package edu.umich.soar.gridmap2d.map;

import edu.umich.soar.gridmap2d.Direction;

public class SearchData {
	public static SearchData[][] newMap(GridMapCells cells) {
		SearchData[][] map = new SearchData[cells.size()][];
		int xy[] = new int[2];
		for(xy[0] = 0; xy[0] < cells.size(); ++xy[0]) {
			map[xy[0]] = new SearchData[cells.size()];
			for(xy[1] = 0; xy[1] < cells.size(); ++xy[1]) {
				map[xy[0]][xy[1]] = new SearchData(cells.getCell(xy));
			}
		}
		buildReferenceMap(cells, map);
		
		return map;
	}
	
	private static void buildReferenceMap(GridMapCells cells, SearchData[][] map) {
		// Build cell reference map
		assert cells != null;
		int [] xy = new int[2];
		for (xy[0] = 0; xy[0] < cells.size(); ++xy[0]) {
			for (xy[1] = 0; xy[1] < cells.size(); ++xy[1]) {
				Cell cell = cells.getCell(xy);
				SearchData cellSearch = SearchData.getCell(map, xy);
				for (Direction dir : Direction.values()) {
					if (dir == Direction.NONE) {
						continue;
					}
					int[] neighborLoc = Direction.translate(cell.getLocation(), dir, new int[2]);
					if (cells.isInBounds(neighborLoc)) {
						SearchData neighbor = SearchData.getCell(map, neighborLoc);
						cellSearch.setNeighbor(dir, neighbor);
					}
				}
			}
		}
	}

	public static SearchData getCell(SearchData[][] map, int[] xy) {
		return map[xy[0]][xy[1]];
	}
	
	private final Cell cell;
	private boolean explored = false;
	private int distance = -1;
	private SearchData parent;
	private final SearchData[] neighbors = new SearchData[5];
	
	private SearchData(Cell cell) {
		this.cell = cell;
	}
	
	public Cell getCell() {
		return cell;
	}

	public boolean isExplored() {
		return explored;
	}

	public void setExplored(boolean explored) {
		this.explored = explored;
	}

	public int getDistance() {
		return distance;
	}

	public void setDistance(int distance) {
		this.distance = distance;
	}

	public SearchData getParent() {
		return parent;
	}

	public void setParent(SearchData parent) {
		this.parent = parent;
	}
	
	public SearchData getNeighbor(Direction d) {
		return neighbors[d.ordinal()];
	}

	public void setNeighbor(Direction d, SearchData s) {
		neighbors[d.ordinal()] = s;
	}
}
