using GLib;
using Gtk;
using Gdk;
using Config;

namespace ValaPanel
{
	namespace Key
	{
		public static const string EDGE = "edge";
		public static const string ALIGNMENT = "alignment";
		public static const string HEIGHT = "height";
		public static const string WIDTH = "width";
		public static const string DYNAMIC = "is-dynamic";
		public static const string AUTOHIDE = "autohide";
		public static const string SHOW_HIDDEN = "show-hidden";
		public static const string STRUT = "strut";
		public static const string DOCK = "dock";
		public static const string MONITOR = "monitor";
		public static const string MARGIN = "panel-margin";
		public static const string ICON_SIZE = "icon-size";
		public static const string BACKGROUND_COLOR = "background-color";
		public static const string FOREGROUND_COLOR = "foreground-color";
		public static const string BACKGROUND_FILE = "background-file";
		public static const string FONT = "font";
		public static const string CORNERS_SIZE = "round-corners-size";
		public static const string USE_BACKGROUND_COLOR = "use-background-color";
		public static const string USE_FOREGROUND_COLOR = "use-foreground-color";
		public static const string USE_FONT = "use-font";
		public static const string FONT_SIZE_ONLY = "font-size-only";
		public static const string USE_BACKGROUND_FILE = "use-background-file";
	}
	public enum AlignmentType
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
	private struct PluginData
	{
		AppletPlugin plugin;
		uint count;
	}
	[CCode (cname = "PanelToplevel")]
	public class Toplevel : Gtk.ApplicationWindow
	{
		private static Peas.Engine engine;
		private static ulong mon_handler;
		private static HashTable<string,PluginData?> loaded_types;
		private HashTable<string,uint> local_applets;
		private Peas.ExtensionSet extset;
		private ToplevelSettings settings;
		private Gtk.Box box;
		private int _mon;
		private int _w;
		private Gtk.Allocation a;
		private Gdk.Rectangle c;

		private IconSizeHints ihints;
		private Gdk.RGBA bgc;
		private Gdk.RGBA fgc;
		private Gtk.CssProvider provider;

		internal ConfigureDialog pref_dialog;

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

		public static const string[] gnames = {Key.WIDTH,Key.HEIGHT,Key.EDGE,Key.ALIGNMENT,
												Key.MONITOR,Key.AUTOHIDE,Key.SHOW_HIDDEN,
												Key.MARGIN,Key.DOCK,Key.STRUT,
												Key.DYNAMIC};
		public static const string[] anames = {Key.BACKGROUND_COLOR,Key.FOREGROUND_COLOR,
												Key.CORNERS_SIZE, Key.BACKGROUND_FILE,
												Key.USE_BACKGROUND_COLOR, Key.USE_FOREGROUND_COLOR,
												Key.USE_BACKGROUND_FILE, Key.USE_FONT,
												Key.FONT_SIZE_ONLY, Key.FONT};

		public string panel_name
		{get; internal construct;}

		private string profile
		{ owned get {
			GLib.Value v = Value(typeof(string));
			this.get_application().get_property("profile",ref v);
			return v.get_string();
			}
		}
		public int height
		{ get; internal set;}
		public int width
		{get {return _w;}
		 internal set {_w = (value > 0) ? ((value <=100) ? value : 100) : 1;}
		}
		public AlignmentType alignment
		{get; internal set;}
		public int panel_margin
		{get; internal set;}
		public Gtk.PositionType edge
		{get; internal set construct;}
		public Gtk.Orientation orientation
		{
			get {
				return (_edge == Gtk.PositionType.TOP || _edge == Gtk.PositionType.BOTTOM)
					? Gtk.Orientation.HORIZONTAL : Gtk.Orientation.VERTICAL;
			}
		}
		public int monitor
		{get {return _mon;}
		 internal set construct{
			int mons = 1;
			var screen = Gdk.Screen.get_default();
			if (screen != null)
				mons = screen.get_n_monitors();
			assert(mons >= 1);
			if (-1 <= value)
				_mon = value;
		 }}
		public bool dock
		{get; internal set;}
		public bool strut
		{get; internal set;}
		public bool autohide
		{get; internal set;}
		public bool show_hidden
		{get; internal set;}
		public bool is_dynamic
		{get; internal set;}
		public bool use_font
		{get; internal set;}
		public bool use_background_color
		{get; internal set;}
		public bool use_foreground_color
		{get; internal set;}
		public bool use_background_file
		{get; internal set;}		
		public bool font_size_only
		{get; internal set;}
		public uint font_size
		{get; internal set;}
		public uint round_corners_size
		{get; internal set;}
		public string font
		{get; internal set;}
		public string background_color
		{owned get {return bgc.to_string();}
		 internal set {bgc.parse(value);}
		}
		public string foreground_color
		{owned get {return fgc.to_string();}
		 internal set {fgc.parse(value);}
		}
		public uint icon_size
		{ get {return (uint) ihints;}
		  internal set {
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
		{get; internal set;}
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
			engine.add_search_path(PLUGINS_DIRECTORY,PLUGINS_DATA);
			loaded_types = new HashTable<string,PluginData?>(str_hash,str_equal);
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
			stderr.printf("Cannot find config file %s\n",config_file);
			return null;
		}
		[CCode (returns_floating_reference = true)]
		public static Toplevel create(Gtk.Application app, string name, int mon, PositionType e)
		{
			return new Toplevel.from_position(app,name,mon,e);
		}
/*
 * Big constructor
 */
		private Toplevel (Gtk.Application app, string name)
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
				setup(false);
		}
		private Toplevel.from_position(Gtk.Application app, string name, int mon, PositionType e)
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
			monitor = mon;
			this.edge = e;
			setup(true);
		}
		private void setup(bool use_internal_values)
		{
			var filename = user_config_file_name("panels",profile,panel_name);
			settings = new ToplevelSettings(filename);
			if (use_internal_values)
			{
				settings.settings.set_int(Key.MONITOR, _mon);
				settings.settings.set_enum(Key.EDGE, edge);
			}
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
			settings_as_action(this,settings.settings,Key.CORNERS_SIZE);
			settings_as_action(this,settings.settings,Key.FONT_SIZE_ONLY);
			settings_as_action(this,settings.settings,Key.USE_BACKGROUND_COLOR);
			settings_as_action(this,settings.settings,Key.USE_FOREGROUND_COLOR);
			settings_as_action(this,settings.settings,Key.USE_FONT);
			settings_as_action(this,settings.settings,Key.USE_BACKGROUND_FILE);
			if (monitor < Gdk.Screen.get_default().get_n_monitors())
				start_ui();
			var panel_app = get_application();
			if (mon_handler != 0)
				mon_handler = Signal.connect(Gdk.Screen.get_default(),"monitors-changed",
											(GLib.Callback)(monitors_changed_cb),panel_app);
		}
		construct
		{
			extset = new Peas.ExtensionSet(engine,typeof(AppletPlugin));
			local_applets = new HashTable<string,uint>(str_hash,str_equal);
			Gdk.Visual visual = this.get_screen().get_rgba_visual();
			if (visual != null)
				this.set_visual(visual);
			this.destroy.connect((a)=>{stop_ui ();});
			a = Gtk.Allocation();
			c = Gdk.Rectangle();
			this.notify.connect((s,p)=> {
				if (p.name == Key.EDGE)
					if (box != null) box.set_orientation(orientation);
				if (p.name == Key.AUTOHIDE)
					ah_visible = (autohide)? false : true;
				if (p.name in gnames)
				{
					this.queue_resize();
					this.update_strut();
				}
				if (p.name in anames)
					this.update_appearance();
			});
			this.add_action_entries(panel_entries,this);
			this.extset.extension_added.connect(on_extension_added);
			engine.load_plugin.connect_after((i)=>
			{
				var ext = extset.get_extension(i);
				on_extension_added(i,ext);
			});
		}
/*
 * Common UI functions
 */
		private void stop_ui()
		{
			if (autohide)
				ah_stop();
			if (pref_dialog != null)
				pref_dialog.response(Gtk.ResponseType.CLOSE);
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
			box.set_hexpand(true);
			box.set_vexpand(true);
			this.add(box);
			box.show();
			this.set_type_hint((dock)? Gdk.WindowTypeHint.DOCK : Gdk.WindowTypeHint.NORMAL);
			settings.init_plugin_list();
			this.show();
			this.stick();
			foreach(var pl in settings.plugins)
				load_applet(pl);
			update_applet_positions();
			this.present();
			this.autohide = autohide;
			initialized = true;
		}

/*
 * Position calculating.
 */
		protected override void size_allocate(Gtk.Allocation alloc)
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
			get_window().get_origin(out x, out y);
			_calculate_position (ref alloc);
			this.a.x = alloc.x;
			this.a.y = alloc.y;
			if (alloc.width != this.a.width || alloc.height != this.a.height || this.a.x != x || this.a.y != y)
			{
				this.a.width = alloc.width;
				this.a.height = alloc.height;
				this.set_size_request(this.a.width, this.a.height);
				this.move(this.a.x, this.a.y);
				this.update_strut();
			}
			if (this.get_mapped())
				establish_autohide();
		}

		private void _calculate_position(ref Gtk.Allocation alloc)
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
			{
				screen.get_monitor_geometry(monitor,out marea);
//~ 				marea = screen.get_monitor_workarea(monitor);
//~ 				var hmod = (autohide && show_hidden) ? 1 : height;
//~ 				switch (edge)
//~ 				{
//~ 					case PositionType.TOP:
//~ 						marea.x -= hmod;
//~ 						marea.height += hmod;
//~ 						break;
//~ 					case PositionType.BOTTOM:
//~ 						marea.height += hmod;
//~ 						break;
//~ 					case PositionType.LEFT:
//~ 						marea.y -= hmod;
//~ 						marea.width += hmod;
//~ 						break;
//~ 					case PositionType.RIGHT:
//~ 						marea.width += hmod;
//~ 						break;
//~ 				}
			}
			if (orientation == Gtk.Orientation.HORIZONTAL)
			{
				alloc.width = width;
				alloc.x = marea.x;
				calculate_width(marea.width,is_dynamic,alignment,panel_margin,ref alloc.width, ref alloc.x);
				alloc.height = (!autohide || ah_visible) ? height :
										show_hidden ? 1 : 0;
				alloc.y = marea.y + ((edge == Gtk.PositionType.TOP) ? 0 : (marea.height - alloc.height));
			}
			else
			{
				alloc.height = width;
				alloc.y = marea.y;
				calculate_width(marea.height,is_dynamic,alignment,panel_margin,ref alloc.height, ref alloc.y);
				alloc.width = (!autohide || ah_visible) ? height :
										show_hidden ? 1 : 0;
				alloc.x = marea.x + ((edge == Gtk.PositionType.LEFT) ? 0 : (marea.width - alloc.width));
			}
		}

		private static void calculate_width(int scrw, bool dyn, AlignmentType align,
											int margin,	ref int panw, ref int x)
		{
			if (!dyn)
			{
				panw = (panw >= 100) ? 100 : (panw <= 1) ? 1 : panw;
				panw = (int)(((double)scrw * (double) panw)/100.0);
			}
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
		private void get_panel_preferred_size (ref Gtk.Requisition min)
		{
			if (!ah_visible && box != null)
				box.get_preferred_size(out min, null);
			var rect = Gtk.Allocation();
			rect.width = min.width;
			rect.height = min.height;
			_calculate_position(ref rect);
			min.width = rect.width;
			min.height = rect.height;
		}
/****************************************************
 *         autohide : borrowed from fbpanel         *
 ****************************************************/

/* Autohide is behaviour when panel hides itself when mouse is "far enough"
 * and pops up again when mouse comes "close enough".
 * Formally, it's a state machine with 3 states that driven by mouse
 * coordinates and timer:
 * 1. VISIBLE - ensures that panel is visible. When/if mouse goes "far enough"
 *      switches to WAITING state
 * 2. WAITING - starts timer. If mouse comes "close enough", stops timer and
 *      switches to VISIBLE.  If timer expires, switches to HIDDEN
 * 3. HIDDEN - hides panel. When mouse comes "close enough" switches to VISIBLE
 *
 * Note 1
 * Mouse coordinates are queried every PERIOD milisec
 *
 * Note 2
 * If mouse is less then GAP pixels to panel it's considered to be close,
 * otherwise it's far
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
			var rect = Gtk.Allocation();
			if (this.ah_state != new_state)
			{
				ah_state = new_state;
				switch (new_state)
				{
					case AutohideState.VISIBLE:
						this.ah_visible = true;
						_calculate_position(ref rect);
						this.move(rect.x,rect.y);
						this.show();
						box.show();
						this.stick();
						this.queue_resize();
						break;
					case AutohideState.WAITING:
						if (hide_timeout > 0) Source.remove(hide_timeout);
						hide_timeout = Timeout.add(2*PERIOD, ah_state_hide_timeout);
						break;
					case AutohideState.HIDDEN:
						if (show_hidden)
						{
							box.hide();
							this.queue_resize();
						}
						else
							this.hide();
						this.ah_visible = false;
						break;
				}
			}
			else if (autohide && ah_far)
			{
				switch(new_state)
				{
					case AutohideState.VISIBLE:
						ah_state_set(AutohideState.WAITING);
						break;
					case AutohideState.WAITING:
						break;
					case AutohideState.HIDDEN:
						if (show_hidden && box.get_visible())
						{
							box.hide();
							this.show();
						}
						else if (this.get_visible()&& !show_hidden)
						{
							this.hide();
							box.show();
						}
						this.ah_visible = false;
						break;
				}
			}
			else
			{
				switch (new_state)
				{
					case AutohideState.VISIBLE:
						break;
					case AutohideState.WAITING:
						if (hide_timeout > 0) Source.remove(hide_timeout);
						hide_timeout = 0;
						ah_state_set(AutohideState.VISIBLE);
						break;
					case AutohideState.HIDDEN:
						ah_state_set(AutohideState.VISIBLE);
						break;
				}
			}
		}
		private bool ah_state_hide_timeout()
		{
			if (!MainContext.current_source().is_destroyed())
			{
				ah_state_set(AutohideState.HIDDEN);
				hide_timeout = 0;
			}
			return false;
		}
		private bool mouse_watch()
		{
			int x, y;
			if (MainContext.current_source().is_destroyed())
				return false;

			var manager = Gdk.Display.get_default().get_device_manager();
			var dev = manager.get_client_pointer();
			dev.get_position(null, out x, out y);

			var cx = a.x;
			var cy = a.y;
			var cw = (c.width != 1) ? c.width : 0;
			var ch = (c.height != 1) ? c.height : 0;
			if (ah_state == AutohideState.HIDDEN)
			{
				switch (edge)
				{
					case Gtk.PositionType.LEFT:
						cw = GAP;
						break;
					case Gtk.PositionType.RIGHT:
						cx += (cw-GAP);
						cw = GAP;
						break;
					case Gtk.PositionType.TOP:
						ch = GAP;
						break;
					case Gtk.PositionType.BOTTOM:
						cy += (ch-GAP);
						ch = GAP;
						break;
				}
			}
			ah_far = ((x<cx)||(x>cx+cw)||(y<cy)||(y>cy+ch));
			ah_state_set(this.ah_state);
			return true;
		}
/* end of the autohide code
 * ------------------------------------------------------------- */
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

		internal Gtk.Menu get_plugin_menu(Applet? pl)
		{
		    var builder = new Builder.from_resource("/org/vala-panel/lib/menus.ui");
		    var gmenu = builder.get_object("panel-context-menu") as GLib.Menu;
		    if (pl != null)
		    {
		        var gmenusection = builder.get_object("plugin-section") as GLib.Menu;
		        pl.update_context_menu(ref gmenusection);
		    }
		    var ret = new Gtk.Menu.from_model(gmenu as MenuModel);
		    if (pl != null)
		        ret.attach_to_widget(pl,null);
		    else
		        ret.attach_to_widget(this,null);
		    ret.show_all();
		    return ret;
		}
/*
 * Plugins stuff.
 */
		internal void add_applet(string type)
		{
			PluginSettings s = settings.add_plugin_settings(type);
			s.default_settings.set_string(Key.NAME,type);
			load_applet(s);
		}
		internal void load_applet(PluginSettings s)
		{
			/* Determine if the plugin is loaded yet. */
			string name = s.default_settings.get_string(Key.NAME);
			if (loaded_types.contains(name))
			{
				PluginData? data = loaded_types.lookup(name);
				if (data!=null)
				{
					place_applet(data.plugin,s);
					data.count +=1;
					loaded_types.insert(name,data);
					return;
				}
			}
			// Got this far we actually need to load the underlying plugin
			unowned Peas.PluginInfo? plugin = null;

			foreach(var plugini in engine.get_plugin_list())
			{
				if (plugini.get_module_name() == name)
				{
					plugin = plugini;
					break;
				}
			}
			if (plugin == null) {
				warning("Could not find plugin: %s", name);
				return;
			}
			engine.try_load_plugin(plugin);
		}
	    private void on_extension_added(Peas.PluginInfo i, Object p)
	    {
	        var plugin = p as AppletPlugin;
	        var type = i.get_module_name();
	        if (!loaded_types.contains(type))
	        {
				var data = PluginData();
				data.plugin = plugin;
				data.count = 0;
				loaded_types.insert(type,data);
			}
			if (local_applets.contains(type))
				return;
	        // Iterate the children, and then load them into the panel
			PluginSettings? pl = null;
			foreach (var s in settings.plugins)
				if (s.default_settings.get_string(Key.NAME) == type)
				{
					pl = s;
					local_applets.insert(type,0);
					load_applet(pl);
					update_applet_positions();
					return;
				}
	    }
		internal void place_applet(AppletPlugin applet_plugin, PluginSettings s)
		{
			s.init_configuration(settings,applet_plugin.plugin_info.get_external_data(Data.CONFIG)!=null);
			var applet = applet_plugin.get_applet_widget(this,s.config_settings,s.number);
			var position = s.default_settings.get_uint(Key.POSITION);
			box.pack_start(applet,false, true);
			box.reorder_child(applet,(int)position);
			if (applet_plugin.plugin_info.get_external_data(Data.EXPANDABLE)!=null)
			{
				s.default_settings.bind(Key.EXPAND,applet,"hexpand",GLib.SettingsBindFlags.GET);
				applet.bind_property("hexpand",applet,"vexpand",BindingFlags.SYNC_CREATE);
			}
			applet.destroy.connect(()=>{applet_removed(applet.number);});
		}
		internal void remove_applet(Applet applet)
		{
			box.remove(applet);
			applet.destroy();
		}
		internal void applet_removed(uint num)
		{
			PluginSettings s = settings.get_settings_by_num(num);
			var name = s.default_settings.get_string(Key.NAME);
			var count = local_applets.lookup(name);
			count--;
			if (count == 0)
				local_applets.remove(name);
			else
				local_applets.insert(name,count);
			var data = loaded_types.lookup(name);
			data.count -= 1;
			if (data.count == 0)
			{
				var pl = loaded_types.lookup(name).plugin;
				loaded_types.remove(name);
				var info = pl.plugin_info;
				engine.try_unload_plugin(info);
			}
			else
				loaded_types.insert(name,data);
			settings.remove_plugin_settings(num);
		}
		internal void update_applet_positions()
		{
		    var children = box.get_children();
		    for (unowned List<unowned Widget> l = children; l != null; l = l.next)
		    {
				var idx = get_applet_settings(l.data as Applet).default_settings.get_uint(Key.POSITION);
				box.reorder_child((l.data as Applet),(int)idx);
		    }
		}
		internal List<unowned Widget> get_applets_list()
		{
			return box.get_children();
		}
		internal unowned List<Peas.PluginInfo> get_all_types()
		{
			return engine.get_plugin_list();
		}
		internal unowned AppletPlugin get_plugin(Applet pl)
		{
			return loaded_types.lookup((settings.get_settings_by_num(pl.number)
										.default_settings.get_string(Key.NAME))).plugin;
		}
		internal PluginSettings get_applet_settings(Applet pl)
		{
			return settings.get_settings_by_num(pl.number);
		}
		internal uint get_applet_position(Applet pl)
		{
			int res;
			box.child_get(pl,"position",out res, null);
			return (uint)res;
		}
		internal void set_applet_position(Applet pl, int pos)
		{
			box.reorder_child(pl,pos);
		}
/*
 * Properties handling
 */
		private bool panel_edge_can_strut(out ulong size)
		{
			ulong s = 0;
			size = 0;
			if (!get_mapped())
				return false;
			if (autohide)
				s =(show_hidden)? 1 : 0;
			else switch (orientation)
			{
				case Gtk.Orientation.VERTICAL:
					s = a.width;
					break;
				case Gtk.Orientation.HORIZONTAL:
					s = a.height;
					break;
				default: return false;
			}
			if (monitor < 0)
			{
				size = s;
				return true;
			}
			if (monitor >= get_screen().get_n_monitors())
				return false;
			Gdk.Rectangle rect, rect2;
			get_screen().get_monitor_geometry(monitor, out rect);
			switch(edge)
			{
		        case PositionType.LEFT:
		            rect.width = rect.x;
		            rect.x = 0;
		            s += rect.width;
		            break;
		        case PositionType.RIGHT:
		            rect.x += rect.width;
		            rect.width = get_screen().get_width() - rect.x;
		            s += rect.width;
		            break;
		        case PositionType.TOP:
		            rect.height = rect.y;
		            rect.y = 0;
		            s += rect.height;
		            break;
		        case PositionType.BOTTOM:
		            rect.y += rect.height;
		            rect.height = get_screen().get_height() - rect.y;
		            s += rect.height;
		            break;
		    }
			if (rect.height == 0 || rect.width == 0) ; /* on a border of monitor */
		    else
		    {
		        var n = get_screen().get_n_monitors();
		        for (var i = 0; i < n; i++)
		        {
		            if (i == monitor)
		                continue;
		            get_screen().get_monitor_geometry(i, out rect2);
		            if (rect.intersect(rect2, null))
		                /* that monitor lies over the edge */
		                return false;
		        }
		    }
			size = s;
			return true;
		}
		private void update_strut()
		{
		    int index;
		    Gdk.Atom atom;
		    ulong strut_size = 0;
		    ulong strut_lower = 0;
		    ulong strut_upper = 0;

		    if (!get_mapped())
		        return;
		    /* most wm's tend to ignore struts of unmapped windows, and that's how
		     * panel hides itself. so no reason to set it. If it was be, it must be removed */
		    if (autohide && !show_hidden && this.strut_size == 0)
		        return;

		    /* Dispatch on edge to set up strut parameters. */
		    switch (edge)
		    {
		        case PositionType.LEFT:
		            index = 0;
		            strut_lower = a.y;
		            strut_upper = a.y + a.height;
		            break;
		        case PositionType.RIGHT:
		            index = 1;
		            strut_lower = a.y;
		            strut_upper = a.y + a.height;
		            break;
		        case PositionType.TOP:
		            index = 2;
		            strut_lower = a.x;
		            strut_upper = a.x + a.width;
		            break;
		        case PositionType.BOTTOM:
		            index = 3;
		            strut_lower = a.x;
		            strut_upper = a.x + a.width;
		            break;
		        default:
		            return;
		    }

		    /* Set up strut value in property format. */
		    ulong desired_strut[12];
		    if (strut &&
		        panel_edge_can_strut(out strut_size))
		    {
		        desired_strut[index] = strut_size;
		        desired_strut[4 + index * 2] = strut_lower;
		        desired_strut[5 + index * 2] = strut_upper-1;
		    }
		    /* If strut value changed, set the property value on the panel window.
		     * This avoids property change traffic when the panel layout is recalculated but strut geometry hasn't changed. */
		    if ((this.strut_size != strut_size) || (this.strut_lower != strut_lower) || (this.strut_upper != strut_upper) || (this.strut_edge != this.edge))
		    {
		        this.strut_size = strut_size;
		        this.strut_lower = strut_lower;
		        this.strut_upper = strut_upper;
		        this.strut_edge = this.edge;
		        /* If window manager supports STRUT_PARTIAL, it will ignore STRUT.
		         * Set STRUT also for window managers that do not support STRUT_PARTIAL. */
				var xwin = get_window();
		        if (strut_size != 0)
		        {
		            atom = Atom.intern_static_string("_NET_WM_STRUT_PARTIAL");
		            Gdk.property_change(xwin,atom,Atom.intern_static_string("CARDINAL"),32,Gdk.PropMode.REPLACE,(uint8[])desired_strut,12);
		            atom = Atom.intern_static_string("_NET_WM_STRUT");
		            Gdk.property_change(xwin,atom,Atom.intern_static_string("CARDINAL"),32,Gdk.PropMode.REPLACE,(uint8[])desired_strut,4);
		        }
		        else
		        {
		            atom = Atom.intern_static_string("_NET_WM_STRUT_PARTIAL");
		            Gdk.property_delete(xwin,atom);
		            atom = Atom.intern_static_string("_NET_WM_STRUT");
		            Gdk.property_delete(xwin,atom);
		        }
		    }
		}
		private void update_appearance()
		{
			if (provider != null)
				this.get_style_context().remove_provider(provider);
			if (font == null)
				return;
		    StringBuilder str = new StringBuilder();
	        str.append_printf(".-simple-panel-background {\n");
            if (use_background_color)
                str.append_printf(" background-color: %s;\n",background_color);
            else
                str.append_printf(" background-color: transparent;\n");
            if (use_background_file)
            {
                str.append_printf(" background-image: url('%s');\n",background_file);
/* Feature proposed: Background repeat */
//~                 if (false)
//~                     str.append_printf(" background-repeat: no-repeat;\n");
            }
            else
                str.append_printf(" background-image: none;\n");
	        str.append_printf("}\n");
/* Feature proposed: Panel Layout and Shadow */
//~ 	        str.append_printf(".-simple-panel-shadow {\n");
//~ 	        str.append_printf(" box-shadow: 0 0 0 3px alpha(0.3, %s);\n",foreground_color);
//~ 	        str.append_printf(" border-style: none;\n margin: 3px;\n");
//~ 	        str.append_printf("}\n");
	        str.append_printf(".-simple-panel-round-corners {\n");
	        str.append_printf(" border-radius: %upx;\n",round_corners_size);
	        str.append_printf("}\n");
		    Pango.FontDescription desc = Pango.FontDescription.from_string(font);		    
	        str.append_printf(".-simple-panel-font-size {\n");
	        str.append_printf(" font-size: %dpx;\n",desc.get_size()/Pango.SCALE);
	        str.append_printf("}\n");
		    str.append_printf(".-simple-panel-font {\n");
	        var family = desc.get_family();
	        var weight = desc.get_weight();
	        var style = desc.get_style();
	        var variant = desc.get_variant();
	        str.append_printf(" font-style: %s;\n",(style == Pango.Style.ITALIC) ? "italic" : ((style == Pango.Style.OBLIQUE) ? "oblique" : "normal"));
	        str.append_printf(" font-variant: %s;\n",(variant == Pango.Variant.SMALL_CAPS) ? "small-caps" : "normal");
	        str.append_printf(" font-weight: %s;\n",(weight <= Pango.Weight.SEMILIGHT) ? "light" : (weight >= Pango.Weight.SEMIBOLD ? "bold" : "normal"));
	        str.append_printf(" font-family: %s;\n",family);
	        str.append_printf("}\n");
			str.append_printf(".-simple-panel-foreground-color {\n");
			str.append_printf(" color: %s;\n",foreground_color);
			str.append_printf("}\n");
		    str.append_printf("\0");
		    var css = str.str;
		    provider = PanelCSS.add_css_to_widget(this as Widget, css);
		    PanelCSS.toggle_class(this as Widget,"-simple-panel-background",use_background_color || use_background_file);
		    PanelCSS.toggle_class(this as Widget,"-simple-panel-shadow",false);
		    PanelCSS.toggle_class(this as Widget,"-simple-panel-round-corners",round_corners_size > 0);
		    PanelCSS.toggle_class(this as Widget,"-simple-panel-font-size",use_font);
		    PanelCSS.toggle_class(this as Widget,"-simple-panel-font", use_font && !font_size_only);
		    PanelCSS.toggle_class(this as Widget,"-simple-panel-foreground-color",use_foreground_color);
		}
/*
 * Actions stuff
 */
		/* If there is a panel on this edge and it is not the panel being configured, set the edge unavailable. */
		internal bool panel_edge_available(int edge, int monitor, bool include_this)
		{
			foreach (var w in application.get_windows())
				if (w is Toplevel)
				{
					Toplevel pl = w as Toplevel;
					if (((pl != this)|| include_this) && (pl.edge == edge) && ((pl._mon == _mon)||pl._mon<0))
						return false;
				}
		    return true;
		}
		/* FIXME: Potentially we can support multiple panels at the same edge,
		 * but currently this cannot be done due to some positioning problems. */
		private static string gen_panel_name(string profile, PositionType edge, int mon)
		{
		    string? edge_str = null;
		    if (edge == PositionType.TOP)
		        edge_str="top";
		    if (edge == PositionType.BOTTOM)
		        edge_str="bottom";
		    if (edge == PositionType.LEFT)
		        edge_str="left";
		    if (edge == PositionType.RIGHT)
		        edge_str="right";
		    string dir = user_config_file_name("panels",profile, null);
		    for(var i = 0; i < int.MAX; ++i )
		    {
		        var name = "%s-m%d-%d".printf(edge_str, mon, i);
		        var f = Path.build_filename( dir, name, null );
		        if( !FileUtils.test( f, FileTest.EXISTS ) )
					return name;
		    }
		    return "panel-max";
		}
		private void activate_new_panel(SimpleAction act, Variant? param)
		{
			int new_mon = -2;
			PositionType new_edge = PositionType.TOP;
			var found = false;
		    /* Allocate the edge. */
		    assert(Gdk.Screen.get_default()!=null);
		    var monitors = Gdk.Screen.get_default().get_n_monitors();
		    /* try to allocate edge on current monitor first */
		    var m = _mon;
		    if (m < 0)
		    {
		        /* panel is spanned over the screen, guess from pointer now */
		        int x, y;
		        var manager = Gdk.Screen.get_default().get_display().get_device_manager();
		        var device = manager.get_client_pointer ();
		        Gdk.Screen scr;
		        device.get_position(out scr, out x, out y);
		        m = scr.get_monitor_at_point(x, y);
		    }
		    for (int e = PositionType.BOTTOM; e >= PositionType.LEFT; e--)
		    {
		        if (panel_edge_available((PositionType)e, m, true))
		        {
		            new_edge = (PositionType)e;
		            new_mon = m;
		            found = true;
		        }
		    }
		    /* try all monitors */
		    if (!found)
			    for(m=0; m<monitors; ++m)
			    {
			        /* try each of the four edges */
			        for(int e = PositionType.BOTTOM; e >= PositionType.LEFT; e--)
			        {
			            if(panel_edge_available((PositionType)e,m,true)) {
			                new_edge = (PositionType)e;
			                new_mon = m;
							found = true;
			            }
			        }
			    }
			if (!found)
			{
			    warning("Error adding panel: There is no room for another panel. All the edges are taken.");
			    var msg = new MessageDialog
			            (this,
			             DialogFlags.DESTROY_WITH_PARENT,
			             MessageType.ERROR,ButtonsType.CLOSE,
			             N_("There is no room for another panel. All the edges are taken."));
				apply_window_icon(msg as Gtk.Window);
			    msg.set_title(_("Error"));
			    msg.run();
			    msg.destroy();
			    return;
			}
		    var new_name = gen_panel_name(profile,new_edge,new_mon);
		    var new_toplevel = Toplevel.create(application,new_name,new_mon,new_edge);
		    new_toplevel.configure("position");
		    new_toplevel.show_all();
		    new_toplevel.queue_draw();
		}
		private void activate_remove_panel(SimpleAction act, Variant? param)
		{
		    var dlg = new MessageDialog.with_markup(this,
													DialogFlags.MODAL,
													MessageType.QUESTION,
													ButtonsType.OK_CANCEL,
													N_("Really delete this panel?\n<b>Warning: This can not be recovered.</b>"));
			apply_window_icon(dlg as Gtk.Window);
		    dlg.set_title(_("Confirm"));
		    var ok = (dlg.run() == ResponseType.OK );
		    dlg.destroy();
		    if( ok )
		    {
		        string pr = this.profile;
		        this.stop_ui();
		        this.destroy();
		        /* delete the config file of this panel */
		        var fname = user_config_file_name("panels",pr,panel_name);
		        FileUtils.unlink( fname );
		    }
		}
		private void activate_panel_settings(SimpleAction act, Variant? param)
		{
			this.configure(param.get_string());
		}
		internal void configure(string page)
		{
			if (pref_dialog == null)
				pref_dialog = new ConfigureDialog(this);
			pref_dialog.prefs_stack.set_visible_child_name(page);
			pref_dialog.present();
			pref_dialog.destroy.connect(()=>{pref_dialog = null;});
		}
	}
}
