package soaruorc;

import orc.*;
import orc.util.*;
import sml.*;

public class SoaruOrc
{
    public static void main(String args[])
    {
	GamePad gp = new GamePad();
	Orc orc = Orc.makeOrc();

	Motor leftMotor = new Motor(orc, 1, true);
	Motor rightMotor = new Motor(orc, 0, false);

	System.out.printf("%15s %15s %15s %15s\n", "left", "right", "left current", "right current");
	while (true) {

	    double left = 0, right = 0;

	    if (true) {
		// independently control left & right wheels mode. "Tank mode"
		left = gp.getAxis(1) * -1;
		right = gp.getAxis(3) * -1;
		
	    } else {
		double fwd = -gp.getAxis(3); // +1 = forward, -1 = back
		double lr  = -gp.getAxis(2);   // +1 = left, -1 = right

		left = fwd - lr;
		right = fwd + lr;

		double max = Math.max(Math.abs(left), Math.abs(right));
		if (max > 1) {
		    left /= max;
		    right /= max;
		}
	    }
	    System.out.printf("%15f %15f %15f %15f\r", left, right, leftMotor.getCurrent(), rightMotor.getCurrent());
	    leftMotor.setPWM(left);
	    rightMotor.setPWM(right);

	    try {
		Thread.sleep(30);
	    } catch (InterruptedException ex) {
	    }
	}
    }
}
