#include "toplevel.h"
#include "css.h"
#include "misc.h"
#include "panel-layout.h"
#include "panel-platform.h"

static const int PERIOD = 200;

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
	GtkRevealer *ah_rev;
	GtkSeparator *ah_sep;
	PanelAutohideState ah_state;
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
	int mon;
	GtkOrientation orientation;
	GtkPositionType edge;
	GtkDialog *pref_dialog;
	char *font;
};

G_DEFINE_TYPE(ValaPanelToplevelUnit, vala_panel_toplevel_unit, GTK_TYPE_APPLICATION_WINDOW)

static void stop_ui(ValaPanelToplevelUnit *self)
{
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
	gtk_widget_add_events(GTK_WIDGET(self),
	                      GDK_BUTTON_PRESS_MASK | GDK_ENTER_NOTIFY_MASK |
	                          GDK_LEAVE_NOTIFY_MASK);
	gtk_widget_realize(GTK_WIDGET(self));
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
	if (self->mon < gdk_screen_get_n_monitors(gtk_widget_get_screen(GTK_WIDGET(self))))
		start_ui(self);
}

G_GNUC_INTERNAL bool panel_edge_available(ValaPanelToplevelUnit *self, uint edge, int monitor,
                                          bool include_this)
{
	g_autoptr(GtkApplication) app;
	g_object_get(self, "application", &app, NULL);
	for (g_autoptr(GList) w = gtk_application_get_windows(app); w != NULL; w = w->next)
		if (VALA_PANEL_IS_TOPLEVEL_UNIT(w))
		{
			ValaPanelToplevelUnit *pl = VALA_PANEL_TOPLEVEL_UNIT(w->data);
			if (((pl != self) || include_this) && (self->edge == edge) &&
			    ((monitor == self->mon) || self->mon < 0))
				return false;
		}
	return true;
}

static void activate_new_panel(GSimpleAction *act, GVariant *param, void *data)
{
	ValaPanelToplevelUnit *self = VALA_PANEL_TOPLEVEL_UNIT(data);
	int new_mon                 = -2;
	GtkPositionType new_edge    = GTK_POS_TOP;
	bool found                  = false;
	/* Allocate the edge. */
	g_assert(gtk_widget_get_screen(GTK_WIDGET(self)) != NULL);
	int monitors = gdk_screen_get_n_monitors(gtk_widget_get_screen(GTK_WIDGET(self)));
	/* try to allocate edge on current monitor first */
	int m = self->mon;
	if (m < 0)
	{
		/* panel is spanned over the screen, guess from pointer now */
		int x, y;
		GdkScreen *scr;
		GdkDevice *dev;
#if GTK_CHECK_VERSION(3, 20, 0)
		GdkSeat *seat = gdk_display_get_default_seat(
		    gdk_screen_get_display(gtk_widget_get_screen(GTK_WIDGET(self))));
		dev = gdk_seat_get_pointer(seat);
#else
		GdkDeviceManager *manager = gdk_display_get_device_manager(
		    gdk_screen_get_display(gtk_widget_get_screen(GTK_WIDGET(self))));
		dev = gdk_device_manager_get_client_pointer(manager);
#endif
		gdk_device_get_position(dev, &scr, &x, &y);
		m = gdk_screen_get_monitor_at_point(scr, x, y);
	}
	for (int e = GTK_POS_BOTTOM; e >= GTK_POS_LEFT; e--)
	{
		if (panel_edge_available(self, (GtkPositionType)e, m, true))
		{
			new_edge = (GtkPositionType)e;
			new_mon  = m;
			found    = true;
		}
	}
	/* try all monitors */
	if (!found)
		for (m = 0; m < monitors; ++m)
		{
			/* try each of the four edges */
			for (int e = GTK_POS_BOTTOM; e >= GTK_POS_LEFT; e--)
			{
				if (panel_edge_available(self, (GtkPositionType)e, m, true))
				{
					new_edge = (GtkPositionType)e;
					new_mon  = m;
					found    = true;
				}
			}
		}
	if (!found)
	{
		g_warning(
		    "Error adding panel: There is no room for another panel. All the edges are "
		    "taken.");
		g_autoptr(GtkWidget) msg = gtk_message_dialog_new(
		    GTK_WINDOW(self),
		    GTK_DIALOG_DESTROY_WITH_PARENT,
		    GTK_MESSAGE_ERROR,
		    GTK_BUTTONS_CLOSE,
		    N_("There is no room for another panel. All the edges are taken."));
		vala_panel_apply_window_icon(GTK_WINDOW(msg));
		gtk_window_set_title(GTK_WINDOW(msg), _("Error"));
		gtk_dialog_run(GTK_DIALOG(msg));
		gtk_widget_destroy(GTK_WIDGET(msg));
		return;
	}
	g_autofree char *new_name = vala_panel_generate_new_hash();
	// FIXME: Translate after adding constructors
	//        ValaPanelToplevelUnit* new_toplevel =
	//        Toplevel.create(application,new_name,new_mon,new_edge);
	//        new_toplevel.configure("position");
	//        new_toplevel.show_all();
	//        new_toplevel.queue_draw();
}
static void activate_remove_panel(GSimpleAction *act, GVariant *param, void *data)
{
	ValaPanelToplevelUnit *self     = VALA_PANEL_TOPLEVEL_UNIT(data);
	g_autoptr(GtkMessageDialog) dlg = GTK_MESSAGE_DIALOG(
	    gtk_message_dialog_new_with_markup(GTK_WINDOW(self),
	                                       GTK_DIALOG_MODAL,
	                                       GTK_MESSAGE_QUESTION,
	                                       GTK_BUTTONS_OK_CANCEL,
	                                       N_("Really delete this panel?\n<b>Warning:"
	                                          "This can not be recovered.</b>")));
	vala_panel_apply_window_icon(GTK_WINDOW(dlg));
	gtk_window_set_title(GTK_WINDOW(dlg), _("Confirm"));
	bool ok = (gtk_dialog_run(GTK_DIALOG(dlg)) == GTK_RESPONSE_OK);
	gtk_widget_destroy(GTK_WIDGET(dlg));
	if (ok)
	{
		g_autofree char *uid  = g_strdup(self->uid);
		g_autofree char *path = NULL;
		g_object_get(self->toplevel_settings, "path", &path, NULL);
		ValaPanelPlatform *mgr = vala_panel_applet_manager_get_manager(self->manager);
		stop_ui(self);
		gtk_widget_destroy(GTK_WIDGET(self));
		/* delete the config file of this panel */
		vala_panel_platform_remove_settings_path(mgr, path, uid);
	}
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

ValaPanelToplevelUnit *vala_panel_toplevel_unit_new_from_position(GtkApplication *app,
                                                                  const char *uid, int mon,
                                                                  GtkPositionType edge)
{
	ValaPanelToplevelUnit *ret =
	    VALA_PANEL_TOPLEVEL_UNIT(g_object_new(vala_panel_toplevel_unit_get_type(),
	                                          "border-width",
	                                          0,
	                                          "decorated",
	                                          false,
	                                          "name",
	                                          "ValaPanel",
	                                          "resizable",
	                                          false,
	                                          "title",
	                                          "ValaPanel",
	                                          "type-hint",
	                                          GDK_WINDOW_TYPE_HINT_DOCK,
	                                          "window-position",
	                                          GTK_WIN_POS_NONE,
	                                          "skip-taskbar-hint",
	                                          true,
	                                          "skip-pager-hint",
	                                          true,
	                                          "accept-focus",
	                                          false,
	                                          "application",
	                                          app,
	                                          "uuid",
	                                          uid));
	ret->mon  = mon;
	ret->edge = edge;
	setup(ret, true);
	return ret;
}
ValaPanelToplevelUnit *vala_panel_toplevel_unit_new_from_uid(GtkApplication *app, char *uid)
{
	ValaPanelToplevelUnit *ret =
	    VALA_PANEL_TOPLEVEL_UNIT(g_object_new(vala_panel_toplevel_unit_get_type(),
	                                          "border-width",
	                                          0,
	                                          "decorated",
	                                          false,
	                                          "name",
	                                          "ValaPanel",
	                                          "resizable",
	                                          false,
	                                          "title",
	                                          "ValaPanel",
	                                          "type-hint",
	                                          GDK_WINDOW_TYPE_HINT_DOCK,
	                                          "window-position",
	                                          GTK_WIN_POS_NONE,
	                                          "skip-taskbar-hint",
	                                          true,
	                                          "skip-pager-hint",
	                                          true,
	                                          "accept-focus",
	                                          false,
	                                          "application",
	                                          app,
	                                          "uuid",
	                                          uid));
	setup(ret, false);
	return ret;
}

// static void size_allocate(GtkWidget *base, GtkAllocation *alloc)
//{
//	int x, y, w;
//	GTK_WIDGET_CLASS(vala_panel_toplevel_unit_parent_class)->size_allocate(base, alloc);
//	ValaPanelToplevelUnit *self = VALA_PANEL_TOPLEVEL_UNIT(base);
//	if (self->is_dynamic && self->layout != NULL)
//	{
//		if (self->orientation == GTK_ORIENTATION_HORIZONTAL)
//			gtk_widget_get_preferred_width(GTK_WIDGET(self->layout), NULL, &w);
//		else
//			gtk_widget_get_preferred_height(GTK_WIDGET(self->layout), NULL, &w);
//		if (w != self->width)
//			g_settings_set_int(self->toplevel_settings, VALA_PANEL_KEY_WIDTH, w);
//	}
//	if (!gtk_widget_get_realized(GTK_WIDGET(self)))
//		return;
//	gdk_window_get_origin(gtk_widget_get_window(GTK_WIDGET(self)), &x, &y);
//	//    _calculate_position (&alloc);
//	//    this.a.x = alloc.x;
//	//    this.a.y = alloc.y;
//	//    if (alloc.width != this.a.width || alloc.height != this.a.height || this.a.x != x ||
//	//    this.a.y != y)
//	//    {
//	//        this.a.width = alloc.width;
//	//        this.a.height = alloc.height;
//	//        this.set_size_request(this.a.width, this.a.height);
//	//        this.move(this.a.x, this.a.y);
//	//        this.update_strut();
//	//    }
//	if (gtk_widget_get_mapped(GTK_WIDGET(self)))
//		establish_autohide(self);
//}

// void _calculate_position(ref Gtk.Allocation alloc)
//{
//    unowned Gdk.Screen screen = this.get_screen();
//    Gdk.Rectangle marea = Gdk.Rectangle();
//    if (monitor < 0)
//    {
//        marea.x = 0;
//        marea.y = 0;
//        marea.width = screen.get_width();
//        marea.height = screen.get_height();
//    }
//    else if (monitor < screen.get_n_monitors())
//    {
//        screen.get_monitor_geometry(monitor,&marea);
////~                 marea = screen.get_monitor_workarea(monitor);
////~                 var hmod = (autohide && show_hidden) ? 1 : height;
////~                 switch (edge)
////~                 {
////~                     case PositionType.TOP:
////~                         marea.x -= hmod;
////~                         marea.height += hmod;
////~                         break;
////~                     case PositionType.BOTTOM:
////~                         marea.height += hmod;
////~                         break;
////~                     case PositionType.LEFT:
////~                         marea.y -= hmod;
////~                         marea.width += hmod;
////~                         break;
////~                     case PositionType.RIGHT:
////~                         marea.width += hmod;
////~                         break;
////~                 }
//    }
//    if (orientation == GTK_ORIENTATION_HORIZONTAL)
//    {
//        alloc.width = width;
//        alloc.x = marea.x;
//        calculate_width(marea.width,is_dynamic,alignment,panel_margin,ref alloc.width, ref
//        alloc.x);
//        alloc.height = (!autohide || ah_visible) ? height :
//                                show_hidden ? 1 : 0;
//        alloc.y = marea.y + ((edge == Gtk.PositionType.TOP) ? 0 : (marea.height - alloc.height));
//    }
//    else
//    {
//        alloc.height = width;
//        alloc.y = marea.y;
//        calculate_width(marea.height,is_dynamic,alignment,panel_margin,ref alloc.height, ref
//        alloc.y);
//        alloc.width = (!autohide || ah_visible) ? height :
//                                show_hidden ? 1 : 0;
//        alloc.x = marea.x + ((edge == Gtk.PositionType.LEFT) ? 0 : (marea.width - alloc.width));
//    }
//}

// static void calculate_width(int scrw, bool dyn, AlignmentType align,
//                                    int margin, ref int panw, ref int x)
//{
//    if (!dyn)
//    {
//        panw = (panw >= 100) ? 100 : (panw <= 1) ? 1 : panw;
//        panw = (int)(((double)scrw * (double) panw)/100.0);
//    }
//    margin = (align != AlignmentType.CENTER && margin > scrw) ? 0 : margin;
//    panw = int.min(scrw - margin, panw);
//    if (align == AlignmentType.START)
//        x+=margin;
//    else if (align == AlignmentType.END)
//    {
//        x += scrw - panw - margin;
//        x = (x < 0) ? 0 : x;
//    }
//    else if (align == AlignmentType.CENTER)
//        x += (scrw - panw)/2;
//}

// static void get_preferred_width(&int min, &int nat)
//{
//    base.get_preferred_width_internal(&min, &nat);
//    Gtk.Requisition req = Gtk.Requisition();
//    this.get_panel_preferred_size(ref req);
//    min = nat = req.width;
//}
// static void get_preferred_height(&int min, &int nat)
//{
//    base.get_preferred_height_internal(&min, &nat);
//    Gtk.Requisition req = Gtk.Requisition();
//    this.get_panel_preferred_size(ref req);
//    min = nat = req.height;
//}
// static void get_panel_preferred_size (ref Gtk.Requisition min)
//{
//    if (!ah_visible && box != NULL)
//        box.get_preferred_size(&min, NULL);
//    var rect = Gtk.Allocation();
//    rect.width = min.width;
//    rect.height = min.height;
//    _calculate_position(ref rect);
//    min.width = rect.width;
//    min.height = rect.height;
//}

/****************************************************
 *         autohide : new version                   *
 ****************************************************/
static bool timeout_func(ValaPanelToplevelUnit *self)
{
	if (self->autohide && self->ah_state == AH_WAITING)
	{
		css_toggle_class(GTK_WIDGET(self), "-panel-transparent", true);
		gtk_revealer_set_reveal_child(self->ah_rev, false);
		self->ah_state = AH_HIDDEN;
	}
	return false;
}

static void ah_show(ValaPanelToplevelUnit *self)
{
	css_toggle_class(GTK_WIDGET(self), "-panel-transparent", false);
	gtk_revealer_set_reveal_child(self->ah_rev, true);
	self->ah_state = AH_VISIBLE;
}

static void ah_hide(ValaPanelToplevelUnit *self)
{
	self->ah_state = AH_WAITING;
	g_timeout_add(PERIOD, (GSourceFunc)timeout_func, self);
}

static bool enter_notify_event(ValaPanelToplevelUnit *self, GdkEventCrossing *event, gpointer data)
{
	ah_show(self);
	return false;
}

static bool leave_notify_event(ValaPanelToplevelUnit *self, GdkEventCrossing *event, gpointer data)
{
	if (self->autohide &&
	    (event->detail != GDK_NOTIFY_INFERIOR && event->detail != GDK_NOTIFY_VIRTUAL))
		ah_hide(self);
	return false;
}

static void grab_notify(ValaPanelToplevelUnit *self, bool was_grabbed, gpointer data)
{
	if (!was_grabbed)
		self->ah_state = AH_GRAB;
	else if (self->autohide)
		ah_hide(self);
}

void vala_panel_toplevel_unit_init(ValaPanelToplevelUnit *self)
{
	// Move this to init, lay&must not be reinit in start/stop UI
	self->layout = vala_panel_applet_layout_new(self->orientation, 0);
	self->ah_rev = GTK_REVEALER(gtk_revealer_new());
	self->ah_sep = GTK_SEPARATOR(gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));
	gtk_revealer_set_reveal_child(self->ah_rev, true);
	GtkBox *box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
	gtk_container_add(GTK_CONTAINER(self), GTK_WIDGET(box));
	gtk_container_add(GTK_CONTAINER(box), GTK_WIDGET(self->ah_rev));
	gtk_container_add(GTK_CONTAINER(box), GTK_WIDGET(self->ah_sep));
	gtk_container_add(GTK_CONTAINER(self->ah_rev), GTK_WIDGET(self->layout));
	g_object_bind_property(self, "orientation", self->layout, "orentation", (GBindingFlags)0);
	g_object_bind_property(self, "orientation", box, "orentation", (GBindingFlags)0);
	g_object_bind_property(self, "orientation", self->ah_sep, "orentation", (GBindingFlags)0);
}

void vala_panel_toplevel_unit_class_init(ValaPanelToplevelUnitClass *parent)
{
}
