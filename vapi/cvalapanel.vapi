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

[CCode (cheader_filename = "lib/c-lib/css.h", cprefix = "",lower_case_cprefix="css_")]
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
namespace ValaPanel
{
    [CCode(cname="GenericConfigType", has_type_id = false, cprefix="CONF_", cheader_filename = "lib/c-lib/generic-config-dialog.h")]
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
    [CCode(cname="AutohideState", has_type_id = false, cprefix="AH_", cheader_filename = "lib/c-lib/panel-manager.h")]
    internal enum AutohideState
    {
        VISIBLE,
        HIDDEN,
        WAITING
    }
    [CCode(cname="AlignmentType", has_type_id = false, cprefix="", cheader_filename = "lib/c-lib/toplevel.h")]
    public enum AlignmentType
    {
        START = 0,
        CENTER = 1,
        END = 2
    }
    [CCode(cname="IconSizeHints", has_type_id = false, cprefix="", cheader_filename = "lib/c-lib/toplevel.h")]
    internal enum IconSizeHints
    {
        XXS = 16,
        XS = 22,
        S = 24,
        M = 32,
        L = 48,
        XL = 96,
        XXL = 128,
        XXXL = 256;
    }
    [CCode(cname = "_user_config_file_name",cheader_filename="lib/definitions.h")]
    public string user_config_file_name(string name1, string profile, string? name2);
    [CCode (cheader_filename="lib/c-lib/misc.h")]
    public static void apply_window_icon(Window w);
    [CCode (cname = "vala_panel_add_gsettings_as_action",cheader_filename="lib/c-lib/misc.h")]
    public static void settings_as_action(ActionMap map, GLib.Settings settings, string prop);
    [CCode(cname = "vala_panel_bind_gsettings",cheader_filename="lib/definitions.h")]
    public static void settings_bind(Object map, GLib.Settings settings, string prop);
    [CCode (cheader_filename="lib/c-lib/misc.h")]
    public static void setup_button(Button b, Image? img = null, string? label = null);
    [CCode (cheader_filename="lib/c-lib/misc.h")]
    public static void setup_label(Label label, string text, bool bold, double factor);
    [CCode (cheader_filename="lib/c-lib/misc.h")]
    public static void scale_button_set_range(ScaleButton b, int lower, int upper);
    [CCode (cheader_filename="lib/c-lib/misc.h")]
    public static void scale_button_set_value_labeled(ScaleButton b, int val);
    [CCode (cname = "ValaPanelRunner",cheader_filename = "app/runner.h")]
    public class Runner : Gtk.Dialog
    {
        [CCode (cname = "vala_panel_runner_new",cheader_filename = "app/runner.h")]
        public inline Runner(Gtk.Application app)
        {
            Object(application: app);
        }
        [CCode (cname = "gtk_run",cheader_filename = "app/runner.h")]
        public void gtk_run();
    }
}
[CCode (cprefix="")]
namespace MenuMaker
{
    [CCode (cheader_filename="lib/c-lib/menu-maker.h",cname="ATTRIBUTE_DND_SOURCE")]
    public static const string ATTRIBUTE_DND_SOURCE;
    [CCode (cheader_filename="lib/c-lib/menu-maker.h",cname="ATTRIBUTE_TOOLTIP")]
    public static const string ATTRIBUTE_TOOLTIP;
    [CCode (cheader_filename="lib/c-lib/launcher.h",cname="activate_menu_launch_id")]
    public static void activate_menu_launch_id(SimpleAction? action, Variant? param, void* user_data);
    [CCode (cheader_filename="lib/c-lib/launcher.h",cname="activate_menu_launch_uri")]
    public static void activate_menu_launch_uri(SimpleAction? action, Variant? param, void* user_data);
    [CCode (cheader_filename="lib/c-lib/launcher.h",cname="activate_menu_launch_command")]
    public static void activate_menu_launch_command(SimpleAction? action, Variant? param, void* user_data);
    [CCode (cheader_filename="lib/c-lib/launcher.h",cname="vala_panel_launch")]
    public static bool launch(DesktopAppInfo info, GLib.List<string>? uris, Gtk.Widget parent);
    [CCode (cheader_filename="lib/c-lib/launcher.h",cname="vala_panel_get_default_for_uri")]
    public static AppInfo get_default_for_uri(string uri);
    [CCode (cheader_filename="lib/c-lib/menu-maker.h",cname="append_all_sections")]
    public static void append_all_sections(GLib.Menu menu1, GLib.MenuModel menu2);
    [CCode (cheader_filename="lib/c-lib/menu-maker.h",cname="apply_menu_properties")]
    public static void apply_menu_properties(List<unowned Widget> w, MenuModel menu);
}

[CCode (cheader_filename = "lib/c-lib/generic-config-dialog.h", cprefix = "",lower_case_cprefix="")]
namespace ValaPanel.Configurator
{
    public static Dialog generic_config_dlg(string title, Gtk.Window parent,
                                    GLib.Settings settings, ...);
}

[CCode (cheader_filename = "lib/c-lib/toplevel.h", cprefix = "VALA_PANEL_KEY_",lower_case_cprefix="VALA_PANEL_KEY_")]
namespace ValaPanel.Key
{
    public static const string EDGE;
    public static const string ALIGNMENT;
    public static const string HEIGHT;
    public static const string WIDTH;
    public static const string DYNAMIC;
    public static const string AUTOHIDE;
    public static const string SHOW_HIDDEN;
    public static const string STRUT;
    public static const string DOCK;
    public static const string MONITOR;
    public static const string MARGIN;
    public static const string ICON_SIZE;
    public static const string BACKGROUND_COLOR;
    public static const string FOREGROUND_COLOR;
    public static const string BACKGROUND_FILE;
    public static const string FONT;
    public static const string CORNERS_SIZE;
    public static const string USE_BACKGROUND_COLOR;
    public static const string USE_FOREGROUND_COLOR;
    public static const string USE_FONT;
    public static const string FONT_SIZE_ONLY;
    public static const string USE_BACKGROUND_FILE;
}
