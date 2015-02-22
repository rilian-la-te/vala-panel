using GLib;
using Gtk;

namespace StatusNotifier
{
	[DBus (use_string_marshalling = true)]
	public enum Category
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
	public enum Status
	{
		[DBus (value = "Passive")]
		PASSIVE,
		[DBus (value = "Active")]
		ACTIVE,
		[DBus (value = "NeedsAttention")]
		NEEDS_ATTENTION
	}
	private struct IconPixmap
	{
	    int width;
	    int height;
	    uint8[] bytes;
	}

	private struct ToolTip
	{
	    string icon_name;
	    IconPixmap[] pixmap;
	    string title;
	    string description;
	}
	[DBus (name = "org.kde.StatusNotifierItem")]
	private interface ItemIface : Object
	{
		/* Base properties */
		public abstract Category category {owned get;}
		public abstract string id {owned get;}
		public abstract Status status {owned get;}
		public abstract string title {owned get;}
		public abstract int window_id {owned get;}
		/* Menu properties */
		public abstract ObjectPath menu {owned get;}
		public abstract bool items_in_menu {owned get;}
		/* Icon properties */
		public abstract string icon_theme_path {owned get;}
		public abstract string icon_name {owned get;}
		public abstract string icon_accessible_desc {owned get;}
		public abstract IconPixmap[] icon_pixmap {owned get;}
		public abstract string overlay_icon_name {owned get;}
		public abstract IconPixmap[] overlay_icon_pixmap {owned get;}
		public abstract string attention_icon_name {owned get;}
		public abstract string attention_accessible_desc {owned get;}
		public abstract IconPixmap[] attention_icon_pixmap {owned get;}
		public abstract string attention_movie_name {owned get;}
		/* Tooltip */
		public abstract ToolTip tool_tip {owned get;}
		/* Methods */
		public abstract void context_menu(int x, int y) throws IOError;
		public abstract void activate(int x, int y) throws IOError;
		public abstract void secondary_activate(int x, int y) throws IOError;
		public abstract void scroll(int delta, string orientation) throws IOError;
		public abstract void x_ayatana_secondary_activate(uint32 timestamp) throws IOError;
		/* Signals */
		public abstract signal void new_title();
		public abstract signal void new_icon();
		public abstract signal void new_icon_theme_path(string icon_theme_path);
		public abstract signal void new_attention_icon();
		public abstract signal void new_overlay_icon();
		public abstract signal void new_tool_tip();
		public abstract signal void new_status(Status status);
		/* Ayatana */
		public abstract string x_ayatana_label {owned get;}
		public abstract string x_ayatana_label_guide {owned get;}
		public abstract uint x_ayatana_ordering_index {get;}
		public abstract signal void x_ayatana_new_label(string label, string guide);
	}

	/* FIXME: Icons always use DirectAccess now */
	public class ItemProxy: Object
	{
		private ItemIface iface;
		private bool use_attention_icon;
		public string bus_name
		{private get; internal construct;}
		public string object_path
		{private get; internal construct;}
			/* Base properties */
		public Category category {get {return iface.category;}}
		public string id {owned get {return iface.id;}}
		public Status status {get; private set;}
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
			try
			{
				iface.x_ayatana_secondary_activate(get_current_event_time());
				return;
			} catch (Error e){/* This only means that method not supported*/}
			iface.secondary_activate(x,y);
		}
		public void scroll(int delta, string orientation) throws Error
		{
			iface.scroll(delta, orientation);
		}
		public ItemProxy(string name, string path)
		{
			Object(bus_name: name, object_path: path);
		}
		construct
		{
			use_attention_icon = false;
			try
			{
				this.iface = Bus.get_proxy_sync (BusType.SESSION,bus_name,object_path);
				uint id;
				id = Bus.watch_name(BusType.SESSION,bus_name,BusNameWatcherFlags.NONE,
					null,
					() => {Bus.unwatch_name(id); this.proxy_destroyed();}
					);
			} catch (IOError e) {stderr.printf ("%s\n", e.message); this.proxy_destroyed();}
			init_properties();
			on_path_cb(iface.icon_theme_path);
			iface.new_status.connect((st)=>{this.status = st;});
			iface.new_icon.connect(()=>{main_icon_direct_cb(); accessible_desc_direct_cb();});
			iface.new_overlay_icon.connect(()=>{overlay_icon_direct_cb(); accessible_desc_direct_cb();});
			iface.new_attention_icon.connect(()=>{attention_icon_direct_cb(); accessible_desc_direct_cb();});
			iface.new_icon_theme_path.connect(on_path_cb);
			iface.x_ayatana_new_label.connect((lb,g)=>{this.label = lb; this.label_guide = g;});
			iface.new_tool_tip.connect(tooltip_direct_cb);
			iface.new_title.connect(title_direct_cb);
		}
		private void init_properties()
		{
			this.title = iface.title;
			this.status = iface.status;
			this.label = iface.x_ayatana_label;
			this.label_guide = iface.x_ayatana_label_guide;
			var new_overlay_icon = change_icon(null,iface.overlay_icon_name, iface.overlay_icon_pixmap);
			if (new_overlay_icon != null)
				overlay_icon = new Emblem(new_overlay_icon);
			this.attention_icon = change_icon(null,iface.attention_icon_name, iface.attention_icon_pixmap);
			this.main_icon = change_icon(null,iface.icon_name, iface.icon_pixmap);
			if (this.attention_icon != null)
			{
				icon = new EmblemedIcon(this.attention_icon,this.overlay_icon);
				this.use_attention_icon = true;
			}
			else
			{
				icon = new EmblemedIcon(this.main_icon,this.overlay_icon);
				this.use_attention_icon = false;
			}
			on_path_cb(iface.icon_theme_path);
			unbox_tooltip(iface.tool_tip);

		}
		private void title_direct_cb()
		{
			try
			{
				ItemIface item = Bus.get_proxy_sync(BusType.SESSION,bus_name, object_path);
				this.title = item.title;
			} catch (Error e) {stderr.printf("Cannot set title: %s\n",e.message);}
		}
		private void on_path_cb(string? new_path)
		{
			if (new_path != null)
			{
				IconTheme.get_default().prepend_search_path(new_path);
				this.icon_theme_path = new_path;
			}
		}
		private void attention_icon_direct_cb()
		{
			try
			{
				ItemIface item = Bus.get_proxy_sync(BusType.SESSION, bus_name, object_path);
				attention_icon = change_icon(attention_icon,item.attention_icon_name,item.attention_icon_pixmap);
				if (attention_icon != null)
				{
					use_attention_icon = true;
					icon = new EmblemedIcon(attention_icon,overlay_icon);
				}
				else
				{
					use_attention_icon = false;
					main_icon = change_icon(main_icon,item.icon_name,item.icon_pixmap);
					icon = new EmblemedIcon(main_icon, overlay_icon);
				}
			} catch (Error e) {stderr.printf("%s\n",e.message);}
		}
		private void overlay_icon_direct_cb()
		{
			try
			{
				ItemIface item = Bus.get_proxy_sync(BusType.SESSION, bus_name, object_path);
				var res_icon = change_icon(overlay_icon,item.overlay_icon_name,item.overlay_icon_pixmap);
				if (res_icon != null)
					overlay_icon = new Emblem(res_icon);
				else
					overlay_icon = null;
				if (icon != null)
				{
					icon.clear_emblems();
					if (overlay_icon != null)
						icon.add_emblem(overlay_icon);
				}
			} catch (Error e) {stderr.printf("%s\n",e.message);}
		}
		private void main_icon_direct_cb()
		{
			try
			{
				ItemIface item = Bus.get_proxy_sync(BusType.SESSION, bus_name, object_path);
				main_icon = change_icon(main_icon,item.icon_name,item.icon_pixmap);
				if (!use_attention_icon)
				{
					icon = new EmblemedIcon(main_icon,overlay_icon);
					use_attention_icon = false;
				}
			} catch (Error e) {stderr.printf("%s\n",e.message);}
		}
		private void tooltip_direct_cb()
		{
			try{
				ItemIface item = Bus.get_proxy_sync(BusType.SESSION, bus_name, object_path);
				unbox_tooltip(item.tool_tip);
			} catch (Error e){stderr.printf("Cannot set tooltip:%s\n",e.message);}
		}
		private void unbox_tooltip(ToolTip tooltip)
		{
			this.has_tooltip = true;
			var raw_text = tooltip.title + tooltip.description;
			var is_pango_markup = true;
			if (raw_text != null)
			{
				try
				{
					Pango.parse_markup(raw_text,-1,'\0',null,null,null);
				} catch (Error e){is_pango_markup = false;}
			}
			if (!is_pango_markup)
			{
				var markup_parser = new QRichTextParser(raw_text);
				markup_parser.translate_markup();
				tooltip_markup = (markup_parser.pango_markup.length > 0) ? markup_parser.pango_markup: tooltip_markup;
				var res_icon = change_icon(tooltip_icon, tooltip.icon_name, tooltip.pixmap);
				tooltip_icon = (markup_parser.icon != null)	? markup_parser.icon: res_icon;
			}
			else
			{
				tooltip_markup = raw_text;
				tooltip_icon = change_icon(tooltip_icon, tooltip.icon_name, tooltip.pixmap);
			}
			Tooltip.trigger_tooltip_query(Gdk.Display.get_default());
		}
		private Icon? change_icon(Icon? prev_icon, string? icon_name, IconPixmap[] pixmaps)
		{
			if (icon_name != null && icon_name.length > 0)
			{
				if (icon_name[0] == '/')
					return new FileIcon(File.new_for_path(icon_name));
				else
				{
					var new_icon = new ThemedIcon.with_default_fallbacks(icon_name+"-symbolic");
					themed_icon_determine_rescan(new_icon);
					return new_icon;
				}
			}
			/* FIXME: Choose pixmap size */
			else if (pixmaps.length > 0)
				return new BytesIcon(new Bytes(pixmaps[0].bytes));
			return null;
		}
		/*FIXME: Workaround for gtk+ < 3.15.8 */
		private void themed_icon_determine_rescan(ThemedIcon icon)
		{
			unowned string[] icon_names = icon.get_names();
			for (int i = icon_names.length - 1; i >= 0; i--)
				if(IconTheme.get_default().has_icon(icon_names[i]))
					return;
			IconTheme.get_default().rescan_if_needed();
		}
		private void accessible_desc_direct_cb()
		{
			try
			{
				ItemIface item = Bus.get_proxy_sync(BusType.SESSION,bus_name, object_path);
				if (item.attention_accessible_desc != null && use_attention_icon)
					accessible_desc = item.attention_accessible_desc;
				else if (item.icon_accessible_desc != null)
					accessible_desc = item.icon_accessible_desc;
				else accessible_desc = null;
			} catch (Error e) {stderr.printf("Method not supported\n");}
		}
	}
}
