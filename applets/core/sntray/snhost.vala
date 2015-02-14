using GLib;
using ValaPanel;

public class SNHost: Object
{
	public string object_path
	{private get; construct;}
	private SNWatcher nested_watcher;
	private SNWatcherIface outer_watcher;
	private uint owned_name;
	private uint watched_name;
	private bool is_nested_watcher;
	private bool watcher_registration_emitted;
	public signal void watcher_registered();
	public signal void watcher_items_changed();
	public static string gen_object_path(Applet pl)
	{
		return "org.kde.StatusNotifierHost-valapanel%s%u".printf(pl.toplevel.name,pl.number);
	}
	public SNHost.from_path(string path)
	{
		Object(object_path: path);
	}
	public SNHost.from_applet(Applet pl)
	{
		Object(object_path: gen_object_path(pl));
	}
	public string[] watcher_items()
	{
		if (is_nested_watcher)
			return nested_watcher.registered_status_notifier_items;
		else
			return outer_watcher.registered_status_notifier_items;
	}
	private void on_bus_aquired(DBusConnection conn)
	{
		try {
			nested_watcher = new SNWatcher();
			conn.register_object ("/StatusNotifierWatcher", nested_watcher);
			nested_watcher.register_status_notifier_host(object_path);
			nested_watcher.status_notifier_item_registered.connect(()=>{watcher_items_changed();});
			nested_watcher.status_notifier_item_unregistered.connect(()=>{watcher_items_changed();});
			if(!watcher_registration_emitted)
			{
				watcher_registered();
				watcher_registration_emitted = true;
			}
		} catch (IOError e) {
			stderr.printf ("Could not register service. Waiting for external watcher\n");
		}
	}
	private void create_nested_watcher()
	{
		owned_name = Bus.own_name (BusType.SESSION, "org.kde.StatusNotifierWatcher", BusNameOwnerFlags.NONE,
			on_bus_aquired,
			() => {is_nested_watcher = true;},
			() =>
				{
					is_nested_watcher = false;
					create_out_watcher();
				});
	}
	private void create_out_watcher()
	{
		try{
			outer_watcher = Bus.get_proxy_sync(BusType.SESSION,"org.kde.StatusNotifierWatcher","/StatusNotifierWatcher");
			watched_name = Bus.watch_name(BusType.SESSION,"org.kde.StatusNotifierWatcher",GLib.BusNameWatcherFlags.NONE,
													null,
													() => {
														Bus.unwatch_name(watched_name);
														is_nested_watcher = true;
														create_nested_watcher();
														}
													);

			outer_watcher.register_status_notifier_host(object_path);
			outer_watcher.status_notifier_item_registered.connect(()=>{watcher_items_changed();});
			outer_watcher.status_notifier_item_unregistered.connect(()=>{watcher_items_changed();});
		} catch (Error e){stderr.printf("%s\n",e.message);}
		if(!watcher_registration_emitted)
		{
			watcher_registered();
			watcher_registration_emitted = true;
		}
	}
	construct
	{
		is_nested_watcher = true;
		watcher_registration_emitted = false;
		create_nested_watcher();
	}
	~SNHost()
	{
		if (is_nested_watcher)
			Bus.unown_name(owned_name);
		else
			Bus.unwatch_name(watched_name);
	}
}
