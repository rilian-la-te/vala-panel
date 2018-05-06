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

//var refreshPropertyOnProxy = function(proxy, property_name) {
//    proxy.g_connection.call(proxy.g_name,
//                            proxy.g_object_path,
//                            'org.freedesktop.DBus.Properties',
//                            'Get',
//                            GLib.Variant.new('(ss)', [ proxy.g_interface_name, property_name ]),
//                            GLib.VariantType.new('(v)'),
//                            Gio.DBusCallFlags.NONE,
//                            -1,
//                            null,
//                            function(conn, result) {
//                                try {
//                                    let value_variant = conn.call_finish(result).deep_unpack()[0]

//                                    proxy.set_cached_property(property_name, value_variant)

//                                    // synthesize a property changed event
//                                    let changed_obj = {}
//                                    changed_obj[property_name] = value_variant
//                                    proxy.emit('g-properties-changed', GLib.Variant.new('a{sv}', changed_obj), [])
//                                } catch (e) {
//                                    // the property may not even exist, silently ignore it
//                                    //Logger.debug("While refreshing property "+property_name+": "+e)
//                                }
//                            })
//}

//var getUniqueBusNameSync = function(bus, name) {
//    if (name[0] == ':')
//        return name;

//    if (typeof bus === "undefined" || !bus)
//        bus = Gio.DBus.session;

//    let variant_name = new GLib.Variant("(s)", [name]);
//    let [unique] = bus.call_sync("org.freedesktop.DBus", "/", "org.freedesktop.DBus",
//                                 "GetNameOwner", variant_name, null,
//                                 Gio.DBusCallFlags.NONE, -1, null).deep_unpack();

//    return unique;
//}

//var traverseBusNames = function(bus, cancellable, callback) {
//    if (typeof bus === "undefined" || !bus)
//        bus = Gio.DBus.session;

//    if (typeof(callback) !== "function")
//        throw new Error("No traversal callback provided");

//    bus.call("org.freedesktop.DBus", "/", "org.freedesktop.DBus",
//             "ListNames", null, new GLib.VariantType("(as)"), 0, -1, cancellable,
//             function (bus, task) {
//                if (task.had_error())
//                    return;

//                let [names] = bus.call_finish(task).deep_unpack();
//                let unique_names = [];

//                for (let name of names) {
//                    let unique = getUniqueBusNameSync(bus, name);
//                    if (unique_names.indexOf(unique) == -1)
//                        unique_names.push(unique);
//                }

//                for (let name of unique_names)
//                    callback(bus, name, cancellable);
//            });
//}

//var introspectBusObject = function(bus, name, cancellable, filterFunction, targetCallback, path) {
//    if (typeof path === "undefined" || !path)
//        path = "/";

//    if (typeof targetCallback !== "function")
//        throw new Error("No introspection callback defined");

//    bus.call (name, path, "org.freedesktop.DBus.Introspectable", "Introspect",
//              null, new GLib.VariantType("(s)"), Gio.DBusCallFlags.NONE, -1,
//              cancellable, function (bus, task) {
//                if (task.had_error())
//                    return;

//                let introspection = bus.call_finish(task).deep_unpack().toString();
//                let node_info = Gio.DBusNodeInfo.new_for_xml(introspection);

//                if ((typeof filterFunction === "function" && filterFunction(node_info) === true) ||
//                    typeof filterFunction === "undefined" || !filterFunction) {
//                    targetCallback(name, path);
//                }

//                if (path === "/")
//                    path = ""

//                for (let sub_nodes of node_info.nodes) {
//                    let sub_path = path+"/"+sub_nodes.path;
//                    introspectBusObject (bus, name, cancellable, filterFunction,
//                                         targetCallback, sub_path);
//                }
//            });
//}

//var dbusNodeImplementsInterfaces = function(node_info, interfaces) {
//    if (!(node_info instanceof Gio.DBusNodeInfo) || !Array.isArray(interfaces))
//        return false;

//    for (let iface of interfaces) {
//        if (node_info.lookup_interface(iface) !== null)
//            return true;
//    }

//    return false;
//}
