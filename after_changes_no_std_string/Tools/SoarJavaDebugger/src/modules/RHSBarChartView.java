package modules;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.TreeSet;

import general.JavaElementXML;
import helpers.FormDataHelper;

import manager.Pane;

import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.FormData;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.MessageBox;
import org.eclipse.swt.widgets.Text;
import org.jfree.chart.ChartFactory;
import org.jfree.chart.JFreeChart;
import org.jfree.chart.axis.CategoryAxis;
import org.jfree.chart.axis.CategoryLabelPositions;
import org.jfree.chart.axis.NumberAxis;
import org.jfree.chart.plot.CategoryPlot;
import org.jfree.chart.plot.PlotOrientation;
import org.jfree.chart.renderer.category.BarRenderer;
import org.jfree.data.category.CategoryDataset;
import org.jfree.data.category.DefaultCategoryDataset;
import org.jfree.experimental.chart.swt.ChartComposite;

import debugger.MainFrame;
import dialogs.PropertiesDialog;
import doc.Document;

import sml.Agent;
import sml.Kernel;
import sml.smlAgentEventId;

public class RHSBarChartView extends AbstractRHSFunView implements Kernel.RhsFunctionInterface
{
	protected String chartTitle = new String();

	@Override
	public void init(MainFrame frame, Document doc, Pane parentPane) {
		if (chartTitle.length() <= 0) {
			chartTitle = getModuleBaseName();
		}
		super.init(frame, doc, parentPane);
	}
	
	@Override
	public String getModuleBaseName() {
		return "rhs_bar_chart_view";
	}

	private int propertiesStartingIndex;

	protected void initProperties(ArrayList<PropertiesDialog.Property> properties) {
		super.initProperties(properties);
		propertiesStartingIndex = properties.size();

		properties.add(new PropertiesDialog.StringProperty("Chart title", chartTitle));
	}
	
	protected void processProperties(ArrayList<PropertiesDialog.Property> properties) {
		super.processProperties(properties);
		chartTitle = ((PropertiesDialog.StringProperty)properties.get(propertiesStartingIndex)).getValue() ;
		chart.setTitle(chartTitle);
	}
	
	class OrderedString implements Comparable<OrderedString> {
		
		OrderedString(String string) {
			this.string = new String(string);
			this.value = new Integer(0);
		}
		
		OrderedString(String string, int value) {
			this.string = new String(string);
			this.value = new Integer(value);
		}
		
		OrderedString(OrderedString ov) {
			this.string = new String(ov.string);
			this.value = new Integer(ov.value);
		}
		
		public String string;
		public Integer value;
		
		// This is backwards because descending iterator is a java 1.6 feature
		public int compareTo(OrderedString other) {
			return this.value - other.value;
		}
		
		public boolean equals(Object obj) {
			OrderedString orderedString;
			try {
				orderedString = (OrderedString)obj;
			} catch (ClassCastException e) {
				return super.equals(obj);
			}
			return this.string.equals(orderedString.string);
		}
		
		public int hashCode() {
			return this.string.hashCode();
		}
	}
	
	boolean clear = false;

	// category -> series -> value
	HashMap<String, HashMap<String, Double> > categoryToSeriesMap = new HashMap<String, HashMap<String, Double> >();
	TreeSet<OrderedString> categoryOrderSet = new TreeSet<OrderedString>();
	TreeSet<OrderedString> seriesOrderSet = new TreeSet<OrderedString>();

	public String rhsFunctionHandler(int eventID, Object data,
			String agentName, String functionName, String argument) {

		// Syntax: 
		//                              |--- commandLine --------------------|
		//      exec <rhs_function_name> <command> [<args>] 
		//      exec <rhs_function_name> addvalue <category> <category-order> <series> <series-order> <value>
		
		String[] commandLine = argument.split("\\s+");
		
		if (commandLine.length < 1) {
			return m_Name + ":" + functionName + ": no command";
		}
		
		if (commandLine[0].equals("--clear")) {
			clear = true;
			return debugMessages ? m_Name + ":" + functionName + ": cleared" : "";
			
		} else if (commandLine[0].equals("addvalue")) {
			if (commandLine.length != 6) {
				return m_Name + ":" + functionName + ": addvalue requires <category> <category-order> <series> <series-order> <value>";
			}
			
			int catOrder = 0;
			try {
				catOrder = Integer.parseInt(commandLine[2]);
			} catch (NumberFormatException e) {
				return m_Name + ":" + functionName + ": addvalue: parsing failed for <category-order> argument";
			}
			
			int serOrder = 0;
			try {
				serOrder = Integer.parseInt(commandLine[4]);
			} catch (NumberFormatException e) {
				return m_Name + ":" + functionName + ": addvalue: parsing failed for <series-order> argument";
			}
			
			double value = 0;
			try {
				value = Double.parseDouble(commandLine[5]);
			} catch (NumberFormatException e) {
				return m_Name + ":" + functionName + ": addvalue: parsing failed for <value> argument";
			}
			
			updateData(commandLine[1], catOrder, commandLine[3], serOrder, value);
			return debugMessages ? m_Name + ":" + functionName + ": Graph updated." : "";
		}		
		
		return m_Name + ":" + functionName + ": unknown command: " + commandLine[0];
	}
	
	private void updateData(String category, int categoryOrder, String series, int seriesOrder, double value) {
		if (!categoryToSeriesMap.containsKey(category)) {
			HashMap<String, Double> seriesToValueMap = new HashMap<String, Double>();
			categoryToSeriesMap.put(category, seriesToValueMap);
			categoryOrderSet.add(new OrderedString(category, categoryOrder));
		}
		
		HashMap<String, Double> seriesToValueMap = categoryToSeriesMap.get(category);
		seriesToValueMap.put(series, value);
		seriesOrderSet.add(new OrderedString(series, seriesOrder));
	}
	
	@Override
	public boolean find(String text, boolean searchDown, boolean matchCase,
			boolean wrap, boolean searchHiddenText) {
		return false;
	}

	@Override
	public void copy() {
	}

	@Override
	public void displayText(String text) {
	}

	ChartComposite frame;
	JFreeChart chart;
	DefaultCategoryDataset dataset;
	Composite rhsBarChartContainer;
	Label rightClickLabel;
	
	@Override
	protected void createDisplayControl(Composite parent) {
		rhsBarChartContainer = new Composite(parent, SWT.NULL);
		FormData attachFull = FormDataHelper.anchorFull(0) ;
		rhsBarChartContainer.setLayoutData(attachFull);
		{
			GridLayout gl = new GridLayout();
			gl.numColumns = 1;
			gl.verticalSpacing = 0;
			gl.marginHeight = 0;
			gl.marginWidth = 0;
			rhsBarChartContainer.setLayout(gl);
		}
		
		dataset = new DefaultCategoryDataset();

		// create the chart...
        chart = ChartFactory.createBarChart(
        	chartTitle,         	  // chart title
            "Category",               // domain axis label
            "Value",                  // range axis label
            dataset,                  // data
            PlotOrientation.HORIZONTAL, // orientation
            true,                     // include legend
            true,                     // tooltips?
            false                     // URLs?
        );
		
		frame = new ChartComposite(rhsBarChartContainer, SWT.NONE, chart, true);
		{
			GridData gd = new GridData(SWT.FILL, SWT.FILL, true, true);
			frame.setLayoutData(gd);
		}
		
		frame.pack();
		
		rightClickLabel = new Label(rhsBarChartContainer, SWT.NONE);
		rightClickLabel.setText("Right click here to access chart properties and remove window...");
		{
			GridData gd = new GridData(SWT.FILL, SWT.NONE, true, false);
			rightClickLabel.setLayoutData(gd);
		}

		createContextMenu(rightClickLabel) ;
	}
	
	@Override
	public org.eclipse.swt.graphics.Color getBackgroundColor() {
		return getMainFrame().getDisplay().getSystemColor(SWT.COLOR_CYAN) ;
	}

	@Override
	protected Control getDisplayControl() {
		return rhsBarChartContainer;
	}

	@Override
	protected void restoreContent(JavaElementXML element) {
	}

	@Override
	protected void storeContent(JavaElementXML element) {
	}
	
	private void updateChart() {
		// TODO: explore processing only changes instead of the entire data each update
		
		// for each category
		Iterator<OrderedString> catIter = this.categoryOrderSet.iterator();
		while (catIter.hasNext()) {
			String category = catIter.next().string;

			// for each series
			HashMap<String, Double> seriesToValueMap = categoryToSeriesMap.get(category);
			Iterator<OrderedString> serIter =  this.seriesOrderSet.iterator();
			while (serIter.hasNext()) {
				String series = serIter.next().string;
				
				// add/set value (add seems to work)
				dataset.addValue(seriesToValueMap.get(series), series, category);
			}
		}
	}
	
	@Override
	protected void updateNow() {
		if (clear) {
			clearDisplay();
			clear = false;
		}
		
		// If Soar is running in the UI thread we can make
		// the update directly.
		if (!Document.kDocInOwnThread)
		{
			updateChart();
			return ;
		}
		
		// Have to make update in the UI thread.
		// Callback comes in the document thread.
        Display.getDefault().asyncExec(new Runnable() {
            public void run() {
            	updateChart();
            }
         }) ;

	}

	@Override
	public void clearDisplay() {
		categoryToSeriesMap.clear();
		categoryOrderSet.clear();
		seriesOrderSet.clear();
		// If Soar is running in the UI thread we can make
		// the update directly.
		if (!Document.kDocInOwnThread)
		{
			dataset.clear();
			return ;
		}
		
		// Have to make update in the UI thread.
		// Callback comes in the document thread.
        Display.getDefault().asyncExec(new Runnable() {
            public void run() {
        		dataset.clear();
            }
         }) ;
	}

	public void onInitSoar() {
		clearDisplay();
	}

	/************************************************************************
	* 
	* Converts this object into an XML representation.
	* 
	*************************************************************************/
	@Override
	public JavaElementXML convertToXML(String title, boolean storeContent) {
		JavaElementXML element = new JavaElementXML(title) ;
		
		// It's useful to record the class name to uniquely identify the type
		// of object being stored at this point in the XML tree.
		Class cl = this.getClass() ;
		element.addAttribute(JavaElementXML.kClassAttribute, cl.getName()) ;

		if (m_Name == null)
			throw new IllegalStateException("We've created a view with no name -- very bad") ;
		
		// Store this object's properties.
		element.addAttribute("Name", m_Name) ;
		element.addAttribute("UpdateOnStop", Boolean.toString(m_UpdateOnStop)) ;
		element.addAttribute("UpdateEveryNthDecision", Integer.toString(m_UpdateEveryNthDecision)) ;
		element.addAttribute("RHSFunctionName", rhsFunName) ;
		element.addAttribute("ChartTitle", chartTitle) ;
		element.addAttribute("DebugMessages", Boolean.toString(debugMessages)) ;
				
		if (storeContent)
			storeContent(element) ;

		element.addChildElement(this.m_Logger.convertToXML("Logger")) ;
		
		return element ;
	}

	/************************************************************************
	* 
	* Rebuild the object from an XML representation.
	* 
	* @param frame			The top level window that owns this window
	* @param doc			The document we're rebuilding
	* @param parent			The pane window that owns this view
	* @param element		The XML representation of this command
	* 
	*************************************************************************/
	@Override
	public void loadFromXML(MainFrame frame, Document doc, Pane parent,
			JavaElementXML element) throws Exception {
		setValues(frame, doc, parent) ;

		m_Name			   	= element.getAttribute("Name") ;
		m_UpdateOnStop	   	= element.getAttributeBooleanDefault("UpdateOnStop", true) ;
		m_UpdateEveryNthDecision = element.getAttributeIntDefault("UpdateEveryNthDecision", 0) ;
		rhsFunName = element.getAttribute("RHSFunctionName");
		chartTitle 			= element.getAttribute("ChartTitle");
		debugMessages		= element.getAttributeBooleanDefault("DebugMessages", true);

		if (rhsFunName == null) {
			rhsFunName = new String();
		}
		
		if (chartTitle == null) {
			chartTitle = new String();
		}
		
		JavaElementXML log = element.findChildByName("Logger") ;
		if (log != null)
			this.m_Logger.loadFromXML(doc, log) ;

		// Register that this module's name is in use
		frame.registerViewName(m_Name, this) ;
		
		// Actually create the window
		init(frame, doc, parent) ;

		// Restore the text we saved (if we chose to save it)
		restoreContent(element) ;
	}


}

/*

Test code:

sp {propose*init
(state <s> ^superstate nil
-^name)
-->
(<s> ^operator <o> +)
(<o> ^name init)
}

sp {apply*init
(state <s> ^operator.name init)
-->
(<s> ^name test)
}

sp {propose*update
(state <s> ^name test
-^toggle on)
-->
(<s> ^operator <o> +)
(<o> ^name update)
}

sp {apply*update
(state <s> ^operator.name update)
-->
(<s> ^toggle on)
(write (crlf) (exec graph |addvalue category1 1 series1 1 0.5|))
(write (crlf) (exec graph |addvalue category2 2 series1 1 0.7|))
(write (crlf) (exec graph |addvalue category3 3 series1 1 0.1|))
(write (crlf) (exec graph |addvalue category1 1 series2 2 0.2|))
(write (crlf) (exec graph |addvalue category2 2 series2 2 0.4|))
(write (crlf) (exec graph |addvalue category3 3 series2 2 0.8|))
}

sp {propose*update2
(state <s> ^name test
^toggle on)
-->
(<s> ^operator <o> +)
(<o> ^name update2)
}

sp {apply*update2
(state <s> ^operator.name update2)
-->
(<s> ^toggle on -)
(write (crlf) (exec graph |addvalue category1 1 series1 1 0.1|))
(write (crlf) (exec graph |addvalue category2 2 series1 1 0.2|))
(write (crlf) (exec graph |addvalue category3 3 series1 1 0.3|))
(write (crlf) (exec graph |addvalue category1 1 series2 2 0.6|))
(write (crlf) (exec graph |addvalue category2 2 series2 2 0.2|))
(write (crlf) (exec graph |addvalue category3 3 series2 2 0.5|))
}
*/
