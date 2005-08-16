package edu.rosehulman.soar.navigator;


import edu.rosehulman.soar.*;
import edu.rosehulman.soar.SoarImages;

import org.eclipse.core.resources.*;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.jface.viewers.*;
import org.eclipse.swt.widgets.Display;
import org.eclipse.ui.*;

import java.util.*;

public class SoarErrorDecorator extends LabelProvider
	implements ILightweightLabelDecorator {
	
	public static SoarErrorDecorator getErrorDecorator() {
		IDecoratorManager decoratorManager =
			SoarPlugin.getDefault().getWorkbench().getDecoratorManager();
		
		if (decoratorManager.getEnabled("edu.rosehulman.soar.navigator.SoarErrorDecorator")) {
			return (SoarErrorDecorator)
				decoratorManager.getBaseLabelProvider("edu.rosehulman.soar.navigator.SoarErrorDecorator");
		}
		return null;
	}
	
	public void refresh(final IResource res) {
		ArrayList tmp = new ArrayList();
		
		tmp.add(res);
		
		IResource parent = res.getParent();
		IProject proj = res.getProject();
		while (parent != null && parent != proj) {
			tmp.add(parent);
			parent = parent.getParent();
		}
		tmp.add(proj);
		
		final LabelProviderChangedEvent event =
			new LabelProviderChangedEvent(this, tmp.toArray());
		
		Display.getDefault().asyncExec(
				new Runnable() {
					public void run() {
						fireLabelProviderChanged(event);
					}
				}
			);
	}

	public void decorate(Object object, IDecoration decoration) {
		if (object instanceof IResource) {
			IResource file = (IResource)object;
			try {
				IMarker[] markers = file.findMarkers(IMarker.PROBLEM, true, IResource.DEPTH_INFINITE);
				if (markers.length > 0) {
					int severity = 0;
					for (int i=0; i<markers.length; ++i) {
						int s = markers[i].getAttribute(IMarker.SEVERITY, IMarker.SEVERITY_WARNING);
						if (s > severity) {
							severity = s;
						}
					}
					
					switch (severity) {
					case IMarker.SEVERITY_ERROR:
						decoration.addOverlay(SoarImages.getImageDescriptor(SoarImages.ERROR));
						break;
					default:
						decoration.addOverlay(SoarImages.getImageDescriptor(SoarImages.WARNING));
						break;	
					}
				}
			} catch (CoreException e) {
				e.printStackTrace();
			}
		}
	}


	public boolean isLabelProperty(Object element, String property) {
		return false;
	}

	

}
