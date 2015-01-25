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
		internal static const string WIDTH = "width";
	}
	[Flags]
	internal enum AppearanceHints
	{
		GNOME,
		BACKGROUND_COLOR,
		FOREGROUND_COLOR,
		BACKGROUND_IMAGE,
		CORNERS,
		FONT,
		FONT_SIZE_ONLY
	}
	[Flags]
	internal enum GeometryHints
	{
		AUTOHIDE,
		SHOW_HIDDEN,
		ABOVE,
		BELOW,
		STRUT,
		DOCK,
		SHADOW,
		ATTACHED,
		DYNAMIC		
	}
	internal enum IconSizeHints
	{
		XXS = 16,
		XS = 22,
		S = 24,
		M = 32,
		L = 48,
		XL = 96,
		XXL = 128,
		XXXL = 256;
	}
	public class Toplevel : Gtk.ApplicationWindow
	{
		private ToplevelSettings settings;
		private PanelToplevel dummy;
		private Gtk.Box box;

		private GeometryHints ghints;
		private int height;
		private int width;
		private Gdk.Rectangle a;
		private Gdk.Rectangle c;

		private AppearanceHints ahints;
		private Gdk.RGBA bgc;
		private Gdk.RGBA fgc;
		private Gtk.CssProvider provider;
		private IconSizeHints ihints;

		internal Gtk.Dialog plugin_pref_dialog;
		internal Gtk.Dialog pref_dialog;


		private uint ah_visible;
		private uint ah_far;
		private uint ah_state;
		private uint mouse_timeout;
		private uint hide_timeout;

		private ulong strut_size;
		private ulong strut_lower;
		private ulong strut_upper;
		private int strut_edge;
		
		private bool initialized;

		private string profile
		{ get {
			string profile;
			GLib.Value v = Value(typeof(string));
			this.get_application().get_property("profile",ref v);
			return v.get_string();
			}
		}
		
		public Gtk.PositionType edge { get; set;}
		public bool use_gnome_theme
		{ get {return AppearanceHints.GNOME in ahints;}
		  set {
			  ahints = (use_gnome_theme == true) ?
				  ahints | AppearanceHints.GNOME :
				  ahints & (~AppearanceHints.GNOME);
			  update_background();
		  }
		}
		public Gtk.Orientation orientation
		{
			get {
				return (_edge == Gtk.PositionType.TOP || _edge == Gtk.PositionType.BOTTOM)
					? Gtk.Orientation.HORIZONTAL : Gtk.Orientation.VERTICAL;
			}
		}
		public int monitor
		{get; set;}
		internal string background
		{owned get {return bgc.to_string();}
		 set {bgc.parse(background);}
		}
		internal string foreground
		{owned get {return fgc.to_string();}
		 set {fgc.parse(foreground);}
		}
		public uint icon_size
		{ get {return (uint) ihints;}
		  set {
			if (icon_size >= (uint)IconSizeHints.XXXL)
				ihints = IconSizeHints.XXL;
			else if (icon_size >= (uint)IconSizeHints.XXL)
				ihints = IconSizeHints.XXL;
			else if (icon_size >= (uint)IconSizeHints.XL)
				ihints = IconSizeHints.XL;
			else if (icon_size >= (uint)IconSizeHints.L)
				ihints = IconSizeHints.L;
			else if (icon_size >= (uint)IconSizeHints.M)
				ihints = IconSizeHints.M;
			else if (icon_size >= (uint)IconSizeHints.S)
				ihints = IconSizeHints.S;
			else if (icon_size >= (uint)IconSizeHints.XS)
				ihints = IconSizeHints.XS;
			else ihints = IconSizeHints.XXS;
		  }
		}
		public string background_file
		{get; set;}

		public static Toplevel allocate(Gtk.Application app)
		{
			Object o =  Object.new(typeof(Toplevel),
			            "border-width", 0,
                        "decorated", false,
                        "name", "ValaPanel",
                        "resizable", false,
                        "title", "ValaPanel",
                        "type-hint", Gdk.WindowTypeHint.DOCK,
                        "window-position", Gtk.WindowPosition.NONE,
                        "skip-taskbar-hint", true,
                        "skip-pager-hint", true,
                        "accept-focus", false,
                        "application", app);
			return (o as Toplevel);
		}

		internal Toplevel()
		{
			Gdk.Visual visual = this.get_screen().get_rgba_visual();
			if (visual != null)
				this.set_visual(visual);
			dummy = new PanelToplevel();
			
			this.destroy.connect((a)=>
			{
				stop_ui ();
			});
		}
		
		private void stop_ui()
		{
			if (pref_dialog != null)
				pref_dialog.response(Gtk.ResponseType.CLOSE);
			if (plugin_pref_dialog != null)
				plugin_pref_dialog.response(Gtk.ResponseType.CLOSE);
			if (initialized)
			{
				this.get_application().remove_window(this);
				Gdk.flush();
				initialized = false;
			}
			if (this.get_child()!=null)
			{
				box.destroy();
				box = null;
			}
		}

/*
 * Position calculating.
 */
		internal override void size_allocate(Gtk.Allocation a)
		{
			int x,y,w;
			if ((ghints & GeometryHints.DYNAMIC) > 0 && box != null)
			{
				if (orientation == Gtk.Orientation.HORIZONTAL)
					box.get_preferred_width(null, out w);
				else
					box.get_preferred_height(null, out w);
				if (w!=width)
						settings.settings.set_int(Key.WIDTH,w);
			}
			if (!this.get_realized())
				return;
			this.get_window().get_origin(out x, out y);
			_calculate_position (out a);
			this.a.x = a.x;
			this.a.y = a.y;
			if (a.width != this.a.width || a.height != this.a.height || this.a.x != x || this.a.y != y)
			{
				this.a.width = a.width;
				this.a.height = a.height;
				this.set_size_request(this.a.width, this.a.height);
				this.move(this.a.x, this.a.y);
				
			}
			if (this.get_mapped())
				establish_autohide ();
		}

		private void _calculate_position(out Gtk.Allocation a)
		{
			var screen = this.get_screen();
			Gdk.Rectangle marea = Gdk.Rectangle();
			if (monitor < 0)
			{
				marea.x = 0;
				marea.y = 0;
				marea.width = screen.get_width();
				marea.height = screen.get_height();
			}
			else if (monitor < screen.get_n_monitors())
				screen.get_monitor_geometry(monitor,out marea);
			if (orientation == Gtk.Orientation.HORIZONTAL)
			{
				a.width = width;
			}
		}

/*
* Gnome Panel hack.
*/
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



/* 
 * Plugins stuff.
 */

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
			box.pack_start(applet,expand, true, 0);
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

		private void set_strut()
		{
		}
		private void establish_autohide()
		{
		}
		private void update_background()
		{
			
		}
	}
}