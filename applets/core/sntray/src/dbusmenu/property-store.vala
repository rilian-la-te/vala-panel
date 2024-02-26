using GLib;
using ValaDBusMenu;

namespace ValaDBusMenu
{
    [Compact]
    private class PropertyStore
    {
        internal const string[] persist_names = {"visible","enabled","type","label","disposition"};
        internal VariantDict dict;
        internal unowned HashTable <string,VariantType> checker;
        public Variant? get_prop(string name)
        {
            unowned VariantType type = checker.lookup(name);
            Variant? prop = dict.lookup_value(name,type);
            return (type != null && prop != null && prop.is_of_type(type)) ? prop : return_default(name);
        }
        public void set_prop(string name, Variant? val)
        {
            unowned VariantType type = checker.lookup(name);
            if (val == null)
                dict.remove(name);
            else if (val != null && type != null && val.is_of_type(type))
                dict.insert_value(name,val);
        }
        public PropertyStore (Variant? props,HashTable <string,VariantType> checker)
        {
            dict = new VariantDict(props);
            this.checker = checker;
        }
        private Variant? return_default(string name)
        {
            if (name == "visible" || name == "enabled")
                return new Variant.boolean(true);
            if (name == "type")
                return new Variant.string("standard");
            if (name == "label")
                return new Variant.string("");
            if (name == "disposition")
                return new Variant.string("normal");
            return null;
        }
    }
}
