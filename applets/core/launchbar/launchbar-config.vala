using GLib;
using Gtk;

namespace LaunchBar
{
    [GtkTemplate (ui = "/org/vala-panel/launchbar/config.ui"), CCode (cname = "LaunchBarConfig")]
    public class ConfigDialog : Dialog
    {

    }
}
