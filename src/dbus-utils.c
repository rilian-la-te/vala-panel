#include <gio/gio.h>

G_GNUC_INTERNAL GVariant *take_names_from_dbus()
{
	GVariant *names;
	g_autoptr(GError) error = NULL;

	GDBusConnection *connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, &error);
	if (connection == NULL)
	{
		g_warning("Unable to connect to dbus: %s", error->message);
		return NULL;
	}

	g_autoptr(GVariant) ret = g_dbus_connection_call_sync(connection,
	                                                      "org.freedesktop.DBus",
	                                                      "/org/freedesktop/DBus",
	                                                      "org.freedesktop.DBus",
	                                                      "ListNames",
	                                                      NULL,
	                                                      G_VARIANT_TYPE("(as)"),
	                                                      G_DBUS_CALL_FLAGS_NONE,
	                                                      -1,
	                                                      NULL,
	                                                      &error);
	if (ret == NULL)
	{
		g_warning("Unable to query dbus: %s", error->message);
		return NULL;
	}
	names = g_variant_get_child_value(ret, 0);
	return names;
}

G_GNUC_INTERNAL GVariant *take_all_object_paths(char *busName)
{
}
