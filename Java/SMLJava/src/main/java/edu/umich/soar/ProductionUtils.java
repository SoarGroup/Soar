package edu.umich.soar;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

import sml.Agent;

public class ProductionUtils
{
    public static String sourceAgentFromJar(Agent agent, String path)
            throws IOException
    {
        InputStream i = ProductionUtils.class.getResourceAsStream(path);
        if (i == null)
        {
            throw new IllegalStateException("Couldn't find resource");
        }

        BufferedReader r = new BufferedReader(new InputStreamReader(i));
        StringBuilder sb = new StringBuilder();

        while (r.ready())
        {
            String tmp = r.readLine();
            sb.append(tmp);
            sb.append('\n');
        }

        return agent.ExecuteCommandLine(sb.toString());
    }
}
