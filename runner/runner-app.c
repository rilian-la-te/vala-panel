#include "runner-app.h"
#include "lib/definitions.h"

struct _ValaPanelRunApplication
{
	GtkWindow *run_dialog;
};

G_DEFINE_TYPE(ValaPanelRunApplication, vala_panel_run_application, GTK_TYPE_APPLICATION)

static ValaPanelRunApplication *vala_panel_run_application_new()
{
	return VALA_PANEL_RUN_APPLICATION(g_object_new(vala_panel_run_application_get_type(),
	                                               "application-id",
	                                               "org.valapanel.extras.Runner",
	                                               "flags",
	                                               0,
	                                               NULL));
}

static void vala_panel_run_application_activate(GApplication *app)
{
	//            if (rd == null) {
	//                rd = new RunDialog(this);
	//            }
	//            rd.present();
}

static void vala_panel_run_application_finalize(GObject *app)
{
	gtk_widget_destroy0(VALA_PANEL_RUN_APPLICATION(app)->run_dialog);
	(*G_OBJECT_CLASS(vala_panel_run_application_parent_class)->finalize)(app);
}
static void vala_panel_run_application_init(ValaPanelRunApplication *self)
{
}

static void vala_panel_run_application_class_init(ValaPanelRunApplicationClass *klass)
{
	GApplicationClass *parent = G_APPLICATION_CLASS(klass);
	parent->activate          = vala_panel_run_application_activate;
	GObjectClass *obj_class   = G_OBJECT_CLASS(klass);
	obj_class->finalize       = vala_panel_run_application_finalize;
}

int main(int argc, char *argv[])
{
	ValaPanelRunApplication *rd = vala_panel_run_application_new();
	return g_application_run(G_APPLICATION(rd), argc, argv);
}
