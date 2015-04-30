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

using ValaPanel;
using Gtk;
public class MenuApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    uint number)
    {
        return new Menu(toplevel,settings,number);
    }
}
namespace Key
{
    internal static const string ICON = "icon-name";
    internal static const string CAPTION = "menu-caption";
    internal static const string IS_MENU_BAR = "is-menu-bar";
    internal static const string IS_SYSTEM_MENU = "is-system-menu";
    internal static const string IS_INTERNAL_MENU = "is-internal-menu";
    internal static const string MODEL_FILE = "model-file";
    internal static const string[] rebuild_keys = {IS_MENU_BAR,IS_INTERNAL_MENU,MODEL_FILE,ICON};
}
internal enum InternalMenu
{
    SETTINGS,
    APPLICATIONS,
    RECENT,
    MOUNTS
}
public class Menu: Applet, AppletConfigurable, AppletMenu
{
    GLib.Menu menu;
    Widget? button;
    AppInfoMonitor? app_monitor;
    FileMonitor? file_monitor;
    ulong show_system_menu_idle;
    internal string? icon
    {get; set;}
    internal bool system
    {get; set;}
    internal bool intern
    {get; set;}
    internal bool bar
    {get; set;}
    internal string? caption
    {get; set;}
    internal string? filename
    {get; set;}
    public Menu(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    uint number)
    {
        base(toplevel,settings,number);
    }
    public Dialog get_config_dialog()
    {
        return Configurator.generic_config_dlg(_("Custom Menu"), toplevel, this,
                                      _("If internal menu is enabled, menu file will not be used, predefeined menu will be used instead."),null, GenericConfigType.TRIM,
                                      _("Is internal menu"), Key.IS_INTERNAL_MENU, GenericConfigType.BOOL,
                                      _("Is system menu (can be keybound)"), Key.IS_SYSTEM_MENU, GenericConfigType.BOOL,
                                      _("Is Menubar"), Key.IS_MENU_BAR, GenericConfigType.BOOL,
                                      _("Icon"), Key.ICON, GenericConfigType.FILE_ENTRY,
                                      _("Caption (for button only)"), Key.CAPTION, GenericConfigType.STR,
                                      _("Menu file name"), Key.MODEL_FILE, GenericConfigType.FILE_ENTRY);
    }
    public void show_system_menu()
    {
        if (system && show_system_menu_idle == 0)
            Timeout.add(200,show_menu);
    }
    public bool show_menu()
    {
        if (GLib.MainContext.current_source().is_destroyed()) return false;
        Gtk.Menu menuw = new Gtk.Menu.from_model(menu);
        MenuMaker.apply_menu_properties(menuw.get_children(),menu);
        menuw.attach_to_widget(this,null);
        menuw.popup(null,null,menu_position_func,
                    0, Gdk.CURRENT_TIME);
        menuw.focus_out_event.connect((event)=>{menuw.destroy();return false;});
        show_system_menu_idle = 0;
        return false;
    }
    public override void create()
    {
        button = null;
        this.set_visible_window(false);
        settings.bind(Key.IS_SYSTEM_MENU,this,"system",SettingsBindFlags.GET);
        settings.bind(Key.IS_MENU_BAR,this,"bar",SettingsBindFlags.GET);
        settings.bind(Key.IS_INTERNAL_MENU,this,"intern",SettingsBindFlags.GET);
        settings.bind(Key.MODEL_FILE,this,"filename",SettingsBindFlags.GET);
        settings.bind(Key.ICON,this,"icon",SettingsBindFlags.GET);
        settings.bind(Key.CAPTION,this,"caption",SettingsBindFlags.GET);
        button = menumodel_widget_create();
        button.show();
        var gtksettings = this.get_settings();
        gtksettings.gtk_shell_shows_menubar = false;
        this.show_all();
        this.notify.connect((pspec)=>{
            if ((pspec.name == "intern")
                || (pspec.name == "caption" && !bar)
                || (pspec.name == "filename" && !intern)
                || (pspec.name == "bar")
                || (pspec.name == "icon"))
            {
                menumodel_widget_destroy();
                button = menumodel_widget_create();
            }
        });
    }
    private Widget menumodel_widget_create()
    {
        menu = create_menumodel() as GLib.Menu;
        if (bar)
            return create_menubar() as Widget;
        else
            return create_menubutton() as Widget;
    }
    private MenuBar create_menubar()
    {
        var menubar = new MenuBar.from_model(menu);
        MenuMaker.apply_menu_properties(menubar.get_children(),menu);
        this.add(menubar);
        this.background_widget = menubar;
        init_background();
        menubar.show();
        var orient = toplevel.orientation == Orientation.HORIZONTAL ? PackDirection.LTR : PackDirection.TTB;
        menubar.set_pack_direction(orient);
        toplevel.notify["edge"].connect(()=>{
            orient = toplevel.orientation == Orientation.HORIZONTAL ? PackDirection.LTR : PackDirection.TTB;
            menubar.set_pack_direction(orient);
        });
        Gtk.IconTheme.get_default().changed.connect(()=>{
            MenuMaker.apply_menu_properties(menubar.get_children(),menu);
        });
        return menubar;
    }
    private MenuButton create_menubutton()
    {
        Image? img = null;
        var menubutton = new MenuButton();
        var int_menu = new Gtk.Menu.from_model(menu);
        MenuMaker.apply_menu_properties(int_menu.get_children(),menu);
        int_menu.show_all();
        menubutton.set_popup(int_menu);
        menubutton.use_popover = false;
        if(icon != null)
        {
            img = new Image();
            try {
                var gicon = Icon.new_for_string(icon);
                ValaPanel.setup_icon(img,gicon,toplevel);
            } catch (Error e){stderr.printf("%s\n",e.message);}
            img.show();
        }
        ValaPanel.setup_button(menubutton as Button,img,caption);
        this.add(menubutton);
        menubutton.show();
        Gtk.IconTheme.get_default().changed.connect(()=>{
            MenuMaker.apply_menu_properties(int_menu.get_children(),menu);
        });
        return menubutton;
    }
    private void menumodel_widget_destroy()
    {
        app_monitor = null;
        file_monitor = null;
        if (this.get_child()!= null)
        {
            this.remove(this.get_child());
            button = null;
            this.background_widget = this;
        }
    }
    private GLib.MenuModel? create_menumodel()
    {
        GLib.MenuModel ret;
        if (intern)
        {
            ret = MenuMaker.create_main_menu(bar,icon);
            app_monitor = AppInfoMonitor.get();
            app_monitor.changed.connect(()=>{
                menumodel_widget_destroy();
                button = menumodel_widget_create();
            });
            file_monitor = null;
        }
        else
        {
            if (filename == null)
                return null;
            var f = File.new_for_path(filename);
            ret = read_menumodel();
            app_monitor = null;
            try {
                file_monitor = f.monitor_file(FileMonitorFlags.SEND_MOVED|FileMonitorFlags.WATCH_HARD_LINKS);
            } catch (Error e) {stderr.printf("%s\n",e.message);}
            file_monitor.changed.connect(()=>{
                menumodel_widget_destroy();
                button = menumodel_widget_create();
            });
        }
        return ret;
    }
    private GLib.MenuModel? read_menumodel()
    {
        var builder = new Builder();
        try{
            builder.add_from_file(filename);
            var ret = builder.get_object("vala-panel-menu") as GLib.MenuModel;
            var gotten = builder.get_object("vala-panel-internal-applications") as GLib.Menu;
            if (gotten != null)
                load_internal_menus(gotten,InternalMenu.APPLICATIONS);
            gotten = builder.get_object("vala-panel-internal-settings") as GLib.Menu;
            if (gotten != null)
                load_internal_menus(gotten,InternalMenu.SETTINGS);
            gotten = builder.get_object("vala-panel-internal-mounts") as GLib.Menu;
            if (gotten != null)
                load_internal_menus(gotten,InternalMenu.MOUNTS);
            gotten = builder.get_object("vala-panel-internal-recent") as GLib.Menu;
            if (gotten != null)
                load_internal_menus(gotten,InternalMenu.RECENT);
            return ret;
        } catch (Error e)
        {
            stderr.printf("%s",e.message);
        }
        return null;
    }
    private static void load_internal_menus(GLib.Menu int_menu, InternalMenu req)
    {
        GLib.MenuModel section;
        switch (req)
        {
            case InternalMenu.APPLICATIONS:
                section = MenuMaker.create_applications_menu(false);
                int_menu.append_section(null,section);
                break;
            case InternalMenu.SETTINGS:
                section = MenuMaker.create_applications_menu(true);
                int_menu.append_section(null,section);
                break;
        }
    }
} // End class

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(MenuApplet));
}
