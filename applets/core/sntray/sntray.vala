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
	ItemBox layout;
	public SNTray (Toplevel top, GLib.Settings? settings, uint number)
	{
		base(top,settings,number);
	}
	public Dialog get_config_dialog()
	{
		var dlg = new ConfigDialog(layout);
		dlg.configure_icon_size = true;
		return dlg;
	}
	public override void create()
	{
		layout = new ItemBox();
		settings.bind(SHOW_APPS,layout,SHOW_APPS,SettingsBindFlags.DEFAULT);
		settings.bind(SHOW_COMM,layout,SHOW_COMM,SettingsBindFlags.DEFAULT);
		settings.bind(SHOW_SYS,layout,SHOW_SYS,SettingsBindFlags.DEFAULT);
		settings.bind(SHOW_HARD,layout,SHOW_HARD,SettingsBindFlags.DEFAULT);
		settings.bind(SHOW_OTHER,layout,SHOW_OTHER,SettingsBindFlags.DEFAULT);
		settings.bind(SHOW_PASSIVE,layout,SHOW_PASSIVE,SettingsBindFlags.DEFAULT);
		settings.bind(INDICATOR_SIZE,layout,"icon-size",SettingsBindFlags.DEFAULT);
		settings.bind(USE_SYMBOLIC,layout,USE_SYMBOLIC,SettingsBindFlags.DEFAULT);
		settings.bind_with_mapping(INDEX_OVERRIDE,layout,INDEX_OVERRIDE,SettingsBindFlags.DEFAULT,
								   (SettingsBindGetMappingShared)get_vardict,
								   (SettingsBindSetMappingShared)set_vardict,
								   (void*)"i",null);
		settings.bind_with_mapping(FILTER_OVERRIDE,layout,FILTER_OVERRIDE,SettingsBindFlags.DEFAULT,
		                           (SettingsBindGetMappingShared)get_vardict,
		                           (SettingsBindSetMappingShared)set_vardict,
		                           (void*)"b",null);
		layout.orientation = (toplevel.orientation == Orientation.HORIZONTAL) ? Orientation.VERTICAL:Orientation.HORIZONTAL;
		toplevel.notify["edge"].connect((o,a)=> {
			layout.orientation = (toplevel.orientation == Orientation.HORIZONTAL) ? Orientation.VERTICAL:Orientation.HORIZONTAL;
		});
		layout.menu_position_func = this.menu_position_func;
		this.add(layout);
		show_all();
	}
	private static bool get_vardict(Value val, Variant variant,void* data)
	{
		var iter = variant.iterator();
		string name;
		Variant inner_val;
		var dict = new HashTable<string,Variant?>(str_hash,str_equal);
		while(iter.next("{sv}",out name, out inner_val))
			dict.insert(name,inner_val);
		val.set_boxed((void*)dict);
		return true;
	}
	private static Variant set_vardict(Value val, VariantType type,void* data)
	{
		var builder = new VariantBuilder(type);
		var table = (HashTable<string,Variant?>)val.get_boxed();
		table.foreach((k,v)=>{
			builder.add("{sv}",k,v);
		});
		return builder.end();
	}
} // End class

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(SNApplet));
}
