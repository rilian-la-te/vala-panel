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
using Gdk;

namespace PanelCSS
{
    public void apply_with_class(Gtk.Widget w, string css, string klass, bool add);
    public void toggle_class(Widget w, string klass, bool apply);
/*    public CssProvider? add_css_to_widget(Widget w, string css)
    {
        unowned StyleContext context = w.get_style_context();
        w.reset_style();
        var provider = new Gtk.CssProvider();
        try
        {
            provider.load_from_data(css,css.length);
            context.add_provider(provider,Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);
            return provider;
        } catch (GLib.Error e) {}
        return null;
    }
    public Gtk.CssProvider? apply_with_provider(Gtk.Widget w, string css, string klass)
    {
        unowned StyleContext context = w.get_style_context();
        w.reset_style();
        var provider = new Gtk.CssProvider();
        try
        {
            provider.load_from_data(css,css.length);
            context.add_provider(provider,Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);
            context.add_class(klass);
            return provider;
        } catch (GLib.Error e) {}
        return null;
    } */

    public Gtk.CssProvider? apply_from_file_to_app_with_provider(string file);
    public void apply_from_resource(Gtk.Widget w, string file, string klass);
/*    public Gtk.CssProvider? apply_from_resource_with_provider(Gtk.Widget w, string file, string klass)
    {
        unowned StyleContext context = w.get_style_context();
        w.reset_style();
        var provider = new Gtk.CssProvider();
        File ruri = File.new_for_uri("resource:/%s".printf(file));
        try
        {
            provider.load_from_file(ruri);
            context.add_provider(provider,Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);
            context.add_class(klass);
            return provider;
        } catch (GLib.Error e) {}
        return null;
    }*/

    public string generate_background(string? name, Gdk.RGBA color);
    public string generate_font_size(int size);
    public string generate_font_color(Gdk.RGBA color);
    public string generate_font_label(double size ,bool bold);
    public string generate_flat_button(Gtk.Widget w, Gtk.PositionType e);
}
