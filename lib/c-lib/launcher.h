#ifndef LAUNCHER_H
#define LAUNCHER_H

#include <gio/gdesktopappinfo.h>
#include <gdk/gdk.h>

gboolean vala_panel_launch(GDesktopAppInfo* app_info, GList* uris);
void child_spawn_func(void* data);
void activate_menu_launch_id (GSimpleAction* action,GVariant* param, gpointer user_data);
void activate_menu_launch_uri (GSimpleAction* action,GVariant* param, gpointer user_data);
void activate_menu_launch_command (GSimpleAction* action,GVariant* param, gpointer user_data);

#endif // LAUNCHER_H
