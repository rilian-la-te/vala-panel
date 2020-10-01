using GLib;
using Gtk;

[CCode (cheader_filename = "config.h", lower_case_cprefix="", cprefix="")]
namespace PanelConfig {
	public const string DATADIR;
	public const string GETTEXT_PACKAGE;
	public const string LOCALE_DIR;
	public const string PLUGINS_DATA;
	public const string PLUGINS_DIRECTORY;
	public const string RELEASE_NAME;
	public const string VERSION;
}
[CCode (cheader_filename = "generic-config-dialog.h", cprefix = "",lower_case_cprefix="")]
namespace ValaPanel.Configurator
{
    public static Dialog generic_config_dlg(string title, Gtk.Window parent,
                                    GLib.Settings settings, ...);
    public static Widget generic_config_widget(GLib.Settings settings, ...);
}
[CCode (cheader_filename = "css.h", cprefix = "",lower_case_cprefix="css_")]
namespace PanelCSS
{
    public void apply_with_class(Gtk.Widget w, string css, string klass, bool add);
    public void toggle_class(Widget w, string klass, bool apply);
	[CCode (cname = "css_add_css_with_provider")]
    public CssProvider? add_css_to_widget(Widget w, string css);
    public Gtk.CssProvider? apply_from_file_to_app_with_provider(string file);
    public string apply_from_file_to_app(string file);
    public void apply_from_resource(Gtk.Widget w, string file, string klass);
    public string generate_background(string? name, Gdk.RGBA color);
    public string generate_font_size(int size);
    public string generate_font_color(Gdk.RGBA color);
    public string generate_font_label(double size ,bool bold);
    public string generate_flat_button(Gtk.Widget w, Gtk.PositionType e);
}
[CCode (cprefix="")]
namespace MenuMaker
{
    [CCode (cheader_filename="menu-maker.h",cname="ATTRIBUTE_DND_SOURCE")]
    public const string ATTRIBUTE_DND_SOURCE;
    [CCode (cheader_filename="menu-maker.h",cname="ATTRIBUTE_TOOLTIP")]
    public const string ATTRIBUTE_TOOLTIP;
    [CCode (cheader_filename="launcher-gtk.h",cname="activate_menu_launch_id")]
    public static void activate_menu_launch_id(SimpleAction? action, Variant? param, void* user_data);
    [CCode (cheader_filename="launcher-gtk.h",cname="activate_menu_launch_uri")]
    public static void activate_menu_launch_uri(SimpleAction? action, Variant? param, void* user_data);
    [CCode (cheader_filename="launcher-gtk.h",cname="activate_menu_launch_command")]
    public static void activate_menu_launch_command(SimpleAction? action, Variant? param, void* user_data);
    [CCode (cheader_filename="launcher-gtk.h",cname="vala_panel_launch")]
    public static bool launch(DesktopAppInfo info, GLib.List<string>? uris, Gtk.Widget parent);
    [CCode (cheader_filename="misc.h",cname="vala_panel_launch_with_context")]
    public static bool launch_with_context(DesktopAppInfo info, AppLaunchContext cxt, GLib.List<string>? uris);
    [CCode (cheader_filename="misc.h",cname="vala_panel_get_default_for_uri")]
    public static AppInfo get_default_for_uri(string uri);
    [CCode (cheader_filename="menu-maker.h",cname="append_all_sections")]
    public static void append_all_sections(GLib.Menu menu1, GLib.MenuModel menu2);
    [CCode (cheader_filename="menu-maker.h",cname="copy_model_items")]
    public static void copy_model_items(GLib.Menu menu1, GLib.MenuModel menu2);
    [CCode (cheader_filename="menu-maker.h",cname="apply_menu_properties")]
    public static void apply_menu_properties(List<unowned Widget> w, MenuModel menu);
}
[CCode (cheader_filename = "applet-widget.h")]
namespace ValaPanel.AppletAction
{
	public const string REMOTE;
	public const string CONFIGURE;
}
namespace ValaPanel {
    [CCode(cname="GenericConfigType", has_type_id = false, cprefix="CONF_", cheader_filename = "generic-config-dialog.h")]
    public enum GenericConfigType
    {
        STR,
        INT,
        BOOL,
        FILE,
        FILE_ENTRY,
        DIRECTORY,
        DIRECTORY_ENTRY,
        TRIM,
        EXTERNAL
    }
	[CCode (cheader_filename = "applet-widget.h")]
	public abstract class Applet : Gtk.Bin {
        [CCode (cheader_filename = "applet-info.h")]
		public const string EXTENSION_POINT;
		[CCode (cheader_filename = "applet-widget-api.h")]
		protected Applet (ValaPanel.Toplevel top, GLib.Settings? s, string uuid);
		public void init_background ();
		public bool is_configurable();
		public virtual void update_context_menu (ref GLib.Menu parent_menu);
		public virtual Widget get_settings_ui();
		public virtual bool remote_command(string name);
		public Gtk.Widget background_widget { get; set; }
		public GLib.Settings? settings { get; construct; }
		public SimpleActionGroup action_group { get; }
		[CCode (cheader_filename = "applet-widget-api.h")]
		public ValaPanel.Toplevel toplevel { get; construct; }
		public string uuid { get; construct; }
	}
	[CCode (cheader_filename = "toplevel.h")]
	public class Toplevel : Gtk.ApplicationWindow {
		public Toplevel (Gtk.Application app, ValaPanel.Platform platform, string name);
		public void configure (string page);
        public void configure_applet (string uuid);
		[NoAccessorMethod]
		public bool autohide { get; internal set; }
		[NoAccessorMethod]
		public string background_color { owned get; internal set; }
		[NoAccessorMethod]
		public string background_file { owned get; internal set; }
		[NoAccessorMethod]
		public bool dock { get; internal set; }
		[NoAccessorMethod]
		public string font { owned get; internal set; }
		[NoAccessorMethod]
		public uint font_size { get; internal set; }
		[NoAccessorMethod]
		public bool font_size_only { get; internal set; }
		[NoAccessorMethod]
		public string foreground_color { owned get; internal set; }
		[NoAccessorMethod]
		public int height { get; internal set; }
		[NoAccessorMethod]
		public uint icon_size { get; internal set; }
		[NoAccessorMethod]
		public bool is_dynamic { get; internal set; }
		[NoAccessorMethod]
		public int monitor { get; internal set construct; }
		[NoAccessorMethod]
		public Gtk.Orientation orientation { get; }
		[NoAccessorMethod]
		public Gravity panel_gravity { get;}
		[NoAccessorMethod]
		public uint round_corners_size { get; internal set; }
		[NoAccessorMethod]
		public bool strut { get; internal set; }
		[NoAccessorMethod]
		public bool use_background_color { get; internal set; }
		[NoAccessorMethod]
		public bool use_background_file { get; internal set; }
		[NoAccessorMethod]
		public bool use_font { get; internal set; }
		[NoAccessorMethod]
		public bool use_foreground_color { get; internal set; }
		[NoAccessorMethod]
		public string uuid { owned get; internal construct; }
		[NoAccessorMethod]
		public int width { get; internal set; }
	}
	[CCode (cheader_filename = "misc-gtk.h")]
	public static void setup_icon (Gtk.Image img, GLib.Icon icon, ValaPanel.Toplevel? top = null, int size = -1);
	[CCode (cheader_filename = "misc-gtk.h")]
	public static void setup_icon_button (Gtk.Button btn, GLib.Icon? icon = null, string? label = null, ValaPanel.Toplevel? top = null);
    [CCode (cheader_filename="misc-gtk.h")]
    public static void apply_window_icon(Window w);
    [CCode (cheader_filename="misc-gtk.h")]
	public static int monitor_num_from_mon(Gdk.Display display, Gdk.Monitor monitor);
    [CCode (cheader_filename="misc.h")]
    public static void reset_schema(GLib.Settings settings);
    [CCode (cheader_filename="misc.h")]
    public static void reset_schema_with_children(GLib.Settings settings);
    [CCode (cname = "vala_panel_add_gsettings_as_action",cheader_filename="misc.h")]
    public static void settings_as_action(ActionMap map, GLib.Settings settings, string prop);
    [CCode(cname = "vala_panel_bind_gsettings",cheader_filename="definitions.h")]
    public static void settings_bind(Object map, GLib.Settings settings, string prop);
    [CCode (cheader_filename="misc-gtk.h")]
    public static void setup_button(Button b, Image? img = null, string? label = null);
    [CCode (cheader_filename="misc-gtk.h")]
    public static void setup_label(Label label, string text, bool bold, double factor);
    [CCode (cheader_filename="misc-gtk.h")]
    public static void scale_button_set_range(ScaleButton b, int lower, int upper);
    [CCode (cheader_filename="misc-gtk.h")]
    public static void scale_button_set_value_labeled(ScaleButton b, int val);
    [CCode (cheader_filename="definitions.h")]
    public static Gtk.Orientation orient_from_gravity(Gravity gravity);
 	[CCode (cheader_filename="definitions.h")]
    public static Gtk.PositionType edge_from_gravity(Gravity gravity);
    [CCode (cheader_filename="definitions.h")]
    public static Gtk.Orientation invert_orient(Gtk.Orientation orient);
    [CCode(cname="PanelGravity", cprefix="", cheader_filename = "panel-platform.h")]
	public enum Gravity
	{
		NORTH_LEFT,
		NORTH_CENTER,
		NORTH_RIGHT,
		SOUTH_LEFT,
		SOUTH_CENTER,
		SOUTH_RIGHT,
		WEST_UP,
		WEST_CENTER,
		WEST_DOWN,
		EAST_UP,
		EAST_CENTER,
		EAST_DOWN
	}
    [CCode (cheader_filename="panel-platform.h")]
    public class Platform : Object
    {
        [CCode (has_construct_function="false")]
        protected Platform();
        public bool start_panels_from_profile(Gtk.Application app,string *profile);
        internal void init_settings(GLib.SettingsBackend backend);
        internal void init_settings_full(string schema,string path, GLib.SettingsBackend backend);
        public long can_strut(Gtk.Window top);
        public void update_strut(Gtk.Window top);
        public void move_to_coords(Gtk.Window top, int x, int y);
        public void move_to_side(Gtk.Window top, Gravity side, int monitor);
		public bool edge_available(Gtk.Window top, Gravity gravity, int monitor);
    }
}
[CCode (cheader_filename = "constants.h", cprefix = "VP_KEY_",lower_case_cprefix="VP_KEY_")]
namespace ValaPanel.Key
{
    public const string GRAVITY;
    public const string HEIGHT;
    public const string WIDTH;
    public const string DYNAMIC;
    public const string AUTOHIDE;
    public const string SHOW_HIDDEN;
    public const string STRUT;
    public const string DOCK;
    public const string MONITOR;
    public const string MARGIN;
    public const string ICON_SIZE;
    public const string BACKGROUND_COLOR;
    public const string FOREGROUND_COLOR;
    public const string BACKGROUND_FILE;
    public const string FONT;
    public const string CORNERS_SIZE;
    public const string USE_BACKGROUND_COLOR;
    public const string USE_FOREGROUND_COLOR;
    public const string USE_FONT;
    public const string FONT_SIZE_ONLY;
    public const string USE_BACKGROUND_FILE;
}
