using GLib;
using Gtk;

private class PanelToplevel : Gtk.Bin
{
}
namespace ValaPanel
{
	namespace Key
	{
		internal static const string EDGE = "edge";
		internal static const string ALIGNMENT = "alignment";
		internal static const string HEIGHT = "height";
		internal static const string WIDTH = "width";
		internal static const string DYNAMIC = "is-dynamic";
		internal static const string AUTOHIDE = "autohide";
		internal static const string SHOW_HIDDEN = "show-hidden";
		internal static const string STRUT = "strut";
		internal static const string DOCK = "dock";
		internal static const string MONITOR = "monitor";
		internal static const string MARGIN = "panel-margin";
		internal static const string ICON_SIZE = "icon-size";
		internal static const string BACKGROUND_COLOR = "background-color";
		internal static const string FOREGROUND_COLOR = "foreground-color";
		internal static const string BACKGROUND_FILE = "background-file";
		internal static const string FONT_SIZE = "font-size";
		internal static const string FONT = "font";
		internal static const string CORNERS_SIZE = "round-corners-size";
		internal static const string USE_BACKGROUND_COLOR = "use-background-color";
		internal static const string USE_FOREGROUND_COLOR = "use-foreground-color";
		internal static const string USE_FONT = "use-font";
		internal static const string FONT_SIZE_ONLY = "font-size-only";
		internal static const string USE_BACKGROUND_FILE = "use-background-file";
		internal static const string USE_GNOME = "use-gnome-theme";

	}
	internal enum AlignmentType
	{
		START = 0,
		CENTER = 1,
		END = 2
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
	internal enum AutohideState
	{
		VISIBLE,
		HIDDEN,
		WAITING
	}
	public class Toplevel : Gtk.ApplicationWindow, Gtk.Orientable
	{
		private static Peas.Engine engine;
		private static ulong mon_handler;
		internal static HashTable<string,string> loaded_types;
		private ToplevelSettings settings;
		private PanelToplevel dummy;
		private Gtk.Box box;
		private int _mon;
		private int _w;
		private Gtk.Allocation a;
		private Gdk.Rectangle c;

		private IconSizeHints ihints;
		private Gdk.RGBA bgc;
		private Gdk.RGBA fgc;
		private Gtk.CssProvider provider;

		internal Gtk.Dialog plugin_pref_dialog;
		internal Gtk.Dialog pref_dialog;


		private bool ah_visible;
		private bool ah_far;
		private AutohideState ah_state;
		private uint mouse_timeout;
		private uint hide_timeout;

		private ulong strut_size;
		private ulong strut_lower;
		private ulong strut_upper;
		private int strut_edge;

		private bool initialized;

		private static const string[] gnames = {Key.WIDTH,Key.HEIGHT,Key.EDGE,Key.ALIGNMENT,
												Key.MONITOR,Key.AUTOHIDE,Key.SHOW_HIDDEN,
												Key.MARGIN,Key.DOCK,Key.STRUT,
												Key.DYNAMIC};
		private static const string[] anames = {Key.USE_GNOME,Key.BACKGROUND_COLOR,Key.FOREGROUND_COLOR,
												Key.CORNERS_SIZE, Key.BACKGROUND_FILE,
												Key.USE_BACKGROUND_COLOR, Key.USE_FOREGROUND_COLOR,
												Key.USE_BACKGROUND_FILE, Key.USE_FONT,
												Key.FONT_SIZE_ONLY, Key.FONT, Key.FONT_SIZE};

		public string panel_name
		{get; private construct;}

		private string profile
		{ get {
			GLib.Value v = Value(typeof(string));
			this.get_application().get_property("profile",ref v);
			return v.get_string();
			}
		}
		internal int height
		{ get; set;}
		internal int width
		{get {return _w;}
		 set {_w = (value > 0) ? ((value <=100) ? value : 100) : 1;}
		}
		private AlignmentType alignment
		{get; set;}
		internal int panel_margin
		{get; set;}
		public Gtk.PositionType edge
		{get; set;}
		public Gtk.Orientation orientation
		{
			get {
				return (_edge == Gtk.PositionType.TOP || _edge == Gtk.PositionType.BOTTOM)
					? Gtk.Orientation.HORIZONTAL : Gtk.Orientation.VERTICAL;
			}
		}
		public int monitor
		{get {return _mon;}
		 set {
			int mons = 1;
			var screen = Gdk.Screen.get_default();
			if (screen != null)
				mons = screen.get_n_monitors();
			assert(mons >= 1);
			if (-1 <= value)
				_mon = value;
		 }}
		public bool dock
		{get; set;}
		public bool strut
		{get; set;}
		public bool autohide
		{get; set;}
		public bool show_hidden
		{get; set;}
		public bool is_dynamic
		{get; set;}
		public bool use_gnome_theme
		{get; set;}
		internal string background_color
		{owned get {return bgc.to_string();}
		 set {bgc.parse(value);}
		}
		internal string foreground_color
		{owned get {return fgc.to_string();}
		 set {fgc.parse(value);}
		}
		public uint icon_size
		{ get {return (uint) ihints;}
		  set {
			if (value >= (uint)IconSizeHints.XXXL)
				ihints = IconSizeHints.XXL;
			else if (value >= (uint)IconSizeHints.XXL)
				ihints = IconSizeHints.XXL;
			else if (value >= (uint)IconSizeHints.XL)
				ihints = IconSizeHints.XL;
			else if (value >= (uint)IconSizeHints.L)
				ihints = IconSizeHints.L;
			else if (value >= (uint)IconSizeHints.M)
				ihints = IconSizeHints.M;
			else if (value >= (uint)IconSizeHints.S)
				ihints = IconSizeHints.S;
			else if (value >= (uint)IconSizeHints.XS)
				ihints = IconSizeHints.XS;
			else ihints = IconSizeHints.XXS;
		  }
		}
		public string background_file
		{get; set;}
		static const GLib.ActionEntry[] panel_entries =
		{
			{"new-panel", activate_new_panel, null, null, null},
			{"remove-panel", activate_remove_panel, null, null, null},
			{"panel-settings", activate_panel_settings, "s", null, null},
		};
/*
 *  Constructors
 */
		static construct
		{
			engine = Peas.Engine.get_default();
			loaded_types = new HashTable<string,string>(str_hash,str_equal);
		}
		private static void monitors_changed_cb(Gdk.Screen scr, void* data)
		{
			var app = data as Gtk.Application;
			var mons = Gdk.Screen.get_default().get_n_monitors();
			foreach(var w in app.get_windows())
			{
				var panel = w as Toplevel;
				if (panel.monitor < mons && !panel.initialized)
					panel.start_ui();
				else if (panel.monitor >=mons && panel.initialized)
					panel.stop_ui();
				else
				{
					panel.ah_state_set(AutohideState.VISIBLE);
					panel.queue_resize();
				}
			}
		}
		[CCode (returns_floating_reference = true)]
		public static Toplevel? load(Gtk.Application app, string config_file, string config_name)
		{
			if (GLib.FileUtils.test(config_file,FileTest.EXISTS))
				return new Toplevel(app,config_name);
			return null;
		}
		public Toplevel (Gtk.Application app, string name)
		{
			Object(border_width: 0,
				decorated: false,
				name: "ValaPanel",
				resizable: false,
				title: "ValaPanel",
				type_hint: Gdk.WindowTypeHint.DOCK,
				window_position: Gtk.WindowPosition.NONE,
				skip_taskbar_hint: true,
				skip_pager_hint: true,
				accept_focus: false,
				application: app,
				panel_name: name);
		}
		construct
		{
			Gdk.Visual visual = this.get_screen().get_rgba_visual();
			if (visual != null)
				this.set_visual(visual);
			dummy = new PanelToplevel();
			this.destroy.connect((a)=>{stop_ui ();});
			a = Gtk.Allocation();
			c = Gdk.Rectangle();
			this.notify.connect((s,p)=> {
				if (p.name in gnames)
				{
					this.queue_draw();
					this.update_strut();
				}
				if (p.name in anames)
					this.update_appearance();
				if (p.name == Key.EDGE)
					box.set_orientation(orientation);
			});
			var filename = user_config_file_name("panels",profile,panel_name);
			settings = new ToplevelSettings(filename);
			this.add_action_entries(panel_entries,this);
			settings_as_action(this,settings.settings,Key.EDGE);
			settings_as_action(this,settings.settings,Key.ALIGNMENT);
			settings_as_action(this,settings.settings,Key.HEIGHT);
			settings_as_action(this,settings.settings,Key.WIDTH);
			settings_as_action(this,settings.settings,Key.DYNAMIC);
			settings_as_action(this,settings.settings,Key.AUTOHIDE);
			settings_as_action(this,settings.settings,Key.STRUT);
			settings_as_action(this,settings.settings,Key.DOCK);
			settings_as_action(this,settings.settings,Key.MARGIN);
			settings_bind(this,settings.settings,Key.MONITOR);
			settings_as_action(this,settings.settings,Key.SHOW_HIDDEN);
			settings_as_action(this,settings.settings,Key.ICON_SIZE);
			settings_as_action(this,settings.settings,Key.BACKGROUND_COLOR);
			settings_as_action(this,settings.settings,Key.FOREGROUND_COLOR);
			settings_as_action(this,settings.settings,Key.BACKGROUND_FILE);
			settings_as_action(this,settings.settings,Key.FONT);
			settings_as_action(this,settings.settings,Key.FONT_SIZE);
			settings_as_action(this,settings.settings,Key.FONT_SIZE_ONLY);
			settings_as_action(this,settings.settings,Key.CORNERS_SIZE);
			settings_as_action(this,settings.settings,Key.USE_GNOME);
			settings_as_action(this,settings.settings,Key.USE_BACKGROUND_COLOR);
			settings_as_action(this,settings.settings,Key.USE_FOREGROUND_COLOR);
			settings_as_action(this,settings.settings,Key.USE_FONT);
			settings_as_action(this,settings.settings,Key.USE_BACKGROUND_FILE);
			if (monitor < Gdk.Screen.get_default().get_n_monitors())
				start_ui();
			var app = get_application();
			if (mon_handler != 0)
				mon_handler = Signal.connect(Gdk.Screen.get_default(),"monitors-changed",
											(GLib.Callback)(monitors_changed_cb),app);
		}
/*
 * Common UI functions
 */
		private void stop_ui()
		{
			if (pref_dialog != null)
				pref_dialog.response(Gtk.ResponseType.CLOSE);
			if (plugin_pref_dialog != null)
				plugin_pref_dialog.response(Gtk.ResponseType.CLOSE);
			if (initialized)
			{
				Gdk.flush();
				initialized = false;
			}
			if (this.get_child()!=null)
			{
				box.destroy();
				box = null;
			}
			this.unmap();
			this.unrealize();
		}

		private void start_ui()
		{
			a.x = a.y = a.width = a.height = 0;
			set_wmclass("panel","vala-panel");
			this.get_application().add_window(this);
			this.add_events(Gdk.EventMask.BUTTON_PRESS_MASK);
			this.realize();
			box = new Box(this.orientation,0);
			box.set_baseline_position(Gtk.BaselinePosition.CENTER);
			box.set_border_width(0);
			this.add(box);
			box.show();
			this.set_type_hint((dock)? Gdk.WindowTypeHint.DOCK : Gdk.WindowTypeHint.NORMAL);
			settings.init_plugin_list();
			foreach(var pl in settings.plugins)
			{
				load_applet(pl);
			}
			update_applet_positions();
			this.present();
			this.autohide = autohide;
			initialized = true;
		}

/*
 * Position calculating.
 */
		protected override void size_allocate(Gtk.Allocation a)
		{
			int x,y,w;
			base.size_allocate(a);
			if (is_dynamic && box != null)
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
			_calculate_position (ref a);
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
				calculate_width(marea.width,alignment,panel_margin,ref a.width, ref a.x);
				a.height = (!autohide || ah_visible) ? height :
										show_hidden ? 1 : 0;
				a.x = marea.x + ((edge == Gtk.PositionType.TOP) ? 0 : marea.width - a.width);
			}
			else
			{
				a.height = width;
				a.y = marea.y;
				calculate_width(marea.height,alignment,panel_margin,ref a.height, ref a.y);
				a.width = (!autohide || ah_visible) ? height :
										show_hidden ? 1 : 0;
				a.y = marea.y + ((edge == Gtk.PositionType.TOP) ? 0 : marea.height - a.height);
			}
		}

		private void calculate_position()
		{
			_calculate_position(ref a);
		}

		private static void calculate_width(int scrw, AlignmentType align, int margin,
											ref int panw, ref int x)
		{
			panw = (panw >= 100) ? 100 : (panw <= 1) ? 1 : panw;
			panw = (int)(((double)scrw * (double) panw)/100.0);
			margin = (align != AlignmentType.CENTER && margin > scrw) ? 0 : margin;
			panw = int.min(scrw - margin, panw);
			if (align == AlignmentType.START)
				x+=margin;
			else if (align == AlignmentType.END)
			{
				x += scrw - panw - margin;
				x = (x < 0) ? 0 : x;
			}
			else if (align == AlignmentType.CENTER)
				x += (scrw - panw)/2;
		}

		protected override void get_preferred_width(out int min, out int nat)
		{
			base.get_preferred_width_internal(out min, out nat);
			Gtk.Requisition req = Gtk.Requisition();
			this.get_panel_preferred_size(ref req);
			min = nat = req.width;
		}
		protected override void get_preferred_height(out int min, out int nat)
		{
			base.get_preferred_height_internal(out min, out nat);
			Gtk.Requisition req = Gtk.Requisition();
			this.get_panel_preferred_size(ref req);
			min = nat = req.height;
		}
		protected void get_panel_preferred_size (ref Gtk.Requisition min)
		{
			if (!ah_visible)
				box.get_preferred_size(out min, null);
			var rect = Gtk.Allocation();
			rect.width = min.width;
			rect.height = min.height;
			_calculate_position(ref rect);
			min.width = rect.width;
			min.height = rect.height;
		}
/*
 * Autohide stuff
 */
 		private static const int PERIOD = 200;
		private static const int GAP = 2;
		protected override bool configure_event(Gdk.EventConfigure e)
		{
			c.width = e.width;
			c.height = e.height;
			c.x = e.x;
			c.y = e.y;
			return base.configure_event (e);
		}

		protected override bool map_event(Gdk.EventAny e)
		{
			if (autohide)
				ah_start();
			return base.map_event(e);
		}

		private void establish_autohide()
		{
			if (autohide)
				ah_start();
			else
			{
				ah_stop();
				ah_state_set(AutohideState.VISIBLE);
			}
		}
		private void ah_start()
		{
			if (mouse_timeout == 0)
				Timeout.add(PERIOD,mouse_watch);
		}
		private void ah_stop()
		{
			if (mouse_timeout != 0)
			{
				Source.remove(mouse_timeout);
				mouse_timeout = 0;
			}
			if (hide_timeout != 0)
			{
				Source.remove(hide_timeout);
				hide_timeout = 0;
			}
		}
		private void ah_state_set(AutohideState new_state)
		{

		}
		private bool mouse_watch()
		{
			int x, y;
			if (MainContext.current_source().is_destroyed())
				return false;

			var manager = Gdk.Display.get_default().get_device_manager();
			var dev = manager.get_client_pointer();
			dev.get_position(null, out x, out y);

			return true;
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
			var type = i.get_module_name();
			add_to_panel(type, pl);
		}
		internal void add_to_panel(string type, Plugin pl)
		{
			var s = settings.add_plugin_settings (type);
			place_applet (pl,s);
		}

		internal void load_applet(PluginSettings s)
		{

		}

		internal void place_applet(Plugin pl, PluginSettings s)
		{
			var f = pl.features;
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
		private void update_applet_positions()
		{

		}

		private void update_strut()
		{

		}
		private void update_appearance()
		{

		}
/*
 * Actions stuff
 */
		public void activate_new_panel(SimpleAction act, Variant? param)
		{

		}
		public void activate_remove_panel(SimpleAction act, Variant? param)
		{

		}
		public void activate_panel_settings(SimpleAction act, Variant? param)
		{

		}
	}
}
