using ValaPanel;
using Gtk;
public class KbLEDApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
		                            GLib.Settings? settings,
		                            uint number)
    {
        return new Kbled(toplevel,settings,number);
    }
}
public class Kbled: Applet, AppletConfigurable
{
	private static const string CAPS_ON = "capslock-on";
	private static const string NUM_ON = "numlock-on";
    IconGrid widget;
    Gtk.Image caps;
    Gtk.Image num;
    Gtk.Image scroll;
    Gdk.Keymap keymap;

    public Kbled(ValaPanel.Toplevel toplevel,
		                            GLib.Settings? settings,
		                            uint number)
    {
		base(toplevel,settings,number);
	}
	public Dialog get_config_dialog()
	{
		Dialog dlg = Configurator.generic_config_dlg(_("Keyboard LED"),
							toplevel, this,
							_("Show CapsLock"), CAPS_ON, GenericConfigType.BOOL,
							_("Show NumLock"), NUM_ON, GenericConfigType.BOOL,
							null);
		dlg.set_size_request(200, -1);	/* Improve geometry */
		return dlg; 
	}
	public override void create()
	{
        widget = new IconGrid(toplevel.orientation, (int)toplevel.icon_size,(int)toplevel.icon_size,0,0,toplevel.height);
        add(widget);
		caps = new Image();
		toplevel.bind_property(Key.ICON_SIZE,caps,"pixel-size",BindingFlags.DEFAULT|BindingFlags.SYNC_CREATE);
		num = new Image();
		toplevel.bind_property(Key.ICON_SIZE,num,"pixel-size",BindingFlags.DEFAULT|BindingFlags.SYNC_CREATE);
		scroll = new Image();
		toplevel.bind_property(Key.ICON_SIZE,scroll,"pixel-size",BindingFlags.DEFAULT|BindingFlags.SYNC_CREATE);
		settings.bind(CAPS_ON,caps,"visible",SettingsBindFlags.GET);
		settings.bind(NUM_ON,num,"visible",SettingsBindFlags.GET);
//~ 		widget.pack_start(scroll, false, false, 0);
        widget.add(caps);
        widget.add(num);

        keymap = Gdk.Keymap.get_default();
        keymap.state_changed.connect(on_state_changed);

        on_state_changed();

        toplevel.notify.connect((o,a)=> {
			if (a.name in Toplevel.gnames)
				widget.set_geometry(toplevel.orientation,(int)toplevel.icon_size,(int)toplevel.icon_size,0,0,toplevel.height);
        });

        show_all();
    }

    /* Handle caps lock changes */
    protected void toggle_caps()
    {
        caps.set_sensitive(keymap.get_caps_lock_state());
        if (keymap.get_caps_lock_state()) {
            caps.set_tooltip_text("Caps lock is active");
            caps.set_from_resource("/org/vala-panel/kbled/capslock-on.png");
        } else {
            caps.set_tooltip_text("Caps lock is not active");
            caps.set_from_resource("/org/vala-panel/kbled/capslock-off.png");
        }
    }

    /* Handle num lock changes */
    protected void toggle_num()
    {
        num.set_sensitive(keymap.get_num_lock_state());
        if (keymap.get_num_lock_state()) {
            caps.set_tooltip_text("Caps lock is active");
            caps.set_from_resource("/org/vala-panel/kbled/numlock-on.png");
        } else {
            caps.set_tooltip_text("Caps lock is not active");
            caps.set_from_resource("/org/vala-panel/kbled/numlock-off.png");
        }
    }
    /* Handle scroll lock changes */
//~     protected void toggle_scroll()
//~     {
//~         num.set_sensitive(keymap.get_num_lock_state());
//~         if (keymap.get_num_lock_state()) {
//~             caps.set_tooltip_text("Caps lock is active");
//~             caps.set_from_resource("/org/vala-panel/kbled/numlock-on.png");
//~         } else {
//~             caps.set_tooltip_text("Caps lock is not active");
//~             caps.set_from_resource("/org/vala-panel/kbled/numlock-off.png");
//~         }
//~     }
    protected void on_state_changed()
    {
        toggle_caps();
        toggle_num();
//~         toggle_scroll();
    }
} // End class

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(KbLEDApplet));
}
