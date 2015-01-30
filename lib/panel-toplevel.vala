private class PanelToplevel : Gtk.Bin
{
}

namespace ValaPanel
{
	namespace Key
	{
		internal static const string EDGE = "edge";
		internal static const string WIDTH = "width";
		internal static const string ICON_SIZE = "icon-size"
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
		DYNAMIC
	}
	internal enum AlignmentType
	{
		START,
		CENTER,
		END
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
		private int h;
		private int width;
		private AlignmentType align;
		private int margin;
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

		private int height
		{
			{ get {
				return ((ghints & GeometryHints.SHADOW) > 0) ? h+5 : h;
			}
			set {h=height}}
		}
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

		[CCode (returns_floating_reference = true)]
		public static Toplevel? load(Gtk.Application app, string config_file, string config_name)
		{

		}
		public Toplevel(Gtk.Application app)
		{
			Object o =  Object(
			            border-width: 0,
                        decorated: false,
                        name: "ValaPanel",
                        resizable: false,
                        title: "ValaPanel",
                        type-hint: Gdk.WindowTypeHint.DOCK,
                        window-position: Gtk.WindowPosition.NONE,
                        skip-taskbar-hint: true,
                        skip-pager-hint: true,
                        accept-focus: false,
                        application: app);
		}
		construct
		{
			Gdk.Visual visual = this.get_screen().get_rgba_visual();
			if (visual != null)
				this.set_visual(visual);
			dummy = new PanelToplevel();
			this.destroy.connect((a)=>{stop_ui ();});
			a = Gdk.Rectangle();
			c = Gdk.Rectangle();
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
		protected override void size_allocate(Gtk.Allocation a)
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

		private void _calculate_position(ref Gtk.Allocation a)
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
				a.x = marea.x;
				calculate_width(marea.width,align,margin,ref a.width, ref a.x);
				a.height = (!autohide || ah_visible) ? height :
										(ghints & GeometryHints.SHOW_HIDDEN > 0) ? 1 : 0;
				a.x = marea.x + ((edge == Gtk.PositionType.TOP) ? 0 : marea.width - a.width);
			}
			else
			{
				a.height = width;
				a.y = marea.y;
				calculate_width(marea.height,align,margin,ref a.height, ref a.y);
				a.width = (!autohide || ah_visible) ? height :
										(ghints & GeometryHints.SHOW_HIDDEN > 0) ? 1 : 0;
				a.y = marea.y + ((edge == Gtk.PositionType.TOP) ? 0 : marea.height - a.height);
			}
		}

		private void calculate_position()
		{
			_calculate_position(ref (this.a as Gtk.Allocation));
		}

		private static void calculate_width(int scrw, AlignmentType align, int margin,
											int margin, ref int panw, ref int x)
		{
			panw = (panw >= 100) ? 100 : (panw <= 1) ? 1 : panw;
			panw = (int)(((double)scrw * (double) panw)/100.0);
			margin = (align != AlignmentType.CENTER && margin > srcw) ? 0 : margin;
			panw = int.min(scrw - margin, panw);
			if (align = AlignmentType.LEFT)
				x+=margin;
			else if (align = AlignmentType.RIGHT)
			{
				x += scrw - panw - margin;
				x = (x < 0) ? 0 : x;
			}
			else if (align = AlignmentType.CENTER)
				x += (scrw - panw)/2;
		}

		protected override void get_preferred_width(out int min, out int nat)
		{
			Gtk.Requisition req = Gtk.Requisition();
			this.get_preferred_size(out req, null);
			min = nat = req.width;
		}
		protected override void get_preferred_height(out int min, out int nat)
		{
			Gtk.Requisition req = Gtk.Requisition();
			this.get_preferred_size(out req, null);
			min = nat = req.height;
		}
		protected override void get_preferred_size (out Gtk.Requisition min,
													out Gtk.Requisition nat)
		{
			if (!ah_visible)
				box.get_preferred_size(out min, out nat);
			var rect = Gdk.Rectangle();
			rect.width = min.width;
			rect.height = min.height;
			_calculate_position(ref rect as Gtk.Allocation);
			min.width = rect.width;
			min.height = rect.height;
			nat = min;
		}
/*
 * Autohide stuff
 */
		protected override bool configure_event(Gdk.EventConfigure evt)
		{
			c.width = e.width;
			c.height = e.height;
			c.x = e.x;
			c.y = e.y;
		}

		protected override bool map_event(Gdk.EventAny e)
		{
			if (ghints & GeometryHints.AUTOHIDE > 0)
				ah_start();
		}

		private void establish_autohide()
		{

		}
		private void ah_start()
		{

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
 * Menus stuff
 */

		protected override bool button_press_event(Gdk.EventButton e)
		{
			if (e.button == 3)
			{
				var menu = get_plugin_menu(null);
				menu.popup(null,null,null,e.button,e.time);
				return true;
			}
			return false;
		}

		public Gtk.Menu get_plugin_menu(Applet? pl)
		{
			return new Gtk.Menu();
		}
/*
 * Plugins stuff.
 */

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

		private void set_strut()
		{
		}
		private void update_background()
		{

		}
	}
}
