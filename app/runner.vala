using Gtk;
using GLib;

namespace ValaPanel
{
	internal class CompletionThread
	{
		public bool running;
		private unowned Gtk.Entry entry;
		private SList<string>? filenames;
		public CompletionThread (Gtk.Entry entry) {
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
	}
	[GtkTemplate (ui = "/org/vala-panel/app/app-runner.ui")]
	[CCode (cname="Runner")]
	internal class Runner : Gtk.Dialog
	{
		[GtkChild (name="main-entry")]
		private Entry main_entry;
		[GtkChild (name="terminal-button")]
		private ToggleButton terminal_button;
		[GtkChild (name="main-box", internal=true)]
		private Box main_box;
		
		private CompletionThread? thread;
		
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
		
		private DesktopAppInfo? match_app_by_exec(string exec)
		{
			DesktopAppInfo? ret = null;
			string exec_path = GLib.Environment.find_program_in_path(exec);
			if (exec_path == null)
				return null;
			foreach(var app in apps_list)
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
				new Thread<void*>("Autocompletion",thread.run); 	
			}
		}
		public override void response(int id)
		{
			if (id == Gtk.ResponseType.OK)
			{
//              FIXME: For Vala Panel (after merge with Vala toplevel)
//				var str = terminal_button.active ?
//							App.terminal + main_entry.get_text()
//							: main_entry.get_text();	
//
				var str = main_entry.get_text();
				try
				{
					var info  = AppInfo.create_from_commandline(str,null,
					terminal_button.active ? AppInfoCreateFlags.NEEDS_TERMINAL : 0);
					var launch = info.launch(null,Gdk.Display.get_default().get_app_launch_context());
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
			this.show_all();
			this.setup_entry_completion();
			this.show_all();
			this.show();
			main_entry.grab_focus();
			main_box.set_orientation(Gtk.Orientation.HORIZONTAL);
			this.present();
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
