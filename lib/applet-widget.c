#include "applet-widget.h"
#include "definitions.h"
#include "vala-panel-compat.h"

typedef struct
{
	GtkDialog *dialog;
	GtkWidget *background;
	ValaPanelToplevel *toplevel;
	GSettings *settings;
	char *uuid;
	GSimpleActionGroup *grp;

} ValaPanelAppletPrivate;

static inline void destroy0(GtkWidget *x)
{
	gtk_widget_destroy0(x);
}

G_DEFINE_TYPE_WITH_PRIVATE(ValaPanelApplet, vala_panel_applet, GTK_TYPE_BIN)

static void activate_remove(GSimpleAction *act, GVariant *param, ValaPanelApplet *self);

static const GActionEntry entries[] = { { "menu", activate_menu, NULL, NULL, NULL },
	                                { "configure", activate_configure, NULL, NULL, NULL },
	                                { "remove", activate_remove, NULL, NULL, NULL },
	                                { NULL } };

enum
{
	VALA_PANEL_APPLET_DUMMY_PROPERTY,
	VALA_PANEL_APPLET_BACKGROUND_WIDGET,
	VALA_PANEL_APPLET_TOPLEVEL,
	VALA_PANEL_APPLET_SETTINGS,
	VALA_PANEL_APPLET_UUID,
	VALA_PANEL_APPLET_GRP
};

// private Dialog? dialog;
// public unowned Gtk.Widget background_widget {get; set;}
// public unowned ValaPanel.Toplevel toplevel {get; construct;}
// public unowned GLib.Settings? settings {get; construct;}
// public string uuid {get; construct;}
// public virtual void update_context_menu(ref GLib.Menu parent_menu){}
// public SimpleActionGroup grp {get; private set;}
// public Applet(ValaPanel.Toplevel top, GLib.Settings? s, string uuid)
//{
//    Object(toplevel: top, settings: s, uuid: uuid);
//}
// construct
//{
//    grp = new SimpleActionGroup();
//    this.set_has_window(false);
//    this.border_width = 0;
//    this.button_release_event.connect((b)=>
//    {
//        if (b.button == 3 &&
//            ((b.state & Gtk.accelerator_get_default_mod_mask ()) == 0))
//        {
//            toplevel.get_plugin_menu(this).popup_at_widget(this,Gdk.Gravity.NORTH,
//            Gdk.Gravity.NORTH,b);
//            return true;
//        }
//        return false;
//    });
//    grp.add_action_entries(remove_entry,this);
//    this.insert_action_group("applet",grp);
//    var cnf = grp.lookup_action("configure") as SimpleAction;
//    var mn = grp.lookup_action("menu") as SimpleAction;
//    cnf.set_enabled(false);
//    cnf.set_enabled(false);
//    set_actions();
//}
// protected override void parent_set(Gtk.Widget? prev_parent)
//{
//    if (prev_parent == null)
//    {
//        if (background_widget == null)
//            background_widget = this;
//        init_background();
//    }
//}
// public void init_background()
//{
//    var color = Gdk.RGBA();
//    color.parse ("transparent");
//    PanelCSS.apply_with_class(background_widget,
//                              PanelCSS.generate_background(null,color),
//                              "-vala-panel-background",
//                              false);
//}
// private void activate_configure(SimpleAction act, Variant? param)
//{
//    show_config_dialog();
//}
// protected virtual void activate_menu(SimpleAction act, Variant? param)
//{
//}
void vala_panel_applet_show_config_dialog(ValaPanelApplet *self)
{
	ValaPanelAppletPrivate *p = vala_panel_applet_get_instance_private(self);
	if (p->dialog == NULL)
	{
		GtkWidget *dlg = vala_panel_applet_get_config_dialog(self);
		g_signal_connect(dlg, "destroy", G_CALLBACK(destroy0), self);
		gtk_native_dialog_set_transient_for(dlg, p->toplevel);
		p->dialog = dlg;
		g_signal_connect(p->dialog, "hide", G_CALLBACK(destroy0), self);
		g_signal_connect(p->dialog, "destroy", G_CALLBACK(destroy0), self);
	}
	gtk_window_present(p->dialog);
}
bool _vala_panel_appletis_configurable(ValaPanelApplet(self))
{
	ValaPanelAppletPrivate *p = vala_panel_applet_get_instance_private(self);
	return g_action_group_get_action_enabled(p->grp, "configure");
}
static void activate_remove(GSimpleAction *act, GVariant *param, ValaPanelApplet *self)
{
	ValaPanelAppletPrivate *p = vala_panel_applet_get_instance_private(self);
	/* If the configuration dialog is open, there will certainly be a crash if the
	 * user manipulates the Configured Plugins list, after we remove this entry.
	 * Close the configuration dialog if it is open. */
	gtk_widget_destroy0(p->toplevel->pref_dialog);
	vala_panel_toplevel_remove_applet(p->toplevel, self);
}
GtkDialog *vala_panel_applet_get_config_dialog(ValaPanelApplet *self)
{
	return NULL;
}
void vala_panel_applet_set_actions(ValaPanelApplet *self)
{
}
static void measure(ValaPanelApplet *self, GtkOrientation orient, int for_size, int *min, int *nat,
                    int *base_min, int *base_nat)
{
	ValaPanelAppletPrivate *p = vala_panel_applet_get_instance_private(self);
	GtkOrientation panel_ori  = vala_panel_toplevel_get_orientation(p->toplevel);
	int height, icon_size;
	g_object_get(p->toplevel,
	             VALA_PANEL_KEY_HEIGHT,
	             &height,
	             VALA_PANEL_KEY_ICON_SIZE,
	             &icon_size,
	             NULL);
	if (panel_ori != orient)
	{
		*min = icon_size;
		*nat = height;
	}
	else
	{
		if (orient == GTK_ORIENTATION_HORIZONTAL)
			GTK_WIDGET_CLASS(vala_panel_applet_parent_class)
			    ->get_preferred_width_for_height(
			        (GtkWidget *)G_TYPE_CHECK_INSTANCE_CAST(self,
			                                                gtk_bin_get_type(),
			                                                GtkBin),
			        for_size,
			        &min,
			        &nat);
		else
			GTK_WIDGET_CLASS(vala_panel_applet_parent_class)
			    ->get_preferred_height_for_width(
			        (GtkWidget *)G_TYPE_CHECK_INSTANCE_CAST(self,
			                                                gtk_bin_get_type(),
			                                                GtkBin),
			        for_size,
			        &min,
			        &nat);
	}
	*base_min = *base_nat = -1;
}

static void vala_panel_applet_get_preferred_height_for_width(GtkWidget *self, int width, int *min,
                                                             int *nat)
{
	int x, y;
	measure(self, GTK_ORIENTATION_VERTICAL, width, &min, &nat, &x, &y);
}
static void vala_panel_applet_get_preferred_width_for_height(GtkWidget *self, int height, int *min,
                                                             int *nat)
{
	int x, y;
	measure(self, GTK_ORIENTATION_HORIZONTAL, height, &min, &nat, &x, &y);
}
GtkSizeRequestMode vala_panel_applet_get_request_mode(ValaPanelApplet *self)
{
	ValaPanelAppletPrivate *p = vala_panel_applet_get_instance_private(self);
	vala_panel_get(p->toplevel, VALA_PANEL_KEY_ORIENTATION, &pos, NULL);
	return (VALA_PANEL_KEY_ORIENTATION == GTK_ORIENTATION_HORIZONTAL)
	           ? GTK_SIZE_REQUEST_WIDTH_FOR_HEIGHT
	           : GTK_SIZE_REQUEST_HEIGHT_FOR_WIDTH;
}
static void vala_panel_applet_get_preferred_width(GtkWidget *self, int *min, int *nat)
{
	ValaPanelAppletPrivate *p = vala_panel_applet_get_instance_private(self);
	GtkOrientation panel_ori  = vala_panel_toplevel_get_orientation(p->toplevel);
	int height, icon_size;
	g_object_get(p->toplevel,
	             VALA_PANEL_KEY_HEIGHT,
	             &height,
	             VALA_PANEL_KEY_ICON_SIZE,
	             &icon_size,
	             NULL);
	*min = icon_size;
	*nat = height;
}
static void vala_panel_applet_get_preferred_height(GtkWidget *self, int *min, int *nat)
{
	ValaPanelAppletPrivate *p = vala_panel_applet_get_instance_private(self);
	GtkOrientation panel_ori  = vala_panel_toplevel_get_orientation(p->toplevel);
	int height, icon_size;
	g_object_get(p->toplevel,
	             VALA_PANEL_KEY_HEIGHT,
	             &height,
	             VALA_PANEL_KEY_ICON_SIZE,
	             &icon_size,
	             NULL);
	*min = icon_size;
	*nat = height;
}

static void vala_panel_applet_init(ValaPanelApplet *self)
{
}

static void vala_panel_applet_get_property(GObject *object, guint property_id, GValue *value,
                                           GParamSpec *pspec)
{
	ValaPanelApplet *self;
	self = G_TYPE_CHECK_INSTANCE_CAST(object, VALA_PANEL_TYPE_APPLET, PanelApplet);
	switch (property_id)
	{
	case VALA_PANEL_APPLET_BACKGROUND_WIDGET:
		g_value_set_object(value, vala_panel_applet_get_background_widget(self));
		break;
	case VALA_PANEL_APPLET_TOPLEVEL:
		g_value_set_object(value, vala_panel_applet_get_toplevel(self));
		break;
	case VALA_PANEL_APPLET_SETTINGS:
		g_value_set_object(value, vala_panel_applet_get_settings(self));
		break;
	case VALA_PANEL_APPLET_UUID:
		g_value_set_string(value, vala_panel_applet_get_uuid(self));
		break;
	case VALA_PANEL_APPLET_GRP:
		g_value_set_object(value, vala_panel_applet_get_grp(self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
}

static void vala_panel_applet_set_property(GObject *object, guint property_id, const GValue *value,
                                           GParamSpec *pspec)
{
	ValaPanelApplet *self;
	self = G_TYPE_CHECK_INSTANCE_CAST(object, VALA_PANEL_TYPE_APPLET, PanelApplet);
	switch (property_id)
	{
	case VALA_PANEL_APPLET_BACKGROUND_WIDGET:
		vala_panel_applet_set_background_widget(self, g_value_get_object(value));
		break;
	case VALA_PANEL_APPLET_TOPLEVEL:
		vala_panel_applet_set_toplevel(self, g_value_get_object(value));
		break;
	case VALA_PANEL_APPLET_SETTINGS:
		vala_panel_applet_set_settings(self, g_value_get_object(value));
	case VALA_PANEL_APPLET_UUID:
		vala_panel_applet_set_uuid(self, g_value_get_string(value));
		break;
	case VALA_PANEL_APPLET_GRP:
		vala_panel_applet_set_grp(self, g_value_get_object(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
		break;
	}
	static void vala_panel_applet_class_init(PanelAppletClass * klass)
	{
		vala_panel_applet_parent_class = g_type_class_peek_parent(klass);
		g_type_class_add_private(klass, sizeof(PanelAppletPrivate));
		//((PanelAppletClass *) klass)->update_context_menu = (void (*) (PanelApplet *,
		//GMenu* *)) vala_panel_applet_real_update_context_menu;
		//((GtkWidgetClass *) klass)->parent_set = (void (*) (GtkWidget *, GtkWidget*))
		//vala_panel_applet_real_parent_set;
		//((PanelAppletClass *) klass)->show_menu = (void (*) (PanelApplet *,
		//GSimpleAction*, GVariant*)) vala_panel_applet_real_activate_menu;
		((GtkWidgetClass *)klass)->get_preferred_height_for_width =
		    (void (*)(GtkWidget *, gint, gint *, gint *))
		        vala_panel_applet_get_preferred_height_for_width;
		((GtkWidgetClass *)klass)->get_preferred_width_for_height =
		    (void (*)(GtkWidget *, gint, gint *, gint *))
		        vala_panel_applet_get_preferred_width_for_height;
		((GtkWidgetClass *)klass)->get_request_mode =
		    (GtkSizeRequestMode(*)(GtkWidget *))vala_panel_applet_get_request_mode;
		((GtkWidgetClass *)klass)->get_preferred_width =
		    (void (*)(GtkWidget *, gint *, gint *))vala_panel_applet_get_preferred_width;
		((GtkWidgetClass *)klass)->get_preferred_height =
		    (void (*)(GtkWidget *, gint *, gint *))vala_panel_applet_get_preferred_height;
		((PanelAppletClass *)klass)->get_config_dialog =
		    (GtkDialog * (*)(PanelApplet *)) vala_panel_applet_get_config_dialog;
		((PanelAppletClass *)klass)->set_actions =
		    (void (*)(PanelApplet *))vala_panel_applet_real_set_actions;
		G_OBJECT_CLASS(klass)->get_property = vala_panel_applet_get_property;
		G_OBJECT_CLASS(klass)->set_property = vala_panel_applet_set_property;
		G_OBJECT_CLASS(klass)->finalize     = vala_panel_applet_finalize;
		g_object_class_install_property(G_OBJECT_CLASS(klass),
		                                VALA_PANEL_APPLET_BACKGROUND_WIDGET,
		                                g_param_spec_object("background-widget",
		                                                    "background-widget",
		                                                    "background-widget",
		                                                    gtk_widget_get_type(),
		                                                    G_PARAM_STATIC_NAME |
		                                                        G_PARAM_STATIC_NICK |
		                                                        G_PARAM_STATIC_BLURB |
		                                                        G_PARAM_READABLE |
		                                                        G_PARAM_WRITABLE));
		g_object_class_install_property(G_OBJECT_CLASS(klass),
		                                VALA_PANEL_APPLET_TOPLEVEL,
		                                g_param_spec_object("toplevel",
		                                                    "toplevel",
		                                                    "toplevel",
		                                                    VALA_PANEL_TYPE_TOPLEVEL,
		                                                    G_PARAM_STATIC_NAME |
		                                                        G_PARAM_STATIC_NICK |
		                                                        G_PARAM_STATIC_BLURB |
		                                                        G_PARAM_READABLE |
		                                                        G_PARAM_WRITABLE |
		                                                        G_PARAM_CONSTRUCT_ONLY));
		g_object_class_install_property(G_OBJECT_CLASS(),
		                                VALA_PANEL_APPLET_SETTINGS,
		                                g_param_spec_object("settings",
		                                                    "settings",
		                                                    "settings",
		                                                    g_settings_get_type(),
		                                                    G_PARAM_STATIC_NAME |
		                                                        G_PARAM_STATIC_NICK |
		                                                        G_PARAM_STATIC_BLURB |
		                                                        G_PARAM_READABLE |
		                                                        G_PARAM_WRITABLE |
		                                                        G_PARAM_CONSTRUCT_ONLY));
		g_object_class_install_property(G_OBJECT_CLASS(klklassass),
		                                VALA_PANEL_APPLET_UUID,
		                                g_param_spec_string("uuid",
		                                                    "uuid",
		                                                    "uuid",
		                                                    NULL,
		                                                    G_PARAM_STATIC_NAME |
		                                                        G_PARAM_STATIC_NICK |
		                                                        G_PARAM_STATIC_BLURB |
		                                                        G_PARAM_READABLE |
		                                                        G_PARAM_WRITABLE |
		                                                        G_PARAM_CONSTRUCT_ONLY));
		g_object_class_install_property(
		    G_OBJECT_CLASS(klass),
		    VALA_PANEL_APPLET_GRP,
		    g_param_spec_object("grp",
		                        "grp",
		                        "grp",
		                        g_simple_action_group_get_type(),
		                        G_PARAM_STATIC_NAME | G_PARAM_STATIC_NICK |
		                            G_PARAM_STATIC_BLURB | G_PARAM_READABLE));
	}
