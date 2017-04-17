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

public class DirmenuApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    uint number)
    {
        return new Dirmenu(toplevel,settings,number);
    }
}
public class Dirmenu: Applet, AppletConfigurable
{
    private struct DirectorySort
    {
        string dirname;
        string collate_key;
    }
    private const string DIR = "dir-path";
    private const string ICON = "icon-name";
    private const string LABEL = "caption";
    internal string dir_path
    {get; set;}
    internal string caption
    {get; set;}
    internal string icon_name
    {get; set;}
    public Dirmenu(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    uint number)
    {
        base(toplevel,settings,number);
    }
    public override void create()
    {
        var button = new MenuButton();
        var img = new Image();
        settings.bind(DIR,this,DIR,SettingsBindFlags.GET);
        settings.bind(ICON,this,ICON,SettingsBindFlags.GET);
        settings.bind(LABEL,this,LABEL,SettingsBindFlags.GET);
        setup_icon(img,set_icon(),toplevel);
        setup_button(button as Button,img,caption);
        button.set_popup(create_menu(dir_path,false));
        this.notify.connect((pspec)=>{
            if (pspec.name == ICON)
                (button.image as Image).set_from_gicon(set_icon(),IconSize.INVALID);
            if (pspec.name == LABEL)
                button.set_label(caption);
            if (pspec.name == DIR)
                button.set_popup(create_menu(dir_path,false));
        });
        this.add(button);
        this.show_all();
    }
    private Icon set_icon(){
        Icon? icon = null;
        try
        {
            icon = Icon.new_for_string(icon_name);
        } catch (GLib.Error e) {}
        return icon ?? new ThemedIcon.with_default_fallbacks("system-file-manager");
    }
    private Gtk.Menu create_menu(string directory,bool at_top)
    {
        /* Create a menu. */
        var menu = new Gtk.Menu();
        menu.set_data("path", directory);
        SList<DirectorySort?> dir_list = new SList<DirectorySort?>();

        /* Scan the specified directory to populate the menu with its subdirectories. */
        Dir? dir = null;
        try {
            dir = Dir.open(directory);
            string? name = null;
            while ((name = dir.read_name()) != null)
            {
                /* Omit hidden files. */
                if (name[0] != '.')
                {
                    string full = Path.build_filename(directory, name);
                    if (FileUtils.test(full, FileTest.IS_DIR))
                    {
                        DirectorySort cur = DirectorySort();
                        cur.dirname = Filename.display_name(name);
                        cur.collate_key = cur.dirname.collate_key();
                        dir_list.insert_sorted(cur,(a,b)=>{return strcmp(a.collate_key,b.collate_key);});
                    }
                }
            }
        } catch (Error e) {stderr.printf("%s\n",e.message);}
        /* The sorted directory name list is complete.  Loop to create the menu. */
        foreach(var cursor in dir_list)
        {
            /* Create and initialize menu item. */
            Gtk.MenuItem item = new Gtk.MenuItem();
            var box = new Box(Orientation.HORIZONTAL,10);
                    var img = new Image.from_gicon(new ThemedIcon.with_default_fallbacks("folder-symbolic"),IconSize.MENU);
            box.pack_start(img,false,true);
            var lbl = new Label(cursor.dirname);
            box.pack_start(lbl,false,true);
            item.add(box);
            Gtk.Menu dummy = new Gtk.Menu();
            item.set_submenu(dummy);
            menu.append(item);

            /* Unlink and free sorted directory name element, but reuse the directory name string. */
            item.set_data("name",cursor.dirname);
            /* Connect signals. */
            item.select.connect(()=>{
                if (item.get_submenu != null)
                {
                    /* On first reference, populate the submenu using the parent directory and the item directory name. */
                    string dpath = item.get_submenu().get_data<string>("path");
                    if (dpath == null)
                    {
                        dpath = Path.build_filename(
                                item.get_parent().get_data<string>("path"),
                            item.get_data<string>("name"));
                        item.set_submenu(create_menu(dpath, true));
                    }
                }
            });
            item.deselect.connect(()=>{item.set_submenu(new Gtk.Menu());});
        }
        /* Create "Open" and "Open in Terminal" items. */
        var item = new Gtk.MenuItem.with_mnemonic(  _("_Open") );
        item.activate.connect(()=>{
                try {
                    GLib.AppInfo.launch_default_for_uri(Filename.to_uri(directory),
                        Gdk.Display.get_default().get_app_launch_context());
                } catch(GLib.Error e){stderr.printf("%s",e.message);}
            });
        var term = new Gtk.MenuItem.with_mnemonic( _("Open in _Terminal") );
        term.activate.connect(()=>{launch_terminal(directory);});

        /* Insert or append based on caller's preference. */
        if (at_top)
        {
            menu.insert(new SeparatorMenuItem(), 0);
            menu.insert(term, 0);
            menu.insert(item, 0);
        }
        else {
            menu.append(new SeparatorMenuItem());
            menu.append(term);
            menu.append(item);
        }

        /* Show the menu and return. */
        menu.show_all();
        return menu;
    }
    private void launch_terminal(string dir)
    {
        string? command;
        string true_dir = dir;
        toplevel.application.get("terminal-command",out command);
        if (!Path.is_absolute(dir))
            true_dir = Posix.realpath(dir);
        try
        {
            string[] argv = command.split(" ");
            var envp = Environ.set_variable(Environ.get(),"PWD",true_dir,true);
            Process.spawn_async(true_dir,argv,envp,SpawnFlags.SEARCH_PATH,null,null);
        }
        catch (GLib.Error e){stderr.printf("Cannot launch terminal: %s\n",e.message);}
    }
    public override bool button_release_event(Gdk.EventButton e)
    {
        if (e.button == 2)
            launch_terminal(dir_path);
        return false;
    }
    public Dialog get_config_dialog()
    {
        return Configurator.generic_config_dlg(_("Directory Menu"),
                            toplevel, this.settings,
                            _("Directory"), DIR, GenericConfigType.DIRECTORY,
                            _("Label"), LABEL, GenericConfigType.STR,
                            _("Icon"), ICON, GenericConfigType.FILE_ENTRY);
    }
} // End class

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(DirmenuApplet));
}
