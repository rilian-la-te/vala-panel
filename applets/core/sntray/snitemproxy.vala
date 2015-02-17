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
internal SNStatus status_from_string(string str)
{
	switch(str)
	{
		case "NeedsAttention":
			return SNStatus.NEEDS_ATTENTION;
		case "Active":
			return SNStatus.ACTIVE;
		case "Passive":
			return SNStatus.PASSIVE;
	}
	return SNStatus.PASSIVE;
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
	public abstract int window_id
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
	[DBus(signature = "a(iiay)")]
	public abstract Variant icon_pixmap
	{owned get;}
	public abstract string overlay_icon_name
	{owned get;}
	[DBus(signature = "a(iiay)")]
	public abstract Variant overlay_icon_pixmap
	{owned get;}
	public abstract string attention_icon_name
	{owned get;}
	[DBus(signature = "a(iiay)")]
	public abstract Variant attention_icon_pixmap
	{owned get;}
	public abstract string attention_movie_name
	{owned get;}
	/* Tooltip */
	[DBus(signature = "(sa(iiay)ss)")]
	public abstract Variant tool_tip {owned get;}
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

[DBus (name = "org.freedesktop.DBus.Properties")]
public interface FreeDesktopProperties : Object{
	[DBus (visible="false")]
	public static const string NAME = "org.freedesktop.DBus.Properties";
	[DBus (visible="false")]
	public static const string KDE_NAME = "org.kde.StatusNotifierItem";
	[DBus (name="Get")]
	public abstract Variant get_one(string interface_name, string property_name) throws IOError;
	public abstract HashTable<string, Variant?> get_all(string interface_name) throws IOError;
	public signal void properties_changed (string source, HashTable<string, Variant?> changed_properties,
											string[] invalid );

}

private enum IconType
{
	MAIN,
	OVERLAY,
	ATTENTION
}

/* "a(iiay)" - array of ByteIcons */
/* "(sa(iiay)ss)" - tooltip */
/* FIXME: Icons always use DirectAccess now */
public class SNItemProxy: Object
{
	private SNItemIface iface;
	private FreeDesktopProperties props_iface;
	public string bus_name
	{private get; internal construct;}
	public string object_path
	{private get; internal construct;}
		/* Base properties */
	public SNCategory category {get {return iface.category;}}
	public string id {owned get {return iface.id;}}
	public SNStatus status {get; private set;}
	public string title {get; private set;}
	/* Menu properties */
	public ObjectPath menu {owned get {return iface.menu;}}
	public bool items_in_menu {get {return iface.items_in_menu;}}
	/* Icon properties */
	public string icon_theme_path {get; private set;}
	public Icon? main_icon {get; private set;}
	public Icon? overlay_icon {get; private set;}
	public Icon? attention_icon {get; private set;}
	public string attention_movie_name {owned get {return iface.attention_movie_name;}}
	/* Tooltip */
	public bool has_tooltip {get; private set;}
	public Icon? tooltip_icon {get; private set;}
	public string? tooltip_markup {get; private set;}
	public signal void proxy_destroyed();
	/* Ayatana */
	public string? label {get; private set;}
	public string? label_guide {get; private set;}
	public uint index {get {return iface.x_ayatana_ordering_index;}}
	public void context_menu(int x, int y) throws Error
	{
		iface.context_menu(x,y);
	}
	public void activate(int x, int y) throws Error
	{
		iface.activate(x,y);
	}
	public void secondary_activate(int x, int y) throws Error
	{
		iface.secondary_activate(x,y);
	}
	public void scroll(int delta, string orientation) throws Error
	{
		iface.scroll(delta, orientation);
	}
	public SNItemProxy(string name, string path)
	{
		Object(bus_name: name, object_path: path);
	}
	construct
	{
		this.bus_name = bus_name;
		this.object_path = object_path;
		try
		{
			this.iface = Bus.get_proxy_sync (BusType.SESSION,bus_name,object_path);
			uint id;
			id = Bus.watch_name(BusType.SESSION,bus_name,GLib.BusNameWatcherFlags.NONE,
				null,
				() => {Bus.unwatch_name(id); this.proxy_destroyed();}
				);
			this.props_iface = Bus.get_proxy_sync (BusType.SESSION,bus_name,object_path,DBusProxyFlags.GET_INVALIDATED_PROPERTIES);
		} catch (IOError e) {stderr.printf ("%s\n", e.message); this.proxy_destroyed();}
		init_properties();
		props_iface.properties_changed.connect((src,props,inv)=>{if (src == FreeDesktopProperties.KDE_NAME) props_changed_burst(props);});
		iface.new_status.connect((st)=>{this.status = st;});
		iface.new_icon.connect(()=>{this.main_icon = one_icon_direct_cb(IconType.MAIN);});
		iface.new_overlay_icon.connect(()=>{this.overlay_icon = one_icon_direct_cb(IconType.OVERLAY);});
		iface.new_attention_icon.connect(()=>{this.attention_icon = one_icon_direct_cb(IconType.ATTENTION);});
		iface.new_icon_theme_path.connect((pt)=>{this.icon_theme_path = pt;});
		iface.x_ayatana_new_label.connect((lb,g)=>{this.label = lb; this.label_guide = g;});
		iface.new_tool_tip.connect(()=>{tooltip_direct_cb();});
		iface.new_title.connect(()=>{title_direct_cb();});
	}
	private void init_properties()
	{
		try{
			var props = props_iface.get_all(FreeDesktopProperties.KDE_NAME);
			props_changed_burst(props);
		} catch (Error e) {stderr.printf("Cannot set properties: %s\n",e.message);}
	}
	private void props_changed_burst(HashTable<string,Variant?> props)
	{
		all_icons_direct_cb(props);
		var label = props.lookup("ToolTip");
		unbox_tooltip(label);
		label = props.lookup("XAyatanaLabel");
		this.label = (label != null) ? label.get_string() : null;
		label = props.lookup("XAyatanaLabelGuide");
		this.label_guide = (label != null) ? label.get_string() : null;
		label = props.lookup("Title");
		this.title = (label != null) ? label.get_string() : null;
		label = props.lookup("Status");
		this.status = (label != null) ? status_from_string(label.get_string()) : SNStatus.PASSIVE;
		label = props.lookup("IconThemePath");
		this.icon_theme_path = (label != null) ? label.get_string() : null;
	}
	private void title_direct_cb()
	{
		try
		{
			var titlev = props_iface.get_one(FreeDesktopProperties.KDE_NAME,"Title");
			this.title = (titlev != null) ? titlev.get_string() : null;
		} catch (Error e) {stderr.printf("Cannot set title: %s\n",e.message);}
	}
	/*FIXME: icons and tooltip always direct now */
	private void all_icons_direct_cb(HashTable<string,Variant?> props)
	{
		/* Main icon */
		var icon_namev = props.lookup("IconName");
		var icon_pixmap = props.lookup("IconPixmap");
		main_icon = from_direct_props(icon_namev,icon_pixmap);
		/* Attention icon */
		icon_namev = props.lookup("AttentionIconName");
		icon_pixmap = props.lookup("AttentionIconPixmap");
		attention_icon = from_direct_props(icon_namev,icon_pixmap);
		/* Overlay icon */
		icon_namev = props.lookup("OverlayIconName");
		icon_pixmap = props.lookup("OverlayIconPixmap");
		overlay_icon = from_direct_props(icon_namev,icon_pixmap);

	}
	private Icon? one_icon_direct_cb(IconType type)
	{
		string appender;
		switch(type)
		{
			case IconType.ATTENTION:
				appender = "Attention";
				break;
			case IconType.OVERLAY:
				appender = "Overlay";
				break;
			default:
				appender = "";
				break;
		}
		Variant? icon_namev = null;
		Variant? icon_pixmap = null;
		try
		{
			icon_namev = props_iface.get_one(FreeDesktopProperties.KDE_NAME,appender+"IconName");
		} catch (Error e) {}
		try
		{
			icon_pixmap = props_iface.get_one(FreeDesktopProperties.KDE_NAME,appender+"IconPixmap");
		} catch (Error e) {}
		return from_direct_props(icon_namev,icon_pixmap);
	}
	private Icon? from_direct_props(Variant? icon_namev, Variant? icon_pixmapv)
	{
		var icon_name = (icon_namev != null) ? icon_namev.get_string() : null;
		return change_icon(icon_name,icon_pixmapv);
	}
	private void tooltip_direct_cb()
	{
		try{
			var tooltipv = props_iface.get_one(FreeDesktopProperties.KDE_NAME,"ToolTip");
			unbox_tooltip(tooltipv);
		} catch (Error e){stderr.printf("Cannot set tooltip:%s\n",e.message);}
	}
	private void unbox_tooltip(Variant? tooltipv)
	{
		if (tooltipv== null)
		{
			this.has_tooltip = true;
			return;
		}
		else
			this.has_tooltip = false;
		var icon_name = tooltipv.get_child_value(0).get_string();
		var icon_pixmap = tooltipv.get_child_value(1);
		tooltip_icon = change_icon(icon_name, icon_pixmap);
		/*FIXME: Markup parser */
		tooltip_markup = tooltipv.get_child_value(2).get_string()+ tooltipv.get_child_value(3).get_string();
//~ 		print("%s\n",tooltip_markup);
	}
	private Icon? change_icon(string? icon_name, Variant? pixmaps)
	{
		Icon? icon = null;
		if (icon_name != null && icon_name.length > 0)
		{
			icon = new ThemedIcon.with_default_fallbacks(icon_name+"-symbolic");
			(icon as ThemedIcon).prepend_name(icon_name+"-panel");
		}
		if (icon == null)
		{
			var first = true;
			if (pixmaps != null)
			{
				var iter = pixmaps.iterator();
				for (var pixmap = iter.next_value(); pixmap!=null; pixmap = iter.next_value())
				{
					Variant pixbuf = pixmap.get_child_value(2);
					if (first)
					{
						var base_icon = new BytesIcon(pixbuf.get_data_as_bytes());
						icon = new EmblemedIcon(base_icon,null);
						first = false;
					}
					else
					{
						var emblem_icon = new BytesIcon(pixbuf.get_data_as_bytes());
						var emblem = new Emblem(emblem_icon);
						(icon as EmblemedIcon).add_emblem(emblem);
					}
				}
			}
			else
				icon = null;
		}
		return icon;
	}
}
