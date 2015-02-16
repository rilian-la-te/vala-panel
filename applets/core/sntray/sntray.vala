using ValaPanel;
using Gtk;
using GLib;
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
	static SNHost host;
	internal bool show_application_status
	{get; set;}
	internal bool show_communications
	{get; set;}
	internal bool show_system
	{get; set;}
	internal bool show_hardware
	{get; set;}
	internal IconSize icon_size
	{get; set;}
	HashTable<string,SNItem> items;
	FlowBox layout;
	static construct
	{
		host = new SNHost.from_path("org.kde.StatusNotifierHost-valapanel%d".printf(Gdk.CURRENT_TIME));
	}
	public SNTray (Toplevel top, GLib.Settings? settings, uint number)
	{
		base(top,settings,number);
	}
	public Dialog get_config_dialog()
	{
		return Configurator.generic_config_dlg(_("StatusNotifier"),
							toplevel, this,
							_("Show applications status items"), SHOW_APPS, GenericConfigType.BOOL,
							_("Show communications applications"), SHOW_COMM, GenericConfigType.BOOL,
							_("Show system services"), SHOW_SYS, GenericConfigType.BOOL,
							_("Show hardware services"), SHOW_HARD, GenericConfigType.BOOL);
	}
	public override void create()
	{
		items = new HashTable<string,SNItem>(str_hash, str_equal);
		layout = new FlowBox();
		settings.bind(SHOW_APPS,this,SHOW_APPS,SettingsBindFlags.GET);
		settings.bind(SHOW_COMM,this,SHOW_COMM,SettingsBindFlags.GET);
		settings.bind(SHOW_SYS,this,SHOW_SYS,SettingsBindFlags.GET);
		settings.bind(SHOW_HARD,this,SHOW_HARD,SettingsBindFlags.GET);
		settings.changed.connect((k)=>{layout.invalidate_filter();});
		layout.selection_mode = SelectionMode.SINGLE;
		layout.activate_on_single_click = true;
        layout.orientation = (toplevel.orientation == Orientation.HORIZONTAL) ? Orientation.VERTICAL:Orientation.HORIZONTAL;
        toplevel.notify["edge"].connect((o,a)=> {
			layout.orientation = (toplevel.orientation == Orientation.HORIZONTAL) ? Orientation.VERTICAL:Orientation.HORIZONTAL;
        });
		layout.child_activated.connect((ch)=>{
			layout.select_child(ch);
			(ch as SNItem).context_menu();
		});
		layout.set_sort_func((ch1,ch2)=>{return (int)(ch1 as SNItem).ordering_index - (int)(ch2 as SNItem).ordering_index;});
		layout.set_filter_func((ch)=>{
			var item = ch as SNItem;
			if (show_application_status && item.cat == SNCategory.APPLICATION) return true;
			if (show_communications && item.cat == SNCategory.COMMUNICATIONS) return true;
			if (show_system && item.cat == SNCategory.SYSTEM) return true;
			if (show_hardware && item.cat == SNCategory.HARDWARE) return true;
			return false;
		});
		this.add(layout);
		host.watcher_registered.connect(()=>{
			recreate_items();
			host.watcher_items_changed.connect(()=>{
				recreate_items();
			});
		});
		show_all();
	}
	private void recreate_items()
	{
		string[] new_items = host.watcher_items();
		foreach (var item in new_items)
		{
			string[] np = item.split("/",2);
			if (!items.contains(item))
			{
				var snitem = new SNItem(np[0],(ObjectPath)("/"+np[1]));
				items.insert(item, snitem);
				layout.add(snitem);
			}
		}
	}
	internal void request_remove_item(FlowBoxChild child, string item)
	{
		items.remove(item);
		layout.remove(child);
		child.destroy();
	}
} // End class

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(SNApplet));
}
