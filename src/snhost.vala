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

namespace StatusNotifier
{
    public class Host: Object
    {
        public string object_path {private get; construct;}
        public bool watcher_registered {get; private set;}
        private Watcher nested_watcher;
        private WatcherIface outer_watcher;
        private uint owned_name;
        private uint watched_name;
        private bool is_nested_watcher;
        public signal void watcher_item_added(string id);
        public signal void watcher_item_removed(string id);
        public Host(string path)
        {
            Object(object_path: path);
        }
        public string[] watcher_items()
        {
            if (is_nested_watcher)
                return nested_watcher.registered_status_notifier_items;
            else
            {
                WatcherIface? outer = null;
                try
                {
                    outer = Bus.get_proxy_sync(BusType.SESSION,"org.kde.StatusNotifierWatcher","/StatusNotifierWatcher");
                } catch (Error e) {stderr.printf("%s\n",e.message);}
                return (outer != null) ? outer.registered_status_notifier_items : outer_watcher.registered_status_notifier_items;
            }
        }
        private void on_bus_aquired(DBusConnection conn)
        {
            try {
                nested_watcher = new Watcher();
                conn.register_object ("/StatusNotifierWatcher", nested_watcher);
                nested_watcher.register_status_notifier_host(object_path);
                nested_watcher.status_notifier_item_registered.connect((id)=>{watcher_item_added(id);});
                nested_watcher.status_notifier_item_unregistered.connect((id)=>{watcher_item_removed(id);});
            } catch (IOError e) {
                stderr.printf ("Could not register service. Waiting for external watcher\n");
            }
        }
        private void create_nested_watcher()
        {
            owned_name = Bus.own_name (BusType.SESSION, "org.kde.StatusNotifierWatcher", BusNameOwnerFlags.NONE,
                on_bus_aquired,
                () => {
                    watcher_registered = true;
                    is_nested_watcher = true;
                },
                () => {
                    is_nested_watcher = false;
                    create_out_watcher();
                });
        }
        private void create_out_watcher()
        {
            try{
                outer_watcher = Bus.get_proxy_sync(BusType.SESSION,"org.kde.StatusNotifierWatcher","/StatusNotifierWatcher");
                watched_name = Bus.watch_name(BusType.SESSION,"org.kde.StatusNotifierWatcher",GLib.BusNameWatcherFlags.NONE,
                                                        () => {
                                                            nested_watcher = null;
                                                            is_nested_watcher = false;
                                                            watcher_registered = true;
                                                            },
                                                        () => {
                                                            Bus.unwatch_name(watched_name);
                                                            is_nested_watcher = true;
                                                            create_nested_watcher();
                                                            }
                                                        );
                outer_watcher.register_status_notifier_host(object_path);
                outer_watcher.status_notifier_item_registered.connect((id)=>{watcher_item_added(id);});
                outer_watcher.status_notifier_item_unregistered.connect((id)=>{watcher_item_removed(id);});
            } catch (Error e) {
                stderr.printf("%s\n",e.message);
                return;
            }
        }
        construct
        {
            is_nested_watcher = true;
            watcher_registered = false;
            create_nested_watcher();
        }
        ~Host()
        {
            if (is_nested_watcher)
                Bus.unown_name(owned_name);
            else
                Bus.unwatch_name(watched_name);
        }
    }
}
