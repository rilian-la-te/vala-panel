using GLib;
using Gtk;

namespace DBusMenu
{
    public interface GtkItemIface : Object
    {
        public abstract unowned Item item {get; protected set construct;}
        public static void parse_shortcut_variant(Variant shortcut, out uint key, out Gdk.ModifierType modifier)
        {
            key = 0;
            modifier = 0;
            VariantIter iter = shortcut.iterator();
            string str;
            while(iter.next("s", out str))
            {
                if (str == "Control") {
                    modifier |= Gdk.ModifierType.CONTROL_MASK;
                } else if (str == "Alt") {
                    modifier |= Gdk.ModifierType.MOD1_MASK;
                } else if (str == "Shift") {
                    modifier |= Gdk.ModifierType.SHIFT_MASK;
                } else if (str == "Super") {
                    modifier |= Gdk.ModifierType.SUPER_MASK;
                } else {
                    Gdk.ModifierType tempmod;
                    accelerator_parse(str, out key, out tempmod);
                }
            }
            return;
        }
    }
}
