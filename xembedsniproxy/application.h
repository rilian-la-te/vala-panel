#ifndef APPLICATIONNEW_H
#define APPLICATIONNEW_H

#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(XEmbedSNIApplication, xembed_sni_application, XEMBED_SNI, APPLICATION,
                     GtkApplication)

G_END_DECLS

#endif // APPLICATIONNEW_H
