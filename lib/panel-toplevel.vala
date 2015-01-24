using Gtk;

//			settings.default_settings.bind(Key.BORDER,this,"border-width",GLib.SettingsBindFlags.GET);
//			settings.default_settings.bind(Key.POSITION,this,"position",GLib.SettingsBindFlags.GET);
//			settings.default_settings.bind(Key.PADDING,this,"padding",GLib.SettingsBindFlags.GET);
//			if (_f & EXPAND_AVAILABLE)
//				settings.default_settings.bind(Key.EXPAND,this,"expand",GLib.SettingsBindFlags.DEFAULT);
private class PanelToplevel : Gtk.Bin
{		
}
private class PanelApplet : Gtk.Bin
{		
}

namespace ValaPanel
{
	public class Toplevel : Gtk.ApplicationWindow
	{
		private Gtk.PositionType _edge;
		public Gtk.PositionType edge {
			get {return _edge;}
			set {
				_edge = edge;
			}
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
		public Gtk.Menu get_plugin_menu(Applet pl)
		{
			return new Gtk.Menu();
		}
	}
}