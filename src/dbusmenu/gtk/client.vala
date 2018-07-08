using GLib;
using Gtk;

namespace ValaDBusMenu
{
    public class GtkClient : Client
    {
        private unowned Gtk.MenuShell root_menu;
        public static Gtk.MenuItem new_item(Item item, bool show_im_pl = true)
        {
            if (item.get_string_property("type") == "separator")
                return new GtkSeparatorItem(item);
            else if (item.get_string_property("type") == "scale")
                return new GtkScaleItem(item);
            return new GtkMainItem(item,show_im_pl);
        }
        public GtkClient(string object_name, string object_path)
        {
            base(object_name,object_path);
            this.root_menu = null;
        }
        public void attach_to_menu(Gtk.MenuShell menu)
        {
            if (iface.icon_theme_path != null && iface.icon_theme_path.length > 0)
                foreach (unowned string path in iface.icon_theme_path)
                    IconTheme.get_default().prepend_search_path(path != null ? path : "");
            root_menu = menu;
            root_menu.foreach((c)=>{menu.remove(c); c.destroy();});
            root_menu.realize.connect(open_cb);
            root_menu.unrealize.connect(close_cb);
            get_root_item().child_added.connect(on_child_added_cb);
            get_root_item().child_moved.connect(on_child_moved_cb);
            get_root_item().child_removed.connect(on_child_removed_cb);
            foreach(unowned Item ch in get_root_item().get_children())
                on_child_added_cb(ch.id,ch);
            root_menu.show();
        }
        public void detach()
        {
            SignalHandler.disconnect_by_data(get_root_item(),this);
            if (root_menu != null)
                root_menu.foreach((c)=>{root_menu.remove(c); c.destroy();});
        }
        private void open_cb()
        {
            get_root_item().handle_event("opened",null,0);
            get_root_item().request_about_to_show();
            root_menu.queue_resize();
            foreach(unowned Item i in get_root_item().get_children()) 
            {
                i.request_about_to_show();
                i.handle_event("opened",null,0);
            }
            foreach(unowned Item i in get_root_item().get_children()) 
            {
                i.handle_event("closed",null,0);
            }
        }
        private void close_cb()
        {
            get_root_item().handle_event("closed",null,0);
        }
        private void on_child_added_cb(int id, Item item)
        {
            Gtk.MenuItem menuitem;
            bool show_image = !(this.root_menu is Gtk.MenuBar);
            menuitem = new_item(item,show_image);
            root_menu.insert(menuitem,get_root_item().get_child_position(item.id));
        }
        private void on_child_moved_cb(int oldpos, int newpos, Item item)
        {
            foreach(Widget ch in root_menu.get_children())
                if ((ch as GtkItemIface).item == item)
                {
                    root_menu.remove(ch);
                    root_menu.insert(ch,newpos);
                }
        }
        private void on_child_removed_cb(int id, Item item)
        {
            foreach(unowned Widget ch in root_menu.get_children())
                if ((ch as GtkItemIface).item == item)
                    ch.destroy();
        }
        public static bool check (string bus_name, string object_path)
        {
            Iface? iface = null;
            try
            {
                iface = Bus.get_proxy_sync(BusType.SESSION,bus_name,object_path);
                if (iface.version < 2)
                    return false;
                else
                    return true;
            } catch (Error e){}
            return false;
        }
    }
}


