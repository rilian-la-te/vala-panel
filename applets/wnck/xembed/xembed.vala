using ValaPanel;
using Gtk;
using XEmbed;
public class XEmbedApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    uint number)
    {
        return new XEmbedTray(toplevel,settings,number);
    }
}
/* Standards reference:  http://standards.freedesktop.org/systemtray-spec/ */
public class XEmbedTray: Applet
{
    private XEmbed.Plugin plugin;
    public XEmbedTray(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    uint number)
    {
        base(toplevel,settings,number);
    }
    public override void create()
    {
        plugin = new XEmbed.Plugin(this);
        this.add(plugin.plugin);
        plugin.plugin.orientation = (toplevel.orientation == Orientation.HORIZONTAL) ? Orientation.VERTICAL:Orientation.HORIZONTAL;
        toplevel.notify["edge"].connect((o,a)=> {
            plugin.plugin.orientation = (toplevel.orientation == Orientation.HORIZONTAL) ? Orientation.VERTICAL:Orientation.HORIZONTAL;
        });
        this.show_all();
    }
} // End class

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(XEmbedApplet));
}
