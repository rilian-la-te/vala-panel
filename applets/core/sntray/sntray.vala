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
public class SNTray: Applet
{
	HashTable<string,SNItem> items;
	FlowBox layout;
	static SNHost host;
	static construct
	{
		host = new SNHost.from_path("org.kde.StatusNotifierHost-valapanel%d".printf(Gdk.CURRENT_TIME));
	}
	public SNTray (Toplevel top, GLib.Settings? settings, uint number)
	{
		base(top,settings,number);
	}
	public override void create()
	{
		items = new HashTable<string,SNItem>(str_hash, str_equal);
		layout = new FlowBox();
		layout.activate_on_single_click = true;
		layout.selection_mode = SelectionMode.SINGLE;
        layout.orientation = (toplevel.orientation == Orientation.HORIZONTAL) ? Orientation.VERTICAL:Orientation.HORIZONTAL;
        toplevel.notify["edge"].connect((o,a)=> {
			layout.orientation = (toplevel.orientation == Orientation.HORIZONTAL) ? Orientation.VERTICAL:Orientation.HORIZONTAL;
        });
		layout.child_activated.connect((ch)=>{
			(ch as SNItem).context_menu();
			layout.unselect_child(ch);
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
		this.remove(child);
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
