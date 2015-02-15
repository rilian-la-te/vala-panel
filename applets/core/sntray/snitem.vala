using GLib;
using Gtk;

[DBus (use_string_marshalling = true)]
public enum SNCategory
{
	[DBus (value = "ApplicationStatus")]
    APPLICATION,
    [DBus (value = "Communications")]
    COMMUNICATIONS,
    [DBus (value = "SystemServices")]
    SYSTEM,
    [DBus (value = "Hardware")]
    HARDWARE
}

[DBus (use_string_marshalling = true)]
public enum SNStatus
{
	[DBus (value = "Passive")]
    PASSIVE,
    [DBus (value = "Active")]
    ACTIVE,
    [DBus (value = "NeedsAttention")]
    NEEDS_ATTENTION
}

[DBus (name = "org.kde.StatusNotifierItem")]
public interface SNItemIface : Object
{
	/* Base properties */
	public abstract SNCategory category
	{owned get;}
	public abstract string id
	{owned get;}
	public abstract SNStatus status
	{owned get;}
	public abstract string title
	{owned get;}
	public abstract string window_id
	{owned get;}
	/* Menu properties */
	[DBus(signature = "o")]
	public abstract ObjectPath menu
	{owned get;}
	public abstract bool items_in_menu
	{owned get;}
	/* Icon properties */
	public abstract string icon_theme_path
	{owned get;}
	public abstract string icon_name
	{owned get;}
	[DBus(signature = "(iiay)")]
	public abstract Variant icon_pixmap
	{owned get;}
	public abstract string overlay_icon_name
	{owned get;}
	[DBus(signature = "(iiay)")]
	public abstract Variant overlay_icon_pixmap
	{owned get;}
	public abstract string attention_icon_name
	{owned get;}
	[DBus(signature = "(iiay)")]
	public abstract Variant attention_icon_pixmap
	{owned get;}
	public abstract string attention_movie_name
	{owned get;}
	/* Tooltip */
	[DBus(signature = "(s(iiay)ss)")]
	public abstract Variant tool_tip
	{owned get;}
	/* Methods */
	public abstract void context_menu(int x, int y) throws IOError;
	public abstract void activate(int x, int y) throws IOError;
	public abstract void secondary_activate(int x, int y) throws IOError;
	public abstract void scroll(int delta, string orientation) throws IOError;
	/* Signals */
	public abstract signal void new_title();
	public abstract signal void new_icon();
	public abstract signal void new_icon_theme_path(string icon_theme_path);
	public abstract signal void new_attention_icon();
	public abstract signal void new_overlay_icon();
	public abstract signal void new_tool_tip();
	public abstract signal void new_status(SNStatus status);
	/* Ayatana */
	public abstract string x_ayatana_label {owned get;}
	public abstract string x_ayatana_label_guide {owned get;}
	public abstract uint x_ayatana_ordering_index {get;}
	public abstract signal void x_ayatana_new_label(string label, string guide);
}

public class SNItem : FlowBoxChild
{
	public ObjectPath object_path
	{private get; internal construct;}
	public string object_name
	{private get; internal construct;}
	public SNStatus status
	{get; private set;}
	public uint ordering_index
	{get {return iface.x_ayatana_ordering_index;}}
	public SNCategory cat
	{get {return iface.category;}}
	private SNItemIface iface;
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
		PanelCSS.apply_from_resource(this,"/org/vala-panel/lib/style.css","grid-child");
		client = null;
		try
		{
			iface = Bus.get_proxy_sync (BusType.SESSION,object_name,object_path);
			uint id;
			id = Bus.watch_name(BusType.SESSION,object_name,GLib.BusNameWatcherFlags.NONE,
				null,
				() => {
						Bus.unwatch_name(id);
						this.get_applet().request_remove_item(this,object_name+(string)object_path);
						}
				);
		} catch (IOError e) {stderr.printf ("%s\n", e.message); this.destroy();}
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
		if (iface != null)
		{
			/* FIXME: It is not by spec */
//~ 			iface.notify.connect(iface_property_cb);
			iface.new_status.connect(iface_new_status_cb);
			iface.new_icon_theme_path.connect(iface_new_path_cb);
			iface.new_icon.connect(iface_new_icon_cb);
			iface.new_tool_tip.connect(setup_tooltip_cb);
			iface.new_overlay_icon.connect(iface_new_overlay_icon_cb);
			iface.new_attention_icon.connect(iface_new_attention_icon_cb);
			iface.x_ayatana_new_label.connect(iface_new_label_cb);
			this.query_tooltip.connect(query_tooltip_cb);
		}
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
		this.show_all();
	}
	private void init_all()
	{
		if (iface.items_in_menu || iface.menu != null)
			setup_inner_menu();
		iface_new_status_cb(iface.status);
		iface_new_path_cb(iface.icon_theme_path);
		iface_new_label_cb(iface.x_ayatana_label,iface.x_ayatana_label_guide);
	}
	private void iface_new_status_cb(SNStatus st)
	{
		this.status = st;
		switch(st)
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
	private void iface_new_path_cb(string path)
	{
		Gtk.IconTheme.get_default().append_search_path(path);
		iface_new_attention_icon_cb();
		iface_new_icon_cb();
		iface_new_overlay_icon_cb();
	}
	private bool change_icon(Image? image, string? icon_name, Variant? pixmap, out Icon? outicon)
	{
		Icon? icon = null;
		if (icon_name != null && icon_name.length > 0)
		{
			icon = new ThemedIcon.with_default_fallbacks(icon_name+"-symbolic");
			(icon as ThemedIcon).prepend_name(icon_name+"-panel");
		}
		if (icon == null)
		{
			uint8 [] pixbuf;
			int width, height;
			if (pixmap != null)
			{
				pixmap.get("iiay",out width, out height, out pixbuf);
				icon = new BytesIcon(new Bytes(pixbuf));
			}
			else
				icon = null;
		}
		if (icon != null)
		{
			if (image != null)
			{
				image.visible = true;
				image.set_from_gicon(icon,IconSize.MENU);
			}
			outicon = icon;
			return true;
		}
		else if (image != null)
			image.visible = false;
		outicon = null;
		return false;
	}
	private void setup_tooltip_cb()
	{
		this.has_tooltip = iface.tool_tip != null;
	}
	private bool query_tooltip_cb(int x, int y, bool keyboard, Tooltip tip)
	{
		if(!this.has_tooltip)
			return false;
		Variant? tooltip = iface.tool_tip;
		if(tooltip == null)
		{
			this.has_tooltip = false;
			return false;
		}
		string icon_name = tooltip.get_child_value(0).get_string();
		Variant icon_pixmap = tooltip.get_child_value(1);
		string name = tooltip.get_child_value(2).get_string();
		string desc = tooltip.get_child_value(3).get_string();
		Icon icon;
		change_icon(null,icon_name,icon_pixmap,out icon);
		tip.set_icon_from_gicon(icon,IconSize.DND);
		tip.set_markup("<b>%s</b>\n%s".printf(name,desc));
		return true;
	}
	private void iface_new_icon_cb()
	{
		if (is_attention_icon)
			return;
		change_icon(image,iface.icon_name, iface.icon_pixmap,null);
	}
	private void iface_new_overlay_icon_cb()
	{
		change_icon(overlay_image,iface.overlay_icon_name,iface.overlay_icon_pixmap,null);
	}
	private void iface_new_attention_icon_cb()
	{
		is_attention_icon = change_icon(image,iface.attention_icon_name,iface.attention_icon_pixmap,null);
		iface_new_icon_cb();
	}
	private void iface_new_label_cb(string? label, string? guide)
	{
		if (label != null)
		{
			this.label.set_text(label);
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
			client = new DBusMenuGTKClient(object_name,iface.menu);
			menu = new Gtk.Menu();
			client.attach_to_menu(menu);
		}
	}
	public void primary_activate()
	{
		int x,y;
		get_applet().popup_position_helper(this,out x,out y);
		try
		{
			iface.activate(x,y);
			return;
		}
		catch (Error e) {}
		try
		{
			iface.secondary_activate(x,y);
		}
		catch (Error e) {
				stderr.printf("%s\n",e.message);
		}
	}
	public void context_menu()
	{
		int x,y;
		if (iface.items_in_menu || iface.menu != null)
		{
			menu.unmap.connect(()=>{(this.get_parent() as FlowBox).unselect_child(this);});
			menu.popup(null,null,get_applet().menu_position_func,0,0);
			return;
		}
		get_applet().popup_position_helper(this,out x,out y);
		try
		{
			iface.context_menu(x,y);
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
				iface.scroll(x,"horizontal");
			if(y != 0)
				iface.scroll(y, "vertical");
		}
		catch (Error e) {
				stderr.printf("%s\n",e.message);
		}
	}
}
