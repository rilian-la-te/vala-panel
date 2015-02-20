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
	public abstract string icon_accessible_desc
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
	public abstract string attention_accessible_desc
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
	private bool use_attention_icon;
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
	public EmblemedIcon? icon
	{get; private set;}
	private Icon? main_icon;
	private Emblem? overlay_icon;
	private Icon? attention_icon;
	public string attention_movie_name {owned get {return iface.attention_movie_name;}}
	/* Tooltip */
	public bool has_tooltip {get; private set;}
	public Icon? tooltip_icon {get; private set;}
	public string? tooltip_markup {get; private set;}
	public string? accessible_desc {get; private set;}
	/* Ayatana */
	public string? label {get; private set;}
	public string? label_guide {get; private set;}
	public uint index {get {return iface.x_ayatana_ordering_index;}}
	/* Signals and functions */
	public signal void proxy_destroyed();
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
		use_attention_icon = false;
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
		iface.new_icon.connect(()=>{one_icon_direct_cb(""); accessible_desc_direct_cb();});
		iface.new_overlay_icon.connect(()=>{one_icon_direct_cb("Overlay"); accessible_desc_direct_cb();});
		iface.new_attention_icon.connect(()=>{one_icon_direct_cb("Attention"); accessible_desc_direct_cb();});
		iface.new_icon_theme_path.connect(on_path_cb);
		iface.x_ayatana_new_label.connect((lb,g)=>{this.label = lb; this.label_guide = g;});
		iface.new_tool_tip.connect(tooltip_direct_cb);
		iface.new_title.connect(title_direct_cb);
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
		var	label = props.lookup("IconThemePath");
		if (label != null)
			on_path_cb(label.get_string());
		label = props.lookup("ToolTip");
		unbox_tooltip(label);
		label = props.lookup("XAyatanaLabel");
		this.label = (label != null) ? label.get_string() : this.label;
		label = props.lookup("XAyatanaLabelGuide");
		this.label_guide = (label != null) ? label.get_string() : this.label_guide;
		label = props.lookup("Title");
		this.title = (label != null) ? label.get_string() : this.title;
		label = props.lookup("Status");
		this.status = (label != null) ? status_from_string(label.get_string()) : this.status;
		all_icons_direct_cb(props);
		label = props.lookup("IconAccessibleDesc");
		var alabel = props.lookup("AttentionAccessibleDesc");
		accessible_desc_cb(label,alabel);
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
		/* Overlay icon */
		var icon_namev = props.lookup("OverlayIconName");
		var icon_pixmap = props.lookup("OverlayIconPixmap");
		change_icon.begin(overlay_icon,icon_namev,icon_pixmap,(obj,res)=>{
			var res_icon = change_icon.end(res);
			if (res_icon != null)
				overlay_icon = new Emblem(res_icon);
			else
				overlay_icon = null;
			if (icon != null)
				icon.clear_emblems();
			if (icon != null && overlay_icon != null)
				icon.add_emblem(overlay_icon);
		});
		/* Attention icon */
		icon_namev = props.lookup("AttentionIconName");
		icon_pixmap = props.lookup("AttentionIconPixmap");
		change_icon.begin(attention_icon,icon_namev,icon_pixmap,(obj,res)=>{
			attention_icon = change_icon.end(res);
			if (attention_icon != null)
				icon = new EmblemedIcon(attention_icon,overlay_icon);
		});
		/* Main icon */
		icon_namev = props.lookup("IconName");
		icon_pixmap = props.lookup("IconPixmap");
		change_icon.begin(main_icon,icon_namev,icon_pixmap,(obj,res)=>{
			main_icon = change_icon.end(res);
			icon = new EmblemedIcon(main_icon,overlay_icon);
		});
	}
	private void on_path_cb(string? new_path)
	{
		if (new_path != null)
		{
			IconTheme.get_default().append_search_path(new_path);
			this.icon_theme_path = new_path;
		}
	}
	private void one_icon_direct_cb(string appender)
	{
		Variant? icon_namev = null;
		Variant? icon_pixmap = null;
		try
		{
			icon_namev = props_iface.get_one(FreeDesktopProperties.KDE_NAME,appender+"IconName");
			if (icon_namev == null)
				icon_pixmap = props_iface.get_one(FreeDesktopProperties.KDE_NAME,appender+"IconPixmap");
		} catch (Error e) {}
		if (appender == "Attention")
			change_icon.begin(attention_icon,icon_namev,icon_pixmap,(obj,res)=>{
				attention_icon = change_icon.end(res);
				if (attention_icon != null)
				{
					use_attention_icon = true;
					icon = new EmblemedIcon(attention_icon,overlay_icon);
				}
				else use_attention_icon = false;
			});
		else if (appender == "Overlay")
			change_icon.begin(overlay_icon,icon_namev,icon_pixmap,(obj,res)=>{
				var res_icon = change_icon.end(res);
				if (res_icon != null)
					overlay_icon = new Emblem(res_icon);
				else
					overlay_icon = null;
				if (icon != null)
					icon.clear_emblems();
				if (icon != null && overlay_icon != null)
					icon.add_emblem(overlay_icon);
			});
		else
			change_icon.begin(main_icon,icon_namev,icon_pixmap,(obj,res)=>{
				main_icon = change_icon.end(res);
				if (!use_attention_icon)
				{
					icon = new EmblemedIcon(main_icon,overlay_icon);
					use_attention_icon = false;
				}
			});
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
		this.has_tooltip = true;
		if (tooltipv== null)
			return;
		var icon_namev = tooltipv.get_child_value(0);
		var icon_pixmap = tooltipv.get_child_value(1);
		var raw_text = tooltipv.get_child_value(2).get_string() + tooltipv.get_child_value(3).get_string();
		if (raw_text != null && raw_text.index_of("</") >= 0)
		{
			var markup_parser = new QRichTextParser(raw_text);
			markup_parser.translate_markup();
			tooltip_markup = (markup_parser.pango_markup.length > 0) ? markup_parser.pango_markup: tooltip_markup;
			change_icon.begin(tooltip_icon, icon_namev, icon_pixmap,(obj,res) => {
				tooltip_icon = (markup_parser.icon != null)	? markup_parser.icon: change_icon.end(res);
			});
		}
		else
		{
			tooltip_markup = raw_text;
			change_icon.begin(tooltip_icon, icon_namev, icon_pixmap,(obj,res) => {
				tooltip_icon = change_icon.end(res);
			});
		}
		Tooltip.trigger_tooltip_query(Gdk.Display.get_default());
	}
	private async Icon? change_icon(Icon? prev_icon, Variant? icon_namev, Variant? pixmaps)
	{
		var icon_name = (icon_namev != null) ? icon_namev.get_string() : null;
		if (icon_name != null && icon_name.length > 0)
		{
			if (icon_name[0] == '/')
				return new FileIcon(File.new_for_path(icon_name));
			else
			{
				var new_icon = new ThemedIcon.with_default_fallbacks(icon_name+"-symbolic");
				themed_icon_generate_fallback(ref new_icon,prev_icon as ThemedIcon);
				return new_icon;
			}
		}
		else if (pixmaps != null)
		{
			var iter = pixmaps.iterator();
			for (var pixmap = iter.next_value(); pixmap!=null; pixmap = iter.next_value())
			{
				/*FIXME: Need a find suitable icon for size. Now just pick smaller */
				Variant pixbuf = pixmap.get_child_value(2);
				return new BytesIcon(pixbuf.get_data_as_bytes());
			}
		}
		return null;
	}
	private void themed_icon_generate_fallback(ref ThemedIcon icon, ThemedIcon? prev_icon)
	{
		var icon_names = icon.get_names();
		for (int i = icon_names.length - 1; i >= 0; i--)
			if(IconTheme.get_default().has_icon(icon_names[i]))
				return;
		IconTheme.get_default().rescan_if_needed();
		if (prev_icon == null)
			return;
		var prev_names = prev_icon.get_names();
		var len = prev_names.length;
		if((len <= 2) || IconTheme.get_default().has_icon(prev_names[1]))
			icon.append_name(prev_names[1]);
		else if (len > 2)
			icon.append_name(prev_names[2]);
	}
	private void accessible_desc_direct_cb()
	{
		try
		{
			var desc = props_iface.get_one(FreeDesktopProperties.KDE_NAME,"IconAccessibleDesc");
			var adesc = props_iface.get_one(FreeDesktopProperties.KDE_NAME,"AttentionAccessibleDesc");
			accessible_desc_cb(desc,adesc);
		} catch (Error e) {}
	}
	private void accessible_desc_cb(Variant? desc, Variant? adesc)
	{
		if (adesc != null && use_attention_icon)
			accessible_desc = adesc.get_string().length > 0 ? adesc.get_string() : null;
		else if (desc != null)
			accessible_desc = desc.get_string().length > 0 ? desc.get_string() : null;
		else
			accessible_desc = null;
	}
}
