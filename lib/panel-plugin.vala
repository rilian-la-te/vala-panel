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
		public abstract Features features
		{get;}
	}

	public abstract class Applet : Gtk.EventBox
	{
		private PanelApplet applet;
		public abstract Features features
		{
			construct;
			get;
		}
		public Gtk.Widget background_widget
		{
			public get; private set;
		}
		public ValaPanel.Toplevel toplevel
		{
			public get; private construct;
		}
		public GLib.Settings settings
		{
			public get; private construct;
		}
		public abstract void create (ValaPanel.Toplevel toplevel, GLib.Settings settings);
		public abstract Gtk.Window get_config_dialog();
		public abstract void invoke_action(PluginAction action);
		public abstract void update_context_menu(ref GLib.Menu parent_menu);
		public Applet(ValaPanel.Toplevel top, GLib.Settings s)
		{
			Object(toplevel: top, settings: s);
		}
		construct
		{
			this.set_has_window(false);
			applet = new PanelApplet();
			this.create(toplevel,this.settings);
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
			PanelCSS.apply_with_class(this,
			                          PanelCSS.generate_background(null,color),
			                          "-vala-panel-background",
			                          true);
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
