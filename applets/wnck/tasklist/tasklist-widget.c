/*
 * Copyright (C) 2008-2010 Nick Schermer <nick@xfce.org>
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <config.h>

#include <math.h>
#include <string.h>

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <libwnck/libwnck.h>

#ifdef GDK_WINDOWING_X11
#include <X11/Xlib.h>
#include <X11/extensions/shape.h>
#include <gdk/gdkx.h>
#endif

#include "applet-widget-api.h"
#include "definitions.h"
#include "tasklist-widget.h"
#include "util-gtk.h"
#include "util.h"
#include "xfce-arrow-button.h"

#define DEFAULT_BUTTON_SIZE (25)
#define DEFAULT_MAX_BUTTON_LENGTH (200)
#define DEFAULT_MIN_BUTTON_LENGTH (DEFAULT_MAX_BUTTON_LENGTH / 4)
#define DEFAULT_ICON_LUCENCY (50)
#define DEFAULT_ELLIPSIZE_MODE (PANGO_ELLIPSIZE_END)
#define DEFAULT_MENU_MAX_WIDTH_CHARS (24)
#define ARROW_BUTTON_SIZE (20)
#define WIREFRAME_SIZE (5) /* same as xfwm4 */
#define DRAG_ACTIVATE_TIMEOUT (500)

#define PANEL_HAS_FLAG(cont, fl) ((cont) & (fl))

/* locking helpers for tasklist->locked */
#define xfce_tasklist_get_panel_plugin(tasklist)                                                   \
	gtk_widget_get_ancestor(GTK_WIDGET(tasklist), vala_panel_applet_get_type())
#define xfce_tasklist_get_toplevel(tasklist)                                                       \
	gtk_widget_get_ancestor(GTK_WIDGET(tasklist), vala_panel_toplevel_get_type())
#define xfce_tasklist_horizontal(tasklist) ((tasklist)->mode == GTK_ORIENTATION_HORIZONTAL)
#define xfce_tasklist_vertical(tasklist) ((tasklist)->mode == GTK_ORIENTATION_VERTICAL)
#define xfce_tasklist_deskbar(tasklist) 0
#define xfce_tasklist_filter_monitors(tasklist)                                                    \
	(!(tasklist)->all_monitors && (tasklist)->n_monitors > 1)
#define xfce_tasklist_geometry_set_invalid(tasklist) ((tasklist)->n_monitors = 0)

enum
{
	PROP_0,
	PROP_GROUPING,
	PROP_INCLUDE_ALL_WORKSPACES,
	PROP_INCLUDE_ALL_MONITORS,
	PROP_FLAT_BUTTONS,
	PROP_SWITCH_WORKSPACE_ON_UNMINIMIZE,
	PROP_SHOW_LABELS,
	PROP_SHOW_ONLY_MINIMIZED,
	PROP_SHOW_WIREFRAMES,
	PROP_SORT_ORDER,
	PROP_WINDOW_SCROLLING,
	PROP_WRAP_WINDOWS,
	PROP_INCLUDE_ALL_BLINKING,
	PROP_MIDDLE_CLICK,
	PROP_LABEL_DECORATIONS,
	PROP_ALL,
};

static GParamSpec *props[PROP_ALL];

struct _XfceTasklistClass
{
	GtkContainerClass __parent__;
};

struct _XfceTasklist
{
	GtkContainer __parent__;

	/* the screen of this tasklist */
	WnckScreen *screen;
	GdkDisplay *gdk_display;

	/* window children in the tasklist */
	GList *windows;

	/* windows we monitor, but that are excluded from the tasklist */
	GSList *skipped_windows;

	/* arrow button of the overflow menu */
	GtkWidget *arrow_button;

	/* classgroups of all the windows in the taskbar */
	GHashTable *class_groups;

	/* normal or iconbox style */
	bool show_labels : 1;

	/* size of the panel plugin */
	int size;

	/* mode (orientation) of the tasklist */
	GtkOrientation mode;

	/* relief of the tasklist buttons */
	GtkReliefStyle button_relief;

	/* whether we show windows from all workspaces or
	 * only the active workspace */
	bool all_workspaces : 1;

	/* whether we switch to another workspace when we try to
	 * unminimize a window on another workspace */
	bool switch_workspace : 1;

	/* whether we only show monimized windows in the
	 * tasklist */
	bool only_minimized : 1;

	/* number of rows of window buttons */
	int nrows;

	/* switch window with the mouse wheel */
	bool window_scrolling : 1;
	bool wrap_windows : 1;

	/* whether we show blinking windows from all workspaces
	 * or only the active workspace */
	bool all_blinking : 1;

	/* action to preform when middle clicking */
	XfceTasklistMClick middle_click;

	/* whether decorate labels when window is not visible */
	bool label_decorations : 1;

	/* whether we only show windows that are in the geometry of
	 * the monitor the tasklist is on */
	bool all_monitors : 1;
	uint n_monitors;
	GdkMonitor *my_monitor;
	GdkRectangle *all_monitors_geometry;

	/* whether we show wireframes when hovering a button in
	 * the tasklist */
	bool show_wireframes : 1;

	/* icon geometries update timeout */
	uint update_icon_geometries_id;

	/* idle monitor geometry update */
	uint update_monitor_geometry_id;

	/* button grouping mode */
	XfceTasklistGrouping grouping;

	/* sorting order of the buttons */
	XfceTasklistSortOrder sort_order;

#ifdef GDK_WINDOWING_X11
	/* wireframe window */
	Window wireframe_window;
#endif

	/* gtk style properties */
	int max_button_length;
	int min_button_length;
	int max_button_size;
	PangoEllipsizeMode ellipsize_mode;
	int minimized_icon_lucency;
	int menu_max_width_chars;

	int n_windows;
};

typedef enum
{
	CHILD_TYPE_WINDOW,
	CHILD_TYPE_GROUP,
	CHILD_TYPE_OVERFLOW_MENU,
	CHILD_TYPE_GROUP_MENU
} XfceTasklistChildType;

typedef struct _XfceTasklistChild XfceTasklistChild;
struct _XfceTasklistChild
{
	/* type of this button */
	XfceTasklistChildType type;

	/* pointer to the tasklist */
	XfceTasklist *tasklist;

	/* button widgets */
	GtkWidget *button;
	GtkWidget *box;
	GtkWidget *icon;
	GtkWidget *label;

	/* drag motion window activate */
	uint motion_timeout_id;
	uint motion_timestamp;

	/* unique id for sorting by insert time,
	 * simply increased for each new button */
	uint unique_id;

	/* last time this window was focused */
	GTimeVal last_focused;

	/* list of windows in case of a group button */
	GSList *windows;

	/* wnck information */
	WnckWindow *window;
	WnckClassGroup *class_group;
};

static const GtkTargetEntry source_targets[] = { { "application/x-wnck-window-id", 0, 0 } };

static void xfce_tasklist_get_property(GObject *object, uint prop_id, GValue *value,
                                       GParamSpec *pspec);
static void xfce_tasklist_set_property(GObject *object, uint prop_id, const GValue *value,
                                       GParamSpec *pspec);
static void xfce_tasklist_finalize(GObject *object);
static void xfce_tasklist_get_preferred_width(GtkWidget *widget, int *minimum_width,
                                              int *natural_width);
static void xfce_tasklist_get_preferred_height(GtkWidget *widget, int *minimum_height,
                                               int *natural_height);
static void xfce_tasklist_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
static void xfce_tasklist_realize(GtkWidget *widget);
static void xfce_tasklist_unrealize(GtkWidget *widget);
static int xfce_tasklist_scroll_event(GtkWidget *widget, GdkEventScroll *event);
static void xfce_tasklist_remove(GtkContainer *container, GtkWidget *widget);
static void xfce_tasklist_forall(GtkContainer *container, int include_internals,
                                 GtkCallback callback, gpointer callback_data);
static GType xfce_tasklist_child_type(GtkContainer *container);
static void xfce_tasklist_arrow_button_toggled(GtkWidget *button, XfceTasklist *tasklist);
static void xfce_tasklist_connect_screen(XfceTasklist *tasklist);
static void xfce_tasklist_disconnect_screen(XfceTasklist *tasklist);
static void xfce_tasklist_gdk_monitors_changed(GdkDisplay *gdk_screen, GdkMonitor *mon,
                                               XfceTasklist *tasklist);
static void xfce_tasklist_active_window_changed(WnckScreen *screen, WnckWindow *previous_window,
                                                XfceTasklist *tasklist);
static void xfce_tasklist_active_workspace_changed(WnckScreen *screen,
                                                   WnckWorkspace *previous_workspace,
                                                   XfceTasklist *tasklist);
static void xfce_tasklist_window_added(WnckScreen *screen, WnckWindow *window,
                                       XfceTasklist *tasklist);
static void xfce_tasklist_window_removed(WnckScreen *screen, WnckWindow *window,
                                         XfceTasklist *tasklist);
static void xfce_tasklist_viewports_changed(WnckScreen *screen, XfceTasklist *tasklist);
static void xfce_tasklist_skipped_windows_state_changed(WnckWindow *window,
                                                        WnckWindowState changed_state,
                                                        WnckWindowState new_state,
                                                        XfceTasklist *tasklist);
static void xfce_tasklist_sort(XfceTasklist *tasklist);
static int xfce_tasklist_update_icon_geometries(gpointer data);
static void xfce_tasklist_update_icon_geometries_destroyed(gpointer data);

/* wireframe */
#ifdef GDK_WINDOWING_X11
static void xfce_tasklist_wireframe_hide(XfceTasklist *tasklist);
static void xfce_tasklist_wireframe_destroy(XfceTasklist *tasklist);
static void xfce_tasklist_wireframe_update(XfceTasklist *tasklist, XfceTasklistChild *child);
#endif

/* tasklist buttons */
static inline bool xfce_tasklist_button_visible(XfceTasklistChild *child, WnckWorkspace *active_ws);
static int xfce_tasklist_button_compare(gconstpointer child_a, gconstpointer child_b,
                                        gpointer user_data);
static GtkWidget *xfce_tasklist_button_proxy_menu_item(XfceTasklistChild *child,
                                                       bool allow_wireframe);
static void xfce_tasklist_button_activate(XfceTasklistChild *child, u_int32_t timestamp);
static XfceTasklistChild *xfce_tasklist_button_new(WnckWindow *window, XfceTasklist *tasklist);

/* tasklist group buttons */
static void xfce_tasklist_group_button_remove(XfceTasklistChild *group_child);
static void xfce_tasklist_group_button_add_window(XfceTasklistChild *group_child,
                                                  XfceTasklistChild *window_child);
static XfceTasklistChild *xfce_tasklist_group_button_new(WnckClassGroup *class_group,
                                                         XfceTasklist *tasklist);

G_DEFINE_TYPE(XfceTasklist, xfce_tasklist, GTK_TYPE_CONTAINER)

static void xfce_tasklist_class_init(XfceTasklistClass *klass)
{
	GObjectClass *gobject_class;
	GtkWidgetClass *gtkwidget_class;
	GtkContainerClass *gtkcontainer_class;

	gobject_class               = G_OBJECT_CLASS(klass);
	gobject_class->get_property = xfce_tasklist_get_property;
	gobject_class->set_property = xfce_tasklist_set_property;
	gobject_class->finalize     = xfce_tasklist_finalize;

	gtkwidget_class                       = GTK_WIDGET_CLASS(klass);
	gtkwidget_class->get_preferred_width  = xfce_tasklist_get_preferred_width;
	gtkwidget_class->get_preferred_height = xfce_tasklist_get_preferred_height;
	gtkwidget_class->size_allocate        = xfce_tasklist_size_allocate;
	gtkwidget_class->realize              = xfce_tasklist_realize;
	gtkwidget_class->unrealize            = xfce_tasklist_unrealize;
	gtkwidget_class->scroll_event         = xfce_tasklist_scroll_event;

	gtkcontainer_class             = GTK_CONTAINER_CLASS(klass);
	gtkcontainer_class->add        = NULL;
	gtkcontainer_class->remove     = xfce_tasklist_remove;
	gtkcontainer_class->forall     = xfce_tasklist_forall;
	gtkcontainer_class->child_type = xfce_tasklist_child_type;

	props[PROP_GROUPING] =
	    g_param_spec_uint("grouping",
	                      NULL,
	                      NULL,
	                      XFCE_TASKLIST_GROUPING_MIN,
	                      XFCE_TASKLIST_GROUPING_MAX + 1 /* TODO drop this later */,
	                      XFCE_TASKLIST_GROUPING_DEFAULT,
	                      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	props[PROP_INCLUDE_ALL_WORKSPACES] =
	    g_param_spec_boolean("include-all-workspaces",
	                         NULL,
	                         NULL,
	                         false,
	                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	props[PROP_INCLUDE_ALL_MONITORS] =
	    g_param_spec_boolean("include-all-monitors",
	                         NULL,
	                         NULL,
	                         true,
	                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	props[PROP_FLAT_BUTTONS] = g_param_spec_boolean("flat-buttons",
	                                                NULL,
	                                                NULL,
	                                                false,
	                                                G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	props[PROP_SWITCH_WORKSPACE_ON_UNMINIMIZE] =
	    g_param_spec_boolean("switch-workspace-on-unminimize",
	                         NULL,
	                         NULL,
	                         true,
	                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	props[PROP_SHOW_LABELS] = g_param_spec_boolean("show-labels",
	                                               NULL,
	                                               NULL,
	                                               true,
	                                               G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	props[PROP_SHOW_ONLY_MINIMIZED] =
	    g_param_spec_boolean("show-only-minimized",
	                         NULL,
	                         NULL,
	                         false,
	                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	props[PROP_SHOW_WIREFRAMES] =
	    g_param_spec_boolean("show-wireframes",
	                         NULL,
	                         NULL,
	                         false,
	                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	props[PROP_SORT_ORDER] = g_param_spec_uint("sort-order",
	                                           NULL,
	                                           NULL,
	                                           XFCE_TASKLIST_SORT_ORDER_MIN,
	                                           XFCE_TASKLIST_SORT_ORDER_MAX,
	                                           XFCE_TASKLIST_SORT_ORDER_DEFAULT,
	                                           G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	props[PROP_WINDOW_SCROLLING] =
	    g_param_spec_boolean("window-scrolling",
	                         NULL,
	                         NULL,
	                         true,
	                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	props[PROP_WRAP_WINDOWS] = g_param_spec_boolean("wrap-windows",
	                                                NULL,
	                                                NULL,
	                                                false,
	                                                G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	props[PROP_INCLUDE_ALL_BLINKING] =
	    g_param_spec_boolean("include-all-blinking",
	                         NULL,
	                         NULL,
	                         true,
	                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	props[PROP_MIDDLE_CLICK] = g_param_spec_uint("middle-click",
	                                             NULL,
	                                             NULL,
	                                             XFCE_TASKLIST_MIDDLE_CLICK_MIN,
	                                             XFCE_TASKLIST_MIDDLE_CLICK_MAX,
	                                             XFCE_TASKLIST_MIDDLE_CLICK_DEFAULT,
	                                             G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
	props[PROP_LABEL_DECORATIONS] =
	    g_param_spec_boolean("label-decorations",
	                         NULL,
	                         NULL,
	                         TRUE,
	                         G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);
	g_object_class_install_properties(gobject_class, PROP_ALL, props);
}

XfceTasklist *xfce_tasklist_new()
{
	return g_object_new(xfce_tasklist_get_type(), NULL);
}

static void xfce_tasklist_init(XfceTasklist *tasklist)
{
	gtk_widget_set_has_window(GTK_WIDGET(tasklist), false);

	tasklist->screen                = NULL;
	tasklist->windows               = NULL;
	tasklist->skipped_windows       = NULL;
	tasklist->mode                  = GTK_ORIENTATION_HORIZONTAL;
	tasklist->nrows                 = 1;
	tasklist->all_workspaces        = false;
	tasklist->button_relief         = GTK_RELIEF_NORMAL;
	tasklist->switch_workspace      = true;
	tasklist->only_minimized        = false;
	tasklist->show_labels           = true;
	tasklist->show_wireframes       = false;
	tasklist->all_monitors          = true;
	tasklist->n_monitors            = 0;
	tasklist->all_monitors_geometry = NULL;
	tasklist->window_scrolling      = true;
	tasklist->wrap_windows          = false;
	tasklist->all_blinking          = true;
	tasklist->middle_click          = XFCE_TASKLIST_MIDDLE_CLICK_DEFAULT;
	tasklist->label_decorations     = true;
	xfce_tasklist_geometry_set_invalid(tasklist);
#ifdef GDK_WINDOWING_X11
	tasklist->wireframe_window = 0;
#endif
	tasklist->update_icon_geometries_id  = 0;
	tasklist->update_monitor_geometry_id = 0;
	tasklist->max_button_length          = DEFAULT_MAX_BUTTON_LENGTH;
	tasklist->min_button_length          = DEFAULT_MIN_BUTTON_LENGTH;
	tasklist->max_button_size            = DEFAULT_BUTTON_SIZE;
	tasklist->minimized_icon_lucency     = DEFAULT_ICON_LUCENCY;
	tasklist->ellipsize_mode             = DEFAULT_ELLIPSIZE_MODE;
	tasklist->grouping                   = XFCE_TASKLIST_GROUPING_DEFAULT;
	tasklist->sort_order                 = XFCE_TASKLIST_SORT_ORDER_DEFAULT;
	tasklist->menu_max_width_chars       = DEFAULT_MENU_MAX_WIDTH_CHARS;
	tasklist->class_groups =
	    g_hash_table_new_full(g_direct_hash,
	                          g_direct_equal,
	                          (GDestroyNotify)g_object_unref,
	                          (GDestroyNotify)xfce_tasklist_group_button_remove);

	/* add style class for the tasklist widget */
	GtkStyleContext *context = gtk_widget_get_style_context(GTK_WIDGET(tasklist));
	gtk_style_context_add_class(context, "tasklist");

	/* widgets for the overflow menu */
	/* TODO support drag-motion and drag-leave */
	tasklist->arrow_button = xfce_arrow_button_new(GTK_ARROW_DOWN);
	gtk_widget_set_parent(tasklist->arrow_button, GTK_WIDGET(tasklist));
	gtk_widget_set_name(tasklist->arrow_button, "panel-tasklist-arrow");
	gtk_button_set_relief(GTK_BUTTON(tasklist->arrow_button), tasklist->button_relief);
	g_signal_connect(G_OBJECT(tasklist->arrow_button),
	                 "toggled",
	                 G_CALLBACK(xfce_tasklist_arrow_button_toggled),
	                 tasklist);
}

static void xfce_tasklist_get_property(GObject *object, uint prop_id, GValue *value,
                                       GParamSpec *pspec)
{
	XfceTasklist *tasklist = XFCE_TASKLIST(object);

	switch (prop_id)
	{
	case PROP_GROUPING:
		g_value_set_uint(value, tasklist->grouping);
		break;

	case PROP_INCLUDE_ALL_WORKSPACES:
		g_value_set_boolean(value, tasklist->all_workspaces);
		break;

	case PROP_INCLUDE_ALL_MONITORS:
		g_value_set_boolean(value, tasklist->all_monitors);
		break;

	case PROP_FLAT_BUTTONS:
		g_value_set_boolean(value, !!(tasklist->button_relief == GTK_RELIEF_NONE));
		break;

	case PROP_SWITCH_WORKSPACE_ON_UNMINIMIZE:
		g_value_set_boolean(value, tasklist->switch_workspace);
		break;

	case PROP_SHOW_LABELS:
		g_value_set_boolean(value, tasklist->show_labels);
		break;

	case PROP_SHOW_ONLY_MINIMIZED:
		g_value_set_boolean(value, tasklist->only_minimized);
		break;

	case PROP_SHOW_WIREFRAMES:
		g_value_set_boolean(value, tasklist->show_wireframes);
		break;

	case PROP_SORT_ORDER:
		g_value_set_uint(value, tasklist->sort_order);
		break;

	case PROP_WINDOW_SCROLLING:
		g_value_set_boolean(value, tasklist->window_scrolling);
		break;

	case PROP_WRAP_WINDOWS:
		g_value_set_boolean(value, tasklist->wrap_windows);
		break;

	case PROP_INCLUDE_ALL_BLINKING:
		g_value_set_boolean(value, tasklist->all_blinking);
		break;

	case PROP_MIDDLE_CLICK:
		g_value_set_uint(value, tasklist->middle_click);
		break;

	case PROP_LABEL_DECORATIONS:
		g_value_set_boolean(value, tasklist->label_decorations);
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void xfce_tasklist_set_property(GObject *object, uint prop_id, const GValue *value,
                                       GParamSpec *pspec)
{
	XfceTasklist *tasklist = XFCE_TASKLIST(object);
	XfceTasklistSortOrder sort_order;

	switch (prop_id)
	{
	case PROP_GROUPING:
		xfce_tasklist_set_grouping(tasklist, g_value_get_uint(value));
		break;

	case PROP_INCLUDE_ALL_WORKSPACES:
		xfce_tasklist_set_include_all_workspaces(tasklist, g_value_get_boolean(value));
		break;

	case PROP_INCLUDE_ALL_MONITORS:
		xfce_tasklist_set_include_all_monitors(tasklist, g_value_get_boolean(value));
		break;

	case PROP_FLAT_BUTTONS:
		xfce_tasklist_set_button_relief(tasklist,
		                                g_value_get_boolean(value) ? GTK_RELIEF_NONE
		                                                           : GTK_RELIEF_NORMAL);
		break;

	case PROP_SHOW_LABELS:
		xfce_tasklist_set_show_labels(tasklist, g_value_get_boolean(value));
		break;

	case PROP_SWITCH_WORKSPACE_ON_UNMINIMIZE:
		tasklist->switch_workspace = g_value_get_boolean(value);
		break;

	case PROP_SHOW_ONLY_MINIMIZED:
		xfce_tasklist_set_show_only_minimized(tasklist, g_value_get_boolean(value));
		break;

	case PROP_SHOW_WIREFRAMES:
		xfce_tasklist_set_show_wireframes(tasklist, g_value_get_boolean(value));
		break;

	case PROP_SORT_ORDER:
		sort_order = g_value_get_uint(value);
		if (tasklist->sort_order != sort_order)
		{
			tasklist->sort_order = sort_order;
			xfce_tasklist_sort(tasklist);
		}
		break;

	case PROP_WINDOW_SCROLLING:
		tasklist->window_scrolling = g_value_get_boolean(value);
		break;

	case PROP_WRAP_WINDOWS:
		tasklist->wrap_windows = g_value_get_boolean(value);
		break;

	case PROP_INCLUDE_ALL_BLINKING:
		tasklist->all_blinking = g_value_get_boolean(value);
		break;

	case PROP_MIDDLE_CLICK:
		tasklist->middle_click = g_value_get_uint(value);
		break;

	case PROP_LABEL_DECORATIONS:
		xfce_tasklist_set_label_decorations(tasklist, g_value_get_boolean(value));
		break;

	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
		break;
	}
}

static void xfce_tasklist_finalize(GObject *object)
{
	XfceTasklist *tasklist = XFCE_TASKLIST(object);

	/* data that should already be freed when disconnecting the screen */
	g_return_if_fail(tasklist->windows == NULL);
	g_return_if_fail(tasklist->skipped_windows == NULL);
	g_return_if_fail(tasklist->screen == NULL);

	/* stop pending timeouts */
	if (tasklist->update_icon_geometries_id != 0)
		g_source_remove(tasklist->update_icon_geometries_id);
	if (tasklist->update_monitor_geometry_id != 0)
		g_source_remove(tasklist->update_monitor_geometry_id);

	/* free the class group hash table */
	g_hash_table_destroy(tasklist->class_groups);

#ifdef GDK_WINDOWING_X11
	/* destroy the wireframe window */
	xfce_tasklist_wireframe_destroy(tasklist);
#endif

	(*G_OBJECT_CLASS(xfce_tasklist_parent_class)->finalize)(object);
}

static void xfce_tasklist_measure(GtkWidget *widget, GtkOrientation orientation,
                                  G_GNUC_UNUSED int for_size, int *minimum, int *natural,
                                  G_GNUC_UNUSED int *minimum_baseline,
                                  G_GNUC_UNUSED int *natural_baseline)
{
	XfceTasklist *tasklist = XFCE_TASKLIST(widget);
	ValaPanelToplevel *top = VALA_PANEL_TOPLEVEL(xfce_tasklist_get_toplevel(tasklist));
	int base_height        = DEFAULT_BUTTON_SIZE;
	int icon_size          = GTK_ICON_SIZE_MENU;
	int mod                = 0;
	if (top)
	{
		int height;
		g_object_get(top, VP_KEY_ICON_SIZE, &icon_size, VP_KEY_HEIGHT, &height, NULL);
		tasklist->nrows = (int)floor(height / (float)icon_size);
		tasklist->nrows = tasklist->nrows < 1 ? 1 : tasklist->nrows;
		base_height     = height;
		mod             = (int)ceil(height % icon_size) / tasklist->nrows;
	}

	if (tasklist->mode == orientation)
	{
		int n_windows;
		GtkRequisition child_req;
		int length;
		GList *li;
		XfceTasklistChild *child;
		int child_height = 0;

		for (li = tasklist->windows, n_windows = 0; li != NULL; li = li->next)
		{
			child = li->data;

			if (gtk_widget_get_visible(child->button))
			{
				gtk_widget_get_preferred_size(child->button, NULL, &child_req);

				child_height = MAX(child_height, child_req.height);

				if (child->type == CHILD_TYPE_GROUP_MENU)
					continue;

				n_windows++;
			}
		}
		child_height = MAX(child_height, icon_size + mod);
		if (tasklist->show_labels)
			child_height = MAX(child_height, tasklist->max_button_length);

		tasklist->n_windows = n_windows;

		length = (int)ceil((n_windows * child_height) / tasklist->nrows);

		tasklist->size = length;

		/* set the requested sizes */
		if (natural != NULL)
			*natural = length > ARROW_BUTTON_SIZE ? length : ARROW_BUTTON_SIZE;

		if (minimum != NULL)
			*minimum = (n_windows == 0) ? 0 : ARROW_BUTTON_SIZE;
	}
	else
	{
		*minimum = base_height;
		*natural = base_height;
	}
}

static void xfce_tasklist_get_preferred_width(GtkWidget *widget, int *minimum_width,
                                              int *natural_width)
{
	int x, y;
	xfce_tasklist_measure(widget,
	                      GTK_ORIENTATION_HORIZONTAL,
	                      -1,
	                      minimum_width,
	                      natural_width,
	                      &x,
	                      &y);
}

static void xfce_tasklist_get_preferred_height(GtkWidget *widget, int *minimum_height,
                                               int *natural_height)
{
	int x, y;
	xfce_tasklist_measure(widget,
	                      GTK_ORIENTATION_VERTICAL,
	                      -1,
	                      minimum_height,
	                      natural_height,
	                      &x,
	                      &y);
}

static int xfce_tasklist_size_sort_window(gconstpointer a, gconstpointer b)
{
	const XfceTasklistChild *child_a = a;
	const XfceTasklistChild *child_b = b;
	glong diff;

	diff = child_a->last_focused.tv_sec - child_b->last_focused.tv_sec;
	if (diff != 0)
		return CLAMP(diff, -1, 1);

	diff = child_a->last_focused.tv_usec - child_b->last_focused.tv_usec;
	return CLAMP(diff, -1, 1);
}

static void xfce_tasklist_size_layout(XfceTasklist *tasklist, GtkAllocation *alloc, int *n_rows,
                                      int *n_cols, int *arrow_position)
{
	int rows;
	int min_button_length;
	int cols;
	GSList *windows_scored = NULL, *lp;
	GList *li;
	XfceTasklistChild *child;
	int max_button_length;
	int n_buttons;
	int n_buttons_target;

	/* if we're in deskbar mode, there are no columns */
	if (xfce_tasklist_deskbar(tasklist) && tasklist->show_labels)
		rows = 1;
	else
		rows = tasklist->nrows;

	if (rows < 1)
		rows = 1;

	cols = tasklist->n_windows / rows;
	if (cols * rows < tasklist->n_windows)
		cols++;

	if (xfce_tasklist_deskbar(tasklist) && tasklist->show_labels)
		min_button_length = MIN(alloc->height / tasklist->nrows, tasklist->max_button_size);
	else if (!tasklist->show_labels)
		min_button_length = alloc->height / tasklist->nrows;
	else
		min_button_length = tasklist->min_button_length;

	*arrow_position = -1; /* not visible */

	/* unset overflow items, we decide about that again
	 * later */
	for (li = tasklist->windows; li != NULL; li = li->next)
	{
		child = li->data;
		if (child->type == CHILD_TYPE_OVERFLOW_MENU)
			child->type = CHILD_TYPE_WINDOW;
	}

	if (min_button_length * cols <= alloc->width)
	{
		/* all the windows seem to fit */
		*n_rows = rows;
		*n_cols = cols;
	}
	else
	{
		/* we need to group something, first create a list with the
		 * windows most suitable for grouping at the beginning that are
		 * (should be) currently visible */
		for (li = tasklist->windows; li != NULL; li = li->next)
		{
			child = li->data;
			if (gtk_widget_get_visible(child->button))
			{
				windows_scored =
				    g_slist_insert_sorted(windows_scored,
				                          child,
				                          xfce_tasklist_size_sort_window);
			}
		}

		if (xfce_tasklist_deskbar(tasklist) || !tasklist->show_labels)
			max_button_length = min_button_length;
		else if (tasklist->max_button_length != -1)
			max_button_length = tasklist->max_button_length;
		else
			max_button_length = DEFAULT_MAX_BUTTON_LENGTH;

		n_buttons = tasklist->n_windows;
		/* Matches the existing behavior (with a bug fix) */
		/* n_buttons_target = MIN ((alloc->width - ARROW_BUTTON_SIZE) / min_button_length *
		 * rows,          *
		 *                         (((alloc->width - ARROW_BUTTON_SIZE) / max_button_length)
		 * + 1) * rows); */

		/* Perhaps a better behavior (tries to display more buttons on the panel, */
		/* yet still within the specified limits) */
		n_buttons_target = (alloc->width - ARROW_BUTTON_SIZE) / min_button_length * rows;

#if 0
      if (tasklist->grouping == XFCE_TASKLIST_GROUPING_AUTO)
        {
          /* try creating group buttons */
        }
#endif

		/* we now push the windows with the lowest score in the
		 * overflow menu */
		if (n_buttons > n_buttons_target)
		{
			for (lp = windows_scored; n_buttons > n_buttons_target && lp != NULL;
			     lp = lp->next, n_buttons--)
			{
				child = lp->data;

				if (child->type == CHILD_TYPE_WINDOW)
					child->type = CHILD_TYPE_OVERFLOW_MENU;
			}

			/* Try to position the arrow widget at the end of the allocation area  *
			 * if that's impossible (because buttons cannot be expanded enough)    *
			 * position it just after the buttons.                                 */
			*arrow_position = MIN(alloc->width - ARROW_BUTTON_SIZE,
			                      n_buttons_target * max_button_length / rows);
		}

		g_slist_free(windows_scored);

		cols = n_buttons / rows;
		if (cols * rows < n_buttons)
			cols++;

		*n_rows = rows;
		*n_cols = cols;
	}
}

static void xfce_tasklist_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
	XfceTasklist *tasklist = XFCE_TASKLIST(widget);
	int rows, cols;
	int row;
	GtkAllocation area = *allocation;
	GList *li;
	XfceTasklistChild *child;
	int i;
	GtkAllocation child_alloc;
	bool direction_rtl = gtk_widget_get_direction(widget) == GTK_TEXT_DIR_RTL;
	int w, x, y, h;
	int area_x, area_width;
	int arrow_position;
	GtkRequisition child_req;

	/* set widget allocation */
	gtk_widget_set_allocation(widget, allocation);
	/* swap integers with vertical orientation */
	if (!xfce_tasklist_horizontal(tasklist))
		vala_panel_transpose_area(area);
	if (area.height > MAX_PANEL_HEIGHT)
	{
		area.height = MIN(area.height, area.width);
		area.width  = MIN(area.height, area.width);
	}
	//  g_return_if_fail (area.height == tasklist->size);

	/* TODO if we compare the allocation with the requisition we can
	 * do a fast path to the child allocation, i think */

	/* useless but hides compiler warning */
	w = h = x = y = rows = cols = 0;

	xfce_tasklist_size_layout(tasklist, &area, &rows, &cols, &arrow_position);

	/* allocate the arrow button for the overflow menu */
	child_alloc.width  = ARROW_BUTTON_SIZE;
	child_alloc.height = area.height;

	if (arrow_position != -1)
	{
		child_alloc.x = area.x;
		child_alloc.y = area.y;

		if (!direction_rtl)
			child_alloc.x += arrow_position;
		else
			child_alloc.x += (area.width - arrow_position);

		area.width = arrow_position;

		/* position the arrow in the correct position */
		if (!xfce_tasklist_horizontal(tasklist))
			vala_panel_transpose_area(child_alloc);
	}
	else
	{
		child_alloc.x = child_alloc.y = -9999;
	}

	if (gtk_widget_get_visible(tasklist->arrow_button))
	{
		gtk_widget_size_allocate(tasklist->arrow_button, &child_alloc);
	}

	area_x     = area.x;
	area_width = area.width;
	h          = area.height / rows;
	/* allocate all the children */
	for (li = tasklist->windows, i = 0; li != NULL; li = li->next)
	{
		child = li->data;

		/* skip hidden buttons */
		if (!gtk_widget_get_visible(child->button))
			continue;

		if (G_LIKELY(child->type == CHILD_TYPE_WINDOW || child->type == CHILD_TYPE_GROUP))
		{
			row = (i % rows);
			if (row == 0)
			{
				x = area_x;
				y = area.y;

				if (xfce_tasklist_vertical(tasklist) &&
				    tasklist->show_labels) /* position the arrow in the correct
				                              position */

				{
					/* fixed width is OK because area.width==w*cols */
					w = MIN(area.height / tasklist->nrows,
					        tasklist->max_button_size);
				}
				else if (tasklist->show_labels)
				{
					/* TODO, this is a work-around, something else goes wrong
					 * with counting the windows... */
					if (cols < 1)
						cols = 1;
					w = area_width / cols--;
					if (tasklist->max_button_length > 0 &&
					    w > tasklist->max_button_length)
						w = tasklist->max_button_length;
				}
				else /* buttons without labels */
				{
					w = h;
				}

				area_width -= w;
				area_x += w;
			}

			child_alloc.y      = y;
			child_alloc.x      = x;
			child_alloc.width  = MAX(w, 1); /* TODO this is a workaround */
			child_alloc.height = h;

			y += h;

			if (direction_rtl)
				child_alloc.x = area.x + area.width - (child_alloc.x - area.x) -
				                child_alloc.width;

			/* allocate the child */
			if (!xfce_tasklist_horizontal(tasklist))
				vala_panel_transpose_area(child_alloc);

			/* increase the position counter */
			i++;
		}
		else
		{
			gtk_widget_get_preferred_size(child->button, NULL, &child_req);

			/* move the button offscreen */
			child_alloc.y = child_alloc.x = -9999;
			child_alloc.width             = child_req.width;
			child_alloc.height            = child_req.height;
		}

		gtk_widget_size_allocate(child->button, &child_alloc);
	}

	/* update icon geometries */
	if (tasklist->update_icon_geometries_id == 0)
		tasklist->update_icon_geometries_id =
		    g_idle_add_full(G_PRIORITY_LOW,
		                    xfce_tasklist_update_icon_geometries,
		                    tasklist,
		                    xfce_tasklist_update_icon_geometries_destroyed);
}

/*static void xfce_tasklist_style_set(GtkWidget *widget, GtkStyle *previous_style)
{
        XfceTasklist *tasklist = XFCE_TASKLIST(widget);
        int max_button_length;
        int max_button_size;
        int min_button_length;

    // let gtk update the widget style
        (*GTK_WIDGET_CLASS(xfce_tasklist_parent_class)->style_set)(widget, previous_style);

    // read the style properties
        gtk_widget_style_get(GTK_WIDGET(tasklist),
                             "max-button-length",
                             &max_button_length,
                             "min-button-length",
                             &min_button_length,
                             "ellipsize-mode",
                             &tasklist->ellipsize_mode,
                             "max-button-size",
                             &max_button_size,
                             "minimized-icon-lucency",
                             &tasklist->minimized_icon_lucency,
                             "menu-max-width-chars",
                             &tasklist->menu_max_width_chars,
                             NULL);

    // update the widget
        if (tasklist->max_button_length != max_button_length ||
            tasklist->max_button_size != max_button_size ||
            tasklist->min_button_length != min_button_length)
        {
                if (max_button_length > 0)
                {
            // prevent abuse of the min/max button length
                        tasklist->max_button_length = MAX(min_button_length, max_button_length);
                        tasklist->min_button_length = MIN(min_button_length, max_button_length);
                }
                else
                {
                        tasklist->max_button_length = max_button_length;
                        tasklist->min_button_length = min_button_length;
                }

                tasklist->max_button_size = max_button_size;

                gtk_widget_queue_resize(widget);
        }
} */

static void xfce_tasklist_realize(GtkWidget *widget)
{
	XfceTasklist *tasklist = XFCE_TASKLIST(widget);

	(*GTK_WIDGET_CLASS(xfce_tasklist_parent_class)->realize)(widget);
	gtk_widget_show(tasklist->arrow_button);
	/* we now have a screen */
	xfce_tasklist_connect_screen(tasklist);
}

static void xfce_tasklist_unrealize(GtkWidget *widget)
{
	XfceTasklist *tasklist = XFCE_TASKLIST(widget);

	/* we're going to loose the screen */
	xfce_tasklist_disconnect_screen(tasklist);

	(*GTK_WIDGET_CLASS(xfce_tasklist_parent_class)->unrealize)(widget);
}

static int xfce_tasklist_scroll_event(GtkWidget *widget, GdkEventScroll *event)
{
	XfceTasklist *tasklist   = XFCE_TASKLIST(widget);
	XfceTasklistChild *child = NULL;
	GList *li, *lnew = NULL;
	GdkScrollDirection scrolling_direction;

	if (!tasklist->window_scrolling)
		return true;

	/* get the current active button */
	for (li = tasklist->windows; li != NULL; li = li->next)
	{
		child = li->data;

		if (gtk_widget_get_visible(child->button) &&
		    gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(child->button)))
			break;
	}

	if (G_UNLIKELY(li == NULL))
		return true;

	if (event->direction != GDK_SCROLL_SMOOTH)
		scrolling_direction = event->direction;
	else if (event->delta_y < 0)
		scrolling_direction = GDK_SCROLL_UP;
	else if (event->delta_y > 0)
		scrolling_direction = GDK_SCROLL_DOWN;
	else if (event->delta_x < 0)
		scrolling_direction = GDK_SCROLL_LEFT;
	else if (event->delta_x > 0)
		scrolling_direction = GDK_SCROLL_RIGHT;
	else
	{
		g_debug("tasklist-xfce: scrolling event with no delta happened");
		return TRUE;
	}

	switch (scrolling_direction)
	{
	case GDK_SCROLL_UP:
		/* find previous button on the tasklist */
		for (lnew = g_list_previous(li); lnew != NULL; lnew = lnew->prev)
		{
			child = lnew->data;
			if (child->window != NULL && gtk_widget_get_visible(child->button))
				break;
		}

		/* wrap if the first button is reached */
		lnew = (lnew == NULL && tasklist->wrap_windows) ? g_list_last(li) : lnew;
		break;

	case GDK_SCROLL_DOWN:
		/* find the next button on the tasklist */
		for (lnew = g_list_next(li); lnew != NULL; lnew = lnew->next)
		{
			child = lnew->data;
			if (child->window != NULL && gtk_widget_get_visible(child->button))
				break;
		}

		/* wrap if the last button is reached */
		lnew = (lnew == NULL && tasklist->wrap_windows) ? g_list_first(li) : lnew;
		break;

	case GDK_SCROLL_LEFT:
		/* TODO */
		break;

	case GDK_SCROLL_RIGHT:
		/* TODO */
		break;

	case GDK_SCROLL_SMOOTH:
		/* TODO */
		break;
	}

	if (lnew != NULL)
		xfce_tasklist_button_activate(lnew->data, event->time);

	return true;
}

static void xfce_tasklist_remove(GtkContainer *container, GtkWidget *widget)
{
	XfceTasklist *tasklist = XFCE_TASKLIST(container);
	bool was_visible;
	GList *li = NULL;

	for (li = tasklist->windows; li != NULL; li = li->next)
	{
		XfceTasklistChild *child = li->data;

		if (child->button == widget)
		{
			tasklist->windows = g_list_delete_link(tasklist->windows, li);

			was_visible = gtk_widget_get_visible(widget);

			gtk_widget_unparent(child->button);

			if (child->motion_timeout_id != 0)
				g_source_remove(child->motion_timeout_id);

			g_slice_free(XfceTasklistChild, child);

			/* queue a resize if needed */
			if (G_LIKELY(was_visible))
				gtk_widget_queue_resize(GTK_WIDGET(container));

			break;
		}
	}
}

static void xfce_tasklist_forall(GtkContainer *container, int include_internals,
                                 GtkCallback callback, gpointer callback_data)
{
	XfceTasklist *tasklist = XFCE_TASKLIST(container);
	GList *children        = tasklist->windows;
	XfceTasklistChild *child;

	if (include_internals)
		(*callback)(tasklist->arrow_button, callback_data);

	while (children != NULL)
	{
		child    = children->data;
		children = children->next;

		(*callback)(child->button, callback_data);
	}
}

static GType xfce_tasklist_child_type(G_GNUC_UNUSED GtkContainer *container)
{
	return GTK_TYPE_WIDGET;
}

static void xfce_tasklist_arrow_button_menu_destroy(GtkWidget *menu, XfceTasklist *tasklist)
{
	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));
	g_return_if_fail(GTK_IS_TOGGLE_BUTTON(tasklist->arrow_button));
	g_return_if_fail(GTK_IS_WIDGET(menu));

	gtk_widget_destroy(menu);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tasklist->arrow_button), false);

#ifdef GDK_WINDOWING_X11
	/* make sure the wireframe is hidden */
	xfce_tasklist_wireframe_hide(tasklist);
#endif
}

static void xfce_tasklist_arrow_button_toggled(GtkWidget *button, XfceTasklist *tasklist)
{
	GList *li;
	XfceTasklistChild *child;
	GtkWidget *mi;
	GtkWidget *menu;

	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));
	g_return_if_fail(GTK_IS_TOGGLE_BUTTON(button));
	g_return_if_fail(tasklist->arrow_button == button);

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
	{
		menu = gtk_menu_new();
		g_signal_connect(G_OBJECT(menu),
		                 "selection-done",
		                 G_CALLBACK(xfce_tasklist_arrow_button_menu_destroy),
		                 tasklist);

		for (li = tasklist->windows; li != NULL; li = li->next)
		{
			child = li->data;

			if (child->type != CHILD_TYPE_OVERFLOW_MENU)
				continue;

			mi = xfce_tasklist_button_proxy_menu_item(child, true);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);
			gtk_widget_show(mi);
		}

		gtk_menu_attach_to_widget(GTK_MENU(menu), button, NULL);
		gtk_menu_popup_at_widget(GTK_MENU(menu),
		                         button,
		                         xfce_tasklist_vertical(tasklist) ? GDK_GRAVITY_WEST
		                                                          : GDK_GRAVITY_NORTH_EAST,
		                         xfce_tasklist_vertical(tasklist) ? GDK_GRAVITY_EAST
		                                                          : GDK_GRAVITY_SOUTH_EAST,
		                         NULL);
	}
}

static void xfce_tasklist_connect_screen(XfceTasklist *tasklist)
{
	GList *windows, *li;

	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));
	g_return_if_fail(tasklist->screen == NULL);
	g_return_if_fail(tasklist->gdk_display == NULL);

	/* set the new screen */
	tasklist->gdk_display = gtk_widget_get_display(GTK_WIDGET(tasklist));
	tasklist->screen      = wnck_screen_get_default();

	/* add all existing windows on this screen */
	windows = wnck_screen_get_windows(tasklist->screen);
	for (li = windows; li != NULL; li = li->next)
		xfce_tasklist_window_added(tasklist->screen, li->data, tasklist);

	/* monitor gdk changes */
	g_signal_connect(G_OBJECT(tasklist->gdk_display),
	                 "monitor-added",
	                 G_CALLBACK(xfce_tasklist_gdk_monitors_changed),
	                 tasklist);
	g_signal_connect(G_OBJECT(tasklist->gdk_display),
	                 "monitor-removed",
	                 G_CALLBACK(xfce_tasklist_gdk_monitors_changed),
	                 tasklist);

	/* monitor screen changes */
	g_signal_connect(G_OBJECT(tasklist->screen),
	                 "active-window-changed",
	                 G_CALLBACK(xfce_tasklist_active_window_changed),
	                 tasklist);
	g_signal_connect(G_OBJECT(tasklist->screen),
	                 "active-workspace-changed",
	                 G_CALLBACK(xfce_tasklist_active_workspace_changed),
	                 tasklist);
	g_signal_connect(G_OBJECT(tasklist->screen),
	                 "window-opened",
	                 G_CALLBACK(xfce_tasklist_window_added),
	                 tasklist);
	g_signal_connect(G_OBJECT(tasklist->screen),
	                 "window-closed",
	                 G_CALLBACK(xfce_tasklist_window_removed),
	                 tasklist);
	g_signal_connect(G_OBJECT(tasklist->screen),
	                 "viewports-changed",
	                 G_CALLBACK(xfce_tasklist_viewports_changed),
	                 tasklist);

	/* update the viewport if not all monitors are shown */
	xfce_tasklist_gdk_monitors_changed(tasklist->gdk_display, NULL, tasklist);
}

static void xfce_tasklist_disconnect_screen(XfceTasklist *tasklist)
{
	GSList *li, *lnext;
	GList *wi, *wnext;
	XfceTasklistChild *child;
	uint n = 5;

	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));
	g_return_if_fail(WNCK_IS_SCREEN(tasklist->screen));
	g_return_if_fail(GDK_IS_DISPLAY(tasklist->gdk_display));

	/* disconnect monitor signals */
	g_signal_handlers_disconnect_by_data(tasklist->screen, tasklist);
	g_signal_handlers_disconnect_by_data(tasklist->gdk_display, tasklist);
	g_return_if_fail(n == 5);

	/* delete all known class groups (and their buttons) */
	g_hash_table_remove_all(tasklist->class_groups);

	/* disconnect from all skipped windows */
	for (li = tasklist->skipped_windows; li != NULL; li = lnext)
	{
		lnext = li->next;
		g_return_if_fail(wnck_window_is_skip_tasklist(WNCK_WINDOW(li->data)));
		xfce_tasklist_window_removed(tasklist->screen, li->data, tasklist);
	}

	/* remove all the windows */
	for (wi = tasklist->windows; wi != NULL; wi = wnext)
	{
		wnext = wi->next;
		child = wi->data;

		/* do a fake window remove */
		g_return_if_fail(child->type != CHILD_TYPE_GROUP);
		g_return_if_fail(WNCK_IS_WINDOW(child->window));
		xfce_tasklist_window_removed(tasklist->screen, child->window, tasklist);
	}

	g_assert(tasklist->windows == NULL);
	g_assert(tasklist->skipped_windows == NULL);

	tasklist->screen      = NULL;
	tasklist->gdk_display = NULL;
}

static void xfce_tasklist_gdk_monitors_changed(GdkDisplay *gdk_screen,
                                               G_GNUC_UNUSED GdkMonitor *mon,
                                               XfceTasklist *tasklist)
{
	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));
	g_return_if_fail(GDK_IS_DISPLAY(gdk_screen));

	if (!tasklist->all_monitors)
	{
		/* update the monitor geometry */
		xfce_tasklist_update_monitor_geometry(tasklist);
	}
}

static void xfce_tasklist_active_window_changed(WnckScreen *screen, WnckWindow *previous_window,
                                                XfceTasklist *tasklist)
{
	WnckWindow *active_window;
	GList *li;
	XfceTasklistChild *child;

	g_return_if_fail(WNCK_IS_SCREEN(screen));
	g_return_if_fail(previous_window == NULL || WNCK_IS_WINDOW(previous_window));
	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));
	g_return_if_fail(tasklist->screen == screen);

	/* get the new active window */
	active_window = wnck_screen_get_active_window(screen);

	for (li = tasklist->windows; li != NULL; li = li->next)
	{
		child = li->data;

		/* update timestamp for window */
		if (child->window == active_window)
			g_get_current_time(&child->last_focused);

		/* set the toggle button state */
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(child->button),
		                             !!(child->window == active_window));
	}
}

static void xfce_tasklist_active_workspace_changed(WnckScreen *screen,
                                                   WnckWorkspace *previous_workspace,
                                                   XfceTasklist *tasklist)
{
	GList *li;
	WnckWorkspace *active_ws;
	XfceTasklistChild *child;

	g_return_if_fail(WNCK_IS_SCREEN(screen));
	g_return_if_fail(previous_workspace == NULL || WNCK_IS_WORKSPACE(previous_workspace));
	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));
	g_return_if_fail(tasklist->screen == screen);

	/* leave when we are locked or show all workspaces. the null
	 * check for @previous_workspace is used to update the tasklist
	 * on setting changes */
	if ((previous_workspace != NULL && tasklist->all_workspaces))
		return;

	/* walk all the children and update their visibility */
	active_ws = wnck_screen_get_active_workspace(screen);
	for (li = tasklist->windows; li != NULL; li = li->next)
	{
		child = li->data;

		if (child->type != CHILD_TYPE_GROUP)
		{
			if (xfce_tasklist_button_visible(child, active_ws))
				gtk_widget_show(child->button);
			else
				gtk_widget_hide(child->button);
		}
	}
}

static void xfce_tasklist_window_added(WnckScreen *screen, WnckWindow *window,
                                       XfceTasklist *tasklist)
{
	XfceTasklistChild *child;
	XfceTasklistChild *group_child = NULL;
	bool found;

	g_return_if_fail(WNCK_IS_SCREEN(screen));
	g_return_if_fail(WNCK_IS_WINDOW(window));
	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));
	g_return_if_fail(tasklist->screen == screen);
	g_return_if_fail(wnck_window_get_screen(window) == screen);

	/* ignore this window, but watch it for state changes */
	if (wnck_window_is_skip_tasklist(window))
	{
		tasklist->skipped_windows = g_slist_prepend(tasklist->skipped_windows, window);
		g_signal_connect(G_OBJECT(window),
		                 "state-changed",
		                 G_CALLBACK(xfce_tasklist_skipped_windows_state_changed),
		                 tasklist);

		return;
	}

	/* create new window button */
	child = xfce_tasklist_button_new(window, tasklist);

	/* initial visibility of the function */
	if (xfce_tasklist_button_visible(child, wnck_screen_get_active_workspace(screen)))
		gtk_widget_show(child->button);

	if (G_LIKELY(child->class_group != NULL))
	{
		/* we need to ref the class group else the value returned from
		 * wnck_window_get_class_group() is null */
		g_return_if_fail(WNCK_IS_CLASS_GROUP(child->class_group));
		g_object_ref(G_OBJECT(child->class_group));

		found = g_hash_table_lookup_extended(tasklist->class_groups,
		                                     child->class_group,
		                                     NULL,
		                                     (gpointer *)&group_child);

		if (G_UNLIKELY(tasklist->grouping == XFCE_TASKLIST_GROUPING_ALWAYS))
		{
			if (group_child == NULL)
			{
				/* create group button for this window and add it */
				group_child =
				    xfce_tasklist_group_button_new(child->class_group, tasklist);
				g_hash_table_insert(tasklist->class_groups,
				                    g_object_ref(child->class_group),
				                    group_child);
			}

			/* add window to the group button */
			xfce_tasklist_group_button_add_window(group_child, child);
		}
		else if (!found)
		{
			/* add group in hash table without button */
			g_hash_table_insert(tasklist->class_groups,
			                    g_object_ref(child->class_group),
			                    NULL);
		}
	}

	gtk_widget_queue_resize(GTK_WIDGET(tasklist));
}

static void xfce_tasklist_window_removed(WnckScreen *screen, WnckWindow *window,
                                         XfceTasklist *tasklist)
{
	GList *li;
	GSList *lp;
	XfceTasklistChild *child;
	// GList             *windows, *lp;
	// bool           remove_class_group = true;
	uint n;

	g_return_if_fail(WNCK_IS_SCREEN(screen));
	g_return_if_fail(WNCK_IS_WINDOW(window));
	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));
	g_return_if_fail(tasklist->screen == screen);

	/* check if the window is in our skipped window list */
	if (wnck_window_is_skip_tasklist(window) &&
	    (lp = g_slist_find(tasklist->skipped_windows, window)) != NULL)
	{
		tasklist->skipped_windows = g_slist_delete_link(tasklist->skipped_windows, lp);
		g_signal_handlers_disconnect_by_func(
		    G_OBJECT(window),
		    G_CALLBACK(xfce_tasklist_skipped_windows_state_changed),
		    tasklist);

		return;
	}

	/* remove the child from the taskbar */
	for (li = tasklist->windows; li != NULL; li = li->next)
	{
		child = li->data;

		if (child->window == window)
		{
			if (child->class_group != NULL)
			{
				/* remove the class group from the internal list if this
				 * was the last window in the group */
				/* TODO
				windows = wnck_class_group_get_windows (child->class_group);
				for (lp = windows; remove_class_group && lp != NULL; lp = lp->next)
				  if (!wnck_window_is_skip_tasklist (WNCK_WINDOW (lp->data)))
				    remove_class_group = false;

				if (remove_class_group)
				  {
				    tasklist->class_groups = g_slist_remove (tasklist->class_groups,
				                                             child->class_group);
				  }*/

				g_return_if_fail(WNCK_IS_CLASS_GROUP(child->class_group));
				g_object_unref(G_OBJECT(child->class_group));
			}

			/* disconnect from all the window watch functions */
			g_return_if_fail(WNCK_IS_WINDOW(window));
			n = g_signal_handlers_disconnect_matched(G_OBJECT(window),
			                                         G_SIGNAL_MATCH_DATA,
			                                         0,
			                                         0,
			                                         NULL,
			                                         NULL,
			                                         child);

#ifdef GDK_WINDOWING_X11
			/* hide the wireframe */
			if (G_UNLIKELY(n > 5 && tasklist->show_wireframes))
			{
				xfce_tasklist_wireframe_hide(tasklist);
				n--;
			}
#endif

			g_return_if_fail(n == 5);

			/* destroy the button, this will free the child data in the
			 * container remove function */
			gtk_widget_destroy(child->button);

			break;
		}
	}
}

static void xfce_tasklist_viewports_changed(WnckScreen *screen, XfceTasklist *tasklist)
{
	WnckWorkspace *active_ws;

	g_return_if_fail(WNCK_IS_SCREEN(screen));
	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));
	g_return_if_fail(tasklist->screen == screen);

	/* pretend we changed workspace, this will update the
	 * visibility of all the buttons */
	active_ws = wnck_screen_get_active_workspace(screen);
	xfce_tasklist_active_workspace_changed(screen, active_ws, tasklist);
}

static void xfce_tasklist_skipped_windows_state_changed(WnckWindow *window,
                                                        WnckWindowState changed_state,
                                                        G_GNUC_UNUSED WnckWindowState new_state,
                                                        XfceTasklist *tasklist)
{
	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));
	g_return_if_fail(WNCK_IS_WINDOW(window));
	g_return_if_fail(g_slist_find(tasklist->skipped_windows, window) != NULL);

	if (changed_state & WNCK_WINDOW_STATE_SKIP_TASKLIST)
	{
		/* remove from list */
		tasklist->skipped_windows = g_slist_remove(tasklist->skipped_windows, window);
		g_signal_handlers_disconnect_by_func(
		    G_OBJECT(window),
		    G_CALLBACK(xfce_tasklist_skipped_windows_state_changed),
		    tasklist);

		/* pretend a normal window insert */
		xfce_tasklist_window_added(wnck_window_get_screen(window), window, tasklist);
	}
}

static void xfce_tasklist_sort(XfceTasklist *tasklist)
{
	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));

	if (tasklist->sort_order != XFCE_TASKLIST_SORT_ORDER_DND)
		tasklist->windows = g_list_sort_with_data(tasklist->windows,
		                                          xfce_tasklist_button_compare,
		                                          tasklist);

	gtk_widget_queue_resize(GTK_WIDGET(tasklist));
}

static int xfce_tasklist_update_icon_geometries(gpointer data)
{
	XfceTasklist *tasklist = XFCE_TASKLIST(data);
	GList *li;
	XfceTasklistChild *child, *child2;
	GtkAllocation alloc;
	GSList *lp;
	int root_x, root_y;
	GtkWidget *toplevel;

	toplevel = gtk_widget_get_toplevel(GTK_WIDGET(tasklist));
	gtk_window_get_position(GTK_WINDOW(toplevel), &root_x, &root_y);
	g_return_val_if_fail(XFCE_IS_TASKLIST(tasklist), false);

	for (li = tasklist->windows; li != NULL; li = li->next)
	{
		child = li->data;

		switch (child->type)
		{
		case CHILD_TYPE_WINDOW:
			gtk_widget_get_allocation(child->button, &alloc);
			g_return_val_if_fail(WNCK_IS_WINDOW(child->window), false);
			wnck_window_set_icon_geometry(child->window,
			                              alloc.x + root_x,
			                              alloc.y + root_y,
			                              alloc.width,
			                              alloc.height);
			break;

		case CHILD_TYPE_GROUP:
			gtk_widget_get_allocation(child->button, &alloc);
			for (lp = child->windows; lp != NULL; lp = lp->next)
			{
				child2 = lp->data;
				g_return_val_if_fail(WNCK_IS_WINDOW(child2->window), false);
				wnck_window_set_icon_geometry(child2->window,
				                              alloc.x + root_x,
				                              alloc.y + root_y,
				                              alloc.width,
				                              alloc.height);
			}
			break;

		case CHILD_TYPE_OVERFLOW_MENU:
			gtk_widget_get_allocation(tasklist->arrow_button, &alloc);
			g_return_val_if_fail(WNCK_IS_WINDOW(child->window), false);
			wnck_window_set_icon_geometry(child->window,
			                              alloc.x + root_x,
			                              alloc.y + root_y,
			                              alloc.width,
			                              alloc.height);
			break;

		case CHILD_TYPE_GROUP_MENU:
			/* we already handled those in the group button */
			break;
		}
	}

	return false;
}

static void xfce_tasklist_update_icon_geometries_destroyed(gpointer data)
{
	XFCE_TASKLIST(data)->update_icon_geometries_id = 0;
}

static int xfce_tasklist_update_monitor_geometry_idle(gpointer data)
{
	XfceTasklist *tasklist = XFCE_TASKLIST(data);
	bool geometry_set      = false;
	GdkWindow *window;

	g_return_val_if_fail(XFCE_IS_TASKLIST(tasklist), false);

	if (!tasklist->all_monitors)
	{
		GdkDisplay *screen = gtk_widget_get_display(GTK_WIDGET(tasklist));
		window             = gtk_widget_get_window(GTK_WIDGET(tasklist));

		if (G_LIKELY(screen != NULL && window != NULL &&
		             (tasklist->n_monitors = gdk_display_get_n_monitors(screen)) > 1))
		{
			/* set the monitor geometry */
			tasklist->my_monitor = gdk_display_get_monitor_at_window(screen, window);

			if (tasklist->all_monitors_geometry)
			{
				tasklist->all_monitors_geometry =
				    g_renew(GdkRectangle,
				            tasklist->all_monitors_geometry,
				            tasklist->n_monitors);
			}
			else
			{
				tasklist->all_monitors_geometry =
				    g_new(GdkRectangle, tasklist->n_monitors);
			}

			for (uint tmp = 0; tmp < tasklist->n_monitors; tmp++)
			{
				GdkMonitor *mon = gdk_display_get_monitor(screen, tmp);
				gdk_monitor_get_geometry(mon,
				                         &tasklist->all_monitors_geometry[tmp]);
			}

			geometry_set = true;
		}
	}

	/* make sure we never poke the window geometry unneeded
	 * in the visibility function */
	if (!geometry_set)
		xfce_tasklist_geometry_set_invalid(tasklist);

	/* update visibility of buttons */
	if (tasklist->screen != NULL)
		xfce_tasklist_active_workspace_changed(tasklist->screen, NULL, tasklist);

	return false;
}

static void xfce_tasklist_update_monitor_geometry_idle_destroy(gpointer data)
{
	XFCE_TASKLIST(data)->update_monitor_geometry_id = 0;
}

static int xfce_tasklist_child_drag_motion_timeout(gpointer data)
{
	XfceTasklistChild *child = data;

	g_return_val_if_fail(XFCE_IS_TASKLIST(child->tasklist), false);
	g_return_val_if_fail(WNCK_IS_SCREEN(child->tasklist->screen), false);

	if (child->type == CHILD_TYPE_WINDOW)
	{
		xfce_tasklist_button_activate(child, child->motion_timestamp);
	}
	else if (child->type == CHILD_TYPE_GROUP)
	{
		/* TODO popup menu */
	}

	return false;
}

static void xfce_tasklist_child_drag_motion_timeout_destroyed(gpointer data)
{
	XfceTasklistChild *child = data;

	child->motion_timeout_id = 0;
	child->motion_timestamp  = 0;
}

static bool xfce_tasklist_child_drag_motion(XfceTasklistChild *child, GdkDragContext *context,
                                            G_GNUC_UNUSED int x, G_GNUC_UNUSED int y,
                                            uint timestamp)
{
	GtkWidget *dnd_widget;

	g_return_val_if_fail(XFCE_IS_TASKLIST(child->tasklist), false);

	/* don't respond to dragging our own children or panel plugins */
	dnd_widget = gtk_drag_get_source_widget(context);
	if (dnd_widget == NULL ||
	    (gtk_widget_get_parent(dnd_widget) != GTK_WIDGET(child->tasklist) &&
	     !VALA_PANEL_IS_APPLET(dnd_widget)))
	{
		child->motion_timestamp = timestamp;
		if (child->motion_timeout_id == 0 &&
		    !gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(child->button)))
		{
			child->motion_timeout_id = gdk_threads_add_timeout_full(
			    G_PRIORITY_LOW,
			    DRAG_ACTIVATE_TIMEOUT,
			    xfce_tasklist_child_drag_motion_timeout,
			    child,
			    xfce_tasklist_child_drag_motion_timeout_destroyed);
		}

		/* keep emitting the signal */
		gdk_drag_status(context, 0, timestamp);

		/* we want to receive leave signal as well */
		return true;
	}
	else if (gtk_drag_dest_find_target(child->button, context, NULL) != GDK_NONE)
	{
		/* dnd to reorder buttons */
		gdk_drag_status(context, GDK_ACTION_MOVE, timestamp);

		return true;
	}

	/* also send drag-motion to other widgets */
	return false;
}

static void xfce_tasklist_child_drag_leave(XfceTasklistChild *child,
                                           G_GNUC_UNUSED GdkDragContext *context,
                                           G_GNUC_UNUSED GtkDragResult result)
{
	g_return_if_fail(XFCE_IS_TASKLIST(child->tasklist));

	if (child->motion_timeout_id != 0)
		g_source_remove(child->motion_timeout_id);
}

static XfceTasklistChild *xfce_tasklist_child_new(XfceTasklist *tasklist)
{
	XfceTasklistChild *child;
	PanelGravity edge;

	g_return_val_if_fail(XFCE_IS_TASKLIST(tasklist), NULL);

	child           = g_slice_new0(XfceTasklistChild);
	child->tasklist = tasklist;

	/* create the window button */
	child->button = xfce_arrow_button_new(GTK_ARROW_NONE);
	gtk_widget_set_parent(child->button, GTK_WIDGET(tasklist));
	gtk_button_set_relief(GTK_BUTTON(child->button), tasklist->button_relief);
	gtk_widget_add_events(GTK_WIDGET(child->button), GDK_SCROLL_MASK | GDK_SMOOTH_SCROLL_MASK);
	g_object_get(xfce_tasklist_get_toplevel(tasklist), VP_KEY_GRAVITY, &edge, NULL);
	g_autofree char *css =
	    css_generate_flat_button(child->button, vala_panel_edge_from_gravity(edge));
	css_add_css_to_widget(child->button, css);
	css_toggle_class(child->button, "-panel-flat-button", true);

	child->box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
	gtk_container_add(GTK_CONTAINER(child->button), child->box);
	gtk_widget_show(child->box);

	child->icon = gtk_image_new();
	if (tasklist->show_labels)
		gtk_box_pack_start(GTK_BOX(child->box), child->icon, false, true, 0);
	else
		gtk_box_pack_start(GTK_BOX(child->box), child->icon, true, true, 0);
	if (tasklist->minimized_icon_lucency > 0)
		gtk_widget_show(child->icon);

	child->label = gtk_label_new(NULL);
	gtk_box_pack_start(GTK_BOX(child->box), child->label, true, true, 0);
	/* gtk_box_reorder_child (GTK_BOX (child->box), child->icon, 0); */
	gtk_label_set_xalign(GTK_LABEL(child->label), 0.0);
	gtk_label_set_yalign(GTK_LABEL(child->label), 0.5);
	gtk_label_set_ellipsize(GTK_LABEL(child->label), tasklist->ellipsize_mode);

	g_autoptr(GtkCssProvider) provider = gtk_css_provider_new();
	gtk_css_provider_load_from_data(provider, ".label-hidden { opacity: 0.75; }", -1, NULL);
	gtk_style_context_add_provider(gtk_widget_get_style_context(child->label),
	                               GTK_STYLE_PROVIDER(provider),
	                               GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	/* don't show the label if we're in iconbox style */
	if (tasklist->show_labels)
		gtk_widget_show(child->label);

	gtk_drag_dest_set(GTK_WIDGET(child->button), 0, NULL, 0, GDK_ACTION_DEFAULT);
	g_signal_connect_swapped(G_OBJECT(child->button),
	                         "drag-motion",
	                         G_CALLBACK(xfce_tasklist_child_drag_motion),
	                         child);
	g_signal_connect_swapped(G_OBJECT(child->button),
	                         "drag-leave",
	                         G_CALLBACK(xfce_tasklist_child_drag_leave),
	                         child);

	return child;
}

/**
 * Wire Frame
 **/
#ifdef GDK_WINDOWING_X11
static void xfce_tasklist_wireframe_hide(XfceTasklist *tasklist)
{
	GdkDisplay *dpy;

	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));

	if (tasklist->wireframe_window != 0)
	{
		/* unmap the window */
		dpy = gtk_widget_get_display(GTK_WIDGET(tasklist));
		XUnmapWindow(GDK_DISPLAY_XDISPLAY(dpy), tasklist->wireframe_window);
	}
}

static void xfce_tasklist_wireframe_destroy(XfceTasklist *tasklist)
{
	GdkDisplay *dpy;

	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));

	if (tasklist->wireframe_window != 0)
	{
		/* unmap and destroy the window */
		dpy = gtk_widget_get_display(GTK_WIDGET(tasklist));
		XUnmapWindow(GDK_DISPLAY_XDISPLAY(dpy), tasklist->wireframe_window);
		XDestroyWindow(GDK_DISPLAY_XDISPLAY(dpy), tasklist->wireframe_window);

		tasklist->wireframe_window = 0;
	}
}

static void xfce_tasklist_wireframe_update(XfceTasklist *tasklist, XfceTasklistChild *child)
{
	Display *dpy;
	GdkDisplay *gdpy;
	int x, y, width, height;
	XSetWindowAttributes attrs;
	GC gc;
	XRectangle xrect;

	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));
	g_return_if_fail(tasklist->show_wireframes == true);
	g_return_if_fail(WNCK_IS_WINDOW(child->window));

	/* get the window geometry */
	wnck_window_get_geometry(child->window, &x, &y, &width, &height);

	gdpy = gtk_widget_get_display(GTK_WIDGET(tasklist));
	dpy  = GDK_DISPLAY_XDISPLAY(gdpy);

	if (G_LIKELY(tasklist->wireframe_window != 0))
	{
		/* reposition the wireframe */
		XMoveResizeWindow(dpy, tasklist->wireframe_window, x, y, width, height);

		/* full window rectangle */
		xrect.x      = 0;
		xrect.y      = 0;
		xrect.width  = width;
		xrect.height = height;

		/* we need to restore the window first */
		XShapeCombineRectangles(dpy,
		                        tasklist->wireframe_window,
		                        ShapeBounding,
		                        0,
		                        0,
		                        &xrect,
		                        1,
		                        ShapeSet,
		                        Unsorted);
	}
	else
	{
		/* set window attributes */
		attrs.override_redirect = true;
		attrs.background_pixel  = 0x000000;

		/* create new window */
		tasklist->wireframe_window = XCreateWindow(dpy,
		                                           DefaultRootWindow(dpy),
		                                           x,
		                                           y,
		                                           width,
		                                           height,
		                                           0,
		                                           CopyFromParent,
		                                           InputOutput,
		                                           CopyFromParent,
		                                           CWOverrideRedirect | CWBackPixel,
		                                           &attrs);
	}

	/* create rectangle what will be 'transparent' in the window */
	xrect.x      = WIREFRAME_SIZE;
	xrect.y      = WIREFRAME_SIZE;
	xrect.width  = width - WIREFRAME_SIZE * 2;
	xrect.height = height - WIREFRAME_SIZE * 2;

	/* substruct rectangle from the window */
	XShapeCombineRectangles(dpy,
	                        tasklist->wireframe_window,
	                        ShapeBounding,
	                        0,
	                        0,
	                        &xrect,
	                        1,
	                        ShapeSubtract,
	                        Unsorted);

	/* map the window */
	XMapWindow(dpy, tasklist->wireframe_window);

	/* create a white gc */
	gc = XCreateGC(dpy, tasklist->wireframe_window, 0, NULL);
	XSetForeground(dpy, gc, 0xffffff);

	/* draw the outer white rectangle */
	XDrawRectangle(dpy, tasklist->wireframe_window, gc, 0, 0, width - 1, height - 1);

	/* draw the inner white rectangle */
	XDrawRectangle(dpy,
	               tasklist->wireframe_window,
	               gc,
	               WIREFRAME_SIZE - 1,
	               WIREFRAME_SIZE - 1,
	               width - 2 * (WIREFRAME_SIZE - 1) - 1,
	               height - 2 * (WIREFRAME_SIZE - 1) - 1);

	XFreeGC(dpy, gc);
}
#endif

/**
 * Tasklist Buttons
 **/
static inline bool xfce_tasklist_button_visible(XfceTasklistChild *child, WnckWorkspace *active_ws)
{
	XfceTasklist *tasklist = XFCE_TASKLIST(child->tasklist);
	GdkRectangle window, intersection;
	GdkMonitor *best_mon = NULL;
	uint best_size       = 0, size, tmp;

	g_return_val_if_fail(active_ws == NULL || WNCK_IS_WORKSPACE(active_ws), false);
	g_return_val_if_fail(XFCE_IS_TASKLIST(tasklist), false);
	g_return_val_if_fail(WNCK_IS_WINDOW(child->window), false);

	if (xfce_tasklist_filter_monitors(tasklist))
	{
		/* center of the window must be on this screen */
		wnck_window_get_geometry(child->window,
		                         &window.x,
		                         &window.y,
		                         &window.width,
		                         &window.height);

		for (tmp = 0; tmp < tasklist->n_monitors; tmp++)
		{
			gdk_rectangle_intersect(&tasklist->all_monitors_geometry[tmp],
			                        &window,
			                        &intersection);
			size = intersection.width * intersection.height;
			if (size > best_size)
			{
				best_size = size;
				best_mon  = gdk_display_get_monitor(tasklist->gdk_display, tmp);
			}
		}
		if (best_mon != tasklist->my_monitor)
			return false;
	}

	if (tasklist->all_workspaces ||
	    (active_ws != NULL && (G_UNLIKELY(wnck_workspace_is_virtual(active_ws))
	                               ? wnck_window_is_in_viewport(child->window, active_ws)
	                               : wnck_window_is_on_workspace(child->window, active_ws))) ||
	    (tasklist->all_blinking &&
	     xfce_arrow_button_get_blinking(XFCE_ARROW_BUTTON(child->button))))
	{
		return (!tasklist->only_minimized || wnck_window_is_minimized(child->window));
	}

	return false;
}

static int xfce_tasklist_button_compare(gconstpointer child_a, gconstpointer child_b,
                                        gpointer user_data)
{
	const XfceTasklistChild *a = child_a, *b = child_b;
	XfceTasklist *tasklist = XFCE_TASKLIST(user_data);
	int retval;
	WnckClassGroup *class_group_a, *class_group_b;
	const char *name_a, *name_b;
	WnckWorkspace *workspace_a, *workspace_b;
	int num_a, num_b;

	g_return_val_if_fail(a->type == CHILD_TYPE_GROUP || WNCK_IS_WINDOW(a->window), 0);
	g_return_val_if_fail(b->type == CHILD_TYPE_GROUP || WNCK_IS_WINDOW(b->window), 0);

	/* just append to the list */
	if (tasklist->sort_order == XFCE_TASKLIST_SORT_ORDER_DND)
		return a->unique_id - b->unique_id;

	if (tasklist->all_workspaces)
	{
		/* get workspace (this is slightly inefficient because the WnckWindow
		 * also stores the workspace number, not the structure, and we use that
		 * for comparing too */
		workspace_a = a->window != NULL ? wnck_window_get_workspace(a->window) : NULL;
		workspace_b = b->window != NULL ? wnck_window_get_workspace(b->window) : NULL;

		/* skip this if windows are in same worspace, or both pinned (== NULL) */
		if (workspace_a != workspace_b)
		{
			/* NULL means the window is pinned */
			if (workspace_a == NULL)
				workspace_a = wnck_screen_get_active_workspace(tasklist->screen);
			if (workspace_b == NULL)
				workspace_b = wnck_screen_get_active_workspace(tasklist->screen);

			/* compare by workspace number */
			num_a = workspace_a ? wnck_workspace_get_number(workspace_a) : -1;
			num_b = workspace_b ? wnck_workspace_get_number(workspace_b) : -1;
			if (num_a != num_b)
				return num_a - num_b;
		}
	}

	if (tasklist->sort_order == XFCE_TASKLIST_SORT_ORDER_GROUP_TITLE ||
	    tasklist->sort_order == XFCE_TASKLIST_SORT_ORDER_GROUP_TIMESTAMP)
	{
		/* compare by class group names */
		class_group_a = a->class_group;
		class_group_b = b->class_group;

		/* skip this if windows are in same group (or both NULL) */
		if (class_group_a != class_group_b)
		{
			name_a = NULL;
			name_b = NULL;

			/* get the group name if available */
			if (G_LIKELY(class_group_a != NULL))
				name_a = wnck_class_group_get_name(class_group_a);
			if (G_LIKELY(class_group_b != NULL))
				name_b = wnck_class_group_get_name(class_group_b);

			/* if there is no class group name, use the window name */
			if (vala_panel_str_is_empty(name_a) && a->window != NULL)
				name_a = wnck_window_get_name(a->window);
			if (vala_panel_str_is_empty(name_b) && b->window != NULL)
				name_b = wnck_window_get_name(b->window);

			if (name_a == NULL)
				name_a = "";
			if (name_b == NULL)
				name_b = "";

			retval = strcasecmp(name_a, name_b);
			if (retval != 0)
				return retval;
		}
		else if (a->type != b->type)
		{
			/* put the group in front of the other window buttons
			 * with the same group */
			return b->type - a->type;
		}
	}

	if (tasklist->sort_order == XFCE_TASKLIST_SORT_ORDER_TIMESTAMP ||
	    tasklist->sort_order == XFCE_TASKLIST_SORT_ORDER_GROUP_TIMESTAMP)
	{
		return a->unique_id - b->unique_id;
	}
	else
	{
		if (a->window != NULL)
			name_a = wnck_window_get_name(a->window);
		else if (a->class_group != NULL)
			name_a = wnck_class_group_get_name(a->class_group);
		else
			name_a = NULL;

		if (b->window != NULL)
			name_b = wnck_window_get_name(b->window);
		else if (b->class_group != NULL)
			name_b = wnck_class_group_get_name(b->class_group);
		else
			name_b = NULL;

		if (name_a == NULL)
			name_a = "";
		if (name_b == NULL)
			name_b = "";

		return strcasecmp(name_a, name_b);
	}
}

static void xfce_tasklist_button_icon_changed(WnckWindow *window, XfceTasklistChild *child)
{
	GdkPixbuf *pixbuf               = NULL;
	g_autoptr(GdkPixbuf) new_pixbuf = NULL;
	GdkPixbuf *lucent               = NULL;
	XfceTasklist *tasklist          = child->tasklist;
	int icon_size;

	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));
	g_return_if_fail(GTK_IS_WIDGET(child->icon));
	g_return_if_fail(WNCK_IS_WINDOW(window));
	g_return_if_fail(child->window == window);

	/* 0 means icons are disabled */
	if (tasklist->minimized_icon_lucency == 0)
		return;
	g_object_get(VALA_PANEL_TOPLEVEL(xfce_tasklist_get_toplevel(tasklist)),
	             VP_KEY_ICON_SIZE,
	             &icon_size,
	             NULL);

	/* get the window icon */
	pixbuf = wnck_window_get_icon(window);
	if (pixbuf == NULL)
		pixbuf = wnck_window_get_mini_icon(window);

	/* leave when there is no valid pixbuf */
	if (G_UNLIKELY(pixbuf == NULL))
	{
		gtk_image_clear(GTK_IMAGE(child->icon));
		return;
	}

	/* create a spotlight version of the icon when minimized */
	if (!tasklist->only_minimized && tasklist->minimized_icon_lucency < 100 &&
	    wnck_window_is_minimized(window))
	{
#ifdef EXO_CHECK_VERSION
		lucent = exo_gdk_pixbuf_lucent(pixbuf, tasklist->minimized_icon_lucency);
		if (G_UNLIKELY(lucent != NULL))
			pixbuf = lucent;
#endif
	}
	if (gdk_pixbuf_get_width(pixbuf) > icon_size)
		new_pixbuf =
		    gdk_pixbuf_scale_simple(pixbuf, icon_size, icon_size, GDK_INTERP_BILINEAR);

	gtk_image_set_from_pixbuf(GTK_IMAGE(child->icon), new_pixbuf ? new_pixbuf : pixbuf);
	gtk_image_set_pixel_size(GTK_IMAGE(child->icon), icon_size);

	if (lucent != NULL && lucent != pixbuf)
		g_object_unref(G_OBJECT(lucent));
}

static void xfce_tasklist_button_name_changed(WnckWindow *window, XfceTasklistChild *child)
{
	const char *name;
	char *label = NULL;

	g_return_if_fail(window == NULL || child->window == window);
	g_return_if_fail(WNCK_IS_WINDOW(child->window));
	g_return_if_fail(XFCE_IS_TASKLIST(child->tasklist));

	name = wnck_window_get_name(child->window);
	gtk_widget_set_tooltip_text(GTK_WIDGET(child->button), name);

	/* create the button label */
	GtkStyleContext *ctx = gtk_widget_get_style_context(child->label);
	gtk_style_context_remove_class(ctx, "label-hidden");

	if (child->tasklist->label_decorations)
	{
		/* create the button label */
		if (!child->tasklist->only_minimized && wnck_window_is_minimized(child->window))
			name = label = g_strdup_printf("[%s]", name);
		else if (wnck_window_is_shaded(child->window))
			name = label = g_strdup_printf("=%s=", name);
	}
	else
	{
		if ((!child->tasklist->only_minimized && wnck_window_is_minimized(child->window)) ||
		    wnck_window_is_shaded(child->window))
			gtk_style_context_add_class(ctx, "label-hidden");
	}

	gtk_label_set_text(GTK_LABEL(child->label), name);

	g_free(label);

	/* if window is null, we have not inserted the button the in
	 * tasklist, so no need to sort, because we insert with sorting */
	if (window != NULL)
		xfce_tasklist_sort(child->tasklist);
}

static void xfce_tasklist_button_state_changed(WnckWindow *window, WnckWindowState changed_state,
                                               WnckWindowState new_state, XfceTasklistChild *child)
{
	bool blink;
	WnckScreen *screen;
	XfceTasklist *tasklist;
	WnckWorkspace *active_ws;

	g_return_if_fail(WNCK_IS_WINDOW(window));
	g_return_if_fail(child->window == window);
	g_return_if_fail(XFCE_IS_TASKLIST(child->tasklist));

	/* remove if the new state is hidding the window from the tasklist */
	if (changed_state & WNCK_WINDOW_STATE_SKIP_TASKLIST)
	{
		screen   = wnck_window_get_screen(window);
		tasklist = child->tasklist;

		/* remove button from tasklist */
		xfce_tasklist_window_removed(screen, window, child->tasklist);

		/* add the window to the skipped_windows list */
		xfce_tasklist_window_added(screen, window, tasklist);

		return;
	}

	/* update the button name */
	if ((changed_state & (WNCK_WINDOW_STATE_SHADED | WNCK_WINDOW_STATE_MINIMIZED)) &&
	    !child->tasklist->only_minimized)
		xfce_tasklist_button_name_changed(window, child);

	/* update the button icon if needed */
	if (changed_state & WNCK_WINDOW_STATE_MINIMIZED)
	{
		if (G_UNLIKELY(child->tasklist->only_minimized))
		{
			if (new_state & WNCK_WINDOW_STATE_MINIMIZED)
				gtk_widget_show(child->button);
			else
				gtk_widget_hide(child->button);
		}
		else
		{
			/* update the icon (lucent) */
			xfce_tasklist_button_icon_changed(window, child);
		}
	}

	/* update the blinking state */
	if ((changed_state & WNCK_WINDOW_STATE_DEMANDS_ATTENTION) ||
	    (changed_state & WNCK_WINDOW_STATE_URGENT))
	{
		/* only start blinking if the window requesting urgency
		 * notification is not the active window */
		blink = wnck_window_or_transient_needs_attention(window);
		if (!blink || (blink && !wnck_window_is_active(window)))
		{
			/* if we have all_blinking set make sure we toggle visibility of the button
			 * in case the window is not in the current workspace */
			active_ws = wnck_screen_get_active_workspace(child->tasklist->screen);
			if (child->tasklist->all_blinking && blink &&
			    !xfce_tasklist_button_visible(child, active_ws))
			{
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(child->button),
				                             false);
				gtk_widget_show(child->button);
			}

			xfce_arrow_button_set_blinking(XFCE_ARROW_BUTTON(child->button), blink);

			if (child->tasklist->all_blinking &&
			    !xfce_tasklist_button_visible(child, active_ws))
				gtk_widget_hide(child->button);
		}
	}
}

static void xfce_tasklist_button_workspace_changed(WnckWindow *window, XfceTasklistChild *child)
{
	XfceTasklist *tasklist = XFCE_TASKLIST(child->tasklist);

	g_return_if_fail(child->window == window);
	g_return_if_fail(XFCE_IS_TASKLIST(child->tasklist));

	xfce_tasklist_sort(tasklist);

	/* make sure we don't have two active windows (bug #6474) */
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(child->button), false);

	if (!tasklist->all_workspaces)
		xfce_tasklist_active_workspace_changed(tasklist->screen, NULL, tasklist);
}

static void xfce_tasklist_button_geometry_changed2(WnckWindow *window, XfceTasklistChild *child)
{
	WnckWorkspace *active_ws;

	g_return_if_fail(child->window == window);
	g_return_if_fail(XFCE_IS_TASKLIST(child->tasklist));
	g_return_if_fail(WNCK_IS_SCREEN(child->tasklist->screen));

	if (xfce_tasklist_filter_monitors(child->tasklist))
	{
		/* check if we need to change the visibility of the button */
		active_ws = wnck_screen_get_active_workspace(child->tasklist->screen);
		if (xfce_tasklist_button_visible(child, active_ws))
			gtk_widget_show(child->button);
		else
			gtk_widget_hide(child->button);
	}
}

static bool xfce_tasklist_button_size_allocate(G_GNUC_UNUSED GtkWidget *widget,
                                               G_GNUC_UNUSED GdkRectangle *allocation,
                                               gpointer user_data)
{
	XfceTasklistChild *child = user_data;
	/* Make sure the icons have the correct size */
	xfce_tasklist_button_icon_changed(child->window, child);

	return true;
}

#ifdef GDK_WINDOWING_X11
static void xfce_tasklist_button_geometry_changed(WnckWindow *window, XfceTasklistChild *child)
{
	g_return_if_fail(child->window == window);
	g_return_if_fail(XFCE_IS_TASKLIST(child->tasklist));

	xfce_tasklist_wireframe_update(child->tasklist, child);
}

static bool xfce_tasklist_button_leave_notify_event(GtkWidget *button,
                                                    G_GNUC_UNUSED GdkEventCrossing *event,
                                                    XfceTasklistChild *child)
{
	g_return_val_if_fail(XFCE_IS_TASKLIST(child->tasklist), false);
	g_return_val_if_fail(child->type != CHILD_TYPE_GROUP, false);

	/* disconnect signals */
	g_signal_handlers_disconnect_by_func(button,
	                                     xfce_tasklist_button_leave_notify_event,
	                                     child);
	g_signal_handlers_disconnect_by_func(child->window,
	                                     xfce_tasklist_button_geometry_changed,
	                                     child);

	/* unmap and destroy the wireframe window */
	xfce_tasklist_wireframe_hide(child->tasklist);

	return false;
}
#endif

static bool xfce_tasklist_button_enter_notify_event(GtkWidget *button,
                                                    G_GNUC_UNUSED GdkEventCrossing *event,
                                                    XfceTasklistChild *child)
{
	g_return_val_if_fail(XFCE_IS_TASKLIST(child->tasklist), false);
	g_return_val_if_fail(child->type != CHILD_TYPE_GROUP, false);
	g_return_val_if_fail(GTK_IS_WIDGET(button), false);
	g_return_val_if_fail(WNCK_IS_WINDOW(child->window), false);

#ifdef GDK_WINDOWING_X11
	/* leave when there is nothing to do */
	if (!child->tasklist->show_wireframes)
		return false;

	/* show wireframe for the child */
	xfce_tasklist_wireframe_update(child->tasklist, child);

	/* connect signal to destroy the window when the user leaves the button */
	g_signal_connect(G_OBJECT(button),
	                 "leave-notify-event",
	                 G_CALLBACK(xfce_tasklist_button_leave_notify_event),
	                 child);

	/* watch geometry changes */
	g_signal_connect(G_OBJECT(child->window),
	                 "geometry-changed",
	                 G_CALLBACK(xfce_tasklist_button_geometry_changed),
	                 child);
#endif

	return false;
}

static bool xfce_tasklist_button_button_press_event(GtkWidget *button, GdkEventButton *event,
                                                    XfceTasklistChild *child)
{
	GtkWidget *menu, *panel_plugin;

	g_return_val_if_fail(XFCE_IS_TASKLIST(child->tasklist), false);
	g_return_val_if_fail(child->type != CHILD_TYPE_GROUP, false);

	if (event->type != GDK_BUTTON_PRESS)
		return false;

	/* send the event to the panel plugin if control is pressed */
	if (PANEL_HAS_FLAG(event->state, GDK_CONTROL_MASK))
	{
		/* send the event to the panel plugin */
		panel_plugin = xfce_tasklist_get_panel_plugin(child->tasklist);
		if (G_LIKELY(panel_plugin != NULL))
			gtk_widget_event(panel_plugin, (GdkEvent *)event);

		return true;
	}

	if (event->button == 3)
	{
		menu = wnck_action_menu_new(child->window);
		g_signal_connect(G_OBJECT(menu),
		                 "selection-done",
		                 G_CALLBACK(gtk_widget_destroy),
		                 NULL);

		gtk_menu_attach_to_widget(GTK_MENU(menu), button, NULL);
		gtk_menu_popup_at_widget(GTK_MENU(menu),
		                         button,
		                         xfce_tasklist_vertical(child->tasklist)
		                             ? GDK_GRAVITY_SOUTH_EAST
		                             : GDK_GRAVITY_NORTH_WEST,
		                         xfce_tasklist_vertical(child->tasklist)
		                             ? GDK_GRAVITY_SOUTH_EAST
		                             : GDK_GRAVITY_SOUTH_WEST,
		                         (GdkEvent *)event);
		return true;
	}

	return false;
}

static bool xfce_tasklist_button_button_release_event(GtkWidget *button, GdkEventButton *event,
                                                      XfceTasklistChild *child)
{
	GtkAllocation allocation;

	g_return_val_if_fail(XFCE_IS_TASKLIST(child->tasklist), false);
	g_return_val_if_fail(child->type != CHILD_TYPE_GROUP, false);

	gtk_widget_get_allocation(button, &allocation);

	/* only respond to in-button events */
	if (event->type == GDK_BUTTON_RELEASE && !(event->x == 0 && event->y == 0) /* 0,0 = outside
	                                                                              the widget in
	                                                                              Gtk */
	    && event->x >= 0 && event->x < allocation.width && event->y >= 0 &&
	    event->y < allocation.height)
	{
		if (event->button == 1)
		{
			/* press the button */
			xfce_tasklist_button_activate(child, event->time);
			return false;
		}
		else if (event->button == 2)
		{
			switch (child->tasklist->middle_click)
			{
			case XFCE_TASKLIST_MIDDLE_CLICK_NOTHING:
				break;

			case XFCE_TASKLIST_MIDDLE_CLICK_CLOSE_WINDOW:
				wnck_window_close(child->window, event->time);
				return true;

			case XFCE_TASKLIST_MIDDLE_CLICK_MINIMIZE_WINDOW:
				if (!wnck_window_is_minimized(child->window))
					wnck_window_minimize(child->window);
				return false;
			}
		}
	}

	return false;
}

static void xfce_tasklist_button_enter_notify_event_disconnected(gpointer data,
                                                                 G_GNUC_UNUSED GClosure *closure)
{
	XfceTasklistChild *child = data;

	g_return_if_fail(WNCK_IS_WINDOW(child->window));

	/* we need to detach the geometry watch because that is connected
	 * to the window we proxy and thus not disconnected when the
	 * proxy dies */
	g_signal_handlers_disconnect_by_func(child->window,
	                                     xfce_tasklist_button_geometry_changed,
	                                     child);

	g_object_unref(G_OBJECT(child->window));
}

static GtkWidget *xfce_tasklist_button_proxy_menu_item(XfceTasklistChild *child,
                                                       bool allow_wireframe)
{
	GtkWidget *mi;
	GtkWidget *image;
	GtkWidget *label;
	GtkWidget *tmp;
	XfceTasklist *tasklist = child->tasklist;

	g_return_val_if_fail(XFCE_IS_TASKLIST(child->tasklist), NULL);
	g_return_val_if_fail(child->type == CHILD_TYPE_OVERFLOW_MENU ||
	                         child->type == CHILD_TYPE_GROUP_MENU,
	                     NULL);
	g_return_val_if_fail(GTK_IS_LABEL(child->label), NULL);
	g_return_val_if_fail(WNCK_IS_WINDOW(child->window), NULL);

	mi  = gtk_menu_item_new();
	tmp = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
	gtk_container_add(GTK_CONTAINER(mi), tmp);
	label = gtk_label_new("");
	gtk_box_pack_end(GTK_BOX(tmp), label, true, true, 0);
	g_object_bind_property(G_OBJECT(child->label),
	                       "label",
	                       G_OBJECT(label),
	                       "label",
	                       G_BINDING_SYNC_CREATE);
	g_object_bind_property(G_OBJECT(child->label),
	                       "label",
	                       G_OBJECT(mi),
	                       "tooltip-text",
	                       G_BINDING_SYNC_CREATE);

	gtk_label_set_max_width_chars(GTK_LABEL(label), tasklist->menu_max_width_chars);
	gtk_label_set_ellipsize(GTK_LABEL(label), tasklist->ellipsize_mode);
	gtk_widget_show(label);
	image = gtk_image_new();
	gtk_box_pack_start(GTK_BOX(tmp), image, false, true, 0);
	gtk_image_set_pixel_size(GTK_IMAGE(image), GTK_ICON_SIZE_MENU);
	g_object_bind_property(G_OBJECT(child->icon),
	                       "pixbuf",
	                       G_OBJECT(image),
	                       "pixbuf",
	                       G_BINDING_SYNC_CREATE);
	gtk_widget_show(image);
	gtk_widget_show(tmp);
	if (allow_wireframe)
	{
		g_object_ref(G_OBJECT(child->window));
		g_signal_connect_data(G_OBJECT(mi),
		                      "enter-notify-event",
		                      G_CALLBACK(xfce_tasklist_button_enter_notify_event),
		                      child,
		                      xfce_tasklist_button_enter_notify_event_disconnected,
		                      0);
	}

	g_signal_connect(G_OBJECT(mi),
	                 "button-press-event",
	                 G_CALLBACK(xfce_tasklist_button_button_press_event),
	                 child);
	g_signal_connect(G_OBJECT(mi),
	                 "button-release-event",
	                 G_CALLBACK(xfce_tasklist_button_button_release_event),
	                 child);

	/* TODO bold labels for urgent windows */
	/* TODO item dnd */

	return mi;
}

static void xfce_tasklist_button_activate(XfceTasklistChild *child, u_int32_t timestamp)
{
	WnckWorkspace *workspace;
	int window_x, window_y;
	int workspace_width, workspace_height;
	int screen_width, screen_height;
	int viewport_x, viewport_y;

	g_return_if_fail(XFCE_IS_TASKLIST(child->tasklist));
	g_return_if_fail(WNCK_IS_WINDOW(child->window));
	g_return_if_fail(WNCK_IS_SCREEN(child->tasklist->screen));

	if (wnck_window_is_active(child->window) ||
	    wnck_window_transient_is_most_recently_activated(child->window))
	{
		/* minimize does not work when this is assigned to the
		 * middle mouse button */
		if (child->tasklist->middle_click != XFCE_TASKLIST_MIDDLE_CLICK_MINIMIZE_WINDOW)
			wnck_window_minimize(child->window);
	}
	else
	{
		/* we only change worksapces/viewports for non-pinned windows
		 * and if all workspaces/viewports are shown or if we have
		 * all blinking enabled and the current button is blinking */
		if ((child->tasklist->all_workspaces && !wnck_window_is_pinned(child->window)) ||
		    (child->tasklist->all_blinking &&
		     xfce_arrow_button_get_blinking(XFCE_ARROW_BUTTON(child->button))))
		{
			workspace = wnck_window_get_workspace(child->window);

			/* only switch workspaces/viewports if switch_workspace is enabled or
			 * we want to restore a minimized window to the current workspace/viewport
			 */
			if (workspace != NULL && (child->tasklist->switch_workspace ||
			                          !wnck_window_is_minimized(child->window)))
			{
				if (G_UNLIKELY(wnck_workspace_is_virtual(workspace)))
				{
					if (!wnck_window_is_in_viewport(child->window, workspace))
					{
						/* viewport info */
						workspace_width =
						    wnck_workspace_get_width(workspace);
						workspace_height =
						    wnck_workspace_get_height(workspace);
						screen_width =
						    wnck_screen_get_width(child->tasklist->screen);
						screen_height =
						    wnck_screen_get_height(child->tasklist->screen);

						/* we only support multiple viewports like compiz
						 * has
						 * (all equally spread across the screen) */
						if ((workspace_width % screen_width) == 0 &&
						    (workspace_height % screen_height) == 0)
						{
							wnck_window_get_geometry(child->window,
							                         &window_x,
							                         &window_y,
							                         NULL,
							                         NULL);

							/* lookup nearest workspace edge */
							viewport_x =
							    window_x - (window_x % screen_width);
							viewport_x =
							    CLAMP(viewport_x,
							          0,
							          workspace_width - screen_width);

							viewport_y =
							    window_y - (window_y % screen_height);
							viewport_y =
							    CLAMP(viewport_y,
							          0,
							          workspace_height - screen_height);

							/* move to the other viewport */
							wnck_screen_move_viewport(child->tasklist
							                              ->screen,
							                          viewport_x,
							                          viewport_y);
						}
						else
						{
							g_warning(
							    "only viewport with equally "
							    "distributed screens are supported: "
							    "%dx%d & %dx%d",
							    workspace_width,
							    workspace_height,
							    screen_width,
							    screen_height);
						}
					}
				}
				else if (wnck_screen_get_active_workspace(
				             child->tasklist->screen) != workspace)
				{
					/* switch to the other workspace before we activate the
					 * window */
					wnck_workspace_activate(workspace, timestamp);
					gtk_main_iteration();
				}
			}
			else if (workspace != NULL && wnck_workspace_is_virtual(workspace) &&
			         !wnck_window_is_in_viewport(child->window, workspace))
			{
				/* viewport info */
				workspace_width  = wnck_workspace_get_width(workspace);
				workspace_height = wnck_workspace_get_height(workspace);
				screen_width     = wnck_screen_get_width(child->tasklist->screen);
				screen_height    = wnck_screen_get_height(child->tasklist->screen);

				/* we only support multiple viewports like compiz has
				 * (all equaly spread across the screen) */
				if ((workspace_width % screen_width) == 0 &&
				    (workspace_height % screen_height) == 0)
				{
					viewport_x = wnck_workspace_get_viewport_x(workspace);
					viewport_y = wnck_workspace_get_viewport_y(workspace);

					/* note that the x and y might be negative numbers, since
					 * they are relative
					 * to the current screen, not to the edge of the screen they
					 * are on. this is
					 * not a problem since the mod result will always be
					 * positive */
					wnck_window_get_geometry(child->window,
					                         &window_x,
					                         &window_y,
					                         NULL,
					                         NULL);

					/* get the new screen position, with the same screen offset
					 */
					window_x = viewport_x + (window_x % screen_width);
					window_y = viewport_y + (window_y % screen_height);

					/* move the window */
					wnck_window_set_geometry(child->window,
					                         WNCK_WINDOW_GRAVITY_CURRENT,
					                         WNCK_WINDOW_CHANGE_X |
					                             WNCK_WINDOW_CHANGE_Y,
					                         window_x,
					                         window_y,
					                         -1,
					                         -1);
				}
				else
				{
					g_warning(
					    "only viewport with equally distributed screens are "
					    "supported: %dx%d & %dx%d",
					    workspace_width,
					    workspace_height,
					    screen_width,
					    screen_height);
				}
			}
		}

		wnck_window_activate_transient(child->window, timestamp);
	}
}

static void xfce_tasklist_button_drag_data_get(G_GNUC_UNUSED GtkWidget *button,
                                               G_GNUC_UNUSED GdkDragContext *context,
                                               GtkSelectionData *selection_data,
                                               G_GNUC_UNUSED uint info,
                                               G_GNUC_UNUSED uint timestamp,
                                               XfceTasklistChild *child)
{
	gulong xid;

	g_return_if_fail(WNCK_IS_WINDOW(child->window));

	xid = wnck_window_get_xid(child->window);
	gtk_selection_data_set(selection_data,
	                       gtk_selection_data_get_target(selection_data),
	                       8,
	                       (guchar *)&xid,
	                       sizeof(gulong));
}

static void xfce_tasklist_button_drag_begin(G_GNUC_UNUSED GtkWidget *button,
                                            GdkDragContext *context, XfceTasklistChild *child)
{
	GdkPixbuf *pixbuf;
	/* GdkPixmap *pixmap; */

	g_return_if_fail(WNCK_IS_WINDOW(child->window));

/* not available in Gtk3 */
#if 0
  if (child->tasklist->show_labels)
    {
      /* FIXME Triggers specific repaint error (bug 11283) */
      //pixmap = gtk_widget_get_snapshot (button, NULL);
      pixmap = NULL;
      if (pixmap != NULL)
        {
          gtk_drag_set_icon_pixmap (context,
              gdk_drawable_get_colormap (GDK_DRAWABLE (pixmap)),
              pixmap, NULL, 0, 0);
          g_object_unref (G_OBJECT (pixmap));

          return;
        }
    }
#endif

	pixbuf = wnck_window_get_icon(child->window);
	if (G_LIKELY(pixbuf != NULL))
		gtk_drag_set_icon_pixbuf(context, pixbuf, 0, 0);
}

static void xfce_tasklist_button_drag_data_received(GtkWidget *button,
                                                    G_GNUC_UNUSED GdkDragContext *context, int x,
                                                    int y, GtkSelectionData *selection_data,
                                                    G_GNUC_UNUSED uint info,
                                                    G_GNUC_UNUSED uint drag_time,
                                                    XfceTasklistChild *child2)
{
	GList *li, *sibling;
	gulong xid;
	XfceTasklistChild *child;
	XfceTasklist *tasklist = XFCE_TASKLIST(child2->tasklist);
	GtkAllocation allocation;

	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));

	if (tasklist->sort_order != XFCE_TASKLIST_SORT_ORDER_DND)
		return;

	gtk_widget_get_allocation(button, &allocation);

	sibling = g_list_find(tasklist->windows, child2);
	g_return_if_fail(sibling != NULL);

	if ((!xfce_tasklist_vertical(tasklist) && x >= allocation.width / 2) ||
	    (xfce_tasklist_vertical(tasklist) && y >= allocation.height / 2))
		sibling = g_list_next(sibling);

	xid = *((gulong *)gtk_selection_data_get_data(selection_data));
	for (li = tasklist->windows; li != NULL; li = li->next)
	{
		child = li->data;

		if (sibling != li                 /* drop on end previous button */
		    && child != child2            /* drop on the same button */
		    && g_list_next(li) != sibling /* drop start of next button */
		    && child->window != NULL && wnck_window_get_xid(child->window) == xid)
		{
			/* swap items */
			tasklist->windows = g_list_delete_link(tasklist->windows, li);
			tasklist->windows = g_list_insert_before(tasklist->windows, sibling, child);

			gtk_widget_queue_resize(GTK_WIDGET(tasklist));

			break;
		}
	}
}

static XfceTasklistChild *xfce_tasklist_button_new(WnckWindow *window, XfceTasklist *tasklist)
{
	XfceTasklistChild *child;
	static uint unique_id_counter = 0;

	g_return_val_if_fail(XFCE_IS_TASKLIST(tasklist), NULL);
	g_return_val_if_fail(WNCK_IS_WINDOW(window), NULL);

	/* avoid integer overflows */
	if (G_UNLIKELY(unique_id_counter >= G_MAXUINT))
		unique_id_counter = 0;

	child              = xfce_tasklist_child_new(tasklist);
	child->type        = CHILD_TYPE_WINDOW;
	child->window      = window;
	child->class_group = wnck_window_get_class_group(window);
	child->unique_id   = unique_id_counter++;

	/* drag and drop to the pager */
	gtk_drag_source_set(child->button,
	                    GDK_BUTTON1_MASK,
	                    source_targets,
	                    G_N_ELEMENTS(source_targets),
	                    GDK_ACTION_MOVE);
	gtk_drag_dest_set(child->button,
	                  GTK_DEST_DEFAULT_DROP,
	                  source_targets,
	                  G_N_ELEMENTS(source_targets),
	                  GDK_ACTION_MOVE);
	g_signal_connect(G_OBJECT(child->button),
	                 "drag-data-get",
	                 G_CALLBACK(xfce_tasklist_button_drag_data_get),
	                 child);
	g_signal_connect(G_OBJECT(child->button),
	                 "drag-begin",
	                 G_CALLBACK(xfce_tasklist_button_drag_begin),
	                 child);
	g_signal_connect(G_OBJECT(child->button),
	                 "drag-data-received",
	                 G_CALLBACK(xfce_tasklist_button_drag_data_received),
	                 child);

	/* note that the same signals should be in the proxy menu item too */
	g_signal_connect(G_OBJECT(child->button),
	                 "enter-notify-event",
	                 G_CALLBACK(xfce_tasklist_button_enter_notify_event),
	                 child);
	g_signal_connect(G_OBJECT(child->button),
	                 "button-press-event",
	                 G_CALLBACK(xfce_tasklist_button_button_press_event),
	                 child);
	g_signal_connect(G_OBJECT(child->button),
	                 "button-release-event",
	                 G_CALLBACK(xfce_tasklist_button_button_release_event),
	                 child);

	/* monitor window changes */
	g_signal_connect(G_OBJECT(child->button),
	                 "size-allocate",
	                 G_CALLBACK(xfce_tasklist_button_size_allocate),
	                 child);
	g_signal_connect(G_OBJECT(window),
	                 "icon-changed",
	                 G_CALLBACK(xfce_tasklist_button_icon_changed),
	                 child);
	g_signal_connect(G_OBJECT(window),
	                 "name-changed",
	                 G_CALLBACK(xfce_tasklist_button_name_changed),
	                 child);
	g_signal_connect(G_OBJECT(window),
	                 "state-changed",
	                 G_CALLBACK(xfce_tasklist_button_state_changed),
	                 child);
	g_signal_connect(G_OBJECT(window),
	                 "workspace-changed",
	                 G_CALLBACK(xfce_tasklist_button_workspace_changed),
	                 child);
	g_signal_connect(G_OBJECT(window),
	                 "geometry-changed",
	                 G_CALLBACK(xfce_tasklist_button_geometry_changed2),
	                 child);

	/* poke functions */
	xfce_tasklist_button_icon_changed(window, child);
	xfce_tasklist_button_name_changed(NULL, child);

	/* insert */
	tasklist->windows = g_list_insert_sorted_with_data(tasklist->windows,
	                                                   child,
	                                                   xfce_tasklist_button_compare,
	                                                   tasklist);

	return child;
}

/**
 * Group Buttons
 **/
static void xfce_tasklist_group_button_menu_minimize_all(XfceTasklistChild *group_child)
{
	GSList *li;
	XfceTasklistChild *child;

	g_return_if_fail(group_child->type == CHILD_TYPE_GROUP);
	g_return_if_fail(WNCK_IS_CLASS_GROUP(group_child->class_group));

	for (li = group_child->windows; li != NULL; li = li->next)
	{
		child = li->data;
		if (gtk_widget_get_visible(child->button) && child->type == CHILD_TYPE_GROUP_MENU)
		{
			g_return_if_fail(WNCK_IS_WINDOW(child->window));
			wnck_window_minimize(child->window);
		}
	}
}

static void xfce_tasklist_group_button_menu_unminimize_all(XfceTasklistChild *group_child)
{
	GSList *li;
	XfceTasklistChild *child;

	g_return_if_fail(group_child->type == CHILD_TYPE_GROUP);
	g_return_if_fail(WNCK_IS_CLASS_GROUP(group_child->class_group));

	for (li = group_child->windows; li != NULL; li = li->next)
	{
		child = li->data;
		if (gtk_widget_get_visible(child->button) && child->type == CHILD_TYPE_GROUP_MENU)
		{
			g_return_if_fail(WNCK_IS_WINDOW(child->window));
			wnck_window_unminimize(child->window, gtk_get_current_event_time());
		}
	}
}

static void xfce_tasklist_group_button_menu_maximize_all(XfceTasklistChild *group_child)
{
	GSList *li;
	XfceTasklistChild *child;

	g_return_if_fail(group_child->type == CHILD_TYPE_GROUP);
	g_return_if_fail(WNCK_IS_CLASS_GROUP(group_child->class_group));

	for (li = group_child->windows; li != NULL; li = li->next)
	{
		child = li->data;
		if (gtk_widget_get_visible(child->button) && child->type == CHILD_TYPE_GROUP_MENU)
		{
			g_return_if_fail(WNCK_IS_WINDOW(child->window));
			wnck_window_maximize(child->window);
		}
	}
}

static void xfce_tasklist_group_button_menu_unmaximize_all(XfceTasklistChild *group_child)
{
	GSList *li;
	XfceTasklistChild *child;

	g_return_if_fail(group_child->type == CHILD_TYPE_GROUP);
	g_return_if_fail(WNCK_IS_CLASS_GROUP(group_child->class_group));

	for (li = group_child->windows; li != NULL; li = li->next)
	{
		child = li->data;
		if (gtk_widget_get_visible(child->button) && child->type == CHILD_TYPE_GROUP_MENU)
		{
			g_return_if_fail(WNCK_IS_WINDOW(child->window));
			wnck_window_unmaximize(child->window);
		}
	}
}

static void xfce_tasklist_group_button_menu_close_all(XfceTasklistChild *group_child)
{
	GSList *li;
	XfceTasklistChild *child;

	g_return_if_fail(WNCK_IS_CLASS_GROUP(group_child->class_group));

	for (li = group_child->windows; li != NULL; li = li->next)
	{
		child = li->data;
		if (gtk_widget_get_visible(child->button) && child->type == CHILD_TYPE_GROUP_MENU)
		{
			g_return_if_fail(WNCK_IS_WINDOW(child->window));
			wnck_window_close(child->window, gtk_get_current_event_time());
		}
	}
}

static GtkWidget *xfce_tasklist_group_button_menu(XfceTasklistChild *group_child,
                                                  bool action_menu_entries)
{
	GSList *li;
	XfceTasklistChild *child;
	GtkWidget *mi;
	GtkWidget *menu;

	g_return_val_if_fail(XFCE_IS_TASKLIST(group_child->tasklist), NULL);
	g_return_val_if_fail(WNCK_IS_CLASS_GROUP(group_child->class_group), NULL);

	menu = gtk_menu_new();

	for (li = group_child->windows; li != NULL; li = li->next)
	{
		child = li->data;
		if (gtk_widget_get_visible(child->button) && child->type == CHILD_TYPE_GROUP_MENU)
		{
			mi = xfce_tasklist_button_proxy_menu_item(child, !action_menu_entries);
			gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);
			gtk_widget_show(mi);

			if (action_menu_entries)
				gtk_menu_item_set_submenu(GTK_MENU_ITEM(mi),
				                          wnck_action_menu_new(child->window));
		}
	}

	if (action_menu_entries)
	{
		mi = gtk_separator_menu_item_new();
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);
		gtk_widget_show(mi);

		mi = gtk_menu_item_new_with_mnemonic(_("Mi_nimize All"));
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);
		g_signal_connect_swapped(G_OBJECT(mi),
		                         "activate",
		                         G_CALLBACK(xfce_tasklist_group_button_menu_minimize_all),
		                         group_child);
		gtk_widget_show(mi);

		mi = gtk_menu_item_new_with_mnemonic(_("Un_minimize All"));
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);
		g_signal_connect_swapped(G_OBJECT(mi),
		                         "activate",
		                         G_CALLBACK(xfce_tasklist_group_button_menu_unminimize_all),
		                         group_child);
		gtk_widget_show(mi);

		mi = gtk_menu_item_new_with_mnemonic(_("Ma_ximize All"));
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);
		g_signal_connect_swapped(G_OBJECT(mi),
		                         "activate",
		                         G_CALLBACK(xfce_tasklist_group_button_menu_maximize_all),
		                         group_child);
		gtk_widget_show(mi);

		mi = gtk_menu_item_new_with_mnemonic(_("_Unmaximize All"));
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);
		g_signal_connect_swapped(G_OBJECT(mi),
		                         "activate",
		                         G_CALLBACK(xfce_tasklist_group_button_menu_unmaximize_all),
		                         group_child);
		gtk_widget_show(mi);

		mi = gtk_separator_menu_item_new();
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);
		gtk_widget_show(mi);

		mi = gtk_menu_item_new_with_mnemonic(_("_Close All"));
		gtk_menu_shell_append(GTK_MENU_SHELL(menu), mi);
		g_signal_connect_swapped(G_OBJECT(mi),
		                         "activate",
		                         G_CALLBACK(xfce_tasklist_group_button_menu_close_all),
		                         group_child);
		gtk_widget_show(mi);
	}

	return menu;
}

static void xfce_tasklist_group_button_menu_destroy(GtkWidget *menu, XfceTasklistChild *group_child)
{
	g_return_if_fail(XFCE_IS_TASKLIST(group_child->tasklist));
	g_return_if_fail(GTK_IS_TOGGLE_BUTTON(group_child->button));
	g_return_if_fail(GTK_IS_WIDGET(menu));

	gtk_widget_destroy(menu);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(group_child->button), false);

#ifdef GDK_WINDOWING_X11
	/* make sure the wireframe is hidden */
	xfce_tasklist_wireframe_hide(group_child->tasklist);
#endif
}

static bool xfce_tasklist_group_button_button_press_event(GtkWidget *button, GdkEventButton *event,
                                                          XfceTasklistChild *group_child)
{
	GtkWidget *panel_plugin;
	GtkWidget *menu;

	g_return_val_if_fail(XFCE_IS_TASKLIST(group_child->tasklist), false);
	g_return_val_if_fail(group_child->type == CHILD_TYPE_GROUP, false);

	if (event->type != GDK_BUTTON_PRESS)
		return false;

	/* send the event to the panel plugin if control is pressed */
	if (PANEL_HAS_FLAG(event->state, GDK_CONTROL_MASK))
	{
		/* send the event to the panel plugin */
		panel_plugin = xfce_tasklist_get_panel_plugin(group_child->tasklist);
		if (G_LIKELY(panel_plugin != NULL))
			gtk_widget_event(panel_plugin, (GdkEvent *)event);

		return true;
	}

	if (event->button == 1 || event->button == 3)
	{
		menu = xfce_tasklist_group_button_menu(group_child, event->button == 3);
		g_signal_connect(G_OBJECT(menu),
		                 "selection-done",
		                 G_CALLBACK(xfce_tasklist_group_button_menu_destroy),
		                 group_child);

		gtk_menu_attach_to_widget(GTK_MENU(menu), button, NULL);
		gtk_menu_popup_at_widget(GTK_MENU(menu),
		                         button,
		                         xfce_tasklist_vertical(group_child->tasklist)
		                             ? GDK_GRAVITY_SOUTH_EAST
		                             : GDK_GRAVITY_NORTH_WEST,
		                         xfce_tasklist_vertical(group_child->tasklist)
		                             ? GDK_GRAVITY_SOUTH_EAST
		                             : GDK_GRAVITY_SOUTH_WEST,
		                         (GdkEvent *)event);

		return true;
	}

	return false;
}

static void xfce_tasklist_group_button_name_changed(WnckClassGroup *class_group,
                                                    XfceTasklistChild *group_child)
{
	const char *name;
	char *label;
	uint n_windows;
	GSList *li;
	XfceTasklistChild *child;

	g_return_if_fail(class_group == NULL || group_child->class_group == class_group);
	g_return_if_fail(XFCE_IS_TASKLIST(group_child->tasklist));
	g_return_if_fail(WNCK_IS_CLASS_GROUP(group_child->class_group));

	/* count number of windows in the menu */
	for (li = group_child->windows, n_windows = 0; li != NULL; li = li->next)
	{
		child = li->data;
		if (gtk_widget_get_visible(child->button) && child->type == CHILD_TYPE_GROUP_MENU)
			n_windows++;
	}

	/* create the button label */
	name = wnck_class_group_get_name(group_child->class_group);
	if (!vala_panel_str_is_empty(name))
		label = g_strdup_printf("%s (%d)", name, n_windows);
	else
		label = g_strdup_printf("(%d)", n_windows);
	gtk_label_set_text(GTK_LABEL(group_child->label), label);
	g_free(label);

	/* don't sort if there is no need to update the sorting (ie. only number
	 * of windows is changed or button is not inserted in the tasklist yet */
	if (class_group != NULL)
		xfce_tasklist_sort(group_child->tasklist);
}

static void xfce_tasklist_group_button_icon_changed(WnckClassGroup *class_group,
                                                    XfceTasklistChild *group_child)
{
	GdkPixbuf *pixbuf;
	uint icon_size;

	g_return_if_fail(XFCE_IS_TASKLIST(group_child->tasklist));
	g_return_if_fail(WNCK_IS_CLASS_GROUP(class_group));
	g_return_if_fail(group_child->class_group == class_group);
	g_return_if_fail(GTK_IS_WIDGET(group_child->icon));

	/* 0 means icons are disabled, although the grouping button does
	 * not use lucient icons */
	if (group_child->tasklist->minimized_icon_lucency == 0)
		return;

	g_object_get(VALA_PANEL_TOPLEVEL(xfce_tasklist_get_toplevel(group_child->tasklist)),
	             VP_KEY_ICON_SIZE,
	             &icon_size,
	             NULL);

	/* get the class group icon */
	if (group_child->tasklist->show_labels)
		pixbuf = wnck_class_group_get_mini_icon(class_group);
	else if (icon_size <= 31)
		pixbuf = wnck_class_group_get_mini_icon(class_group);
	else
		pixbuf = wnck_class_group_get_icon(class_group);

	if (G_LIKELY(pixbuf != NULL))
		gtk_image_set_from_pixbuf(GTK_IMAGE(group_child->icon), pixbuf);
	else
		gtk_image_clear(GTK_IMAGE(group_child->icon));
}

static void xfce_tasklist_group_button_remove(XfceTasklistChild *group_child)
{
	GSList *li;
	uint n;
	XfceTasklistChild *child;

	/* leave if hash table triggers this function where no group
	 * child is set */
	if (group_child == NULL)
		return;

	g_return_if_fail(XFCE_IS_TASKLIST(group_child->tasklist));
	g_return_if_fail(WNCK_IS_CLASS_GROUP(group_child->class_group));
	g_return_if_fail(group_child->type == CHILD_TYPE_GROUP);
	g_return_if_fail(g_list_find(group_child->tasklist->windows, group_child) != NULL);

	/* disconnect from all the group watch functions */
	n = g_signal_handlers_disconnect_matched(G_OBJECT(group_child->class_group),
	                                         G_SIGNAL_MATCH_DATA,
	                                         0,
	                                         0,
	                                         NULL,
	                                         NULL,
	                                         group_child);
	g_return_if_fail(n == 2);

	/* disconnect from visible windows */
	for (li = group_child->windows; li != NULL; li = li->next)
	{
		child = li->data;
		g_return_if_fail(GTK_IS_BUTTON(child->button));
		n = g_signal_handlers_disconnect_matched(G_OBJECT(child->button),
		                                         G_SIGNAL_MATCH_DATA,
		                                         0,
		                                         0,
		                                         NULL,
		                                         NULL,
		                                         group_child);
		g_return_if_fail(n == 2);
	}

	g_slist_free(group_child->windows);
	group_child->windows = NULL;

	/* destroy the button, this will free the remaining child
	 * data in the container remove function */
	gtk_widget_destroy(group_child->button);
}

static void xfce_tasklist_group_button_child_visible_changed(XfceTasklistChild *group_child)
{
	XfceTasklistChild *child;
	GSList *li;
	int visible_counter = 0;
	XfceTasklistChildType type;

	g_return_if_fail(group_child->type == CHILD_TYPE_GROUP);
	g_return_if_fail(WNCK_IS_CLASS_GROUP(group_child->class_group));
	g_return_if_fail(XFCE_IS_TASKLIST(group_child->tasklist));
	g_return_if_fail(group_child->tasklist->grouping != XFCE_TASKLIST_GROUPING_NEVER);

	for (li = group_child->windows; li != NULL; li = li->next)
	{
		child = li->data;
		if (gtk_widget_get_visible(child->button))
			visible_counter++;
	}

	if (visible_counter > 1)
	{
		/* show the button and take the windows */
		gtk_widget_show(group_child->button);
		type = CHILD_TYPE_GROUP_MENU;
	}
	else
	{
		/* hide the button and ungroup the buttons */
		gtk_widget_hide(group_child->button);
		type = CHILD_TYPE_WINDOW;
	}

	for (li = group_child->windows; li != NULL; li = li->next)
	{
		child = li->data;
		if (gtk_widget_get_visible(child->button))
			child->type = type;
	}

	gtk_widget_queue_resize(GTK_WIDGET(group_child->tasklist));

	xfce_tasklist_group_button_name_changed(NULL, group_child);
}

static void xfce_tasklist_group_button_child_destroyed(XfceTasklistChild *group_child,
                                                       GtkWidget *child_button)
{
	GSList *li, *lnext;
	XfceTasklistChild *child;
	uint n_children;

	g_return_if_fail(group_child->type == CHILD_TYPE_GROUP);
	g_return_if_fail(GTK_IS_BUTTON(child_button));
	g_return_if_fail(group_child->windows != NULL);
	g_return_if_fail(XFCE_IS_TASKLIST(group_child->tasklist));
	g_return_if_fail(WNCK_IS_CLASS_GROUP(group_child->class_group));

	for (li = group_child->windows, n_children = 0; li != NULL; li = lnext)
	{
		child = li->data;
		lnext = li->next;
		if (G_UNLIKELY(child->button == child_button))
			group_child->windows = g_slist_delete_link(group_child->windows, li);
		else
			n_children++;
	}

	if ((group_child->tasklist->grouping == XFCE_TASKLIST_GROUPING_ALWAYS && n_children > 0))
#if 0
      || (group_child->tasklist->grouping == XFCE_TASKLIST_GROUPING_AUTO
          && n_children > 1))
#endif
	{
		xfce_tasklist_group_button_child_visible_changed(group_child);
		xfce_tasklist_group_button_name_changed(NULL, group_child);
	}
	else
	{
		/* self destroy */
		g_object_ref(G_OBJECT(group_child->class_group));
		g_hash_table_replace(group_child->tasklist->class_groups,
		                     group_child->class_group,
		                     NULL);
	}
}

static void xfce_tasklist_group_button_add_window(XfceTasklistChild *group_child,
                                                  XfceTasklistChild *window_child)
{
	g_return_if_fail(group_child->type == CHILD_TYPE_GROUP);
	g_return_if_fail(window_child->type != CHILD_TYPE_GROUP);
	g_return_if_fail(WNCK_IS_CLASS_GROUP(group_child->class_group));
	g_return_if_fail(WNCK_IS_WINDOW(window_child->window));
	g_return_if_fail(window_child->class_group == group_child->class_group);
	g_return_if_fail(XFCE_IS_TASKLIST(group_child->tasklist));
	g_return_if_fail(g_slist_find(group_child->windows, window_child) == NULL);

	/* watch child visibility changes */
	g_signal_connect_swapped(G_OBJECT(window_child->button),
	                         "notify::visible",
	                         G_CALLBACK(xfce_tasklist_group_button_child_visible_changed),
	                         group_child);
	g_signal_connect_swapped(G_OBJECT(window_child->button),
	                         "destroy",
	                         G_CALLBACK(xfce_tasklist_group_button_child_destroyed),
	                         group_child);

	/* add to internal list */
	group_child->windows = g_slist_prepend(group_child->windows, window_child);

	/* update visibility */
	xfce_tasklist_group_button_child_visible_changed(group_child);
}

static XfceTasklistChild *xfce_tasklist_group_button_new(WnckClassGroup *class_group,
                                                         XfceTasklist *tasklist)
{
	XfceTasklistChild *child;

	g_return_val_if_fail(XFCE_IS_TASKLIST(tasklist), NULL);
	g_return_val_if_fail(WNCK_IS_CLASS_GROUP(class_group), NULL);

	child              = xfce_tasklist_child_new(tasklist);
	child->type        = CHILD_TYPE_GROUP;
	child->class_group = class_group;

	/* note that the same signals should be in the proxy menu item too */
	g_signal_connect(G_OBJECT(child->button),
	                 "button-press-event",
	                 G_CALLBACK(xfce_tasklist_group_button_button_press_event),
	                 child);

	/* monitor class group changes */
	g_signal_connect(G_OBJECT(class_group),
	                 "icon-changed",
	                 G_CALLBACK(xfce_tasklist_group_button_icon_changed),
	                 child);
	g_signal_connect(G_OBJECT(class_group),
	                 "name-changed",
	                 G_CALLBACK(xfce_tasklist_group_button_name_changed),
	                 child);

	/* poke functions */
	xfce_tasklist_group_button_icon_changed(class_group, child);
	xfce_tasklist_group_button_name_changed(NULL, child);

	/* insert */
	tasklist->windows = g_list_insert_sorted_with_data(tasklist->windows,
	                                                   child,
	                                                   xfce_tasklist_button_compare,
	                                                   tasklist);

	return child;
}

/**
 * Potential Public Functions
 **/
void xfce_tasklist_set_include_all_workspaces(XfceTasklist *tasklist, bool all_workspaces)
{
	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));

	all_workspaces = !!all_workspaces;

	if (tasklist->all_workspaces != all_workspaces)
	{
		tasklist->all_workspaces = all_workspaces;

		if (tasklist->screen != NULL)
		{
			/* update visibility of buttons */
			xfce_tasklist_active_workspace_changed(tasklist->screen, NULL, tasklist);

			/* make sure sorting is ok */
			xfce_tasklist_sort(tasklist);
		}
	}
}

void xfce_tasklist_set_include_all_monitors(XfceTasklist *tasklist, bool all_monitors)
{
	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));

	all_monitors = !!all_monitors;

	if (tasklist->all_monitors != all_monitors)
	{
		tasklist->all_monitors = all_monitors;

		/* set the geometry to invalid or update the geometry and
		 * update the visibility of the buttons */
		if (all_monitors)
		{
			xfce_tasklist_geometry_set_invalid(tasklist);

			/* update visibility of buttons */
			xfce_tasklist_active_workspace_changed(tasklist->screen, NULL, tasklist);
		}
		else if (tasklist->gdk_display != NULL)
		{
			xfce_tasklist_gdk_monitors_changed(tasklist->gdk_display, NULL, tasklist);
		}
	}
}

void xfce_tasklist_set_button_relief(XfceTasklist *tasklist, GtkReliefStyle button_relief)
{
	GList *li;
	XfceTasklistChild *child;

	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));

	if (tasklist->button_relief != button_relief)
	{
		tasklist->button_relief = button_relief;

		/* change the relief of all buttons in the list */
		for (li = tasklist->windows; li != NULL; li = li->next)
		{
			child = li->data;
			gtk_button_set_relief(GTK_BUTTON(child->button), button_relief);
		}

		/* arrow button for overflow menu */
		gtk_button_set_relief(GTK_BUTTON(tasklist->arrow_button), button_relief);
	}
}

void xfce_tasklist_set_show_labels(XfceTasklist *tasklist, bool show_labels)
{
	GList *li;
	XfceTasklistChild *child;

	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));

	show_labels = !!show_labels;

	if (tasklist->show_labels != show_labels)
	{
		tasklist->show_labels = show_labels;

		/* change the mode of all the buttons */
		for (li = tasklist->windows; li != NULL; li = li->next)
		{
			child = li->data;

			/* show or hide the label */
			if (show_labels)
			{
				gtk_widget_show(child->label);
				gtk_box_set_child_packing(GTK_BOX(child->box),
				                          child->icon,
				                          false,
				                          false,
				                          0,
				                          GTK_PACK_START);
			}
			else
			{
				gtk_widget_hide(child->label);
				gtk_box_set_child_packing(GTK_BOX(child->box),
				                          child->icon,
				                          true,
				                          true,
				                          0,
				                          GTK_PACK_START);
			}

			/* update the icon (we use another size for
			 * icon box mode) */
			if (child->type == CHILD_TYPE_GROUP)
				xfce_tasklist_group_button_icon_changed(child->class_group, child);
			else
				xfce_tasklist_button_icon_changed(child->window, child);
			gtk_widget_queue_resize(GTK_WIDGET(tasklist));
		}
	}
}

void xfce_tasklist_set_show_only_minimized(XfceTasklist *tasklist, bool only_minimized)
{
	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));

	only_minimized = !!only_minimized;

	if (tasklist->only_minimized != only_minimized)
	{
		tasklist->only_minimized = only_minimized;

		/* update all windows */
		if (tasklist->screen != NULL)
			xfce_tasklist_active_workspace_changed(tasklist->screen, NULL, tasklist);
	}
}

void xfce_tasklist_set_show_wireframes(XfceTasklist *tasklist, bool show_wireframes)
{
	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));

	tasklist->show_wireframes = !!show_wireframes;

#ifdef GDK_WINDOWING_X11
	/* destroy the window if needed */
	xfce_tasklist_wireframe_destroy(tasklist);
#endif
}

void xfce_tasklist_set_label_decorations(XfceTasklist *tasklist, bool label_decorations)
{
	GList *li;
	XfceTasklistChild *child;

	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));

	if (tasklist->label_decorations != label_decorations)
	{
		tasklist->label_decorations = label_decorations;

		for (li = tasklist->windows; li != NULL; li = li->next)
		{
			child = li->data;
			xfce_tasklist_button_name_changed(NULL, child);
		}
	}
}

void xfce_tasklist_set_grouping(XfceTasklist *tasklist, XfceTasklistGrouping grouping)
{
	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));

	/* TODO avoid overflow, because we allows + 1 in the object */
	if (grouping > XFCE_TASKLIST_GROUPING_MAX)
		grouping = XFCE_TASKLIST_GROUPING_MAX;

	if (tasklist->grouping != grouping)
	{
		tasklist->grouping = grouping;

		if (tasklist->screen != NULL)
		{
			xfce_tasklist_disconnect_screen(tasklist);
			xfce_tasklist_connect_screen(tasklist);
		}
	}
}

static void xfce_tasklist_update_orientation(XfceTasklist *tasklist)
{
	bool horizontal;
	GList *li;
	//	XfceTasklistChild *child;

	horizontal = !xfce_tasklist_vertical(tasklist);

	/* update the tasklist */
	for (li = tasklist->windows; li != NULL; li = li->next)
	{
		//		child = li->data;
		/* update task box */
		//      gtk_orientable_set_orientation (GTK_ORIENTABLE (child->box),
		//          horizontal ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL);

		/* update the label */
		if (horizontal)
		{
			//          /* gtk_box_reorder_child (GTK_BOX (child->box), child->icon, 0);
			//          */
			//          gtk_label_set_xalign (GTK_LABEL (child->label), 0.0);
			//          gtk_label_set_yalign (GTK_LABEL (child->label), 0.5);
			//          gtk_label_set_angle (GTK_LABEL (child->label), 0);
			//          gtk_label_set_ellipsize (GTK_LABEL (child->label),
			//                                   child->tasklist->ellipsize_mode);
		}
		else
		{
			//          /* gtk_box_reorder_child (GTK_BOX (child->box), child->icon,
			//          -1); */
			//          gtk_label_set_yalign (GTK_LABEL (child->label), 0.0);
			//          gtk_label_set_xalign (GTK_LABEL (child->label), 0.5);
			//          gtk_label_set_angle (GTK_LABEL (child->label), 270);
			//          gtk_label_set_ellipsize (GTK_LABEL (child->label),
			//          PANGO_ELLIPSIZE_NONE);
		}
	}

	gtk_widget_queue_resize(GTK_WIDGET(tasklist));
}

void xfce_tasklist_update_edge(XfceTasklist *tasklist, GtkPositionType edge)
{
	/* update the tasklist */
	for (GList *li = tasklist->windows; li != NULL; li = li->next)
	{
		XfceTasklistChild *child = li->data;
		g_autofree char *css     = css_generate_flat_button(child->button, edge);
		css_add_css_to_widget(child->button, css);
	}
}

void xfce_tasklist_set_nrows(XfceTasklist *tasklist, int nrows)
{
	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));
	g_return_if_fail(nrows >= 1);

	if (tasklist->nrows != nrows)
	{
		tasklist->nrows = nrows;
		gtk_widget_queue_resize(GTK_WIDGET(tasklist));
	}
}

void xfce_tasklist_set_orientation(XfceTasklist *tasklist, GtkOrientation mode)
{
	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));

	if (tasklist->mode != mode)
	{
		tasklist->mode = mode;
		xfce_tasklist_update_orientation(tasklist);
	}
}

void xfce_tasklist_set_size(XfceTasklist *tasklist, int size)
{
	g_return_if_fail(XFCE_IS_TASKLIST(tasklist));

	if (tasklist->size != size)
	{
		tasklist->size = size;
		gtk_widget_queue_resize(GTK_WIDGET(tasklist));
	}
}

void xfce_tasklist_update_monitor_geometry(XfceTasklist *tasklist)
{
	if (tasklist->update_monitor_geometry_id == 0)
	{
		tasklist->update_monitor_geometry_id =
		    gdk_threads_add_idle_full(G_PRIORITY_LOW,
		                              xfce_tasklist_update_monitor_geometry_idle,
		                              tasklist,
		                              xfce_tasklist_update_monitor_geometry_idle_destroy);
	}
}
