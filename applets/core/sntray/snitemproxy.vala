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
}
