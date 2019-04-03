/*
 * xfce4-sntray-plugin
 * Copyright (C) 2015-2017 Konstantin Pugin <ria.freelander@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
using GLib;
using Gtk;

namespace StatusNotifier
{
    [DBus (name = "org.kde.StatusNotifierWatcher")]
    public interface WatcherIface: Object
    {
        /* Signals */
        public signal void status_notifier_item_registered(string item);
        public signal void status_notifier_host_registered();
        public signal void status_notifier_item_unregistered(string item);
        public signal void status_notifier_host_unregistered();
        /* Public properties */
        public abstract string[] registered_status_notifier_items {owned get;}
        public abstract bool is_status_notifier_host_registered {get;}
        public abstract int protocol_version {get;}
        /* Public methods */
        public abstract void register_status_notifier_item(string service) throws Error;
        public abstract void register_status_notifier_host(string service) throws Error;
    }
    [DBus (name = "org.kde.StatusNotifierWatcher")]
    public class Watcher : Object
    {
        /* Signals */
        public signal void status_notifier_item_registered(string item);
        public signal void status_notifier_host_registered();
        public signal void status_notifier_item_unregistered(string item);
        public signal void status_notifier_host_unregistered();
        /* Hashes */
        private HashTable<string,uint> name_watcher = new HashTable<string,uint>(str_hash, str_equal);
        private HashTable<string,uint> hosts = new HashTable<string,uint>(str_hash, str_equal);
        /* Public properties */
        public string[] registered_status_notifier_items {owned get {return get_registered_items();}}
        public bool is_status_notifier_host_registered {get; private set; default = true;}
        public int protocol_version {get {return 0;}}
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
                warning("Trying to register already registered item. Reregistering new...");
                remove(id);
            }
            var name_handler = Bus.watch_name(BusType.SESSION,name,GLib.BusNameWatcherFlags.NONE,
                                                ()=>{
                                                    try {
                                                        ItemIface ping_iface = Bus.get_proxy_sync(BusType.SESSION,name,path);
                                                        ping_iface.notify.connect((pspec)=>{
                                                            if (ping_iface.id == null ||
                                                            ping_iface.title == null ||
                                                            ping_iface.id.length <= 0 ||
                                                            ping_iface.title.length <= 0)
                                                                remove(get_id(name,path));
                                                        });
                                                    } catch (Error e) {remove(get_id(name,path));}
                                                },
                                                () => {remove(get_id(name,path));}
                                                );
            name_watcher.insert(id,name_handler);
            status_notifier_item_registered(id);
            this.notify_property("registered-status-notifier-items");
        }
        public void register_status_notifier_host(string service) throws Error
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
            if(name != 0)
                Bus.unwatch_name(name);
            name_watcher.remove(id);
            status_notifier_item_unregistered(outer);
            this.notify_property("registered-status-notifier-items");
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
        ~Watcher()
        {
            name_watcher.foreach((k,v)=>{Bus.unwatch_name(v);});
            hosts.foreach((k,v)=>{Bus.unwatch_name(v);});
        }
    }
}
