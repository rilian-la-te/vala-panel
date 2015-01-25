using Gtk;
using Peas;

private class PanelToplevel : Gtk.Bin
{		
}

namespace ValaPanel
{
	namespace Key 
	{
		internal static const string EDGE = "edge";
	}
	[Flags]
	public enum AppearanceHints
	{
		GNOME,
		BACKGROUND_COLOR,
		FOREGROUND_COLOR,
		BACKGROUND_IMAGE,
		CORNERS,
		FONT,
		FONT_SIZE_ONLY
	}
	public class Toplevel : Gtk.ApplicationWindow
	{
		private AppearanceHints hints;
		private Gtk.PositionType _edge;
		private ToplevelSettings settings;
		private PanelToplevel dummy;
		private Gtk.Box _box;
		internal Gtk.Box box
		{
			get {return _box;}
		}
		public Gtk.PositionType edge {
			get {return _edge;}
			set {
				_edge = edge;
			}
		}
		public bool use_gnome_theme
		{ get {return AppearanceHints.GNOME in hints;}
		  set {
			  hints = (use_gnome_theme == true) ?
				  hints | AppearanceHints.GNOME :
				  hints & (~AppearanceHints.GNOME);
			  update_background();
		  }
		}


#if HAVE_GTK313
		protected override Gtk.WidgetPath get_path_for_child(Gtk.Widget child)
		{
			Gtk.WidgetPath path = base.get_path_for_child(child);
#else
		protected override unowned Gtk.WidgetPath get_path_for_child(Gtk.Widget child)
		{
			unowned Gtk.WidgetPath path = base.get_path_for_child(child);
#endif
			if (use_gnome_theme)
			{
				path.iter_set_object_type(0, typeof(PanelToplevel));
				for (int i=0; i<path.length(); i++)
				{
					if (path.iter_get_object_type(i) == typeof(Applet))
					{
						path.iter_set_object_type(i, typeof(PanelApplet));
						break;
					}
				}
			}
		return path;
		}
		
		public void popup_position_helper(Gtk.Widget near, Gtk.Widget popup,
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
			near.get_allocation(out a);
			near.get_window().get_origin(out x, out y);
			if (near.get_has_window())
			{
				x += a.x;
				y += a.y;
			}
			switch (edge)
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
			if (near.has_screen())
				screen = near.get_screen();
			else
				screen = Gdk.Screen.get_default();
			var monitor = screen.get_monitor_at_point(x,y);
			a = (Gtk.Allocation)screen.get_monitor_workarea(monitor);
			x.clamp(a.x,a.x + a.width - pa.width);
			y.clamp(a.y,a.y + a.height - pa.height);
		}

		private void on_extension_added(Peas.PluginInfo i, Object p)
		{
			var pl = p as ValaPanel.Plugin;
			var type = i.get_external_data("Type");
			add_to_panel(type, pl);
		}
		internal void add_to_panel(string type, Plugin pl)
		{
			var s = settings.add_plugin_settings (type);
			place_applet (pl,s);
		}
		
		internal void place_applet(Plugin pl, PluginSettings s)
		{
			var f = pl.get_features();
			s.init_configuration(settings,((f & Features.CONFIG) != 0));
			var applet = pl.get_applet_widget(this,s.config_settings);
			bool expand = false;
			if ((f & Features.EXPAND_AVAILABLE) != 0)
				expand = s.default_settings.get_boolean(Key.EXPAND);
			_box.pack_start(applet,expand, true, 0);
			s.default_settings.bind(Key.BORDER,applet,"border-width",GLib.SettingsBindFlags.GET);
			s.default_settings.bind(Key.POSITION,applet,"position",GLib.SettingsBindFlags.GET);
			s.default_settings.bind(Key.PADDING,applet,"padding",GLib.SettingsBindFlags.GET);
			if ((f & Features.EXPAND_AVAILABLE)!=0)
				s.default_settings.bind(Key.EXPAND,applet,"expand",GLib.SettingsBindFlags.DEFAULT);
		}
		public Gtk.Menu get_plugin_menu(Applet pl)
		{
			return new Gtk.Menu();
		}
		
		private void update_background()
		{
			
		}
	}
}