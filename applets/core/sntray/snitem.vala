using GLib;
using Gtk;

public class SNItem : FlowBoxChild
{
	public ObjectPath object_path
	{private get; internal construct;}
	public string object_name
	{private get; internal construct;}
	public SNStatus status
	{get; private set;}
	public uint ordering_index
	{get {return proxy.index;}}
	public SNCategory cat
	{get {return proxy.category;}}
	private SNItemProxy proxy;
	private EventBox ebox;
	private Box box;
	private Label label;
	private Image image;
	private Image overlay_image;
	private Overlay icon_overlay;
	private bool is_attention_icon;
	DBusMenuGTKClient? client;
	Gtk.Menu menu;
	public SNItem (string n, ObjectPath p)
	{
		Object(object_path: p, object_name: n);
	}
	construct
	{
		is_attention_icon = false;
		PanelCSS.apply_from_resource(this,"/org/vala-panel/lib/style.css","grid-child");
		proxy = new SNItemProxy(object_name,object_path);
		proxy.bind_property("has-tooltip",this,"has-tooltip",BindingFlags.SYNC_CREATE);
		client = null;
		ebox = new EventBox();
		box = new Box(Orientation.HORIZONTAL,0);
		label = new Label(null);
		image = new Image();
		overlay_image = new Image();
		icon_overlay = new Overlay();
		icon_overlay.add(image);
		icon_overlay.add_overlay(overlay_image);
		box.add(icon_overlay);
		box.add(label);
		ebox.add(box);
		this.add(ebox);
		init_all();
		ebox.scroll_event.connect((e)=>{
			double dx,dy;
			int x,y;
			if (e.get_scroll_deltas(out dx, out dy))
			{
				x = (int) Math.round(dx);
				y = (int) Math.round(dy);
			}
			else
			{
				x = (int) Math.round(e.x - e.x_root);
				y = (int) Math.round(e.y - e.y_root);
			}
			scroll(x,y);
		});
		ebox.button_press_event.connect((e)=>{
			if (e.type == Gdk.EventType.DOUBLE_BUTTON_PRESS)
				this.primary_activate();
			return false;
		});
		ebox.enter_notify_event.connect((e)=>{
			this.get_style_context().add_class("-panel-launch-button-selected");
		});
		ebox.leave_notify_event.connect((e)=>{
			this.get_style_context().remove_class("-panel-launch-button-selected");
		});
		proxy.notify.connect((pspec)=>{
			if (pspec.name == "status")
				iface_new_status_cb();
			if (pspec.name == "main-icon")
				iface_new_icon_cb();
			if (pspec.name == "overlay-icon")
				iface_new_overlay_icon_cb();
			if (pspec.name == "attention-icon")
				iface_new_attention_icon_cb();
			if (pspec.name == "label")
				iface_new_label_cb();
			if (pspec.name == "main-icon")
				iface_new_icon_cb();
			if (pspec.name == "category" || pspec.name == "status")
				this.changed();
		});
		proxy.proxy_destroyed.connect(()=>{
			this.get_applet().request_remove_item(this,object_name+(string)object_path);
		});
		this.query_tooltip.connect(query_tooltip_cb);
		this.show_all();
	}
	private void init_all()
	{
		if (proxy.items_in_menu || proxy.menu != null)
			setup_inner_menu();
		iface_new_status_cb();
		iface_new_path_cb();
		iface_new_label_cb();
	}
	private void iface_new_path_cb()
	{
		IconTheme.get_default().append_search_path(proxy.icon_theme_path);
		iface_new_icon_cb();
		iface_new_overlay_icon_cb();
		iface_new_attention_icon_cb();
	}
	private void iface_new_status_cb()
	{
		this.status = proxy.status;
		switch(this.status)
		{
			case SNStatus.PASSIVE:
			case SNStatus.ACTIVE:
				this.get_style_context().remove_class(STYLE_CLASS_NEEDS_ATTENTION);
				break;
			case SNStatus.NEEDS_ATTENTION:
				this.get_style_context().add_class(STYLE_CLASS_NEEDS_ATTENTION);
				break;
		}
	}
	private bool query_tooltip_cb(int x, int y, bool keyboard, Tooltip tip)
	{
		tip.set_icon_from_gicon(proxy.tooltip_icon,IconSize.DND);
		tip.set_markup(proxy.tooltip_markup ?? proxy.title);
		return true;
	}
	private void iface_new_icon_cb()
	{
		if (is_attention_icon)
			return;
		if (proxy.main_icon != null)
		{
			image.set_from_gicon(proxy.main_icon,IconSize.MENU);
			image.show();
		}
		else
			image.hide();
			
	}
	private void iface_new_overlay_icon_cb()
	{
		if (proxy.overlay_icon != null)
		{
			overlay_image.set_from_gicon(proxy.overlay_icon,IconSize.MENU);
			overlay_image.show();
		}
		else
			overlay_image.hide();
	}
	private void iface_new_attention_icon_cb()
	{
		if (proxy.attention_icon != null)
		{
			image.set_from_gicon(proxy.attention_icon,IconSize.MENU);
			is_attention_icon = true;
			image.show();
		}
		iface_new_icon_cb();
	}
	private void iface_new_label_cb()
	{
		if (proxy.label != null)
		{
			this.label.set_text(proxy.label);
			this.label.show();
		}
		else
			this.label.hide();
		/* FIXME: Guided labels */
	}
	private SNTray get_applet()
	{
		return this.get_parent().get_parent() as SNTray;
	}
	private void setup_inner_menu()
	{
		/*FIXME: MenuModel support */
		if (client == null)
		{
			client = new DBusMenuGTKClient(object_name,proxy.menu);
			menu = new Gtk.Menu();
			client.attach_to_menu(menu);
			menu.attach_to_widget(this,null);
		}
	}
	public void primary_activate()
	{
		int x,y;
		get_applet().popup_position_helper(this,out x,out y);
		try
		{
			proxy.activate(x,y);
			return;
		}
		catch (Error e) {}
		try
		{
			proxy.secondary_activate(x,y);
		}
		catch (Error e) {
				stderr.printf("%s\n",e.message);
		}
	}
	public void context_menu()
	{
		int x,y;
		if (proxy.items_in_menu || proxy.menu != null)
		{
			menu.unmap.connect(()=>{(this.get_parent() as FlowBox).unselect_child(this);});
			menu.popup(null,null,get_applet().menu_position_func,0,0);
			return;
		}
		get_applet().popup_position_helper(this,out x,out y);
		try
		{
			proxy.context_menu(x,y);
		}
		catch (Error e) {
				stderr.printf("%s\n",e.message);
		}
	}
	public void scroll (int x, int y)
	{
		try
		{
			if(x != 0)
				proxy.scroll(x,"horizontal");
			if(y != 0)
				proxy.scroll(y, "vertical");
		}
		catch (Error e) {
				stderr.printf("%s\n",e.message);
		}
	}
}
