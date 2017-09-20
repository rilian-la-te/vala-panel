using GLib;

namespace DBusMenu
{
    [DBus (name = "com.canonical.dbusmenu")]
    public class Serializer: Object
    {
        [DBus (visible="false")]
        public static int last_id;
        public uint version {get {return 3;}}
        public string text_direction {owned get {return request_text_direction();}}
        [DBus (use_string_marshalling = true)]
        public Status status {get; internal set;}
        public string[] icon_theme_path {get; internal set;}
        /* layout signature is "(ia{sv}av)" */
        public void get_layout(int parent_id,
                        int recursion_depth,
                        string[] property_names,
                        out uint revision,
                        [DBus (signature = "(ia{sv}av)")] out Variant layout)
        {
            /* Does not use recursion_depth.*/
            var parent_item = all_items.lookup(parent_id);
            layout = parent_item.serialize();
            revision = layout_revision;
        }
        /* properties signature is "a(ia{sv})" */
        public void get_group_properties(
                            int[] ids,
                            string[] property_names,
                            [DBus (signature = "a(ia{sv})")] out Variant properties)
        {
            /*Return all properties instead of requested*/
            Variant[] items = {};
            foreach (var id in ids)
            {
                var item = all_items.lookup(id);
                var builder = new VariantBuilder(new VariantType("(ia{sv})"));
                builder.add("i",item.id);
                builder.add_value(item.serialize_properties());
                items += builder.end();
            }
            properties = new Variant.array(new VariantType("(ia{sv})"),items);
        }
        [DBus (name="GetProperty")]
        public void get_item_property(int id, string name, out Variant value)
        {
            value = all_items.lookup(id).get_variant_property(name);
        }
        public void event(int id, string event_id, Variant? data, uint timestamp)
        {
            /* FIXME: Close/Open handler */
            if (event_id == "clicked")
                all_items.lookup(id).activated();
            else if (event_id == "value-changed" && data != null)
                all_items.lookup(id).value_changed(data.get_double());
        }
        /* events signature is a(isvu) */
        public void event_group([DBus (signature = "a(isvu)")] Variant events,
                                        out int[] id_errors)
        {
            var iter = events.iterator();
            int chid;
            string ch_event;
            Variant? ch_data;
            uint ch_timestamp;
            while(iter.next("(isvu)",out chid, out ch_event, out ch_data, out ch_timestamp))
                event(chid,ch_event,ch_data,ch_timestamp);
            id_errors = {};
        }
        public void about_to_show(int id, out bool need_update)
        {
            /*FIXME: Stub. Always return true*/
            need_update = true;
        }
        public void about_to_show_group(int[] ids, out int[] updates_needed, out int[] id_errors)
        {
            var updates_bool = new bool [ids.length];
            for (int i = 0; i < ids.length; i++)
                about_to_show(ids[i],out updates_bool[i]);
            id_errors = {};
            updates_needed = (int[])updates_bool;
        }
        /*updated properties signature is a(ia{sv}), removed is a(ias)*/
        public signal void items_properties_updated(
                                [DBus (signature = "a(ia{sv})")] Variant updated_props,
                                [DBus (signature="a(ias)")] Variant removed_props);
        public signal void layout_updated(uint revision, int parent = 0);
        public signal void item_activation_requested(int id, uint timestamp);
        public signal void x_valapanel_item_value_changed(int id, uint timestamp);
        private string request_text_direction()
        {
            string env = Environment.get_variable("DBUSMENU_TEXT_DIRECTION");
            if (env != null) {
                if (env == "ltr" || env == "rtl") {
                    return env;
                } else {
                    warning("Value of 'DBUSMENU_TEXT_DIRECTION' is '%s' which is not one of 'rtl' or 'ltr'", env);
                }
            }
//            else
//            {
//                string default_dir = C_("default text direction", "ltr");
//                if (default_dir == "ltr" || default_dir == "rtl") {
//                    return default_dir;
//                } else {
//                    warning("Translation has an invalid value '%s' for default text direction.  Defaulting to left-to-right.", default_dir);
//                    return "ltr";
//                }
//            }
            return "ltr";
        }
        [DBus (visible="false")]
        public Serializer()
        {
            all_items = new HashTable<int,ServerItem>(direct_hash,direct_equal);
            var item = new ServerItem();
            item.set_variant_property("children-display",new Variant.string("submenu"));
            item.parent_id = -1;
            item.id = 0;
            all_items.insert(item.id,item);
        }
        [DBus (visible="false")]
        public void append_item (ServerItem item, int parent_id = 0)
        {
            last_id++;
            item.id = last_id;
            item.parent_id = parent_id;
            all_items.insert(last_id,item);
            all_items.lookup(parent_id).children.append(item);
        }
        [DBus (visible="false")]
        public void prepend_item (ServerItem item, int parent_id = 0)
        {
            last_id++;
            item.id = last_id;
            item.parent_id = parent_id;
            all_items.insert(last_id,item);
            all_items.lookup(parent_id).children.prepend(item);
        }
        [DBus (visible="false")]
        public void remove_item(int id)
        {
            var item = all_items.lookup(id);
            var parent = all_items.lookup(item.parent_id);
            parent.children.remove(item);
            all_items.remove(id);
        }
        private HashTable<int,ServerItem> all_items;
        private uint layout_revision;
    }
}
