#include <X11/X.h>
#include <gtk/gtk.h>

#include "vala-panel-compat.h"

G_BEGIN_DECLS
/* Representative of a balloon message. */
typedef struct _balloon_message
{
	struct _balloon_message *flink; /* Forward link */
	Window window;                  /* X window ID */
	long timeout;          /* Time in milliseconds to display message; 0 if no timeout */
	long length;           /* Message string length */
	long id;               /* Client supplied unique message ID */
	long remaining_length; /* Remaining length expected of incomplete message */
	char *string;          /* Message string */
} BalloonMessage;

/* Representative of a tray client. */
typedef struct _tray_client
{
	struct _tray_client *client_flink; /* Forward link to next task in X window ID order */
	struct _tray_plugin *tr;           /* Back pointer to tray plugin */
	Window window;                     /* X window ID */
	GtkWidget *socket;                 /* Socket */
} TrayClient;

/* Private context for system tray plugin. */
typedef struct _tray_plugin
{
	GtkWidget *plugin; /* Back pointer to Plugin */
	PanelApplet *applet;
	TrayClient *client_list;             /* List of tray clients */
	BalloonMessage *incomplete_messages; /* List of balloon messages for which we are awaiting
	                                        data */
	BalloonMessage *messages; /* List of balloon messages actively being displayed or waiting to
	                             be
	                             displayed */
	GtkWidget *balloon_message_popup; /* Popup showing balloon message */
	guint balloon_message_timer;      /* Timer controlling balloon message */
	GtkWidget *invisible;             /* Invisible window that holds manager selection */
	Window invisible_window;          /* X window ID of invisible window */
	GdkAtom selection_atom;           /* Atom for _NET_SYSTEM_TRAY_S%d */
} TrayPlugin;

TrayPlugin *tray_constructor(PanelApplet *applet);
void tray_destructor(gpointer data);
G_END_DECLS
