using Gtk;
using Peas;

private class PanelApplet : Gtk.Bin
{		
}

namespace ValaPanel
{
	[Flags]
	public enum Features
	{
		CONFIG,
		MENU,
		ONE_PER_SYSTEM,
		EXPAND_AVAILABLE,
		CONTEXT_MENU
	}
	public enum PluginAction
	{
		MENU
	}

	public interface Plugin : Peas.ExtensionBase
	{
		public abstract ValaPanel.Applet get_applet_widget(ValaPanel.Toplevel toplevel,
		                                                   GLib.Settings settings);
		public abstract Features get_features();
	}
	
	public abstract class Applet : Gtk.Bin
	{
		private GLib.Settings _settings;
		private ValaPanel.Toplevel _toplevel;
		private Gtk.Widget back;
		private PanelApplet applet;
		internal uint number;
		public abstract Features features
		{
			construct;
			get;
		}
		public Gtk.Widget background_widget
		{
			get {return back;}
		}
		public ValaPanel.Toplevel toplevel
		{
			get {return _toplevel;}
		}
		public GLib.Settings settings
		{
			get {return _settings;}
		}
		public abstract void constructor (ValaPanel.Toplevel toplevel, GLib.Settings settings);
		public abstract Gtk.Window get_config_dialog();
		public abstract void invoke_action(PluginAction action);
		public abstract void update_context_menu(ref GLib.Menu parent_menu);
		internal Applet(ValaPanel.Toplevel toplevel, ValaPanel.PluginSettings settings)
		{
			this._toplevel = toplevel;
			this.number = settings.number;
			this._settings = settings.config_settings;
			applet = new PanelApplet();
			this.constructor(toplevel,this.settings);
			if (back == null)
				back = this;
			init_background();
			this.button_press_event.connect((b)=>
			{
				if (b.button == 3 &&
				    ((b.state & Gtk.accelerator_get_default_mod_mask ()) == 0))
				{
					_toplevel.get_plugin_menu(this).popup(null,null,null,
					                                      b.button,b.time);
					return true;
				}
				return false;
			});
		}
		public void init_background()
		{
			Gdk.RGBA color = Gdk.RGBA();
			color.parse ("transparent");
			PanelCSS.apply_with_class(this,
			                          PanelCSS.generate_background(null,color),
			                          "-vala-panel-background",
			                          true);
			PanelCSS.apply_with_class(back,
			                          PanelCSS.generate_background(null,color),
			                          "-vala-panel-background",
			                          true);
		}
		public void set_popup_position(Gtk.Widget popup)
		{
			int x,y;
			_toplevel.popup_position_helper(this,popup,out x, out y);
			popup.get_window().move(x,y);
		}
	}
}