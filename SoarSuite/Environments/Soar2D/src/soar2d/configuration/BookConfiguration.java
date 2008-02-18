package soar2d.configuration;

import java.io.File;
import java.util.Iterator;
import java.util.List;

import org.jdom.Element;

import soar2d.configuration.LoadError;

public class BookConfiguration extends BaseConfiguration implements IConfiguration {

	BookConfiguration() {
		super();
		
		this.mapPath += "book" + System.getProperty("file.separator");
		this.agentPath += "book" + System.getProperty("file.separator");
		this.map = new File(this.mapPath + "default.bmap");
	}

	public String getMapExt() {
		return "bmap";
	}

	public String getTitle() {
		return "Book";
	}
	
	public void rules(Element rules) throws LoadError {
		List<Element> children = (List<Element>)rules.getChildren();
		if (children.size() < 1) {
			return;
		}
		if (children.size() > 1) {
			throw new LoadError("Only one rules set is allowed");
		}
		Element child = children.get(0);
		if (child.getName().equalsIgnoreCase(getTitle()) == false) {
			throw new LoadError("Expected " + getTitle() + " rules, got " + child.getName());
		}

		children = (List<Element>)child.getChildren();
		Iterator<Element> iter = children.iterator();
		while (iter.hasNext()) {
			child = iter.next();
			
			if (child.getName().equalsIgnoreCase(kTagColoredRooms)) {
				this.setColoredRooms(true);
				
			} else if (child.getName().equalsIgnoreCase(kTagSpeed)) {
				try {
					this.setSpeed(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagSpeed);
				}
				
			} else if (child.getName().equalsIgnoreCase(kTagRotateSpeed)) {
				try {
					this.setRotateSpeed(Float.parseFloat(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagRotateSpeed);
				}
				
			} else if (child.getName().equalsIgnoreCase(kTagBookCellSize)) {
				try {
					this.setBookCellSize(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagBookCellSize);
				}
				
			} else if (child.getName().equalsIgnoreCase(kTagCycleTimeSlice)) {
				try {
					this.setCycleTimeSlice(Integer.parseInt(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagCycleTimeSlice);
				}
				
			} else if (child.getName().equalsIgnoreCase(kTagVisionCone)) {
				try {
					this.setVisionCone(Double.parseDouble(child.getTextTrim()));
				} catch (NumberFormatException e) {
					throw new LoadError("Error parsing " + kTagVisionCone);
				}
				
			} else if (child.getName().equalsIgnoreCase(kTagBlocksBlock)) {
				this.setBlocksBlock(Boolean.parseBoolean(child.getTextTrim()));
				
			} else if (child.getName().equalsIgnoreCase(kTagContinuous)) {
				this.setContinuous(Boolean.parseBoolean(child.getTextTrim()));
				
			} else {
				throw new LoadError("Unrecognized tag: " + child.getName());
			}
		}		
	}

	public void rulesSave(Element rules) {
		Element book = new Element(getTitle().toLowerCase());
		if (this.getColoredRooms()) {
			book.addContent(new Element(kTagColoredRooms));
		}
		book.addContent(new Element(kTagSpeed).setText(Integer.toString(this.getSpeed())));
		book.addContent(new Element(kTagRotateSpeed).setText(Float.toString(this.getRotateSpeed())));
		book.addContent(new Element(kTagBookCellSize).setText(Integer.toString(this.getBookCellSize())));
		book.addContent(new Element(kTagCycleTimeSlice).setText(Integer.toString(this.getCycleTimeSlice())));
		book.addContent(new Element(kTagVisionCone).setText(Double.toString(this.getVisionCone())));
		book.addContent(new Element(kTagBlocksBlock).setText(Boolean.toString(this.getBlocksBlock())));
		book.addContent(new Element(kTagContinuous).setText(Boolean.toString(this.getContinuous())));
		
		rules.addContent(book);
	}
	
	private static final String kTagColoredRooms = "colored-rooms";
	private static final String kTagSpeed = "speed";
	private boolean coloredRooms = false;
	public boolean getColoredRooms() {
		return this.coloredRooms;
	}
	public void setColoredRooms(boolean coloredRooms) {
		this.coloredRooms = coloredRooms;
	}
	
	private int speed = 16;
	public int getSpeed() {
		return this.speed;
	}
	public void setSpeed(int speed) {
		this.speed = speed;
	}
	private static final String kTagRotateSpeed = "rotate-speed";
	private float rotateSpeed = (float)java.lang.Math.PI / 4;
	public float getRotateSpeed() {
		return this.rotateSpeed;
	}
	public void setRotateSpeed(float rotateSpeed) {
		this.rotateSpeed = rotateSpeed;
	}
	
	private static final String kTagBookCellSize = "book-cell-size";
	private int bookCellSize = 16;
	public int getBookCellSize() {
		return this.bookCellSize;
	}
	public void setBookCellSize(int bookCellSize) {
		this.bookCellSize = bookCellSize;
	}
	
	private static final String kTagCycleTimeSlice = "cycle-time-slice";
	private int cycleTimeSlice = 50;
	public int getCycleTimeSlice() {
		return this.cycleTimeSlice;
	}
	public void setCycleTimeSlice(int cycleTimeSlice) {
		this.cycleTimeSlice = cycleTimeSlice;
	}
	
	private static final String kTagVisionCone = "vision-cone";
	private double visionCone = Math.PI; // 180 degrees
	public double getVisionCone() {
		return this.visionCone;
	}
	public void setVisionCone(double visionCone) {
		this.visionCone = visionCone;
	}

	private static final String kTagBlocksBlock = "blocks-block";
	private boolean blocksBlock = true;
	public boolean getBlocksBlock() {
		return this.blocksBlock;
	}
	public void setBlocksBlock(boolean blocksBlock) {
		this.blocksBlock = blocksBlock;
	}

	private static final String kTagContinuous = "continuous";
	private boolean continuous = false;
	public boolean getContinuous() {
		return this.continuous;
	}
	public void setContinuous(boolean continuous) {
		this.continuous = continuous;
		if (continuous == false) {
			this.blocksBlock = false;
		}
	}

	public void copy(IConfiguration config) {
		BookConfiguration bConfig = (BookConfiguration)config;
		this.coloredRooms = bConfig.coloredRooms;
		this.speed = bConfig.speed;
		this.bookCellSize = bConfig.bookCellSize;
		this.cycleTimeSlice = bConfig.cycleTimeSlice;
		this.visionCone = bConfig.visionCone;
		this.blocksBlock = bConfig.blocksBlock;
		this.continuous = bConfig.continuous;
	}
	public void setDefaultTerminals(Configuration configuration) {
		configuration.setTerminalWinningScore(0);
		configuration.setTerminalFoodRemaining(false);
	}

	public boolean getRunTilOutput() {
		return false;
	}

	public BaseConfiguration getModule() {
		return this;
	}

}
