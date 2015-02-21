using GLib;
using Gtk;

namespace StatusNotifier
{
	[DBus (name = "org.kde.StatusNotifierWatcher")]
	public interface WatcherIface: Object
	{
		/* Signals */
		public signal void status_notifier_item_registered(out string item);
		public signal void status_notifier_host_registered();
		public signal void status_notifier_item_unregistered(out string item);
		public signal void status_notifier_host_unregistered();
		/* Public properties */
		public abstract string[] registered_status_notifier_items
		{owned get;protected set;}
		public abstract bool is_status_notifier_host_registered
		{get;}
		public abstract int protocol_version
		{get;}
		/* Public methods */
		public abstract void register_status_notifier_item(string service) throws IOError;
		public abstract void register_status_notifier_host(string service) throws IOError;
	}
	[DBus (name = "org.kde.StatusNotifierWatcher")]
	public class Watcher : Object
	{
		/* Signals */
		public signal void status_notifier_item_registered(out string item);
		public signal void status_notifier_host_registered();
		public signal void status_notifier_item_unregistered(out string item);
		public signal void status_notifier_host_unregistered();
		/* Hashes */
		private HashTable<string,uint> name_watcher;
		private HashTable<string,uint> hosts;
		/* Public properties */
		public string[] registered_status_notifier_items
		{owned get; protected set;}
		public bool is_status_notifier_host_registered
		{get {return true;}}
		public int protocol_version
		{get {return 1;}}
		/* Public methods */
		public void register_status_notifier_item(string service, BusName sender)
		{
			var is_path = (service[0]=='/') ? true : false;
			string path, name;
			if (is_path)
			{
				name = (string)sender;
				path = service;
			}
			else
			{
				name = service;
				path = "/StatusNotifierItem";
			}
			var id = get_id(name,path);
			if (id in name_watcher)
			{
				warning("Trying to register already registered item");
			}
			else
			{
				var name_handler = Bus.watch_name(BusType.SESSION,name,GLib.BusNameWatcherFlags.NONE,
													null,
													() => {remove(get_id(name,path));}
													);
				print("%s,%s\n",name,path);
				name_watcher.insert(id,name_handler);
				registered_status_notifier_items = get_registered_items();
				status_notifier_item_registered(out id);
				/* FIXME: PropertiesChanged for RegisteredStatusNotifierItems*/
			}
		}
		public void register_status_notifier_host(string service) throws IOError
		{
			/* FIXME: Hosts management untested with non-ValaPanel hosts*/
			hosts.insert(service,Bus.watch_name(BusType.SESSION,service,GLib.BusNameWatcherFlags.NONE,
					null,
					() => {remove_host(service);}
					));
			status_notifier_host_registered();
		}
		private void remove_host(string id)
		{
			uint name = hosts.lookup(id);
			hosts.remove(id);
			Bus.unwatch_name(name);
			status_notifier_host_unregistered();
		}
		private void remove(string id)
		{
			string outer = id.dup();
			uint name = name_watcher.lookup(id);
			name_watcher.remove(id);
			Bus.unwatch_name(name);
			status_notifier_item_unregistered(out outer);
			registered_status_notifier_items = get_registered_items();
			/* FIXME PropertiesChanged for RegisteredStatusNotifierItems*/
		}
		private string get_id(string name, string path)
		{
			return name + path;
		}
		private string[] get_registered_items()
		{
			var items_list = name_watcher.get_keys();
			string [] ret = {};
			foreach(var item in items_list)
				ret += item;
			return ret;
		}
		construct
		{
			name_watcher = new HashTable<string,uint>(str_hash, str_equal);
			hosts = new HashTable<string,uint>(str_hash, str_equal);
		}
		~Watcher()
		{
			name_watcher.foreach((k,v)=>{Bus.unwatch_name(v);});
			hosts.foreach((k,v)=>{Bus.unwatch_name(v);});
		}
	}
}
