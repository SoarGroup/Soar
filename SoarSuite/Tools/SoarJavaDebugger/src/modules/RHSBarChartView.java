package modules;

import java.util.HashSet;

import general.JavaElementXML;
import helpers.FormDataHelper;

import manager.Pane;

import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.FormData;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
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

	public String rhsFunctionHandler(int eventID, Object data,
			String agentName, String functionName, String argument) {

		if (!functionName.equals(rhsFunName)) {
			return "Unknown rhs function " + rhsFunName + " received in window " + getName() + ".";
		}
		
		return "rhsFunctionHandler not implemented.";
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

    /**
     * Returns a sample dataset.
     * 
     * @return The dataset.
     */
    private static CategoryDataset createDataset() {
        
        // row keys...
        String series1 = "First";
        String series2 = "Second";
        String series3 = "Third";

        // column keys...
        String category1 = "Category 1";
        String category2 = "Category 2";
        String category3 = "Category 3";
        String category4 = "Category 4";
        String category5 = "Category 5";

        // create the dataset...
        DefaultCategoryDataset dataset = new DefaultCategoryDataset();

        dataset.addValue(1.0, series1, category1);
        dataset.addValue(4.0, series1, category2);
        dataset.addValue(3.0, series1, category3);
        dataset.addValue(5.0, series1, category4);
        dataset.addValue(5.0, series1, category5);

        dataset.addValue(5.0, series2, category1);
        dataset.addValue(7.0, series2, category2);
        dataset.addValue(6.0, series2, category3);
        dataset.addValue(8.0, series2, category4);
        dataset.addValue(4.0, series2, category5);

        dataset.addValue(4.0, series3, category1);
        dataset.addValue(3.0, series3, category2);
        dataset.addValue(2.0, series3, category3);
        dataset.addValue(3.0, series3, category4);
        dataset.addValue(6.0, series3, category5);
        
        return dataset;
        
    }
    
    /**
     * Creates a sample chart.
     * 
     * @param dataset  the dataset.
     * 
     * @return The chart.
     */
    private static JFreeChart createChart(CategoryDataset dataset) {
        
        // create the chart...
        JFreeChart chart = ChartFactory.createBarChart(
            "Bar Chart Demo",         // chart title
            "Category",               // domain axis label
            "Value",                  // range axis label
            dataset,                  // data
            PlotOrientation.VERTICAL, // orientation
            true,                     // include legend
            true,                     // tooltips?
            false                     // URLs?
        );

        // NOW DO SOME OPTIONAL CUSTOMISATION OF THE CHART...

        // set the background color for the chart...
        chart.setBackgroundPaint(java.awt.Color.white);

        // get a reference to the plot for further customisation...
        CategoryPlot plot = (CategoryPlot) chart.getPlot();
        plot.setBackgroundPaint(java.awt.Color.lightGray);
        plot.setDomainGridlinePaint(java.awt.Color.white);
        plot.setDomainGridlinesVisible(true);
        plot.setRangeGridlinePaint(java.awt.Color.white);

        // set the range axis to display integers only...
        final NumberAxis rangeAxis = (NumberAxis) plot.getRangeAxis();
        rangeAxis.setStandardTickUnits(NumberAxis.createIntegerTickUnits());

        // disable bar outlines...
        BarRenderer renderer = (BarRenderer) plot.getRenderer();
        renderer.setDrawBarOutline(false);

        CategoryAxis domainAxis = plot.getDomainAxis();
        domainAxis.setCategoryLabelPositions(
            CategoryLabelPositions.createUpRotationLabelPositions(Math.PI / 6.0)
        );
        // OPTIONAL CUSTOMISATION COMPLETED.
        
        return chart;
        
    }
    
	@Override
	protected void createDisplayControl(Composite parent) {
		
        JFreeChart chart = createChart(createDataset());
		frame = new ChartComposite(parent, SWT.NONE, chart, true);
		
		FormData attachFull = FormDataHelper.anchorFull(0) ;
		frame.setLayoutData(attachFull);
		
		frame.pack();

		createContextMenu(frame) ;
	}
	
	@Override
	public org.eclipse.swt.graphics.Color getBackgroundColor() {
		return getMainFrame().getDisplay().getSystemColor(SWT.COLOR_CYAN) ;
	}

	@Override
	protected Control getDisplayControl() {
		return frame;
	}

	@Override
	protected void restoreContent(JavaElementXML element) {
		// TODO Auto-generated method stub

	}

	@Override
	protected void storeContent(JavaElementXML element) {
		// TODO Auto-generated method stub

	}

	@Override
	protected void updateNow() {

	}

	@Override
	public void clearDisplay() {

	}


}
