package soar2d.xml;

import java.util.Stack;

public class SyntaxException extends LoaderException {
	static final long serialVersionUID = 2;
	public SyntaxException(Stack<String> path, String message) {
		super(path, message);
	}
}
