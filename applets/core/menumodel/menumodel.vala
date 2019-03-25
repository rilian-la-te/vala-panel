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

public class MenuPPlugin : AppletPlugin
{
    public override Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    string number)
    {
        return new MenuP(toplevel,settings,number);
    }
}

namespace Key
{
    internal const string ICON = "icon-name";
    internal const string CAPTION = "menu-caption";
    internal const string IS_MENU_BAR = "is-menu-bar";
    internal const string IS_SYSTEM_MENU = "is-system-menu";
    internal const string IS_INTERNAL_MENU = "is-internal-menu";
    internal const string MODEL_FILE = "model-file";
}

namespace MenuMaker
{
    extern static GLib.MenuModel create_applications_menu(bool do_settings);
    extern static GLib.MenuModel create_main_menu(bool submenus, string? icon);
}

internal enum InternalMenu
{
    SETTINGS,
    APPLICATIONS,
    RECENT,
    MOUNTS
}

public class MenuP: Applet
{
    GLib.Menu? menu;
    unowned Container? button;
    unowned Gtk.Menu? int_menu;
    AppInfoMonitor? app_monitor;
    FileMonitor? file_monitor;
    ulong show_system_menu_idle;
    internal string? icon {get; set;}
    internal bool system {get; set;}
    internal bool intern {get; set;}
    internal bool bar {get; set;}
    internal string? caption {get; set;}
    internal string? filename {get; set;}
    public MenuP(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    string number)
    {
         base(toplevel,settings,number);
         (this.action_group.lookup_action(AppletAction.CONFIGURE) as SimpleAction).set_enabled(true);
         (this.action_group.lookup_action(AppletAction.REMOTE) as SimpleAction).set_enabled(true);
         button = null;
         settings.bind(Key.IS_SYSTEM_MENU,this,"system",SettingsBindFlags.GET);
         settings.bind(Key.IS_MENU_BAR,this,"bar",SettingsBindFlags.GET);
         settings.bind(Key.IS_INTERNAL_MENU,this,"intern",SettingsBindFlags.GET);
         settings.bind(Key.MODEL_FILE,this,"filename",SettingsBindFlags.GET);
         settings.bind(Key.ICON,this,"icon",SettingsBindFlags.GET);
         settings.bind(Key.CAPTION,this,"caption",SettingsBindFlags.GET);
         var w = menumodel_widget_create();
         button = w;
         this.add(button);
         unowned Gtk.Settings gtksettings = this.get_settings();
         gtksettings.gtk_shell_shows_menubar = false;
         button.show();
         this.show();
         settings.changed.connect((key)=>{
             if ((key == Key.IS_INTERNAL_MENU)
                 || (key == Key.MODEL_FILE && !intern)
                 || (key == Key.IS_MENU_BAR)
                 || (key == Key.ICON && bar))
             {
                 menumodel_widget_rebuild();
             }
             else if (key == Key.CAPTION && !bar)
             {
                 unowned Button btn = button as Button;
                 btn.label = caption;
             }
             else if (key == Key.ICON && !bar)
             {
                 try
                 {
                     unowned Button btn = button as Button;
                     (btn.image as Gtk.Image).gicon = Icon.new_for_string(icon);
                 } catch (Error e){stderr.printf("%s\n",e.message);}
             }
         });
     }
    public override void destroy()
    {
        menumodel_widget_destroy();
        base.destroy();
    }
    public override Widget get_settings_ui()
    {
        return Configurator.generic_config_widget(this.settings,
                                      _("If internal menu is enabled, menu file will not be used,\n predefeined menu will be used instead."),null, GenericConfigType.TRIM,
                                      _("Is internal menu"), Key.IS_INTERNAL_MENU, GenericConfigType.BOOL,
                                      _("Is system menu (can be keybound)"), Key.IS_SYSTEM_MENU, GenericConfigType.BOOL,
                                      _("Is Menubar"), Key.IS_MENU_BAR, GenericConfigType.BOOL,
                                      _("Icon"), Key.ICON, GenericConfigType.FILE_ENTRY,
                                      _("Caption (for button only)"), Key.CAPTION, GenericConfigType.STR,
                                      _("Menu file name"), Key.MODEL_FILE, GenericConfigType.FILE_ENTRY);
    }
    public override bool remote_command(string command_name)
    {
        if (command_name == "menu" && system && show_system_menu_idle == 0)
        {
            Timeout.add(200,show_menu_int);
            return true;
        }
        return false;
    }
    public bool show_menu_int()
    {
        if (GLib.MainContext.current_source().is_destroyed()) return false;
        if (int_menu != null)
            int_menu.popup_at_widget(this,Gdk.Gravity.NORTH,Gdk.Gravity.NORTH,null);
        else
        {
            unowned Gtk.MenuBar menubar = button as Gtk.MenuBar;
            menubar.select_first(false);
        }
        show_system_menu_idle = 0;
        return false;
    }
    private void menumodel_widget_rebuild()
    {
        menumodel_widget_destroy();
        var btn = menumodel_widget_create();
        button = btn;
        this.add(button);
    }
    private Container menumodel_widget_create()
    {
        menu = create_menumodel() as GLib.Menu;
        Container? ret;
        if (bar)
            ret = create_menubar() as Container;
        else
            ret = create_menubutton() as Container;
        return ret;
    }
    private Gtk.MenuBar create_menubar()
    {
        int_menu = null;
        if(menu == null)
            return new MenuBar();
        var menubar = new MenuBar.from_model(menu);
        MenuMaker.apply_menu_properties(menubar.get_children(),menu);
        this.background_widget = menubar;
        init_background();
        menubar.show();
        var orient = toplevel.orientation == Orientation.HORIZONTAL ? PackDirection.LTR : PackDirection.TTB;
        menubar.set_pack_direction(orient);
        toplevel.notify["panel-gravity"].connect(()=>{
            orient = toplevel.orientation == Orientation.HORIZONTAL ? PackDirection.LTR : PackDirection.TTB;
            menubar.set_pack_direction(orient);
        });
        return menubar;
    }
    private ToggleButton create_menubutton()
    {
        Image? img = null;
        var menubutton = new ToggleButton();
        if(menu == null)
            return menubutton;
        var menuw = new Gtk.Menu.from_model(menu);
        int_menu = menuw;
        MenuMaker.apply_menu_properties(int_menu.get_children(),menu);
        int_menu.attach_to_widget(menubutton,null);
        menubutton.toggled.connect(()=>{
            if(menubutton.active && !int_menu.visible)
                int_menu.popup_at_widget(this,Gdk.Gravity.NORTH,Gdk.Gravity.NORTH,null);
            else
                int_menu.popdown();
        });
        int_menu.hide.connect(()=>{
            menubutton.active = false;
        });
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
        menubutton.show();
        return menubutton;
    }
    private void menumodel_widget_destroy()
    {
        this.background_widget = this;
        if (int_menu is Widget)
        {
            if(button != null)
                int_menu.detach();
            if(int_menu is Widget)
                int_menu.destroy();
            int_menu = null;
        }
        if (button is Widget)
        {
            if(button is Widget)
                button.destroy();
            button = null;
        }
        if (app_monitor != null)
        {
            SignalHandler.disconnect_by_data(app_monitor,this);
            app_monitor = null;
        }
        if (file_monitor != null)
        {
            SignalHandler.disconnect_by_data(file_monitor,this);
            file_monitor = null;
        }
    }
    private GLib.MenuModel? create_menumodel()
    {
        GLib.MenuModel ret;
        if (intern)
        {
            ret = MenuMaker.create_main_menu(bar,icon);
            app_monitor = AppInfoMonitor.get();
            app_monitor.changed.connect(menumodel_widget_rebuild);
            file_monitor = null;
        }
        else
        {
            if (filename == null)
                return null;
            ret = read_menumodel();
            try {
                var f = File.new_for_path(filename);
                file_monitor = f.monitor(FileMonitorFlags.WATCH_MOVES);
            } catch (Error e) {stderr.printf("%s\n",e.message);}
            file_monitor.changed.connect(menumodel_widget_rebuild);
            app_monitor = null;
        }
        return ret;
    }
    private GLib.MenuModel? read_menumodel()
    {
        var builder = new Builder();
        try{
            builder.add_from_file(filename);
            unowned GLib.Menu ret = builder.get_object("vala-panel-menu") as GLib.Menu;
            unowned GLib.Menu gotten = builder.get_object("vala-panel-internal-applications") as GLib.Menu;
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
                MenuMaker.copy_model_items(int_menu,section);
                break;
            case InternalMenu.SETTINGS:
                section = MenuMaker.create_applications_menu(true);
                MenuMaker.copy_model_items(int_menu,section);
                break;
        }
    }
} // End class

[ModuleInit]
public void g_io_menumodel_load(GLib.TypeModule module)
{
    // boilerplate - all modules need this
    module.use();
    GLib.IOExtensionPoint.implement(ValaPanel.Applet.EXTENSION_POINT,typeof(MenuPPlugin),"org.valapanel.menumodel",10);
}

public void g_io_menumodel_unload(GLib.IOModule module)
{
    // boilerplate - all modules need this
}
