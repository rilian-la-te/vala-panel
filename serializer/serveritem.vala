using GLib;

namespace DBusMenu
{
    public class ServerItem: Object
    {
        public int id
        {get; set;}
        internal int parent_id
        {get; set;}
        private HashTable<string,Variant?> properties;
        internal List<unowned ServerItem> children;
        public signal void property_changed(string name, Variant? val);
        public ServerItem()
        {
            children = new List<unowned ServerItem>();
            properties = new HashTable<string,Variant?>(str_hash,str_equal);
            properties.insert("type",new Variant.string("normal"));
        }
        public void set_variant_property(string name, Variant? val)
        {
            var old_value = properties.lookup(name);
            properties.insert(name, val);
            var new_value = properties.lookup(name);
            if ((old_value ?? new_value) == null)
                return;
            if (new_value != null && old_value == null
                || old_value == null && new_value != null
                || !old_value.equal(new_value))
            {
                this.property_changed(name,val);
            }
        }
        public Variant? get_variant_property(string name)
        {
            return properties.lookup(name);
        }
        public Variant serialize ()
        {
            var builder = new VariantBuilder(new VariantType("(ia{sv}av)"));
            builder.add("i",this.id);
            builder.add_value(serialize_properties());
            Variant[] serialized_children = {};
            foreach (var ch in children)
                serialized_children += new Variant.variant(ch.serialize());
            var array = new Variant.array(VariantType.VARIANT,serialized_children);
            builder.add_value(array);
            return builder.end();
        }
        public Variant serialize_properties()
        {
            var dict = new VariantDict();
            properties.foreach((k,v)=>{
                dict.insert_value(k,v);
            });
            return dict.end();
        }
        public signal void activated();
        public signal void value_changed(double new_value);
    }
}
