[CCode (cheader_filename = "xembed-ccode.h")]
namespace XEmbed
{
    [Compact, CCode(cname="TrayPlugin",free_function="tray_destructor")]
    public class Plugin
    {
        public Gtk.FlowBox plugin;
        [CCode (cname = "tray_constructor")]
        public Plugin(ValaPanel.Applet applet);
    }
}
