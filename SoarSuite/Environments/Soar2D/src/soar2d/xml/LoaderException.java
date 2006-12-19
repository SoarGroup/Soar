package soar2d.xml;

import java.util.Stack;

public class LoaderException extends Throwable {
	static final long serialVersionUID = 1;
	protected String message;
	
	public LoaderException(Stack<String> path, String message) {
		if (path != null) {
			this.message = "Path: ";
			if (path.size() == 0) {
				this.message += "empty\n";
			} else {
				String pathString = "";
				while (path.size() > 0) {
					pathString = "<" + path.pop() + ">" + pathString;
				}
				this.message += pathString + "\n";
			}
		}
		this.message += message;
	}
	
	public String getMessage() {
		return message;
	}
}

