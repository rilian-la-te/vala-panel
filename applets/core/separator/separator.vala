using ValaPanel;
using Gtk;
public class SepApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
		                            GLib.Settings? settings,
		                            uint number)
    {
        return new Sep(toplevel,settings,number);
    }
}
public class Sep: Applet, AppletConfigurable
{
    Separator widget;
	private static const string KEY_SIZE = "size";
	private static const string KEY_SHOW_SEPARATOR = "show-separator";
	internal int size
	{get; set;}
	internal bool show_separator
	{get; set;}
    public Sep(ValaPanel.Toplevel toplevel,
		                            GLib.Settings? settings,
		                            uint number)
    {
		base(toplevel,settings,number);
	}
	public override void create()
	{
		widget = new Separator(toplevel.orientation == Orientation.HORIZONTAL ? Orientation.VERTICAL : Orientation.HORIZONTAL);
		this.add(widget);
		toplevel.notify["edge"].connect((pspec)=>{
			widget.orientation = toplevel.orientation == Orientation.HORIZONTAL ? Orientation.VERTICAL : Orientation.HORIZONTAL;
			if (toplevel.orientation == Orientation.HORIZONTAL)
				this.set_size_request(size,2);
			else this.set_size_request(2,size);
		});
		this.notify.connect((pspec)=>{
			if (toplevel.orientation == Orientation.HORIZONTAL)
				this.set_size_request(size,2);
			else this.set_size_request(2,size);
		});
		settings.bind(KEY_SIZE,this,KEY_SIZE,SettingsBindFlags.GET);
		settings.bind(KEY_SHOW_SEPARATOR,this,KEY_SHOW_SEPARATOR,SettingsBindFlags.GET);
		this.bind_property(KEY_SHOW_SEPARATOR,widget,"visible",BindingFlags.SYNC_CREATE);
		this.show_all();
	}
	public Dialog get_config_dialog()
	{
		return Configurator.generic_config_dlg(_("Separator Applet"),
							toplevel, this,
							_("Size"), KEY_SIZE, GenericConfigType.INT,
							_("Visible separator"), KEY_SHOW_SEPARATOR, GenericConfigType.BOOL);
	}
} // End class

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(SepApplet));
}
