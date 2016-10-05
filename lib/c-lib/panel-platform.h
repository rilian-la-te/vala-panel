#ifndef PANELPOSITIONIFACE_H
#define PANELPOSITIONIFACE_H

#include "toplevel.h"
#include <glib-object.h>
#include <stdbool.h>

G_BEGIN_DECLS

G_DECLARE_INTERFACE(ValaPanelPlatform, vala_panel_platform, VALA_PANEL, PLATFORM, GObject)

typedef enum {
	AH_HIDDEN,
	AH_WAITING,
	AH_VISIBLE,
} PanelAutohideState;

struct _ValaPanelPlatformInterface
{
	GTypeInterface g_iface;
	/*loading*/
	bool (*start_panels_from_profile)(ValaPanelPlatform *self, GtkApplication *app,
	                                  const char *profile);
	/*struts*/
	long (*can_strut)(ValaPanelPlatform *f, ValaPanelToplevelUnit *top);
	void (*update_strut)(ValaPanelPlatform *f, ValaPanelToplevelUnit *top);
	/*autohide*/
	bool (*ah_mouse_watch)(ValaPanelPlatform *f, ValaPanelToplevelUnit *top);
	/*positioning requests*/
	void (*move_to_coords)(ValaPanelPlatform *f, ValaPanelToplevelUnit *top, int x, int y);
	void (*move_to_side)(ValaPanelPlatform *f, ValaPanelToplevelUnit *top,
	                     GtkPositionType alloc);
	/*GSettings management*/
	GSettings *(*get_settings_for_scheme)(ValaPanelPlatform *self, const char *scheme,
	                                      const char *path);
	void (*remove_settings_path)(ValaPanelPlatform *self, const char *path, const char *name);
	gpointer padding[12];
};

bool vala_panel_platform_start_panels_from_profile(ValaPanelPlatform *self, GtkApplication *app,
                                                   const char *profile);
long vala_panel_platform_can_strut(ValaPanelPlatform *f, ValaPanelToplevelUnit *top);
void vala_panel_platform_update_strut(ValaPanelPlatform *f, ValaPanelToplevelUnit *top);
bool vala_panel_platform_ah_mouse_watch(ValaPanelPlatform *f, ValaPanelToplevelUnit *top);
void vala_panel_platform_move_to_coords(ValaPanelPlatform *f, ValaPanelToplevelUnit *top, int x,
                                        int y);
void vala_panel_platform_move_to_side(ValaPanelPlatform *f, ValaPanelToplevelUnit *top,
                                      GtkPositionType alloc);
GSettings *vala_panel_platform_get_settings_for_scheme(ValaPanelPlatform *self, const char *scheme,
                                                       const char *path);
void vala_panel_platform_remove_settings_path(ValaPanelPlatform *self, const char *path,
                                              const char *child_name);

G_END_DECLS

#endif // PANELPOSITIONIFACE_H
