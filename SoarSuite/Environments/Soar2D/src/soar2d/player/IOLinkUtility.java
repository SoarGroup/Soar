package soar2d.player;
import sml.Agent;
import sml.FloatElement;
import sml.Identifier;
import sml.IntElement;
import sml.StringElement;
import sml.WMElement;

public class IOLinkUtility {
	static boolean CreateOrAddStatus(Agent agent, Identifier command, String status) {
		assert agent != null;
		assert command != null;
		assert status != null;
		assert status.length() > 0;
		
		for (int i = 0; i < command.GetNumberChildren(); ++i) {
			WMElement childWME = command.GetChild(i);
			assert childWME != null;
			assert childWME.GetAttribute() != null;
			
			if (childWME.GetAttribute().equals("status")) {
				StringElement statusWME = childWME.ConvertToStringElement();
				assert statusWME != null;
				agent.Update(statusWME, status);
			}
		}
		
		agent.CreateStringWME(command, "status", status);
		return true;
	}
}
