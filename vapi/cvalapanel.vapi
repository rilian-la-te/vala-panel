/*
 * vala-panel
 * Copyright (C) 2016 Konstantin Pugin <ria.freelander@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

using Gtk;
using GLib;

[CCode (cheader_filename = "css.h", cprefix = "",lower_case_cprefix="css_")]
namespace PanelCSS
{
    public void apply_with_class(Gtk.Widget w, string css, string klass, bool add);
    public void toggle_class(Widget w, string klass, bool apply);
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
[CCode (cheader_filename = "applet-widget.h")]
namespace ValaPanel.AppletAction
{
	public const string MENU;
	public const string CONFIGURE;
}
namespace ValaPanel
{
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
    [CCode(cname="PanelAppletPackType", cprefix="PACK_", cheader_filename = "panel-layout.h")]
    public enum AppletPackType
    {
        START,
        CENTER,
        END
    }
    [CCode (cheader_filename="panel-layout.h")]
    public void applet_set_position_metadata(Gtk.Widget applet, int metadata);
    [CCode (cheader_filename="panel-layout.h")]
    public int applet_get_position_metadata(Gtk.Widget applet);
    [CCode (cheader_filename="panel-layout.h")]
    public class AppletLayout : Gtk.Box
    {
		public AppletLayout(Gtk.Orientation orient, int spacing);
    }
    [CCode(cheader_filename="settings-manager.h")]
    public const string PLUGIN_SCHEMA;
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
    [CCode(cname="PanelAutohideState", cprefix="AH_", cheader_filename = "panel-platform.h")]
    internal enum AutohideState
    {
        VISIBLE,
        HIDDEN,
		GRAB,
        WAITING
    }
    [CCode(cname="PanelAlignmentType", cprefix="ALIGN_", cheader_filename = "panel-platform.h,vala-panel-enums.h")]
    public enum AlignmentType
    {
        START,
        CENTER,
        END
    }
    [CCode(cname="PanelIconSizeHints", cprefix="", cheader_filename = "panel-platform.h,vala-panel-enums.h")]
    internal enum IconSizeHints
    {
        XXS,
        XS,
        S,
        M,
        L,
        XL,
        XXL,
        XXXL;
    }
    [CCode(cname = "_user_config_file_name",cheader_filename="definitions.h")]
    internal string user_config_file_name(string name1, string profile, string? name2);
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
    public static Gtk.Orientation orient_from_gravity(Gravity gravity);
 	[CCode (cheader_filename="definitions.h")]
    public static Gtk.PositionType edge_from_gravity(Gravity gravity);
    [CCode (cheader_filename="definitions.h")]
    public static Gtk.Orientation invert_orient(Gtk.Orientation orient);
    [Compact]
    [CCode (cheader_filename="settings-manager.h",copy_function="g_boxed_copy",free_function="g_boxed_free",type_id="vala_panel_core_settings_get_type()")]
    internal class CoreSettings
    {
        internal HashTable<string,UnitSettings> all_units;
        internal GLib.SettingsBackend backend;
        internal GLib.Settings core_settings;
        internal string root_name;
        internal string root_schema;
        internal string root_path;
        internal static string get_uuid();
        internal CoreSettings(string schema, string path, string root, GLib.SettingsBackend backend);
        internal unowned UnitSettings add_unit_settings(string name, bool is_toplevel);
        internal unowned UnitSettings add_unit_settings_full(string name, string uuid, bool is_toplevel);
        internal void remove_unit_settings(string name);
        internal void remove_unit_settings_full(string name, bool destroy);
        internal unowned UnitSettings get_by_uuid(string uuid);
        internal bool init_unit_list();
    }
    [Compact]
    [CCode (cheader_filename="settings-manager.h",copy_function="g_boxed_copy",free_function="g_boxed_free",type_id="vala_panel_unit_settings_get_type()")]
    internal class UnitSettings
    {
        internal GLib.Settings default_settings;
        internal GLib.Settings custom_settings;
        internal string uuid;
        internal string path_elem;
		internal bool is_toplevel();
        internal UnitSettings(CoreSettings settings, string? name, string uuid);
    }
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
        internal unowned CoreSettings get_settings();
        public long can_strut(Gtk.Window top);
        public void update_strut(Gtk.Window top);
        public void move_to_coords(Gtk.Window top, int x, int y);
        public void move_to_side(Gtk.Window top, Gravity side, int monitor);
    }
}
[CCode (cprefix="")]
namespace MenuMaker
{
    [CCode (cheader_filename="menu-maker.h",cname="ATTRIBUTE_DND_SOURCE")]
    public const string ATTRIBUTE_DND_SOURCE;
    [CCode (cheader_filename="menu-maker.h",cname="ATTRIBUTE_TOOLTIP")]
    public const string ATTRIBUTE_TOOLTIP;
    [CCode (cheader_filename="launcher.h",cname="activate_menu_launch_id")]
    public static void activate_menu_launch_id(SimpleAction? action, Variant? param, void* user_data);
    [CCode (cheader_filename="launcher.h",cname="activate_menu_launch_uri")]
    public static void activate_menu_launch_uri(SimpleAction? action, Variant? param, void* user_data);
    [CCode (cheader_filename="launcher.h",cname="activate_menu_launch_command")]
    public static void activate_menu_launch_command(SimpleAction? action, Variant? param, void* user_data);
    [CCode (cheader_filename="launcher.h",cname="vala_panel_launch")]
    public static bool launch(DesktopAppInfo info, GLib.List<string>? uris, Gtk.Widget parent);
    [CCode (cheader_filename="launcher.h",cname="vala_panel_launch_with_context")]
    public static bool launch_with_context(DesktopAppInfo info, AppLaunchContext cxt, GLib.List<string>? uris);
    [CCode (cheader_filename="launcher.h",cname="vala_panel_get_default_for_uri")]
    public static AppInfo get_default_for_uri(string uri);
    [CCode (cheader_filename="menu-maker.h",cname="append_all_sections")]
    public static void append_all_sections(GLib.Menu menu1, GLib.MenuModel menu2);
    [CCode (cheader_filename="menu-maker.h",cname="apply_menu_properties")]
    public static void apply_menu_properties(List<unowned Widget> w, MenuModel menu);
}

[CCode (cheader_filename = "generic-config-dialog.h", cprefix = "",lower_case_cprefix="")]
namespace ValaPanel.Configurator
{
    public static Dialog generic_config_dlg(string title, Gtk.Window parent,
                                    GLib.Settings settings, ...);
    public static Widget generic_config_widget(GLib.Settings settings, ...);
}
[CCode (cheader_filename = "constants.h", cprefix = "VALA_PANEL_",lower_case_cprefix="VALA_PANEL_")]
namespace ValaPanel.Settings
{
	public const string CORE_SCHEMA;
	public const string CORE_PATH;
	public const string CORE_UNITS;
	public const string OBJECT_PATH;
	public const string OBJECT_PATH_TEMPLATE;
	public const string TOPLEVEL_ID;
}

[CCode (cheader_filename = "constants.h", cprefix = "VALA_PANEL_KEY_",lower_case_cprefix="VALA_PANEL_KEY_")]
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
    internal const string NAME;
    internal const string EXPAND;
    internal const string CAN_EXPAND;
    internal const string PACK;
    internal const string POSITION;
}
[CCode (cheader_filename="config.h", lower_case_cprefix="")]
namespace Config
{
   public const string DATADIR;
   public const string PKGDATADIR;
   public const string GETTEXT_PACKAGE;
   public const string RELEASE_NAME;
   public const string VERSION;
   public const string VERSION_INFO;
   public const string PLUGINS_DIRECTORY;
   public const string PLUGINS_DATA;
   public const string INSTALL_PREFIX;
   public const string LOCALE_DIR;
}
