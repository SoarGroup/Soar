package sps.sm;

import java.io.PrintWriter;

public class Connection {
	PrintWriter getOutputStream() {
		return new PrintWriter(System.out);
	}
}
