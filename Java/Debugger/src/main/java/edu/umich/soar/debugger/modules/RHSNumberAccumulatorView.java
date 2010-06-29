package edu.umich.soar.debugger.modules;

public class RHSNumberAccumulatorView extends RHSFunTextView
{
    public String getModuleBaseName()
    {
        return "rhs_number_accumulator";
    }

    @Override
    protected void updateNow()
    {
        if (clear)
        {
            clearDisplay();
            clear = false;
        }
        setTextSafely(Double.toString(totalValue));
    }

    double totalValue = 0;

    @Override
    public String rhsFunctionHandler(int eventID, Object data,
            String agentName, String functionName, String argument)
    {

        if (functionName.equals("--clear"))
        {
            clear = true;
            return debugMessages ? m_Name + ":" + functionName
                    + ": set to clear" : "";
        }

        double value = 0;
        try
        {
            value = Double.parseDouble(argument);
        }
        catch (NumberFormatException e)
        {
            return m_Name + ":" + functionName + ": Unknown argument to "
                    + rhsFunName;
        }

        totalValue += value;

        return debugMessages ? m_Name + ":" + functionName
                + ": Total value changed to: " + totalValue : "";
    }

    @Override
    public void clearDisplay()
    {
        totalValue = 0;
        super.clearDisplay();
    }

    @Override
    public void onInitSoar()
    {
        clearDisplay();
        updateNow();
    }
}
