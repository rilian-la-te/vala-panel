#ifndef GENERICCONFIGDIALOG_H
#define GENERICCONFIGDIALOG_H

#include <glib-object.h>
#include <glib.h>
#include <gtk/gtk.h>

#include "lib/applets-new/applet-widget.h"

G_BEGIN_DECLS

typedef enum {
        CONF_STR,
        CONF_INT,
        CONF_BOOL,
        CONF_FILE,
        CONF_DIRECTORY,
        CONF_TRIM,
        CONF_EXTERNAL
} GenericConfigType;

GtkDialog *generic_config_dlg(const char *title, GtkWindow *parent, GSettings *settings,
                              ...);

G_END_DECLS

#endif // GENERICCONFIGDIALOG_H
