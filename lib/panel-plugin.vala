using Gtk;
using Peas;
using Config;

namespace ValaPanel
{
	namespace Data
	{
		public const string ONE_PER_SYSTEM = "ValaPanel-OnePerSystem";
		public const string CONFIG = "ValaPanel-Configurable";
		public const string EXPANDABLE = "ValaPanel-Expandable";
	}
	public enum AppletPackType
	{
		START = 0,
		CENTER = 2,
		END = 1
	}
	public interface AppletPlugin : Peas.ExtensionBase
	{
		public abstract ValaPanel.Applet get_applet_widget(ValaPanel.Toplevel toplevel,
		                                                   GLib.Settings? settings,
		                                                   uint number);
	}
	public interface AppletMenu
	{
		public abstract void show_system_menu();
	}
	public interface AppletConfigurable
	{
		[CCode (returns_floating_reference = true)]	
		public abstract Gtk.Dialog get_config_dialog();
	}
	[CCode (cname = "PanelApplet")]
	public abstract class Applet : Gtk.EventBox
	{
		private Dialog? dialog;
		static const GLib.ActionEntry[] config_entry =
		{
			{"configure",activate_configure,null,null,null}
		};
		static const GLib.ActionEntry[] remove_entry =
		{
			{"remove",activate_remove,null,null,null}
		};
		public Gtk.Widget background_widget
		{
			get; set;
		}
		public ValaPanel.Toplevel toplevel
		{
			get; construct;
		}
		public unowned GLib.Settings? settings
		{
			get; construct;
		}
		public uint number
		{
			internal get; construct;
		}
		public abstract void create();
		public virtual void update_context_menu(ref GLib.Menu parent_menu){}
		public Applet(ValaPanel.Toplevel top, GLib.Settings? s, uint num)
		{
			Object(toplevel: top, settings: s, number: num);
		}
		construct
		{
			SimpleActionGroup grp = new SimpleActionGroup();
			this.set_has_window(false);
			this.create();
			if (background_widget == null)
				background_widget = this;
			init_background();
			this.button_press_event.connect((b)=>
			{
				if (b.button == 3 &&
				    ((b.state & Gtk.accelerator_get_default_mod_mask ()) == 0))
				{
					toplevel.get_plugin_menu(this).popup(null,null,null,
					                                      b.button,b.time);
					return true;
				}
				return false;
			});
			if (this is AppletConfigurable)
				grp.add_action_entries(config_entry,this);
			grp.add_action_entries(remove_entry,this);
			this.insert_action_group("applet",grp);
		}
		public void init_background()
		{
			Gdk.RGBA color = Gdk.RGBA();
			color.parse ("transparent");
			PanelCSS.apply_with_class(background_widget,
			                          PanelCSS.generate_background(null,color),
			                          "-vala-panel-background",
			                          true);
		}
		public void popup_position_helper(Gtk.Widget popup,
		                                  out int x, out int y)
		{
			Gtk.Allocation pa;
			Gtk.Allocation a;
			Gdk.Screen screen;
			popup.realize();
			popup.get_allocation(out pa);
			if (popup.is_toplevel())
			{
				Gdk.Rectangle ext;
				popup.get_window().get_frame_extents(out ext);
				pa.width = ext.width;
				pa.height = ext.height;
			}
			get_allocation(out a);
			get_window().get_origin(out x, out y);
			if (get_has_window())
			{
				x += a.x;
				y += a.y;
			}
			switch (toplevel.edge)
			{
				case Gtk.PositionType.TOP:
					y+=a.height;
					break;
				case Gtk.PositionType.BOTTOM:
					y-=pa.height;
					break;
				case Gtk.PositionType.LEFT:
					x+=a.width;
					break;
				case Gtk.PositionType.RIGHT:
					x-=pa.width;
					break;
			}
			if (has_screen())
				screen = get_screen();
			else
				screen = Gdk.Screen.get_default();
			var monitor = screen.get_monitor_at_point(x,y);
			a = (Gtk.Allocation)screen.get_monitor_workarea(monitor);
			x.clamp(a.x,a.x + a.width - pa.width);
			y.clamp(a.y,a.y + a.height - pa.height);
		}
		public void set_popup_position(Gtk.Widget popup)
		{
			int x,y;
			popup_position_helper(popup,out x, out y);
			popup.get_window().move(x,y);
		}
		private void activate_configure(SimpleAction act, Variant? param)
		{
			show_config_dialog();
		}
		internal void show_config_dialog()
		{
			if (dialog == null)
		    {
				int x,y;
				var dlg = (this as AppletConfigurable).get_config_dialog();
				this.destroy.connect(()=>{dlg.response(Gtk.ResponseType.CLOSE);});
				/* adjust config dialog window position to be near plugin */
				dlg.set_transient_for(toplevel);
				popup_position_helper(dlg,out x, out y);
				dlg.move(x,y);
				dialog = dlg;
				dialog.unmap.connect(()=>{dialog.destroy(); dialog = null;});
			}
			dialog.present();
		}
		private void activate_remove(SimpleAction act, Variant? param)
		{		
		    /* If the configuration dialog is open, there will certainly be a crash if the
		     * user manipulates the Configured Plugins list, after we remove this entry.
		     * Close the configuration dialog if it is open. */
		    if (toplevel.pref_dialog != null)
		    {
		        toplevel.pref_dialog.destroy();
		        toplevel.pref_dialog = null;
		    }
		    toplevel.remove_applet(this);
		}
	}
}
