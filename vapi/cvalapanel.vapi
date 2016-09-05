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
}
[CCode (cprefix="")]
namespace MenuMaker
{
    [CCode (cheader_filename="lib/c-lib/launcher.h",cname="activate_menu_launch_id")]
    public static void activate_menu_launch_id(SimpleAction? action, Variant? param, void* user_data);
    [CCode (cheader_filename="lib/c-lib/launcher.h",cname="activate_menu_launch_uri")]
    public static void activate_menu_launch_uri(SimpleAction? action, Variant? param, void* user_data);
    [CCode (cheader_filename="lib/c-lib/launcher.h",cname="activate_menu_launch_command")]
    public static void activate_menu_launch_command(SimpleAction? action, Variant? param, void* user_data);
    [CCode (cheader_filename="lib/c-lib/launcher.h",cname="vala_panel_launch")]
    public static bool launch(DesktopAppInfo info, GLib.List<string>? uris, Gtk.Widget parent);
}

[CCode (cheader_filename = "lib/c-lib/generic-config-dialog.h", cprefix = "",lower_case_cprefix="")]
namespace ValaPanel.Configurator
{
    public static Dialog generic_config_dlg(string title, Gtk.Window parent,
                                    GLib.Settings settings, ...);
}
