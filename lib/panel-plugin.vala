using Gtk;
using Peas;

namespace ValaPanel
{
	namespace Data
	{
		public const string ONE_PER_SYSTEM = "ValaPanel-OnePerSystem";
		public const string CONFIG = "ValaPanel-Configurable";
		public const string EXPANDABLE = "ValaPanel-Expandable";
	}
	internal static const string PLUGINS_DIRECTORY = Config.PACKAGE_LIB_DIR+"/vala-panel/applets";
	internal static const string PLUGINS_DATA = Config.PACKAGE_DATA_DIR+"/applets";
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
	public interface AppletPluginActionable : AppletPlugin
	{
		public abstract void add_actions(Toplevel top);
		public abstract void remove_actions(Toplevel top);
	}
	public interface AppletConfigurable
	{
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
		internal void init_background()
		{
			Gdk.RGBA color = Gdk.RGBA();
			color.parse ("transparent");
			PanelCSS.apply_with_class(background_widget,
			                          PanelCSS.generate_background(null,color),
			                          "-vala-panel-background",
			                          true);
		}
		public void set_popup_position(Gtk.Widget popup)
		{
			int x,y;
			toplevel.popup_position_helper(this,popup,out x, out y);
			popup.get_window().move(x,y);
		}
		private void activate_configure(SimpleAction act, Variant? param)
		{
			show_config_dialog();
		}
		internal void show_config_dialog()
		{
			if (dialog != null)
		    {
				int x,y;
				var dlg = (this as AppletConfigurable).get_config_dialog();
				this.destroy.connect(()=>{dlg.response(Gtk.ResponseType.CLOSE);});
				/* adjust config dialog window position to be near plugin */
				dlg.set_transient_for(toplevel);
				toplevel.popup_position_helper(this,dlg,out x, out y);
				dlg.move(x,y);
				dialog = dlg;
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
