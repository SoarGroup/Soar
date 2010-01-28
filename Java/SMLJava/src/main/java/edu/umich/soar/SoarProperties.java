package edu.umich.soar;

import java.io.IOException;
import java.util.Properties;

public class SoarProperties {
    private static Properties properties;
    public static Properties getProperties() {
	if (properties != null) {
	    return properties;
	}
	
	properties = new Properties();
	try {
	    properties.load(ClassLoader.getSystemResourceAsStream("soar.properties"));
	} catch (IOException e) {
	    // TODO Auto-generated catch block
	    e.printStackTrace();
	    return null;
	}

	return properties;
    }
    
    /**
     * @param args
     */
    public static void main(String[] args) {
	
	System.out.println("soar.home: " + getProperties().getProperty("soar.home"));
    }

}
