package edu.umich.soar.room.map;

public class Commands {
	static void memberAppend(StringBuilder sb, boolean active, String name) {
		if (active) {
			sb.append("(");
			sb.append(name);
			sb.append(")");
		}
	}
	static void memberAppend(StringBuilder sb, boolean active, String name, String value) {
		if (active) {
			sb.append("(");
			sb.append(name);
			sb.append(":");
			sb.append(value);
			sb.append(")");
		}
	}
}
