using GLib;

namespace ValaDBusMenu
{
    public class Item : Object
    {
        private static HashTable<string,VariantType> checker;
        static construct
        {
            checker = new HashTable<string,VariantType>(str_hash,str_equal);
            checker.insert("visible", VariantType.BOOLEAN);
            checker.insert("enabled", VariantType.BOOLEAN);
            checker.insert("label", VariantType.STRING);
            checker.insert("type", VariantType.STRING);
            checker.insert("children-display", VariantType.STRING);
            checker.insert("toggle-type", VariantType.STRING);
            checker.insert("icon-name", VariantType.STRING);
            checker.insert("accessible-desc", VariantType.STRING);
            checker.insert("shortcut", new VariantType("aas"));
            checker.insert("toggle-state", VariantType.INT32);
            checker.insert("icon-data", new VariantType("ay"));
            checker.insert("disposition", VariantType.STRING);
            checker.insert("x-valapanel-secondary-icon-name", VariantType.STRING);
            checker.insert("x-valapanel-icon-size", VariantType.INT32);
            checker.insert("x-valapanel-min-value", VariantType.DOUBLE);
            checker.insert("x-valapanel-current-value", VariantType.DOUBLE);
            checker.insert("x-valapanel-max-value", VariantType.DOUBLE);
            checker.insert("x-valapanel-step-increment", VariantType.DOUBLE);
            checker.insert("x-valapanel-page-increment", VariantType.DOUBLE);
            checker.insert("x-valapanel-draw-value", VariantType.BOOLEAN);
            checker.insert("x-valapanel-format-value", VariantType.STRING);
/*
 * JAyatana properties
 */
            checker.insert("jayatana-menuid", VariantType.INT32);
            checker.insert("jayatana-windowxid", VariantType.VARIANT);
            checker.insert("jayatana-parent-menuid", VariantType.INT32);
            checker.insert("jayatana-need-open", VariantType.BOOLEAN);
            checker.insert("jayatana-hashcode", VariantType.INT32);
        }
        private unowned Client client;
        private PropertyStore store;
        private List<int> children_ids;
        public int id {get; private set;}
        internal DateTime gc_tag;
        public signal void property_changed(string name, Variant? val);
        public signal void child_added(int id, Item item);
        public signal void child_removed(int id, Item item);
        public signal void child_moved(int oldpos, int newpos, Item item);
        public signal void removing();
        public Item (int id, Client iface, Variant props, List<int> children_ids)
        {
            this.children_ids = children_ids.copy();
            this.client = iface;
            this.store = new PropertyStore(props, checker);
            this.id = id;
        }
        ~Item()
        {
            removing();
        }
        public Variant get_variant_property(string name)
        {
            return store.get_prop(name);
        }
        public string get_string_property(string name)
        {
            return store.get_prop(name).get_string();
        }
        public bool get_bool_property(string name)
        {
            return (store.get_prop(name)!=null) ? store.get_prop(name).get_boolean() : false;
        }
        public int get_int_property(string name)
        {
            return (store.get_prop(name)!=null) ? store.get_prop(name).get_int32() : 0;
        }
        public List<int> get_children_ids()
        {
            return children_ids.copy();
        }
        public void set_variant_property(string name, Variant? val)
        {
            var old_value = this.store.get_prop(name);
            this.store.set_prop(name, val);
            var new_value = this.store.get_prop(name);
            if ((old_value ?? new_value) == null)
                return;
            if (new_value != null && old_value == null
                || old_value == null && new_value != null
                || !old_value.equal(new_value))
                this.property_changed(name,new_value);
        }
        public void add_child(int id, int pos)
        {
            children_ids.insert(id,pos);
            child_added(id,client.get_item(id));
        }
        public void remove_child(int id)
        {
            children_ids.remove(id);
            child_removed(id,client.get_item(id));
        }
        public void move_child(int id, int newpos)
        {
            var oldpos = children_ids.index(id);
            if (oldpos == newpos)
                return;
            children_ids.remove(id);
            children_ids.insert(id,newpos);
            child_moved(oldpos,newpos,client.get_item(id));
        }
        public List<unowned Item> get_children()
        {
            List<unowned Item> ret = new List<unowned Item>();
            foreach (var id in children_ids)
                ret.append(client.get_item(id));
            return ret;
        }
        public int get_child_position(int child_id)
        {
            return children_ids.index(child_id);
        }
        public void handle_event(string event_id, Variant? data, uint timestamp)
        {
            if (client is ValaDBusMenu.Client)
                client.handle_item_event(this.id,event_id,data,timestamp);
        }
        public void request_about_to_show()
        {
            if (client is ValaDBusMenu.Client)
                client.request_about_to_show(this.id);
        }
    }
}
