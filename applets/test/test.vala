using ValaPanel;
public class LockKeysApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
		                            GLib.Settings? settings,
		                            uint number)
    {
        return new LockKeysAppletImpl(toplevel,settings,number);
    }
    public Features features
    {get{
		return Features.EXPAND_AVAILABLE;
	}}
}

public class LockKeysAppletImpl : Applet
{

    Gtk.Box widget;
    Gtk.Label caps;
    Gtk.Label num;
    Gdk.Keymap map;

    public LockKeysAppletImpl(ValaPanel.Toplevel toplevel,
		                            GLib.Settings? settings,
		                            uint number)
    {
		base(toplevel,settings,number);
	}
	public override void create()
	{
        widget = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
        add(widget);

        /* Pretty labels, probably use icons in future */
        caps = new Gtk.Label("<big><b>A</b></big>");
        caps.use_markup = true;
        caps.margin = 5;
        num = new Gtk.Label("<big><b>1</b></big>");
        num.use_markup = true;
        num.margin = 5;
        widget.pack_start(caps, false, false, 0);
        widget.pack_start(num, false, false, 0);

        map = Gdk.Keymap.get_default();
        map.state_changed.connect(on_state_changed);

        on_state_changed();

        toplevel.notify["edge"].connect((o,a)=> {
            widget.set_orientation(toplevel.orientation);
        });

        show_all();
    }

    /* Handle caps lock changes */
    protected void toggle_caps()
    {
        caps.set_sensitive(map.get_caps_lock_state());
        if (map.get_caps_lock_state()) {
            caps.set_tooltip_text("Caps lock is active");
            caps.get_style_context().remove_class("dim-label");
        } else {
            caps.set_tooltip_text("Caps lock is not active");
            caps.get_style_context().add_class("dim-label");
        }
    }

    /* Handle num lock changes */
    protected void toggle_num()
    {
        num.set_sensitive(map.get_num_lock_state());
        if (map.get_num_lock_state()) {
            num.set_tooltip_text("Num lock is active");
            num.get_style_context().remove_class("dim-label");
        } else {
            num.set_tooltip_text("Num lock is not active");
            num.get_style_context().add_class("dim-label");
        }
    }

    protected void on_state_changed()
    {
        toggle_caps();
        toggle_num();
    }
} // End class

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(LockKeysApplet));
}
