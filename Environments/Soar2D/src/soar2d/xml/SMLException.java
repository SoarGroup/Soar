package soar2d.xml;

import java.util.Stack;

import sml.ElementXML;

class SMLException extends LoaderException {
	static final long serialVersionUID = 3;
	public SMLException(Stack<String> path, String message) {
		super(path, message);
		this.message += "ElementXML: " + ElementXML.GetLastParseErrorDescription() + "\n";
	}
}

