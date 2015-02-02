using Gtk;
using Peas;

namespace ValaPanel
{
	internal static const string PLUGINS_DIRECTORY = Config.PACKAGE_LIB_DIR+"/vala-panel/applets";
	internal static const string PLUGINS_DATA = Config.PACKAGE_DATA_DIR+"/applets";
	[Flags]
	public enum Features
	{
		CONFIG,
		MENU,
		ONE_PER_SYSTEM,
		EXPAND_AVAILABLE,
		CONTEXT_MENU
	}
	public enum PluginPackType
	{
		START,
		CENTER,
		END
	}
	public enum PluginAction
	{
		MENU
	}

	public interface AppletPlugin : Peas.ExtensionBase
	{
		public abstract ValaPanel.Applet get_applet_widget(ValaPanel.Toplevel toplevel,
		                                                   GLib.Settings? settings,
		                                                   uint number);
		public abstract Features features
		{get;}
	}


	[CCode (cname = "PanelApplet")]
	public abstract class Applet : Gtk.EventBox
	{
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
		public virtual Gtk.Window? get_config_dialog()
		{
			return null;
		}
		public virtual void invoke_action(PluginAction action){}
		public virtual void update_context_menu(ref GLib.Menu parent_menu){}
		public Applet(ValaPanel.Toplevel top, GLib.Settings? s, uint num)
		{
			Object(toplevel: top, settings: s, number: num);
		}
		construct
		{
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
	}
}
