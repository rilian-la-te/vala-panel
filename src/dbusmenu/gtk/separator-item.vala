using GLib;
using Gtk;

namespace ValaDBusMenu
{
    public class GtkSeparatorItem: SeparatorMenuItem, GtkItemIface
    {
        private const string[] allowed_properties = {"visible","enabled"};
        public unowned Item item {get; protected set construct;}
        public GtkSeparatorItem(Item item)
        {
            this.item = item;
            this.show_all();
            this.init();
            item.property_changed.connect(on_prop_changed_cb);
            item.removing.connect(()=>{this.destroy();});
        }
        private void on_prop_changed_cb(string name, Variant? val)
        {
            switch (name)
            {
                case "visible":
                    this.visible = val.get_boolean();
                    break;
                case "enabled":
                    this.sensitive = val.get_boolean();
                    break;
            }
        }
        private void init()
        {
            foreach (var prop in allowed_properties)
                on_prop_changed_cb(prop,item.get_variant_property(prop));
        }
    }
}
