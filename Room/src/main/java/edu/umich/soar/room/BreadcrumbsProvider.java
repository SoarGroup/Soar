package edu.umich.soar.room;

interface BreadcrumbsProvider {
	boolean areBreadcrumbsEnabled();
	void setBreadcrumbsEnabled(boolean setting);
	void clearBreadcrumbs();
}
