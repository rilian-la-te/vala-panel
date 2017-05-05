using GLib;
using Gtk;

[CCode (cheader_filename = "config.h", upper_case_cprefix="")]
namespace PanelConfig {
	public const string DATADIR;
	public const string GETTEXT_PACKAGE;
	public const string INSTALL_PREFIX;
	public const string LOCALE_DIR;
	public const string PKGDATADIR;
	public const string PLUGINS_DATA;
	public const string PLUGINS_DIRECTORY;
	public const string RELEASE_NAME;
	public const string VERSION;
	public const string VERSION_INFO;
}
[CCode (cheader_filename = "generic-config-dialog.h", cprefix = "",lower_case_cprefix="")]
namespace ValaPanel.Configurator
{
    public static Dialog generic_config_dlg(string title, Gtk.Window parent,
                                    GLib.Settings settings, ...);
    public static Widget generic_config_widget(GLib.Settings settings, ...);
}
[CCode (cheader_filename = "applet-widget.h")]
namespace ValaPanel.AppletAction
{
	public const string MENU;
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
	[CCode (cheader_filename = "vala-panel-compat.h")]
	public interface AppletPlugin : Peas.ExtensionBase {
		public abstract ValaPanel.Applet get_applet_widget (ValaPanel.Toplevel toplevel, GLib.Settings? settings, string number);
	}
	[CCode (cheader_filename = "applet-widget.h")]
	public abstract class Applet : Gtk.Bin {
		[CCode (cheader_filename = "applet-widget-api.h")]
		public Applet (ValaPanel.Toplevel top, GLib.Settings? s, string uuid);
		public void init_background ();
		public void show_config_dialog ();
		public bool is_configurable();
		public virtual void update_context_menu (ref GLib.Menu parent_menu);
		public virtual Widget get_settings_ui();
		public Gtk.Widget background_widget { get; set; }
		public GLib.Settings? settings { get; construct; }
		public SimpleActionGroup action_group { get; }
		[CCode (cheader_filename = "applet-widget-api.h")]
		public ValaPanel.Toplevel toplevel { get; construct; }
		public string uuid { get; construct; }
	}
	[CCode (cheader_filename = "vala-panel-compat.h")]
	public class Toplevel : Gtk.ApplicationWindow {
		public Toplevel (Gtk.Application app, ValaPanel.Platform platform, string name);
		public void configure (string page);
		public bool autohide { get; internal set; }
		public string background_color { owned get; internal set; }
		public string background_file { get; internal set; }
		public bool dock { get; internal set; }
		public Gtk.PositionType edge { get; internal set construct; }
		public string font { get; internal set; }
		public uint font_size { get; internal set; }
		public bool font_size_only { get; internal set; }
		public string foreground_color { owned get; internal set; }
		public int height { get; internal set; }
		public uint icon_size { get; internal set; }
		public bool is_dynamic { get; internal set; }
		public int monitor { get; internal set construct; }
		public Gtk.Orientation orientation { get; }
		public int panel_margin { get; internal set; }
		public uint round_corners_size { get; internal set; }
		public bool strut { get; internal set; }
		public bool use_background_color { get; internal set; }
		public bool use_background_file { get; internal set; }
		public bool use_font { get; internal set; }
		public bool use_foreground_color { get; internal set; }
		public string uuid { get; internal construct; }
		public int width { get; internal set; }
	}
	[CCode (cheader_filename = "vala-panel-compat.h")]
	public static void setup_icon (Gtk.Image img, GLib.Icon icon, ValaPanel.Toplevel? top = null, int size = -1);
	[CCode (cheader_filename = "vala-panel-compat.h")]
	public static void setup_icon_button (Gtk.Button btn, GLib.Icon? icon = null, string? label = null, ValaPanel.Toplevel? top = null);
    [CCode (cheader_filename="misc.h")]
    public static void apply_window_icon(Window w);
    [CCode (cheader_filename="misc.h")]
	public static int monitor_num_from_mon(Gdk.Display display, Gdk.Monitor monitor);
    [CCode (cheader_filename="misc.h")]
    public static void reset_schema(GLib.Settings settings);
    [CCode (cheader_filename="misc.h")]
    public static void reset_schema_with_children(GLib.Settings settings);
    [CCode (cname = "vala_panel_add_gsettings_as_action",cheader_filename="misc.h")]
    public static void settings_as_action(ActionMap map, GLib.Settings settings, string prop);
    [CCode(cname = "vala_panel_bind_gsettings",cheader_filename="definitions.h")]
    public static void settings_bind(Object map, GLib.Settings settings, string prop);
    [CCode (cheader_filename="misc.h")]
    public static void setup_button(Button b, Image? img = null, string? label = null);
    [CCode (cheader_filename="misc.h")]
    public static void setup_label(Label label, string text, bool bold, double factor);
    [CCode (cheader_filename="misc.h")]
    public static void scale_button_set_range(ScaleButton b, int lower, int upper);
    [CCode (cheader_filename="misc.h")]
    public static void scale_button_set_value_labeled(ScaleButton b, int val);
    [CCode (cheader_filename="definitions.h")]
    public static Gtk.Orientation orient_from_edge(Gtk.PositionType edge);
    [CCode (cheader_filename="definitions.h")]
    public static Gtk.Orientation invert_orient(Gtk.Orientation orient);
    [Compact]
    [CCode (cheader_filename="settings-manager.h",copy_function="g_boxed_copy",free_function="g_boxed_free",type_id="vala_panel_core_settings_get_type()")]
    public class CoreSettings
    {
        public static string get_uuid();
        public CoreSettings(string schema, string path, string root, GLib.SettingsBackend backend);
        public unowned UnitSettings add_unit_settings(string name, bool is_toplevel);
        public unowned UnitSettings add_unit_settings_full(string name, string uuid, bool is_toplevel);
        public void remove_unit_settings(string name);
        public void remove_unit_settings_full(string name, bool destroy);
        public unowned UnitSettings get_by_uuid(string uuid);
    }
    [Compact]
    [CCode (cheader_filename="settings-manager.h",copy_function="g_boxed_copy",free_function="g_boxed_free",type_id="vala_panel_unit_settings_get_type()")]
    public class UnitSettings
    {
        internal GLib.Settings custom_settings;
        internal string uuid;
		internal bool is_toplevel();
    }
    [CCode (cheader_filename="panel-platform.h")]
    public class Platform : Object
    {
        [CCode (has_construct_function="false")]
        protected Platform();
        public virtual bool start_panels_from_profile(Gtk.Application app,string *profile);
        public void init_settings(GLib.SettingsBackend backend);
        public void init_settings_full(string schema,string path, GLib.SettingsBackend backend);
        public unowned CoreSettings get_settings();
        public virtual long can_strut(Gtk.Window top);
        public virtual void update_strut(Gtk.Window top);
        public virtual void move_to_coords(Gtk.Window top, int x, int y);
        public virtual void move_to_side(Gtk.Window top, Gtk.PositionType side, int monitor);
    }
}
[CCode (cheader_filename = "constants.h", cprefix = "VALA_PANEL_KEY_",lower_case_cprefix="VALA_PANEL_KEY_")]
namespace ValaPanel.Key
{
    public const string EDGE;
    public const string ALIGNMENT;
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
