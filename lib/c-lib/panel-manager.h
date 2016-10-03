#ifndef PANELPOSITIONIFACE_H
#define PANELPOSITIONIFACE_H

#include "toplevel.h"
#include <glib-object.h>
#include <stdbool.h>

G_BEGIN_DECLS

G_DECLARE_INTERFACE(ValaPanelManager, vala_panel_manager, VALA_PANEL, MANAGER, GObject)

typedef enum {
	AH_HIDDEN,
	AH_WAITING,
	AH_VISIBLE,
} PanelAutohideState;

struct _ValaPanelManagerInterface
{
	GTypeInterface g_iface;
	/*loading*/
	bool (*start_panels_from_profile)(ValaPanelManager *self, GtkApplication *app,
	                                  const char *profile);
	/*struts*/
	long (*can_strut)(ValaPanelManager *f, ValaPanelToplevelUnit *top);
	void (*update_strut)(ValaPanelManager *f, ValaPanelToplevelUnit *top);
	/*autohide*/
	void (*ah_start)(ValaPanelManager *f, ValaPanelToplevelUnit *top);
	void (*ah_stop)(ValaPanelManager *f, ValaPanelToplevelUnit *top);
	void (*ah_state_set)(ValaPanelManager *f, ValaPanelToplevelUnit *top,
	                     PanelAutohideState state);
	/*positioning requests*/
	void (*move_to_alloc)(ValaPanelManager *f, ValaPanelToplevelUnit *top,
	                      GtkAllocation *alloc);
	void (*move_to_side)(ValaPanelManager *f, ValaPanelToplevelUnit *top,
	                     GtkPositionType alloc);
	/*GSettings management*/
	GSettings *(*get_settings_for_scheme)(ValaPanelManager *self, const char *scheme,
	                                      const char *path);
	void (*remove_settings_path)(ValaPanelManager *self, const char *path, const char *name);
	gpointer padding[12];
};

bool vala_panel_manager_start_panels_from_profile(ValaPanelManager *self, GtkApplication *app,
                                                  const char *profile);
long vala_panel_manager_can_strut(ValaPanelManager *f, ValaPanelToplevelUnit *top);
void vala_panel_manager_update_strut(ValaPanelManager *f, ValaPanelToplevelUnit *top);
void vala_panel_manager_ah_start(ValaPanelManager *f, ValaPanelToplevelUnit *top);
void vala_panel_manager_ah_stop(ValaPanelManager *f, ValaPanelToplevelUnit *top);
void vala_panel_manager_ah_state_set(ValaPanelManager *f, ValaPanelToplevelUnit *top,
                                     PanelAutohideState st);
void vala_panel_manager_move_to_alloc(ValaPanelManager *f, ValaPanelToplevelUnit *top,
                                      GtkAllocation *alloc);
void vala_panel_manager_move_to_side(ValaPanelManager *f, ValaPanelToplevelUnit *top,
                                     GtkPositionType alloc);
GSettings *vala_panel_manager_get_settings_for_scheme(ValaPanelManager *self, const char *scheme,
                                                      const char *path);
void vala_panel_manager_remove_settings_path(ValaPanelManager *self, const char *path,
                                             const char *child_name);

G_END_DECLS

#endif // PANELPOSITIONIFACE_H
