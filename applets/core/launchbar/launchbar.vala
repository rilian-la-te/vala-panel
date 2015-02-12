using ValaPanel;
using Gtk;
using GLib;
public class LaunchbarApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
		                            GLib.Settings? settings,
		                            uint number)
    {
        return new Launchbar(toplevel,settings,number);
    }
}
private class LaunchButton: FlowBoxChild
{
	public string id
	{internal get; internal construct;}
	internal int icon_size
	{get; set;}
	private AppInfo? info;
	private string? uri;
	private string icon_name;
	private string commandline;
	private bool term;
	internal LaunchButton(string str)
	{
		Object(id: str);
	}
	construct
	{
		this.set_has_window(false);
		var ebox = new EventBox();
		Icon? icon;
		get_style_context().remove_class("grid-child");
		PanelCSS.apply_from_resource(this,"/org/vala-panel/lib/style.css","-panel-launch-button");
		try 
		{
			Variant? variant = null;
			Variant.parse(new VariantType("(ssb)"),id,null,null);
			variant.get("(ssb)",out icon_name, out commandline, out term);
			info = AppInfo.create_from_commandline(commandline,null,(term)
													?AppInfoCreateFlags.NEEDS_TERMINAL
													:AppInfoCreateFlags.NONE);
			icon = Icon.new_for_string(icon_name);
		} catch(Error e){}
		try
		{
			uri = Filename.from_uri(id);
			string type = ContentType.guess(Filename.from_uri(uri),null,null);
			info = AppInfo.get_default_for_type(type,true);
		} catch(Error e) {}
		if (info == null)
			info = new DesktopAppInfo(id) as AppInfo;
		Image img = new Image();
		if (icon == null)
			icon = info.get_icon();
		setup_icon(img,icon,null,24);
		this.bind_property("icon-size",img,"pixel-size",BindingFlags.DEFAULT|BindingFlags.SYNC_CREATE);
		ebox.enter_notify_event.connect((e)=>{
			this.get_style_context().add_class("-panel-launch-button-selected");
		});
		ebox.leave_notify_event.connect((e)=>{
			this.get_style_context().remove_class("-panel-launch-button-selected");
		});
		ebox.add(img);
		this.add(ebox);
	}
	internal void launch()
	{
		if (info == null)
		{
			warning("No AppInfo for id: %s",id);
			return;
		}
		var context = this.get_toplevel().get_display().get_app_launch_context();
		try
		{
			if (uri != null)
			{
				List<string> uri_l = new List<string>();
				uri_l.append(uri);
				info.launch_uris(uri_l,context);
			}
			else
				info.launch(null,context);
		} catch (GLib.Error e) {stderr.printf("%s",e.message);}
	}
}
public class Launchbar: Applet, AppletConfigurable
{
	private static const string BUTTONS = "launch-buttons";
    FlowBox layout;
    string[]? ids;

    public Launchbar(ValaPanel.Toplevel toplevel,
		                            GLib.Settings? settings,
		                            uint number)
    {
		base(toplevel,settings,number);
	}
	public Dialog get_config_dialog()
	{
		Dialog dlg = Configurator.generic_config_dlg(_("Keyboard LED"),
							toplevel, this,
							_("Show CapsLock"), null, GenericConfigType.TRIM);
		return dlg; 
	}
	public override void create()
	{
		layout = new FlowBox();
        layout.orientation = (toplevel.orientation == Orientation.HORIZONTAL) ? Orientation.VERTICAL:Orientation.HORIZONTAL;
        layout.activate_on_single_click = true;
        layout.selection_mode = SelectionMode.SINGLE;
        add(layout);
        toplevel.notify["edge"].connect((o,a)=> {
			layout.orientation = (toplevel.orientation == Orientation.HORIZONTAL) ? Orientation.VERTICAL:Orientation.HORIZONTAL;
        });
        layout.set_sort_func(launchbar_layout_sort_func);
        var loaded_ids = settings.get_strv(BUTTONS);
        load_buttons(loaded_ids);
        settings.changed.connect(()=>{
			loaded_ids = settings.get_strv(BUTTONS);
			load_buttons(loaded_ids);
		});
		layout.child_activated.connect((ch)=>{
			var lb = ch as LaunchButton;
			lb.launch();
			layout.unselect_child(lb);
		});
        show_all();
    }
    private void load_buttons(string[] loaded_ids)
    {
		var prev_ids = ids;
		ids = loaded_ids;
		foreach(var w in layout.get_children())
		{
			var lb = w as LaunchButton;
			if (!(lb.id in ids))
				layout.remove(w);
		}
		foreach(var id in loaded_ids)
		{
			if (!(id in prev_ids))
			{
				var btn = new LaunchButton(id);
				toplevel.bind_property(Key.ICON_SIZE,btn,"icon-size",BindingFlags.SYNC_CREATE);
				layout.add(btn);
				layout.show_all();
			}
		}
	}
	private int launchbar_layout_sort_func(FlowBoxChild a, FlowBoxChild b)
	{
		var lb_1 = a as LaunchButton;
		var lb_2 = b as LaunchButton;
		var inta = -1;
		var intb = -1;
		for (int i=0; i < ids.length; i++) {
			if(lb_1.id == ids[i]) inta = i;
			if(lb_2.id == ids[i]) intb = i;
		}
		return inta - intb;
	}
} // End class

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(LaunchbarApplet));
}
