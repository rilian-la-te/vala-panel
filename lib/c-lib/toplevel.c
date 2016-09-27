#include "toplevel.h"
#include "panel-layout.h"
#include "panel-manager.h"

struct _ValaPanelToplevelUnit
{
	GtkApplicationWindow __parent__;
	ValaPanelAppletManager *manager;
	ValaPanelAppletLayout *layout;
	bool initialized;
	char *uid;
	int height;
	int widgth;
	GtkOrientation orientation;
	GdkGravity gravity;
};

G_DEFINE_TYPE(ValaPanelToplevelUnit, vala_panel_toplevel_unit, GTK_TYPE_APPLICATION_WINDOW)

void vala_panel_toplevel_unit_init(ValaPanelToplevelUnit *self)
{
}

void vala_panel_toplevel_unit_class_init(ValaPanelToplevelUnitClass *parent)
{
}
