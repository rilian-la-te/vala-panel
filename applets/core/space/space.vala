using ValaPanel;
using Gtk;
public class SpaceApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
		                            GLib.Settings? settings,
		                            uint number)
    {
        return new Space(toplevel,settings,number);
    }
}
public class Space: Applet, AppletConfigurable
{
	private static const string KEY_SIZE = "size";
	internal int size
	{get; set;}

    public Space(ValaPanel.Toplevel toplevel,
		                            GLib.Settings? settings,
		                            uint number)
    {
		base(toplevel,settings,number);
	}
	public override void create()
	{
		this.set_has_window(false);
		toplevel.notify["edge"].connect((pspec)=>{
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
		this.show_all();
	}
	public Dialog get_config_dialog()
	{
		return Configurator.generic_config_dlg(_("Space Applet"),
							toplevel, this,
							_("Size"), KEY_SIZE, GenericConfigType.INT);
	}
} // End class

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(SpaceApplet));
}
