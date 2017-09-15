using GLib;

namespace DBusMenu
{
    [DBus (use_string_marshalling = true)]
    public enum Status
    {
        [DBus (value = "normal")]
        NORMAL,
        [DBus (value = "notice")]
        NOTICE
    }
    [DBus (name = "com.canonical.dbusmenu")]
    public interface Iface : DBusProxy
    {
        public abstract uint version {get;}
        public abstract string text_direction {owned get;}
        [DBus (use_string_marshalling = true)]
        public abstract Status status {get;}
        public abstract string[] icon_theme_path {owned get;}
        /* layout signature is "(ia{sv}av)" */
        public abstract void get_layout(int parent_id,
                        int recursion_depth,
                        string[] property_names,
                        out uint revision,
                        [DBus (signature = "(ia{sv}av)")] out Variant layout) throws IOError;
        /* properties signature is "a(ia{sv})" */
        public abstract void get_group_properties(
                            int[] ids,
                            string[] property_names,
                            [DBus (signature = "a(ia{sv})")] out Variant properties) throws IOError;
        public abstract void get_property(int id, string name, out Variant value) throws IOError;
        public abstract void event(int id, string event_id, Variant? data, uint timestamp) throws IOError;
        /* events signature is a(isvu) */
        public abstract void event_group( [DBus (signature = "a(isvu)")] Variant events,
                                        out int[] id_errors) throws IOError;
        public abstract void about_to_show(int id, out bool need_update) throws IOError;
        public abstract void about_to_show_group(int[] ids, out int[] updates_needed, out int[] id_errors) throws IOError;
        /*updated properties signature is a(ia{sv}), removed is a(ias)*/
        public abstract signal void items_properties_updated(
                                [DBus (signature = "a(ia{sv})")] Variant updated_props,
                                [DBus (signature="a(ias)")] Variant removed_props);
        public abstract signal void layout_updated(uint revision, int parent);
        public abstract signal void item_activation_requested(int id, uint timestamp);
        public abstract signal void x_valapanel_item_value_changed(int id, uint timestamp);
    }
}