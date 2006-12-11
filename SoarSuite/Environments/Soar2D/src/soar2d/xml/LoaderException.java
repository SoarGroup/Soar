package soar2d.xml;

import java.util.Stack;

class LoaderException extends Throwable {
	static final long serialVersionUID = 1;
	protected String message;
	
	public LoaderException(Stack<String> path, String message) {
		if (path != null) {
			this.message = "Path: ";
			if (path.size() == 0) {
				this.message += "empty\n";
			} else {
				while (path.size() > 0) {
					this.message += "<" + path.pop() + ">";
				}
				this.message += "\n";
			}
		}
		this.message += message;
	}
	
	public String getMessage() {
		return message;
	}
}

