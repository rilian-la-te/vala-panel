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
using Config;
using PanelCSS;

namespace ValaPanel
{
    namespace Key
    {
        public const string RUN = "run-command";
        public const string LOGOUT = "logout-command";
        public const string SHUTDOWN = "shutdown-command";
        public const string TERMINAL = "terminal-command";
        public const string DARK = "is-dark";
        public const string CUSTOM = "is-custom";
        public const string CSS = "css";
    }
    public static int main(string[] args)
    {
        var app = new App();
        return app.run(args);
    }
    [Compact,Immutable]
    public class SpawnData
    {
        internal Posix.pid_t pid; /* getpgid(getppid()); */
        public SpawnData()
        {
            pid = Posix.getpgid(Posix.getppid());
        }
        public void child_spawn_func()
        {
            Posix.setpgid(0,this.pid);
        }
    }
    public class App: Gtk.Application
    {
        private const string SCHEMA = "org.valapanel";
        private const string NAME = "global";
        private const string PATH = "/org/vala-panel/";
        private bool started = false;
        private bool restart = false;
        private Dialog? pref_dialog;
        private GLib.Settings config;
        private bool _dark;
        private bool _custom;
        private string _css;
        private CssProvider provider;
        public string profile {get; internal set; default = "default";}
        public string run_command {get; internal set;}
        public string terminal_command {get; internal set;}
        public string logout_command {get; internal set;}
        public string shutdown_command {get; internal set;}
        public bool is_dark
        {get {return _dark;}
                internal set {_dark = value; apply_styling();}}
        public bool is_custom
        {get {return _custom;}
                internal set {_custom = value; apply_styling();}}
        public string css
        {get {return _css;}
                internal set {_css = value; apply_styling();}}

        private const OptionEntry[] options =
        {
            { "version", 'v', 0, OptionArg.NONE, null, N_("Print version and exit"), null },
            { "profile", 'p', 0, OptionArg.STRING, null, N_("Use specified profile"), N_("profile") },
            { "command", 'c', 0, OptionArg.STRING, null, N_("Run command on already opened panel"), N_("cmd") },
            { null }
        };
        private const GLib.ActionEntry[] app_entries =
        {
            {"preferences", activate_preferences, null, null, null},
            {"panel-preferences", activate_panel_preferences, "s", null, null},
            {"about", activate_about, null, null, null},
            {"menu", activate_menu, null, null, null},
            {"run", activate_run, null, null, null},
            {"logout", activate_logout, null, null, null},
            {"shutdown", activate_shutdown, null, null, null},
            {"restart", activate_restart, null, null, null},
            {"quit", activate_exit, null, null, null},
        };
        private const GLib.ActionEntry[] menu_entries =
        {
            {"launch-id",activate_menu_id , "s", null, null},
            {"launch-uri",activate_menu_uri, "s", null, null},
            {"launch-command",activate_menu_command , "s", null, null,},
        };
        public App()
        {
            Object(application_id: "org.valapanel.application",
#if VALA_0_26
                    flags: GLib.ApplicationFlags.HANDLES_COMMAND_LINE,
                    resource_base_path: "/org/vala-panel/app");
#else
                    flags: GLib.ApplicationFlags.HANDLES_COMMAND_LINE);
#endif
        }

        construct
        {
            add_main_option_entries(options);
            started = false;
        }
        private string system_config_file_name(string name1, string? name2)
        {
            return GLib.Path.build_filename(name1,GETTEXT_PACKAGE,_profile,name2);
        }
        private string user_config_file_name(string name1, string? name2)
        {
            return GLib.Path.build_filename(GLib.Environment.get_user_config_dir(),
                                GETTEXT_PACKAGE,_profile,name1,name2);
        }
        private void activate_preferences (SimpleAction act, Variant? param)
        {
            if(pref_dialog!=null)
            {
                pref_dialog.present();
                return;
            }
            var builder = new Builder.from_resource("/org/vala-panel/app/pref.ui");
            pref_dialog = builder.get_object("app-pref") as Dialog;
            this.add_window(pref_dialog);
            unowned Widget w = builder.get_object("logout") as Widget;
            config.bind(Key.LOGOUT,w,"text",SettingsBindFlags.DEFAULT);
            w = builder.get_object("shutdown") as Widget;
            config.bind(Key.SHUTDOWN,w,"text",SettingsBindFlags.DEFAULT);
            w = builder.get_object("css-chooser") as Widget;
            config.bind(Key.CUSTOM,w,"sensitive",SettingsBindFlags.DEFAULT);
            unowned FileChooserButton f = w as FileChooserButton;
            f.set_filename(css);
            f.file_set.connect((a)=>{
                var file = f.get_filename();
                if (file != null)
                    this.activate_action(Key.CSS,file);
            });
            pref_dialog.present();
            pref_dialog.hide.connect(()=>{pref_dialog.destroy();pref_dialog = null;});
            pref_dialog.response.connect_after(()=>{pref_dialog.destroy();pref_dialog = null;});
        }
        public void activate_panel_preferences(SimpleAction simple, Variant? param)
        {
            unowned Gtk.Application app = this;
            foreach(unowned Window win in app.get_windows())
            {
                if (win is Toplevel)
                {
                    unowned Toplevel p = win as Toplevel;
                    if (p.panel_name == param.get_string())
                    {
                        p.configure("position");
                        break;
                    }
                }
                stderr.printf("No panel with this name found.\n");
            }
        }
        public void activate_menu(SimpleAction simple, Variant? param)
        {
            unowned Gtk.Application app = this;
            foreach(unowned Window win in app.get_windows())
            {
                if (win is Toplevel)
                {
                    unowned Toplevel p = win as Toplevel;
                    foreach(unowned Widget pl in p.get_applets_list())
                    {
                        if (pl is AppletMenu)
                            (pl as AppletMenu).show_system_menu();
                    }
                }
            }
        }
        protected override void startup()
        {
            base.startup();
            this.mark_busy();
            GLib.Intl.setlocale(LocaleCategory.CTYPE,"");
            GLib.Intl.bindtextdomain(Config.GETTEXT_PACKAGE,Config.LOCALE_DIR);
            GLib.Intl.bind_textdomain_codeset(Config.GETTEXT_PACKAGE,"UTF-8");
            GLib.Intl.textdomain(Config.GETTEXT_PACKAGE);
            Gtk.IconTheme.get_default().append_search_path(Config.DATADIR+"/images");
            add_action_entries(app_entries,this);
            add_action_entries(menu_entries,this);
        }
        protected override void shutdown()
        {
            var list = new SList<Window>();
            foreach (var w in this.get_windows())
                list.append(w);
            foreach (var w in list)
            {
                w.application = null;
                w.destroy();
            }
            base.shutdown();
            if (restart)
            {
                char[] cwd = new char[1024];
                Posix.getcwd(cwd);
                var data = new SpawnData();
                string[] argv = {Config.GETTEXT_PACKAGE,"-p",this.profile};
                try
                {
                    Process.spawn_async((string)cwd,argv,
                                        Environ.get(),
                                        SpawnFlags.SEARCH_PATH,
                                        data.child_spawn_func,null);
                } catch (Error e){}
            }
        }

        protected override void activate()
        {
            if (!started)
            {
                ensure_user_config();
                load_settings();
                Gdk.get_default_root_window().set_events(Gdk.EventMask.STRUCTURE_MASK
                                                        |Gdk.EventMask.SUBSTRUCTURE_MASK
                                                        |Gdk.EventMask.PROPERTY_CHANGE_MASK);
                if (!start_all_panels())
                {
                    warning("Config files are not found\n");
                    this.quit();
                }
                else
                {
                    started = true;
                    apply_styling();
                    this.unmark_busy();
                }
            }
        }

        protected override int handle_local_options(VariantDict opts)
        {
            if (opts.contains("version"))
            {
                stdout.printf(_("%s - Version %s\n"),GLib.Environment.get_application_name(),
                                                    Config.VERSION);
                return 0;
            }
            return -1;
        }
        protected override int command_line(ApplicationCommandLine cmdl)
        {
            string? profile_name;
            string? command;
            var options = cmdl.get_options_dict();
            if (options.lookup("profile","s",out profile_name) && !this.started)
                profile = profile_name;
            if (options.lookup("command","s",out command))
            {
                string name;
                Variant param;
                try
                {
                    GLib.Action.parse_detailed_name(command,out name,out param);
                }
                catch (Error e)
                {
                    cmdl.printerr(_("%s: invalid command. Cannot parse."), command);
                }
                var action = lookup_action(name);
                if (action != null)
                    action.activate(param);
                else
                {
                    var actions = string.joinv(" ",list_actions());
                    cmdl.printerr(_("%s: invalid command - %s. Doing nothing.\nValid commands: %s\n"),
                                    GLib.Environment.get_application_name(),command,actions);
                }
            }
            activate();
            return 0;
        }
        private static void start_panels_from_dir(Gtk.Application app, string dirname)
        {
            Dir dir;
            try
            {
                dir = Dir.open(dirname,0);
            } catch (FileError e)
            {
                stdout.printf("Cannot load directory: %s\n",e.message);
                return;
            }
            string? name;
            while ((name = dir.read_name()) != null)
            {
                string cfg = GLib.Path.build_filename(dirname,name);
                if (!(cfg.contains("~") && cfg[0] !='.'))
                {
                    var panel = Toplevel.load(app,cfg,name);
                    if (panel != null)
                        app.add_window(panel);
                }
            }
        }
        private bool start_all_panels()
        {
            var panel_dir = user_config_file_name("panels",null);
            start_panels_from_dir((Gtk.Application)this,panel_dir);
            if (this.get_windows() != null)
                return true;
            unowned string[] dirs = GLib.Environment.get_system_config_dirs();
            if (dirs == null)
                return false;
            foreach(unowned string dir in dirs)
            {
                var sys_dir = system_config_file_name(dir,null);
                if(FileUtils.test(sys_dir,FileTest.EXISTS))
                {
                    try
                    {
                        Process.spawn_command_line_sync(("cp -r %s %s").printf(sys_dir,
                                                                        Path.build_filename(GLib.Environment.get_user_config_dir(),GETTEXT_PACKAGE)));
                        start_panels_from_dir((Gtk.Application)this,panel_dir);
                    } catch (Error e) {stderr.printf("%s\n",e.message);}
                    if (this.get_windows() != null)
                        return true;
                }
            }
            return (this.get_windows() != null);
        }

        private void ensure_user_config()
        {
            var dir = user_config_file_name("panels",null);
            GLib.DirUtils.create_with_parents(dir,0700);
        }

        private void apply_styling()
        {
            unowned Gtk.Settings gtksettings = Gtk.Settings.get_default();
            if (gtksettings != null && gtksettings.gtk_application_prefer_dark_theme != is_dark)
                gtksettings.gtk_application_prefer_dark_theme = is_dark;
            if (is_custom)
            {
                if (provider != null)
                    Gtk.StyleContext.remove_provider_for_screen(Gdk.Screen.get_default(),provider);
                provider = PanelCSS.apply_from_file_to_app_with_provider(css);
            }
            else if (provider != null)
            {
                Gtk.StyleContext.remove_provider_for_screen(Gdk.Screen.get_default(),provider);
                provider = null;
            }
        }
        private void load_settings()
        {
            unowned string[] dirs = GLib.Environment.get_system_config_dirs();
            var loaded = false;
            string? file = null;
            string? user_file = null;
            foreach (unowned string dir in dirs)
            {
                file = system_config_file_name(dir,"config");
                if (GLib.FileUtils.test(file,FileTest.EXISTS))
                {
                    loaded = true;
                    break;
                }
            }
            user_file = user_config_file_name("config",null);
            if (!GLib.FileUtils.test(user_file,FileTest.EXISTS) && loaded)
            {
                var src = File.new_for_path(file);
                var dest = File.new_for_path(user_file);
                try{src.copy(dest,FileCopyFlags.BACKUP,null,null);}
                catch(Error e){warning("Cannot init global config: %s\n",e.message);}
            }
            var config_backend = new GLib.KeyfileSettingsBackend(user_file,PATH,NAME);
            config = new GLib.Settings.with_backend_and_path(SCHEMA,config_backend,PATH);
            settings_bind(this,config,Key.RUN);
            settings_bind(this,config,Key.LOGOUT);
            settings_bind(this,config,Key.SHUTDOWN);
            settings_bind(this,config,Key.TERMINAL);
            settings_as_action(this,config,Key.DARK);
            settings_as_action(this,config,Key.CUSTOM);
            settings_as_action(this,config,Key.CSS);
        }
        internal void activate_about(SimpleAction action, Variant? param)
        {
            var builder = new Builder.from_resource("/org/vala-panel/app/about.ui");
            unowned AboutDialog d = builder.get_object("valapanel-about") as AboutDialog;
            d.set_version(Config.VERSION);
            d.window_position = Gtk.WindowPosition.CENTER;
            d.present();
            d.response.connect((id)=>{d.destroy();});
            d.hide.connect(()=>{d.destroy();});
        }
        internal void activate_run(SimpleAction action, Variant? param)
        {
            Variant variant = new Variant.string(run_command);
            MenuMaker.activate_menu_launch_command(null,variant,this);
        }
        internal void activate_logout(SimpleAction action, Variant? param)
        {
            Variant variant = new Variant.string(logout_command);
            MenuMaker.activate_menu_launch_command(null,variant,this);
        }
        internal void activate_shutdown(SimpleAction action, Variant? param)
        {
            Variant variant = new Variant.string(shutdown_command);
            MenuMaker.activate_menu_launch_command(null,variant,this);
        }
        internal void activate_exit(SimpleAction action, Variant? param)
        {
            this.restart = false;
            this.quit();
        }
        internal void activate_restart(SimpleAction action, Variant? param)
        {
            this.restart = true;
            this.quit();
        }
        internal void activate_menu_id(SimpleAction action, Variant? param)
        {
            MenuMaker.activate_menu_launch_id(action,param,this);
        }
        internal void activate_menu_uri(SimpleAction action, Variant? param)
        {
            MenuMaker.activate_menu_launch_uri(action,param,this);
        }
        internal void activate_menu_command(SimpleAction action, Variant? param)
        {
            MenuMaker.activate_menu_launch_command(action,param,this);
        }
    }
}
