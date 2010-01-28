package edu.umich.soar.sps.control.robot;

import java.util.ArrayList;
import java.util.List;

public class SendMessages {
	public static List<String> toTokens(String message) {
		List<String> result = new ArrayList<String>();
		for (String token : message.split(" ")) {
			if (!token.isEmpty())
				result.add(token);
		}
		return result;
	}

}
