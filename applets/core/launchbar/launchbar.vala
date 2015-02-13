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
	internal LaunchButton(string str)
	{
		Object(id: str);
	}
	construct
	{
		var commit = false;
		var ebox = new EventBox();
		Icon? icon;
		get_style_context().remove_class("grid-child");
		PanelCSS.apply_from_resource(this,"/org/vala-panel/lib/style.css","-panel-launch-button");
		if (info == null)
			info = new DesktopAppInfo(id) as AppInfo;
		if (info == null)
		{
			uri = id;
			string type = ContentType.guess(id,null,null);
			info = AppInfo.get_default_for_type(type,true);
		}
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
		ebox.show();
		drag_source_set(this,Gdk.ModifierType.BUTTON2_MASK,MenuMaker.menu_targets,Gdk.DragAction.MOVE);
		drag_source_set_icon_gicon(this,icon);
		this.drag_begin.connect((context)=>{
			this.get_launchbar().request_remove_id(id);
			this.hide();
		});
		this.drag_data_get.connect((context,data,type,time)=>{
			var uri_list = new string[1];
			uri_list[0]=id;
			data.set_uris(uri_list);
		});
		this.drag_data_delete.connect((context)=>{
			commit = true;
			this.destroy();
		});
		this.drag_failed.connect((context,result)=>{
			if (result == Gtk.DragResult.USER_CANCELLED)
			{
				this.show();
				this.get_launchbar().undo_removal_request();
			}
			else
				commit = true;
		});
		this.drag_end.connect((context)=>{
			if (commit)
				this.get_launchbar().commit_ids();
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
	private Launchbar get_launchbar()
	{
		return this.get_parent().get_parent() as Launchbar;
	}
}
public class Launchbar: Applet, AppletConfigurable
{
	private static const string BUTTONS = "launch-buttons";
    FlowBox layout;
    string[]? ids;
    string[]? prev_ids;

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
		Gtk.drag_dest_set (
                layout,                     // widget that will accept a drop
                DestDefaults.MOTION       // default actions for dest on DnD
                | DestDefaults.HIGHLIGHT,
                MenuMaker.menu_targets,              // lists of target to support
                Gdk.DragAction.COPY|Gdk.DragAction.MOVE        // what to do with data after dropped
            );
        layout.orientation = (toplevel.orientation == Orientation.HORIZONTAL) ? Orientation.VERTICAL:Orientation.HORIZONTAL;
        layout.activate_on_single_click = true;
        layout.selection_mode = SelectionMode.SINGLE;
        add(layout);
        toplevel.notify["edge"].connect((o,a)=> {
			layout.orientation = (toplevel.orientation == Orientation.HORIZONTAL) ? Orientation.VERTICAL:Orientation.HORIZONTAL;
        });
        layout.drag_drop.connect(drag_drop_cb);
        layout.drag_data_received.connect(drag_data_received_cb);
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
    internal void request_remove_id(string id)
    {
		int idx;
		for(idx = 0; idx< ids.length; idx++)
			if (ids[idx]==id) break;
		prev_ids = ids;
		ids = concat_strv_uniq(ids[0:idx],ids[idx+1:ids.length]);
	}
	internal void commit_ids()
	{
		settings.set_strv(BUTTONS,ids);
	}
	internal void undo_removal_request()
	{
		ids = prev_ids;
		prev_ids = null;
	}
    private bool drag_drop_cb (Widget widget, Gdk.DragContext context,
                               int x, int y, uint time)
    {
        bool is_valid_drop_site = true;
        if (context.list_targets() != null)
        {
            var target_type = (Gdk.Atom) context.list_targets().nth_data (0);
            Gtk.drag_get_data (widget, context, target_type, time);
        }
        else
			is_valid_drop_site = false;
        return is_valid_drop_site;
    }
    private void drag_data_received_cb (Widget widget, Gdk.DragContext context,
										int x, int y,
    									SelectionData selection_data,
    									uint target_type, uint time)
    {
		bool delete_selection_data = false;
        bool dnd_success = false;
        int index = 0;
        Gdk.Rectangle r = {x, y, 1, 1};
        var flowbox = widget as FlowBox;
        foreach(var w in flowbox.get_children())
        {
			if (w.intersect(r,null))
				break;
			index++;
		}
        if (context.get_suggested_action() == Gdk.DragAction.MOVE) {
			delete_selection_data = true;
        }
        if ((selection_data != null) && (selection_data.get_length() >= 0)) {
            string []? new_ids = null;
            var loaded_ids = selection_data.get_uris();
            if (index>=0)
				new_ids = concat_strv_uniq(ids[0:index],loaded_ids,ids[index:ids.length]);
			else
				new_ids = concat_strv_uniq(loaded_ids,ids);
			settings.set_strv(BUTTONS,new_ids);
			dnd_success = true;
        }
        if (dnd_success == false) {
            stderr.printf ("Invalid DnD data.\n");
        }
        Gtk.drag_finish(context,dnd_success,delete_selection_data,time);
	}
    private void load_buttons(string[] loaded_ids)
    {
		prev_ids = ids;
		ids = loaded_ids;
		foreach(var w in layout.get_children())
		{
			var lb = w as LaunchButton;
			if (!(lb.id in ids))
				layout.remove(w);
		}
		foreach(var id in ids)
		{
			if (!(id in prev_ids))
			{
				var btn = new LaunchButton(id);
				toplevel.bind_property(Key.ICON_SIZE,btn,"icon-size",BindingFlags.SYNC_CREATE);
				layout.add(btn);
				layout.show_all();
			}
		}
		layout.invalidate_sort();
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
	private string[] concat_strv_uniq(string[] arr1, string[]? arr2 = null, string[]? arr3 = null)
	{
		var res = arr1;
		if (arr2 != null)
			foreach(var s in arr2)
				if(!(s in res)) res+=s;
		if (arr3 != null)
			foreach(var s in arr3)
				if(!(s in res)) res+=s;
		return res;
	}
} // End class

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(LaunchbarApplet));
}
