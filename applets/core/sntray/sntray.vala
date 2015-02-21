using Gtk;
using GLib;
using ValaPanel;
using StatusNotifier;

public class SNApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
		                            GLib.Settings? settings,
		                            uint number)
    {
        return new SNTray(toplevel,settings,number);
    }
}
public class SNTray: Applet, AppletConfigurable
{
	static const string SHOW_APPS = "show-application-status";
	static const string SHOW_COMM = "show-communications";
	static const string SHOW_SYS = "show-system";
	static const string SHOW_HARD = "show-hardware";
	static const string SHOW_PASSIVE = "show-passive";
	static const string INDICATOR_SIZE = "indicator-size";

	ItemBox layout;
	public SNTray (Toplevel top, GLib.Settings? settings, uint number)
	{
		base(top,settings,number);
	}
	public Dialog get_config_dialog()
	{
		return Configurator.generic_config_dlg(_("StatusNotifier"),
							toplevel, this,
							_("Indicator icon size"), INDICATOR_SIZE, GenericConfigType.INT,
							_("Show applications status items"), SHOW_APPS, GenericConfigType.BOOL,
							_("Show communications applications"), SHOW_COMM, GenericConfigType.BOOL,
							_("Show system services"), SHOW_SYS, GenericConfigType.BOOL,
							_("Show hardware services"), SHOW_HARD, GenericConfigType.BOOL,
							_("Show passive items"), SHOW_HARD, GenericConfigType.BOOL);
	}
	public override void create()
	{
		layout = new ItemBox();
		settings.bind(SHOW_APPS,layout,SHOW_APPS,SettingsBindFlags.GET);
		settings.bind(SHOW_COMM,layout,SHOW_COMM,SettingsBindFlags.GET);
		settings.bind(SHOW_SYS,layout,SHOW_SYS,SettingsBindFlags.GET);
		settings.bind(SHOW_HARD,layout,SHOW_HARD,SettingsBindFlags.GET);
		settings.bind(SHOW_PASSIVE,layout,SHOW_PASSIVE,SettingsBindFlags.GET);
		settings.bind(INDICATOR_SIZE,layout,"icon-size",SettingsBindFlags.GET);
		settings.changed.connect((k)=>{layout.invalidate_filter();});
        layout.orientation = (toplevel.orientation == Orientation.HORIZONTAL) ? Orientation.VERTICAL:Orientation.HORIZONTAL;
        toplevel.notify["edge"].connect((o,a)=> {
			layout.orientation = (toplevel.orientation == Orientation.HORIZONTAL) ? Orientation.VERTICAL:Orientation.HORIZONTAL;
        });
        layout.menu_position_func = this.menu_position_func;
		this.add(layout);
		show_all();
	}
} // End class

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(SNApplet));
}
