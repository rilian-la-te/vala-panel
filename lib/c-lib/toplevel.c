#include "toplevel.h"
#include "css.h"
#include "panel-layout.h"
#include "panel-manager.h"

static void activate_new_panel(GSimpleAction *act, GVariant *param, void *data);
static void activate_remove_panel(GSimpleAction *act, GVariant *param, void *data);
static void activate_panel_settings(GSimpleAction *act, GVariant *param, void *data);

static const GActionEntry panel_entries[] =
    { { "new-panel", activate_new_panel, NULL, NULL, NULL, { 0 } },
      { "remove-panel", activate_remove_panel, NULL, NULL, NULL, { 0 } },
      { "panel-settings", activate_panel_settings, "s", NULL, NULL, { 0 } } };

struct _ValaPanelToplevelUnit
{
	GtkApplicationWindow __parent__;
	ValaPanelAppletManager *manager;
	ValaPanelAppletLayout *layout;
	GSettings *toplevel_settings;
	GtkCssProvider *provider;
	bool initialized;
	bool dock;
	bool autohide;
	bool use_background_color;
	bool use_foreground_color;
	bool use_background_file;
	bool use_font;
	bool font_size_only;
	uint round_corners_size;
	GdkRGBA background_color;
	GdkRGBA foreground_color;
	char *background_file;
	char *uid;
	int height;
	int widgth;
	int mon;
	GtkOrientation orientation;
	GtkPositionType edge;
	GtkDialog *pref_dialog;
	char *font;
};

G_DEFINE_TYPE(ValaPanelToplevelUnit, vala_panel_toplevel_unit, GTK_TYPE_APPLICATION_WINDOW)

static void stop_ui(ValaPanelToplevelUnit *self)
{
	if (self->autohide)
		vala_panel_manager_ah_stop(vala_panel_applet_manager_get_manager(self->manager),
		                           self);
	if (self->pref_dialog != NULL)
		gtk_dialog_response(self->pref_dialog, GTK_RESPONSE_CLOSE);
	if (self->initialized)
	{
		gdk_flush();
		self->initialized = false;
	}
	if (gtk_bin_get_child(GTK_BIN(self)))
	{
		gtk_widget_hide(GTK_WIDGET(self->layout));
	}
}

static void start_ui(ValaPanelToplevelUnit *self)
{
	//    a.x = a.y = a.width = a.height = 0;
	gtk_window_set_wmclass(GTK_WINDOW(self), "panel", "vala-panel");
	gtk_application_add_window(gtk_window_get_application(GTK_WINDOW(self)), GTK_WINDOW(self));
	gtk_widget_add_events(GTK_WIDGET(self), GDK_BUTTON_PRESS_MASK);
	gtk_widget_realize(GTK_WIDGET(self));
	// Move this to init, layout must not be reinit in start/stop UI
	self->layout = vala_panel_applet_layout_new(self->orientation, 0);
	gtk_container_add(GTK_CONTAINER(self), GTK_WIDGET(self->layout));
	gtk_widget_show(GTK_WIDGET(self->layout));
	gtk_window_set_type_hint(GTK_WINDOW(self),
	                         (self->dock) ? GDK_WINDOW_TYPE_HINT_DOCK
	                                      : GDK_WINDOW_TYPE_HINT_NORMAL);
	gtk_widget_show(GTK_WIDGET(self));
	gtk_window_stick(GTK_WINDOW(self));
	vala_panel_applet_layout_load_applets(self->layout, self->manager, self->toplevel_settings);
	gtk_window_present(GTK_WINDOW(self));
	self->autohide = g_settings_get_boolean(self->toplevel_settings, VALA_PANEL_KEY_AUTOHIDE);
	self->initialized = true;
}

static void setup(ValaPanelToplevelUnit *self, bool use_internal_values)
{
	if (use_internal_values)
	{
		g_settings_set_int(self->toplevel_settings, VALA_PANEL_KEY_MONITOR, self->mon);
		g_settings_set_enum(self->toplevel_settings, VALA_PANEL_KEY_EDGE, self->edge);
	}
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->toplevel_settings,
	                                   VALA_PANEL_KEY_EDGE);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->toplevel_settings,
	                                   VALA_PANEL_KEY_ALIGNMENT);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->toplevel_settings,
	                                   VALA_PANEL_KEY_HEIGHT);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->toplevel_settings,
	                                   VALA_PANEL_KEY_WIDTH);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->toplevel_settings,
	                                   VALA_PANEL_KEY_DYNAMIC);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->toplevel_settings,
	                                   VALA_PANEL_KEY_AUTOHIDE);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->toplevel_settings,
	                                   VALA_PANEL_KEY_STRUT);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->toplevel_settings,
	                                   VALA_PANEL_KEY_DOCK);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->toplevel_settings,
	                                   VALA_PANEL_KEY_MARGIN);
	vala_panel_bind_gsettings(self, self->toplevel_settings, VALA_PANEL_KEY_MONITOR);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->toplevel_settings,
	                                   VALA_PANEL_KEY_SHOW_HIDDEN);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->toplevel_settings,
	                                   VALA_PANEL_KEY_ICON_SIZE);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->toplevel_settings,
	                                   VALA_PANEL_KEY_BACKGROUND_COLOR);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->toplevel_settings,
	                                   VALA_PANEL_KEY_FOREGROUND_COLOR);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->toplevel_settings,
	                                   VALA_PANEL_KEY_BACKGROUND_FILE);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->toplevel_settings,
	                                   VALA_PANEL_KEY_FONT);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->toplevel_settings,
	                                   VALA_PANEL_KEY_CORNERS_SIZE);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->toplevel_settings,
	                                   VALA_PANEL_KEY_FONT_SIZE_ONLY);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->toplevel_settings,
	                                   VALA_PANEL_KEY_USE_BACKGROUND_COLOR);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->toplevel_settings,
	                                   VALA_PANEL_KEY_USE_FOREGROUND_COLOR);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->toplevel_settings,
	                                   VALA_PANEL_KEY_USE_FONT);
	vala_panel_add_gsettings_as_action(G_ACTION_MAP(self),
	                                   self->toplevel_settings,
	                                   VALA_PANEL_KEY_USE_BACKGROUND_FILE);
	//    if (monitor < Gdk.Screen.get_default().get_n_monitors())
	//        start_ui();
	//    unowned Gtk.Application panel_app = get_application();
	//    if (mon_handler != 0)
	//        mon_handler = Signal.connect(Gdk.Screen.get_default(),"monitors-changed",
	//                                    (GLib.Callback)(monitors_changed_cb),panel_app);
}

static void activate_new_panel(GSimpleAction *act, GVariant *param, void *data)
{
	//    int new_mon = -2;
	//    PositionType new_edge = PositionType.TOP;
	//    var found = false;
	//    /* Allocate the edge. */
	//    assert(Gdk.Screen.get_default()!=null);
	//    var monitors = Gdk.Screen.get_default().get_n_monitors();
	//    /* try to allocate edge on current monitor first */
	//    var m = _mon;
	//    if (m < 0)
	//    {
	//        /* panel is spanned over the screen, guess from pointer now */
	//        int x, y;
	//        var manager = Gdk.Screen.get_default().get_display().get_device_manager();
	//        var device = manager.get_client_pointer ();
	//        Gdk.Screen scr;
	//        device.get_position(out scr, out x, out y);
	//        m = scr.get_monitor_at_point(x, y);
	//    }
	//    for (int e = PositionType.BOTTOM; e >= PositionType.LEFT; e--)
	//    {
	//        if (panel_edge_available((PositionType)e, m, true))
	//        {
	//            new_edge = (PositionType)e;
	//            new_mon = m;
	//            found = true;
	//        }
	//    }
	//    /* try all monitors */
	//    if (!found)
	//        for(m=0; m<monitors; ++m)
	//        {
	//            /* try each of the four edges */
	//            for(int e = PositionType.BOTTOM; e >= PositionType.LEFT; e--)
	//            {
	//                if(panel_edge_available((PositionType)e,m,true)) {
	//                    new_edge = (PositionType)e;
	//                    new_mon = m;
	//                    found = true;
	//                }
	//            }
	//        }
	//    if (!found)
	//    {
	//        warning("Error adding panel: There is no room for another panel. All the edges are
	//        taken.");
	//        var msg = new MessageDialog
	//                (this,
	//                 DialogFlags.DESTROY_WITH_PARENT,
	//                 MessageType.ERROR,ButtonsType.CLOSE,
	//                 N_("There is no room for another panel. All the edges are taken."));
	//        apply_window_icon(msg as Gtk.Window);
	//        msg.set_title(_("Error"));
	//        msg.run();
	//        msg.destroy();
	//        return;
	//    }
	//    var new_name = gen_panel_name(profile,new_edge,new_mon);
	//    var new_toplevel = Toplevel.create(application,new_name,new_mon,new_edge);
	//    new_toplevel.configure("position");
	//    new_toplevel.show_all();
	//    new_toplevel.queue_draw();
}
static void activate_remove_panel(GSimpleAction *act, GVariant *param, void *data)
{
	//    var dlg = new MessageDialog.with_markup(this,
	//                                            DialogFlags.MODAL,
	//                                            MessageType.QUESTION,
	//                                            ButtonsType.OK_CANCEL,
	//                                            N_("Really delete this panel?\n<b>Warning:
	//                                            This can not be recovered.</b>"));
	//    apply_window_icon(dlg as Gtk.Window);
	//    dlg.set_title(_("Confirm"));
	//    var ok = (dlg.run() == ResponseType.OK );
	//    dlg.destroy();
	//    if( ok )
	//    {
	//        string pr = this.profile;
	//        this.stop_ui();
	//        this.destroy();
	//        /* delete the config file of this panel */
	//        var fname = user_config_file_name("panels",pr,panel_name);
	//        FileUtils.unlink( fname );
	//    }
}
static void activate_panel_settings(GSimpleAction *act, GVariant *param, void *data)
{
	//    this.configure(param.get_string());
}

G_GNUC_INTERNAL void update_appearance(ValaPanelToplevelUnit *self)
{
	if (self->provider)
		gtk_style_context_remove_provider(gtk_widget_get_style_context(GTK_WIDGET(self)),
		                                  GTK_STYLE_PROVIDER(self->provider));
	if (!self->font)
		return;
	g_autoptr(GString) str            = g_string_new("");
	g_autofree char *background_color = gdk_rgba_to_string(&self->background_color);
	g_string_append_printf(str, ".-vala-panel-background {\n");
	if (self->use_background_color)
		g_string_append_printf(str, " background-color: %s;\n", background_color);
	else
		g_string_append_printf(str, " background-color: transparent;\n");
	if (self->use_background_file)
	{
		g_string_append_printf(str,
		                       " background-image: url('%s');\n",
		                       self->background_file);
		/* Feature proposed: Background repeat */
		//~                 if (false)
		//~                     g_string_append_printf(str," background-repeat:
		// no-repeat;\n");
	}
	else
		g_string_append_printf(str, " background-image: none;\n");
	g_string_append_printf(str, "}\n");
	/* Feature proposed: Panel Layout and Shadow */
	//~             g_string_append_printf(str,".-vala-panel-shadow {\n");
	//~             g_string_append_printf(str," box-shadow: 0 0 0 3px alpha(0.3,
	//%s);\n",foreground_color);
	//~             g_string_append_printf(str," border-style: none;\n margin: 3px;\n");
	//~             g_string_append_printf(str,"}\n");
	g_string_append_printf(str, ".-vala-panel-round-corners {\n");
	g_string_append_printf(str, " border-radius: %upx;\n", self->round_corners_size);
	g_string_append_printf(str, "}\n");
	PangoFontDescription *desc = pango_font_description_from_string(self->font);
	g_string_append_printf(str, ".-vala-panel-font-size {\n");
	g_string_append_printf(str,
	                       " font-size: %dpx;\n",
	                       pango_font_description_get_size(desc) / PANGO_SCALE);
	g_string_append_printf(str, "}\n");
	g_string_append_printf(str, ".-vala-panel-font {\n");
	const char *family   = pango_font_description_get_family(desc);
	PangoWeight weight   = pango_font_description_get_weight(desc);
	PangoStyle style     = pango_font_description_get_style(desc);
	PangoVariant variant = pango_font_description_get_variant(desc);
	g_string_append_printf(str,
	                       " font-style: %s;\n",
	                       (style == PANGO_STYLE_ITALIC)
	                           ? "italic"
	                           : ((style == PANGO_STYLE_OBLIQUE) ? "oblique" : "normal"));
	g_string_append_printf(str,
	                       " font-variant: %s;\n",
	                       (variant == PANGO_VARIANT_SMALL_CAPS) ? "small-caps" : "normal");
	g_string_append_printf(str,
	                       " font-weight: %s;\n",
	                       (weight <= PANGO_WEIGHT_SEMILIGHT)
	                           ? "light"
	                           : (weight >= PANGO_WEIGHT_SEMIBOLD ? "bold" : "normal"));
	g_string_append_printf(str, " font-family: %s;\n", family);
	g_string_append_printf(str, "}\n");
	g_string_append_printf(str, ".-vala-panel-foreground-color {\n");
	g_autofree char *foreground_color = gdk_rgba_to_string(&self->foreground_color);
	g_string_append_printf(str, " color: %s;\n", foreground_color);
	g_string_append_printf(str, "}\n");
	g_autofree char *css = str->str;
	self->provider       = css_add_css_to_widget(GTK_WIDGET(self), css);
	css_toggle_class(GTK_WIDGET(self),
	                 "-vala-panel-background",
	                 self->use_background_color || self->use_background_file);
	css_toggle_class(GTK_WIDGET(self), "-vala-panel-shadow", false);
	css_toggle_class(GTK_WIDGET(self),
	                 "-vala-panel-round-corners",
	                 self->round_corners_size > 0);
	css_toggle_class(GTK_WIDGET(self), "-vala-panel-font-size", self->use_font);
	css_toggle_class(GTK_WIDGET(self),
	                 "-vala-panel-font",
	                 self->use_font && !self->font_size_only);
	css_toggle_class(GTK_WIDGET(self),
	                 "-vala-panel-foreground-color",
	                 self->use_foreground_color);
}

void vala_panel_toplevel_unit_init(ValaPanelToplevelUnit *self)
{
}

void vala_panel_toplevel_unit_class_init(ValaPanelToplevelUnitClass *parent)
{
}
