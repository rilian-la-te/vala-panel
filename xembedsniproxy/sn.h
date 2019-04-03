#ifndef __SN_H__
#define __SN_H__

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <stdbool.h>
#include <xcb/xcb.h>
#include <xcb/xcb_ewmh.h>

#include "sn-common.h"

G_BEGIN_DECLS

G_DECLARE_DERIVABLE_TYPE(StatusNotifierItem, sn_item, SN, ITEM, GObject)

#define SN_ERROR g_quark_from_static_string("StatusNotifier error")
typedef enum
{
	SN_ERROR_NO_CONNECTION = 0,
	SN_ERROR_NO_NAME,
	SN_ERROR_NO_WATCHER,
	SN_ERROR_NO_HOST
} StatusNotifierError;

typedef enum
{
	SN_ICON = 0,
	SN_TOOLTIP_ICON,
	SN_ICONS_NUM /*< skip >*/
} StatusNotifierIcon;

struct _StatusNotifierItemClass
{
	GObjectClass parent_class;

	/* signals */
	void (*registration_failed)(StatusNotifierItem *sn, GError *error);
};

StatusNotifierItem *status_notifier_item_new_from_xcb_window(const char *id, SnCategory category,
                                                             xcb_connection_t *conn,
                                                             xcb_window_t window);

G_END_DECLS

#endif /* __SN_H__ */
