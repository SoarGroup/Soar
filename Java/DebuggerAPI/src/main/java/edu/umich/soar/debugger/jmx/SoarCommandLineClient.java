package edu.umich.soar.debugger.jmx;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ServerSocket;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import javax.management.InstanceNotFoundException;
import javax.management.MBeanServerConnection;
import javax.management.MBeanServerInvocationHandler;
import javax.management.MalformedObjectNameException;
import javax.management.ObjectName;
import javax.management.remote.JMXConnector;
import javax.management.remote.JMXConnectorFactory;
import javax.management.remote.JMXServiceURL;

public class SoarCommandLineClient
{
    private File soarHome;
    private JMXConnector connector;
    private Process process;
    private boolean managed = false;
    
    public SoarCommandLineClient(String soarHome)
    {
        setSoarHome(soarHome);
    }
    
    public void setSoarHome(String soarHome)
    {
        if (soarHome == null)
            throw new IllegalArgumentException("Must provide path to Soar home.");
        this.soarHome = new File(soarHome);
    }
    
    /**
     * Retrieve an unused port to listen on.
     * 
     * @return An unused port
     * @throws IOException If there is a networking error.
     */
    private int getUnusedPort() throws IOException
    {
        ServerSocket s = null;
        try
        {
            s = new ServerSocket(0);
            
            return s.getLocalPort();
        }
        finally
        {
            if(s != null)
            {
                try
                {
                    s.close();
                }
                catch (IOException e)
                {
                    e.printStackTrace();
                }
            }
        }
    }
    
    public SoarCommandLineMXBean openDebuggerProxy(String agent, int port) throws IOException
    {
        return openDebuggerProxy(agent, "localhost", port);
    }
    
    /**
     * @return True if the process is still running.
     */
    private boolean isProcessRunning()
    {
        if(process == null)
        {
            return false;
        }
        try
        {
            process.exitValue();
            return false;
        }
        catch(IllegalThreadStateException e)
        {
            // exitValue() throws this exception if the process has not 
            // terminated
            return true;
        }
    }
    
    private void openJmxConnection(String host, int port) throws IOException
    {
        JMXServiceURL url = new JMXServiceURL("service:jmx:rmi:///jndi/rmi://" 
                + host + ":" + port + "/jmxrmi");
        
        int tryCount = 0;
        while(managed && isProcessRunning())
        {
            try 
            {
                connector = JMXConnectorFactory.connect(url);
                return;
            }
            catch (IOException e)
            {
                if(tryCount >= 20)
                {
                    throw e;
                }
            }
            try
            {
                Thread.sleep(500);
            }
            catch (InterruptedException e)
            {
            }
            tryCount++;
        }
        throw new IOException("Failed to connect to debugger process. Process exited.");
    }

    public SoarCommandLineMXBean openDebuggerProxy(String agent, String host, int port) throws IOException
    {
        openJmxConnection(host, port);
       
        int tryCount = 0;
        while (tryCount < 20 && (!managed || isProcessRunning()))
        {
            try 
            {
                ObjectName objectName = new ObjectName("SoarCommandLine:name=" + agent);
                MBeanServerConnection conn = connector.getMBeanServerConnection();
                
                conn.getObjectInstance(objectName);
                return (SoarCommandLineMXBean) MBeanServerInvocationHandler.newProxyInstance(conn, 
                        objectName, SoarCommandLineMXBean.class, false);
            } 
            catch (MalformedObjectNameException e)
            {
                e.printStackTrace();
                return null;
            }
            catch (InstanceNotFoundException e)
            {
            }            
            
            try
            {
                Thread.sleep(500);
            }
            catch (InterruptedException e)
            {
            }
        }
        throw new IOException("Failed to retrieve debugger proxy.");
    }
    
    private class ProcessStreamConsumer
    {
        ProcessStreamConsumer(final InputStream in, final OutputStream out)
        {
            ExecutorService exec = Executors.newSingleThreadExecutor();
            exec.execute(new Runnable()
            {
                @Override
                public void run()
                {
                    try 
                    {
                        while (true)
                        {
                            int b = in.read();
                            if (b == -1)
                                break;
                            out.write(b);
                        }
                    } 
                    catch (IOException e) 
                    {
                    }
                }
            });
        }
        
    }
    
    private boolean isMacOSX() {
        String osName = System.getProperty("os.name");
        return osName.startsWith("Mac OS X");
    }
    
    public boolean spawnDebuggerWithJMX(int port) throws IOException
    {
        File debugger = new File(soarHome.getAbsolutePath() 
                + File.separator + "bin" + File.separator + "SoarJavaDebugger.jar");
        List<String> cmd = new ArrayList<String>();
        // assume java is on the system path
        cmd.add("java");

        // Set the JMX server port and other settings
        cmd.add("-Dcom.sun.management.jmxremote.port=" + port);
        cmd.add("-Dcom.sun.management.jmxremote.authenticate=false");
        cmd.add("-Dcom.sun.management.jmxremote.ssl=false");
        
        if (isMacOSX())
            cmd.add("-XstartOnFirstThread");

        cmd.add("-jar");
        cmd.add(debugger.getAbsolutePath());

        // extend environment
        Map<String, String> envMap = new HashMap<String, String>(System.getenv());
        envMap.put("SOAR_HOME", soarHome.getAbsolutePath());
        
        String temp = soarHome.getAbsolutePath() + File.separator + "bin";
        if (envMap.containsKey("PATH"))
            temp += File.pathSeparator + envMap.get("PATH");
        envMap.put("PATH", temp);
        
        temp = soarHome.getAbsolutePath() + File.separator + "lib";
        if (envMap.containsKey("LD_LIBRARY_PATH"))
            temp += File.pathSeparator + envMap.get("LD_LIBRARY_PATH");
        envMap.put("LD_LIBRARY_PATH", temp);
        
        temp = soarHome.getAbsolutePath() + File.separator + "lib";
        if (envMap.containsKey("DYLD_LIBRARY_PATH"))
            temp += File.pathSeparator + envMap.get("DYLD_LIBRARY_PATH");
        envMap.put("DYLD_LIBRARY_PATH", temp);

        List<String> env = new ArrayList<String>();
        for (Map.Entry<String, String> entry : envMap.entrySet())
        {
            env.add(entry.getKey() + "=" + entry.getValue());
        }
        
        // Run the process
        this.process = Runtime.getRuntime().exec(cmd.toArray(new String[cmd.size()]), env.toArray(new String[env.size()]), soarHome);
        new ProcessStreamConsumer(this.process.getInputStream(), System.out);
        new ProcessStreamConsumer(this.process.getErrorStream(), System.err);
        return process != null;
    }
    
    public SoarCommandLineMXBean startDebuggerGetProxy() throws IOException
    {
        int port = getUnusedPort();
        if (!spawnDebuggerWithJMX(port))
            return null;
        managed = true;
        return openDebuggerProxy("soar1", port);
    }

    public static void main(String[] args) throws IOException
    {
        SoarCommandLineClient scli = new SoarCommandLineClient("/Users/voigtjr/sandbox");
        SoarCommandLineMXBean proxy = scli.startDebuggerGetProxy();
        if (proxy != null)
            proxy.executeCommandLine("p s1");
    }

}
