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

using Gtk;
using GLib;

namespace ValaPanel
{
    [Compact, CCode (ref_function = "vala_panel_completion_thread_ref", unref_function = "vala_panel_completion_thread_unref")]
    internal class CompletionThread
    {
        public bool running;
        internal unowned Gtk.Entry entry;
        internal SList<string>? filenames;
        internal Volatile ref_count;
        public CompletionThread (Gtk.Entry entry)
        {
            ref_count = 1;
            this.entry = entry;
            running = true;
        }

        public void* run ()
        {
            SList<string> list = new SList<string>();
            var dirs = GLib.Environment.get_variable("PATH").split(":",0);
            foreach(var dir in dirs)
            {
                if (!running)
                    break;
                try
                {
                    Dir gdir = Dir.open(dir,0);
                    string? name = null;
                    while(running && (name = gdir.read_name())!= null)
                    {
                        var filename = GLib.Path.build_filename(dir,name);
                        if (GLib.FileUtils.test(filename,FileTest.IS_EXECUTABLE))
                        {
                            if (!running)
                                break;
                            if (list.find_custom(name,(GLib.CompareFunc)Posix.strcmp) == null)
                                list.append(name);
                        }
                    }

                }
                catch (Error e)
                {
                    stderr.printf("%s\n",e.message);
                    continue;
                }
            }
            filenames = (owned)list;
            Idle.add(() =>
            {
                if(running)
                    setup_entry_completion();
                return false;
            });
            return null;
        }
        private void setup_entry_completion()
        {
            var comp = new EntryCompletion();
            comp.set_minimum_key_length(2);
            comp.set_inline_completion(true);
            comp.set_popup_set_width(true);
            comp.set_popup_single_match(false);
            var store = new Gtk.ListStore(1,typeof(string));
            foreach(var filename in filenames)
            {
                var it = Gtk.TreeIter();
                store.append(out it);
                store.set(it,0,filename,-1);
            }
            comp.set_model(store);
            comp.set_text_column(0);
            entry.set_completion(comp);
            comp.complete();
        }
        public unowned CompletionThread @ref ()
        {
            GLib.AtomicInt.inc (ref this.ref_count);
            return this;
        }
        public void unref ()
        {
            if (GLib.AtomicInt.dec_and_test (ref this.ref_count))
                this.free ();
        }
        private extern void free ();
    }
    [GtkTemplate (ui = "/org/vala-panel/app/app-runner.ui"),CCode (cname="Runner")]
    internal class Runner : Gtk.Dialog
    {
        [GtkChild (name="main-entry")]
        private Entry main_entry;
        [GtkChild (name="terminal-button")]
        private ToggleButton terminal_button;
        [GtkChild (name="main-box", internal=true)]
        private Box main_box;

        private CompletionThread? thread;
        private Thread<void*> thread_ref;

        private GLib.List<GLib.AppInfo> apps_list;
        private AppInfoMonitor monitor;
        private bool cached;
        private App app
        {
            get{return this.get_application() as App;}
            set{set_application(value as Gtk.Application);}
        }

        public Runner(App app)
        {
            Object(application: app);
        }
        ~Runner()
        {
            thread.running = false;
            this.application = null;
        }
        construct
        {
            PanelCSS.apply_from_resource(this,"/org/vala-panel/app/style.css","-panel-run-dialog");
            main_box.get_style_context().add_class("-panel-run-header");
            //FIXME: Implement cache
            cached = false;
            this.set_visual(this.get_screen().get_rgba_visual());
            this.set_default_response(Gtk.ResponseType.OK);
            this.set_keep_above(true);
            thread = null;
            monitor = AppInfoMonitor.get();
            apps_list = AppInfo.get_all();
            monitor.changed.connect(()=>
            {
                apps_list = AppInfo.get_all();
            });
        }

        private unowned DesktopAppInfo? match_app_by_exec(string exec)
        {
            unowned DesktopAppInfo? ret = null;
            string exec_path = GLib.Environment.find_program_in_path(exec);
            if (exec_path == null)
                return null;
            foreach(unowned AppInfo app in apps_list)
            {
                var app_exec = app.get_executable();
                if (app_exec == null)
                    continue;

                var len = exec.length;
                var pexec = exec;
                if (GLib.Path.is_absolute(app_exec))
                {
                    pexec = exec_path;
                    len = exec_path.length;
                }
                if (app_exec == pexec)
                {
                    ret = app as DesktopAppInfo;
                    break;
                }
            }

            if (GLib.FileUtils.test(exec_path,GLib.FileTest.IS_SYMLINK))
            {
                char[] arr = new char[1024];
                var len = Posix.readlink(exec_path, arr);
                arr[len] = '\0';
                unowned string target = (string)arr;
                ret = match_app_by_exec(target);
                if (ret == null)
                {
                    var basename = GLib.Path.get_basename(target);
                    var locate = GLib.Environment.find_program_in_path(basename);
                    if (locate != null && locate == target)
                    {
                        ret = match_app_by_exec(basename);
                    }
                }
            }
            return ret;
        }

        private void setup_entry_completion()
        {
            if (cached)
            {
                //FIXME: Implement cache;
            }
            else
            {
                thread = new CompletionThread(main_entry);
                thread_ref = new Thread<void*>("Autocompletion",thread.run);
            }
        }
        protected override void response(int id)
        {
            if (id == Gtk.ResponseType.OK)
            {
//              FIXME: For Vala Panel (after merge with Vala toplevel)
//              var str = terminal_button.active ?
//                          App.terminal + main_entry.get_text()
//                          : main_entry.get_text();
//
                var str = main_entry.get_text();
                try
                {
                    var info  = AppInfo.create_from_commandline(str,null,
                    terminal_button.active ? AppInfoCreateFlags.NEEDS_TERMINAL : 0) as DesktopAppInfo;
                    var data = new MenuMaker.SpawnData();
                    var launch = info.launch_uris_as_manager(null,
                                                             this.get_display().get_app_launch_context(),
                                                             SpawnFlags.SEARCH_PATH,
                                                             data.child_spawn_func,MenuMaker.launch_callback);
                    if (!launch)
                    {
                        Signal.stop_emission_by_name(this,"response");
                        return;
                    }
                } catch (Error e)
                {
                    Signal.stop_emission_by_name(this,"response");
                    return;
                }
            }
            thread.running = false;
            this.destroy();
        }
        public void gtk_run()
        {
            this.setup_entry_completion();
            this.show_all();
            main_entry.grab_focus();
            main_box.set_orientation(Gtk.Orientation.HORIZONTAL);
            this.present_with_time(Gtk.get_current_event_time());
        }
        [GtkCallback]
        private void on_close_button_clicked()
        {
            this.response(ResponseType.CLOSE);
        }
        [GtkCallback]
        private void on_entry_changed()
        {
            DesktopAppInfo? app = null;
            if (main_entry.get_text()!=null && main_entry.get_text() != "")
            {
                app = match_app_by_exec(main_entry.get_text());
            }
            if (app != null)
            {
                main_entry.set_icon_from_gicon(Gtk.EntryIconPosition.PRIMARY,app.get_icon());
            }
            else
                main_entry.set_icon_from_icon_name(Gtk.EntryIconPosition.PRIMARY,"system-run-symbolic");
        }
        [GtkCallback]
        private void on_entry_activated()
        {
            this.response(Gtk.ResponseType.OK);
        }
        [GtkCallback]
        private void on_icon_activated(Gtk.EntryIconPosition pos, Gdk.Event e)
        {
            if (pos == Gtk.EntryIconPosition.SECONDARY && e.button.button == 1)
                this.response(Gtk.ResponseType.OK);
        }
    }
}
