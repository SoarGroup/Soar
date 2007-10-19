package modules;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.TreeSet;

import general.JavaElementXML;
import helpers.FormDataHelper;

import manager.Pane;
import modules.RHSOperatorTextView.OperatorValue;

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

public class RHSBarChartView extends AbstractUpdateView  implements Kernel.AgentEventInterface, Kernel.RhsFunctionInterface {

	public RHSBarChartView()
	{
	}
	
	@Override
	public String getModuleBaseName() {
		return "rhs_bar_chart_view";
	}

	// Assume this may be empty! (no function is registered)
	protected String rhsFunName = new String();
	
	int rhsCallback = -1;

	@Override
	public void showProperties() {
		PropertiesDialog.Property properties[] = new PropertiesDialog.Property[2] ;
		
		properties[0] = new PropertiesDialog.IntProperty("Update automatically every n'th decision (0 => none)", m_UpdateEveryNthDecision) ;
		properties[1] = new PropertiesDialog.StringProperty("Name of RHS function to use to update this window", rhsFunName) ;
		
		boolean ok = PropertiesDialog.showDialog(m_Frame, "Properties", properties) ;
		
		if (ok) {
			m_UpdateEveryNthDecision = ((PropertiesDialog.IntProperty)properties[0]).getValue() ;

			// TODO: abstractify some of this code, it is repeated in many of the new RHS widgets
			if (this.getAgentFocus() != null)
			{
				// Make sure we're getting the events to match the new settings
				this.unregisterForAgentEvents(this.getAgentFocus()) ;
				this.registerForAgentEvents(this.getAgentFocus()) ;
			}
			
			String tempRHSFunName = ((PropertiesDialog.StringProperty)properties[1]).getValue() ;
			tempRHSFunName = tempRHSFunName.trim();
			
			// Make sure new one is different than old one and not zero length string
			if (tempRHSFunName.length() <= 0 || tempRHSFunName.equals(rhsFunName)) {
				return;
			}
			
			// BUGBUG: There can potentially other RHS function widgets with clashing RHS functions.
			// This situation is managed in RHSFunTextView but the scope is limited to that tree of widgets.
			// There needs to be some kind of RHS function management for all widgets using them.
			
			Agent agent = m_Frame.getAgentFocus() ;
			if (agent == null) {
				return;
			}

			// Try and register this, message and return if failure
			Kernel kernel = agent.GetKernel();
			int tempRHSCallback = kernel.AddRhsFunction(tempRHSFunName, this, null);
			
			// TODO: Verify that error check here is correct, and fix registerForAgentEvents
			// BUGBUG: remove true
			if (tempRHSCallback <= 0) {
				// failed to register callback
				MessageBox errorDialog = new MessageBox(this.m_Frame.getShell(), SWT.ICON_ERROR | SWT.OK);
				errorDialog.setMessage("Failed to change RHS function name \"" + tempRHSFunName + "\".");
				errorDialog.open();
				return;
			}
			
			// unregister old rhs fun
			boolean registerOK = true ;

			if (rhsCallback != -1)
				registerOK = kernel.RemoveRhsFunction(rhsCallback);
			
			rhsCallback = -1;

			// save new one
			rhsFunName = tempRHSFunName;
			rhsCallback = tempRHSCallback;
			
			if (!registerOK)
				throw new IllegalStateException("Problem unregistering for events") ;
		
		} // Careful, returns in the previous block!
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

	// category -> series -> value
	HashMap<String, HashMap<String, Double> > categoryToSeriesMap = new HashMap<String, HashMap<String, Double> >();
	TreeSet<OrderedString> categoryOrderSet = new TreeSet<OrderedString>();
	TreeSet<OrderedString> seriesOrderSet = new TreeSet<OrderedString>();

	public String rhsFunctionHandler(int eventID, Object data,
			String agentName, String functionName, String argument) {

		if (!functionName.equals(rhsFunName)) {
			return "Unknown rhs function " + rhsFunName + " received in window " + getName() + ".";
		}
		
		// Syntax: 
		//                              |--- commandLine --------------------|
		//      exec <rhs_function_name> <command> [<args>] 
		//      exec <rhs_function_name> addvalue <category> <category-order> <series> <series-order> <value>
		
		String[] commandLine = argument.split("\\s+");
		
		if (commandLine.length < 1) {
			return "RHSBarChartView: no command";
		}
		
		if (commandLine[0].equals("addvalue")) {
			if (commandLine.length != 6) {
				return "RHSBarChartView: addvalue requires <category> <category-order> <series> <series-order> <value>";
			}
			
			int catOrder = 0;
			try {
				catOrder = Integer.parseInt(commandLine[2]);
			} catch (NumberFormatException e) {
				return "RHSBarChartView: addvalue: parsing failed for <category-order> argument";
			}
			
			int serOrder = 0;
			try {
				serOrder = Integer.parseInt(commandLine[4]);
			} catch (NumberFormatException e) {
				return "RHSBarChartView: addvalue: parsing failed for <series-order> argument";
			}
			
			double value = 0;
			try {
				value = Double.parseDouble(commandLine[5]);
			} catch (NumberFormatException e) {
				return "RHSBarChartView: addvalue: parsing failed for <value> argument";
			}
			
			updateData(commandLine[1], catOrder, commandLine[3], serOrder, value);
			return "Graph updated.";
		}		
		
		return "RHSBarChartView: unknown command: " + commandLine[0];
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
	protected void registerForAgentEvents(Agent agent)
	{
		super.registerForAgentEvents(agent);
		
		if (rhsFunName.length() <= 0) {
			return;
		}
		
		if (agent == null)
			return ;

		Kernel kernel = agent.GetKernel();
		rhsCallback = kernel.AddRhsFunction(rhsFunName, this, null);

		if (rhsCallback <= 0) {
			// failed to register callback
			rhsCallback = -1;
			rhsFunName = "";
			throw new IllegalStateException("Problem registering for events") ;
		}
	}

	@Override
	protected void unregisterForAgentEvents(Agent agent)
	{
		super.unregisterForAgentEvents(agent);
	
		if (agent == null)
			return ;
		
		boolean ok = true ;

		Kernel kernel = agent.GetKernel();

		if (rhsCallback != -1)
			ok = kernel.RemoveRhsFunction(rhsCallback);
		
		rhsFunName = "";
		rhsCallback = -1;

		if (!ok)
			throw new IllegalStateException("Problem unregistering for events") ;
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

	int rhsFunInitSoarHandler = -1;

	@Override
	protected void registerForViewAgentEvents(Agent agent) {
		rhsFunInitSoarHandler  = agent.GetKernel().RegisterForAgentEvent(smlAgentEventId.smlEVENT_AFTER_AGENT_REINITIALIZED, this, this) ;
	}
	
	@Override
	protected boolean unregisterForViewAgentEvents(Agent agent) {
		if (agent == null)
			return true;

		boolean ok = true;
		
		if (rhsFunInitSoarHandler != -1)
			ok = agent.GetKernel().UnregisterForAgentEvent(rhsFunInitSoarHandler);
		
		rhsFunInitSoarHandler = -1;
		
		return ok;
	}
	
	@Override
	protected void clearViewAgentEvents() {
		rhsFunInitSoarHandler = -1;
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
            "RHS Bar Chart",         // chart title
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
		rightClickLabel.setText("Right click here to access properties and remove window...");
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
		// TODO Auto-generated method stub

	}

	@Override
	protected void storeContent(JavaElementXML element) {
		// TODO Auto-generated method stub

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

	@Override
	public void agentEventHandler(int eventID, Object data, String agentName)
	{
		// Note: We need to check the agent names match because although this is called an agentEventHandler it's
		// an event registered with the kernel -- so it's sent to all listeners, not just the agent that is reinitializing.
		if (eventID == smlAgentEventId.smlEVENT_AFTER_AGENT_REINITIALIZED.swigValue()) {
			onInitSoar();
			updateNow() ;
		}
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
