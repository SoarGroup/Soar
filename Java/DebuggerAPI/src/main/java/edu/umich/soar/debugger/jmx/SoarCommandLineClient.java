package edu.umich.soar.debugger.jmx;

import java.io.IOException;

import javax.management.MBeanServerConnection;
import javax.management.MBeanServerInvocationHandler;
import javax.management.MalformedObjectNameException;
import javax.management.ObjectName;
import javax.management.remote.JMXConnector;
import javax.management.remote.JMXConnectorFactory;
import javax.management.remote.JMXServiceURL;

public class SoarCommandLineClient
{
    private String jmxHost = "localhost";
    private int jmxPort = 15155;
    private JMXConnector connector;

    public SoarCommandLineClient() throws IOException, MalformedObjectNameException, NullPointerException
    {
        JMXServiceURL url = new JMXServiceURL("service:jmx:rmi:///jndi/rmi://" + jmxHost + ":" + jmxPort + "/jmxrmi");

        System.out.println("Attempting connection to " + url);
        connector = JMXConnectorFactory.connect(url);
        ObjectName objectName = new ObjectName("SoarCommandLine:name=soar1");
        MBeanServerConnection conn = connector.getMBeanServerConnection();
//        Set<ObjectName> names = new TreeSet<ObjectName>(conn.queryNames(null, null));
//        for (ObjectName name : names) {
//            System.out.println("\tObjectName = " + name);
//        }
        SoarCommandLineMXBean scli = (SoarCommandLineMXBean) MBeanServerInvocationHandler.newProxyInstance(conn, 
                objectName, SoarCommandLineMXBean.class, false);
        String result = scli.executeCommandLine("p s1");
        System.out.println(result);
    }
    
    public static void main(String[] args) throws IOException, MalformedObjectNameException, NullPointerException
    {
        new SoarCommandLineClient();
    }

}
