/*
 * vala-panel
 * Copyright (C) 2015 Konstantin Pugin <ria.freelander@gmail.com>
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

using GLib;
using Gtk;

namespace MenuMaker
{
    private static void parse_app_info(GLib.DesktopAppInfo info, Gtk.Builder builder)
    {
        if (info.should_show())
        {
            unowned GLib.Menu menu_link;
            var found = false;
            var action = "app.launch-id('%s')".printf(info.get_id());
            var item = new GLib.MenuItem(info.get_name(),action);
            if (info.get_icon() != null)
                item.set_icon(info.get_icon());
            else
                item.set_attribute(GLib.Menu.ATTRIBUTE_ICON,"s","application-x-executable");
            item.set_attribute(ATTRIBUTE_DND_SOURCE,"b",true);
            if(info.get_description() != null)
                item.set_attribute(ATTRIBUTE_TOOLTIP,"s",info.get_description());
            unowned string cats_str = info.get_categories() ?? " ";
            var cats = cats_str.split_set(";");
            foreach (unowned string cat in cats)
            {
                menu_link = (GLib.Menu)builder.get_object(cat.down());
                if (menu_link != null)
                {
                    found = true;
                    menu_link.append_item(item);
                    break;
                }
            }
            if (!found)
            {
                menu_link = (GLib.Menu)builder.get_object("other");
                menu_link.append_item(item);
            }
        }
    }

    public static GLib.MenuModel applications_model(string[] cats)
    {
        var builder = new Gtk.Builder.from_resource("/org/vala-panel/menumodel/system-menus.ui");
        unowned GLib.Menu menu = (GLib.Menu) builder.get_object("applications-menu");
        var list = AppInfo.get_all();
        foreach (unowned AppInfo info in list)
            parse_app_info((GLib.DesktopAppInfo)info,builder);
        for(int i = 0; i < menu.get_n_items(); i++)
        {
            i = (i < 0) ? 0 : i;
            string cat;
            bool in_cat = menu.get_item_attribute(i,"x-valapanel-cat","s",out cat);
            var submenu = (GLib.Menu)menu.get_item_link(i,GLib.Menu.LINK_SUBMENU);
            if (submenu.get_n_items() <= 0 || (in_cat && cat in cats))
            {
                menu.remove(i);
                i--;
            }
            if (i >= menu.get_n_items() || menu.get_n_items() <= 0)
                break;
        }
        menu.freeze();
        return (GLib.MenuModel) menu;
    }

    public static static GLib.MenuModel create_applications_menu(bool do_settings)
    {
        string[] apps_cats = {"audiovideo","education","game","graphics","system",
                            "network","office","utility","development","other"};
        string[] settings_cats = {"settings"};
        if (do_settings)
            return applications_model(apps_cats);
        else
            return applications_model(settings_cats);
    }

    public static GLib.MenuModel create_places_menu()
    {
        var builder = new Gtk.Builder.from_resource ("/org/vala-panel/menumodel/system-menus.ui");
        unowned GLib.Menu menu = (GLib.Menu) builder.get_object("places-menu");
        unowned GLib.Menu section = (GLib.Menu) builder.get_object("folders-section");
        var item = new GLib.MenuItem(_("Home"),null);
        item.set_attribute(GLib.Menu.ATTRIBUTE_ICON,"s","user-home");
        string path = null;
        try
        {
            path = GLib.Filename.to_uri (GLib.Environment.get_home_dir ());
        }
        catch (GLib.ConvertError e){}
        item.set_action_and_target("app.launch-uri","s",path);
        item.set_attribute(ATTRIBUTE_DND_SOURCE,"b",true);
        section.append_item(item);
        item = new GLib.MenuItem(_("Desktop"),null);
        try
        {
            path = GLib.Filename.to_uri (GLib.Environment.get_user_special_dir(
                                           GLib.UserDirectory.DESKTOP));
        }
        catch (GLib.ConvertError e){}
        item.set_attribute(GLib.Menu.ATTRIBUTE_ICON,"s","user-desktop");
        item.set_attribute("dnd-target","b",true);
        item.set_action_and_target("app.launch-uri","s",path);
        section.append_item(item);
        section = (GLib.Menu) builder.get_object("recent-section");
        var info = new GLib.DesktopAppInfo("gnome-search-tool.desktop");
        if (info == null)
            info = new GLib.DesktopAppInfo("mate-search-tool.desktop");
        if (info != null)
        {
            item = new GLib.MenuItem(_("Search..."),null);
            item.set_attribute(GLib.Menu.ATTRIBUTE_ICON,"s","system-search");
            if(info.get_description() != null)
                item.set_attribute(ATTRIBUTE_TOOLTIP,"s",info.get_description());
            item.set_attribute(ATTRIBUTE_DND_SOURCE,"b",true);
            item.set_action_and_target("app.launch-id","s",info.get_id());
            section.prepend_item(item);
        }
        section.remove(1);
        return (GLib.MenuModel) menu;
    }

    public static GLib.MenuModel create_system_menu()
    {
        var builder = new Gtk.Builder.from_resource ("/org/vala-panel/menumodel/system-menus.ui");
        unowned GLib.Menu menu = (GLib.Menu) builder.get_object("settings-section");
        menu.append_section(null,MenuMaker.create_applications_menu (true));
        var info = new GLib.DesktopAppInfo("gnome-control-center.desktop");
        if (info == null)
            info = new GLib.DesktopAppInfo("matecc.desktop");
        if (info == null)
            info = new GLib.DesktopAppInfo("cinnamon-settings.desktop");
        if (info == null)
            info = new GLib.DesktopAppInfo("xfce4-settings-manager.desktop");
        if (info == null)
            info = new GLib.DesktopAppInfo("kdesystemsettings.desktop");
        if (info != null)
        {
            var item = new GLib.MenuItem(_("Control center"),null);
            item.set_attribute(GLib.Menu.ATTRIBUTE_ICON,"s","preferences-system");
            if(info.get_description() != null)
                item.set_attribute(ATTRIBUTE_TOOLTIP,"s",info.get_description());
            item.set_attribute(ATTRIBUTE_DND_SOURCE,"b",true);
            item.set_action_and_target("app.launch-id","s",info.get_id());
            menu.append_item(item);
        }
        menu.freeze();
        menu = (GLib.Menu) builder.get_object("system-menu");
        menu.freeze();
        return (GLib.MenuModel) menu;
    }

    public static GLib.MenuModel create_main_menu(bool submenus, string? icon)
    {
        var menu = new GLib.Menu();
        if (submenus)
        {
            var item = new GLib.MenuItem.submenu (_("Applications"),
                                                  MenuMaker.create_applications_menu (false));
            item.set_attribute(GLib.Menu.ATTRIBUTE_ICON,"s",icon);
            menu.append_item(item);
            menu.append_submenu(_("Places"),MenuMaker.create_places_menu());
            menu.append_submenu(_("System"),MenuMaker.create_system_menu());
        }
        else
        {
            menu.append("%s %s".printf(_("Vala Panel"),Config.VERSION),"foo.improper-action");
            menu.append_section(null,MenuMaker.create_applications_menu (false));
            var section = new GLib.Menu();
            section.append_submenu(_("Places"),MenuMaker.create_places_menu());
            menu.append_section(null,section);
            MenuMaker.append_all_sections(menu,MenuMaker.create_system_menu());
        }
        menu.freeze();
        return (GLib.MenuModel) menu;
    }
}
