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
    [GtkTemplate (ui = "/org/vala-panel/app/app-runner.ui"),CCode (cname="Runner")]
    internal class Runner : Gtk.Dialog
    {
        [GtkChild (name="main-entry")]
        private Entry main_entry;
        [GtkChild (name="terminal-button")]
        private ToggleButton terminal_button;
        [GtkChild (name="main-box", internal=true)]
        private Box main_box;
        private AsyncTask task;
        private Cancellable cancellable;
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
            cancellable.cancel();
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
        }
        /* Async matcher API */
        private static void create_file_list(Task task, Object source, void* data, Cancellable cancellable)
        {
            SList<string> list = new SList<string>();
            var dirs = GLib.Environment.get_variable("PATH").split(":",0);
            foreach(unowned string dir in dirs)
            {
                if (cancellable.is_cancelled())
                    return;
                try
                {
                    Dir gdir = Dir.open(dir,0);
                    string? name = null;
                    while(!cancellable.is_cancelled() && (name = gdir.read_name())!= null)
                    {
                        var filename = GLib.Path.build_filename(dir,name);
                        if (GLib.FileUtils.test(filename,FileTest.IS_EXECUTABLE))
                        {
                            if (cancellable.is_cancelled())
                                return;
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
            task.return_pointer((owned)list,(a)=>{
                slist_free_full((SList)a,free);
            });
            return;
        }
        private void setup_entry_completion()
        {
            if (cached)
            {
                //FIXME: Implement cache;
            }
            else
            {
                cancellable = new Cancellable();
                task = AsyncTask.create(this,cancellable,(obj,res)=>{
                    try
                    {
                        void* pointer = task.propagate_pointer();
                        unowned SList<string> filenames = (SList)pointer;
                        unowned EntryCompletion comp = main_entry.get_completion();
                        unowned Gtk.ListStore store = comp.get_model() as Gtk.ListStore;
                        store.clear();
                        foreach(unowned string filename in filenames)
                            store.insert_with_values(null,-1,0,filename,-1);
                        comp.complete();
                        slist_free_full(filenames,free);
                    } catch (Error e)
                    {
                        stderr.printf("%s\n",e.message);
                    }
                });
                task.run_in_thread(create_file_list);
            }
        }
        /* --------------------------------------------------*/
        private DesktopAppInfo? match_app_by_exec(string exec)
        {
            DesktopAppInfo? ret = null;
            string exec_path = GLib.Environment.find_program_in_path(exec);
            if (exec_path == null)
                return null;
            foreach(unowned AppInfo app in AppInfo.get_all())
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
                        ret = match_app_by_exec(basename);
                }
            }
            return ret;
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
                                                             SpawnFlags.SEARCH_PATH | SpawnFlags.DO_NOT_REAP_CHILD,
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
            cancellable.cancel();
            this.destroy();
        }
        public void gtk_run()
        {
            this.setup_entry_completion();
            this.show_all();
            main_entry.grab_focus();
            this.present_with_time(Gtk.get_current_event_time());
        }
        [GtkCallback]
        private void on_entry_changed()
        {
            DesktopAppInfo? app = null;
            if (main_entry.get_text()!=null && main_entry.get_text() != "")
                app = match_app_by_exec(main_entry.get_text());
            if (app != null)
                main_entry.set_icon_from_gicon(Gtk.EntryIconPosition.PRIMARY,app.get_icon());
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
