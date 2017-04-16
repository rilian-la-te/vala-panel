/*
 * vala-panel
 * Copyright (C) 2015 Konstantin Pugin <ria.freelander@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

using GLib;
using Gtk;
using Gdk;
using Config;

namespace ValaPanel
{
    public class Toplevel : Gtk.ApplicationWindow
    {
    /************************************************************************
     * Core -----------------------------------------------------------------
     ************************************************************************/
        private unowned Gtk.Revealer ah_rev = null;
        private unowned Gtk.Box box;
        private Gtk.Menu context_menu;
        internal ConfigureDialog pref_dialog;
        private bool initialized;
        public const string[] gnames = {Key.WIDTH,Key.HEIGHT,Key.EDGE,Key.ALIGNMENT,
                                                Key.MONITOR,Key.AUTOHIDE,Key.SHOW_HIDDEN,
                                                Key.MARGIN,Key.DOCK,Key.STRUT,
                                                Key.DYNAMIC};
        public const string[] anames = {Key.BACKGROUND_COLOR,Key.FOREGROUND_COLOR,
                                                Key.CORNERS_SIZE, Key.BACKGROUND_FILE,
                                                Key.USE_BACKGROUND_COLOR, Key.USE_FOREGROUND_COLOR,
                                                Key.USE_BACKGROUND_FILE, Key.USE_FONT,
                                                Key.FONT_SIZE_ONLY, Key.FONT};


        public string panel_name {get; internal construct;}

        private string profile
        { owned get {
            GLib.Value v = Value(typeof(string));
            this.get_application().get_property("profile",ref v);
            return v.get_string();
            }
        }
        public int height { get; internal set;}
        private int _w;
        public int width
        {get {return _w;}
            internal set {_w = (value > 0) ? ((value <=100) ? value : 100) : 1;}
        }
        const GLib.ActionEntry[] panel_entries =
        {
            {"new-panel", activate_new_panel, null, null, null},
            {"remove-panel", activate_remove_panel, null, null, null},
            {"panel-settings", activate_panel_settings, "s", null, null},
        };
        /**************************************************************************
         * Appearance -------------------------------------------------------------
         **************************************************************************/
        private IconSizeHints ihints;
        private Gdk.RGBA bgc;
        private Gdk.RGBA fgc;
        private Gtk.CssProvider provider;
        public bool use_font {get; internal set;}
        public bool use_background_color {get; internal set;}
        public bool use_foreground_color {get; internal set;}
        public bool use_background_file {get; internal set;}
        public bool font_size_only {get; internal set;}
        public uint font_size {get; internal set;}
        public uint round_corners_size {get; internal set;}
        public string font {get; internal set;}
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
        public string background_file {get; internal set;}
        private void update_appearance()
        {
            if (provider != null)
                this.get_style_context().remove_provider(provider);
            if (font == null)
                return;
            StringBuilder str = new StringBuilder();
            str.append_printf(".-vala-panel-background {\n");
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
        //~             str.append_printf(".-vala-panel-shadow {\n");
        //~             str.append_printf(" box-shadow: 0 0 0 3px alpha(0.3, %s);\n",foreground_color);
        //~             str.append_printf(" border-style: none;\n margin: 3px;\n");
        //~             str.append_printf("}\n");
            str.append_printf(".-vala-panel-round-corners {\n");
            str.append_printf(" border-radius: %upx;\n",round_corners_size);
            str.append_printf("}\n");
            Pango.FontDescription desc = Pango.FontDescription.from_string(font);
            str.append_printf(".-vala-panel-font-size {\n");
            str.append_printf(" font-size: %dpx;\n",desc.get_size()/Pango.SCALE);
            str.append_printf("}\n");
            str.append_printf(".-vala-panel-font {\n");
            var family = desc.get_family();
            var weight = desc.get_weight();
            var style = desc.get_style();
            var variant = desc.get_variant();
            str.append_printf(" font-style: %s;\n",(style == Pango.Style.ITALIC) ? "italic" : ((style == Pango.Style.OBLIQUE) ? "oblique" : "normal"));
            str.append_printf(" font-variant: %s;\n",(variant == Pango.Variant.SMALL_CAPS) ? "small-caps" : "normal");
            str.append_printf(" font-weight: %s;\n",(weight <= Pango.Weight.SEMILIGHT) ? "light" : (weight >= Pango.Weight.SEMIBOLD ? "bold" : "normal"));
            str.append_printf(" font-family: %s;\n",family);
            str.append_printf("}\n");
            str.append_printf(".-vala-panel-foreground-color {\n");
            str.append_printf(" color: %s;\n",foreground_color);
            str.append_printf("}\n");
            var css = str.str;
            provider = PanelCSS.add_css_to_widget(this as Widget, css);
            PanelCSS.toggle_class(this as Widget,"-vala-panel-background",use_background_color || use_background_file);
            PanelCSS.toggle_class(this as Widget,"-vala-panel-shadow",false);
            PanelCSS.toggle_class(this as Widget,"-vala-panel-round-corners",round_corners_size > 0);
            PanelCSS.toggle_class(this as Widget,"-vala-panel-font-size",use_font);
            PanelCSS.toggle_class(this as Widget,"-vala-panel-font", use_font && !font_size_only);
            PanelCSS.toggle_class(this as Widget,"-vala-panel-foreground-color",use_foreground_color);
        }
        /*********************************************************************************************
         * Positioning
         *********************************************************************************************/
        private int _mon;
        private Gtk.Allocation a;
        internal AlignmentType alignment {get; internal set;}
        public int panel_margin {get; internal set;}
        public Gtk.PositionType edge {get; internal set construct;}
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
        public bool dock {get; internal set;}
        public bool strut {get; internal set;}
        public bool is_dynamic {get; internal set;}
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
                    panel.queue_resize();
                }
            }
        }
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
                    settings.default_settings.set_int(Key.WIDTH,w);
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
        }
        private void _calculate_position(ref Gtk.Allocation alloc)
        {
            unowned Gdk.Screen screen = this.get_screen();
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
//~                 marea = screen.get_monitor_workarea(monitor);
//~                 var hmod = (autohide) ? GAP : height;
//~                 switch (edge)
//~                 {
//~                     case PositionType.TOP:
//~                         marea.x -= hmod;
//~                         marea.height += hmod;
//~                         break;
//~                     case PositionType.BOTTOM:
//~                         marea.height += hmod;
//~                         break;
//~                     case PositionType.LEFT:
//~                         marea.y -= hmod;
//~                         marea.width += hmod;
//~                         break;
//~                     case PositionType.RIGHT:
//~                         marea.width += hmod;
//~                         break;
//~                 }
            }
            if (orientation == Gtk.Orientation.HORIZONTAL)
            {
                alloc.width = width;
                alloc.x = marea.x;
                calculate_width(marea.width,is_dynamic,alignment,panel_margin,ref alloc.width, ref alloc.x);
                alloc.height = (!autohide || (ah_rev != null && ah_rev.reveal_child)) ? height :
                                        GAP;
                alloc.y = marea.y + ((edge == Gtk.PositionType.TOP) ? 0 : (marea.height - alloc.height));
            }
            else
            {
                alloc.height = width;
                alloc.y = marea.y;
                calculate_width(marea.height,is_dynamic,alignment,panel_margin,ref alloc.height, ref alloc.y);
                alloc.width = (!autohide || (ah_rev != null && ah_rev.reveal_child)) ? height :
                                         GAP;
                alloc.x = marea.x + ((edge == Gtk.PositionType.LEFT) ? 0 : (marea.width - alloc.width));
            }
        }

        private static void calculate_width(int scrw, bool dyn, AlignmentType align,
                                            int margin, ref int panw, ref int x)
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
            var rect = Gtk.Allocation();
            rect.width = min.width;
            rect.height = min.height;
            _calculate_position(ref rect);
            min.width = rect.width;
            min.height = rect.height;
        }
        /*************************************************************************************
         * Plugins stuff
         *************************************************************************************/
        private static unowned Platform platform = null;
        private static unowned CoreSettings core_settings = null;
        private struct PluginData
        {
            unowned AppletEngine plugin;
            int count;
        }
        private static Peas.Engine engine;
        private static ulong mon_handler;
        private static Peas.ExtensionSet extset;
        private static HashTable<string,PluginData?> loaded_types;
        private HashTable<string,int> local_applets;
        private unowned UnitSettings settings;
        static construct
        {
            engine = Peas.Engine.get_default();
            engine.add_search_path(PLUGINS_DIRECTORY,PLUGINS_DATA);
            loaded_types = new HashTable<string,PluginData?>(str_hash,str_equal);
            extset = new Peas.ExtensionSet(engine,typeof(AppletPlugin));
        }
        internal void add_applet(string type)
        {
            unowned UnitSettings s = core_settings.add_unit_settings(type,false);
            s.default_settings.set_string(Key.NAME,type);
            load_applet(s);
        }
        internal void load_applet(UnitSettings s)
        {
            /* Determine if the plugin is loaded yet. */
            string name = s.default_settings.get_string(Key.NAME);
            if (loaded_types.contains(name))
            {
                unowned PluginData? data = loaded_types.lookup(name);
                if (data!=null)
                {
                    place_applet(data.plugin,s);
                    data.count +=1;
                    var count = local_applets.lookup(name);
                    count += 1;
                    local_applets.insert(name,count);
                    loaded_types.insert(name,data);
                    return;
                }
            }
            // Got this far we actually need to load the underlying plugin
            unowned Peas.PluginInfo? plugin = null;

            foreach(unowned Peas.PluginInfo plugini in engine.get_plugin_list())
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
            unowned AppletEngine plugin = p as AppletEngine;
            unowned string type = i.get_module_name();
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
            unowned UnitSettings? pl = null;
            foreach (var applet in settings.default_settings.get_strv(Key.APPLETS))
            {
                unowned UnitSettings s = core_settings.get_by_uuid(applet);
                if (s.default_settings.get_string(Key.NAME) == type)
                {
                    pl = s;
                    local_applets.insert(type,0);
                    load_applet(pl);
                    update_applet_positions();
                    return;
                }
            }
        }
        internal void place_applet(AppletEngine engine, UnitSettings s)
        {
            string oafid = s.default_settings.get_string(Key.NAME);
            var aw = engine.get_applet_widget_by_oafid(this,s.custom_settings,oafid,s.uuid);
            unowned Applet applet = aw;
            var position = s.default_settings.get_uint(Key.POSITION);
            box.pack_start(applet,false, true);
            box.reorder_child(applet,(int)position);
//            if (applet_plugin.plugin_info.get_external_data(Data.EXPANDABLE)!=null)
//            {
                s.default_settings.bind(Key.EXPAND,applet,"hexpand",GLib.SettingsBindFlags.GET);
                applet.bind_property("hexpand",applet,"vexpand",BindingFlags.SYNC_CREATE);
//            }
            applet.destroy.connect(()=>{applet_removed(applet.uuid);});
        }
        internal void remove_applet(Applet applet)
        {
            applet.destroy();
        }
        internal void applet_removed(string uuid)
        {
            if (this.in_destruction())
                return;
            unowned UnitSettings s = core_settings.get_by_uuid(uuid);
            var name = s.default_settings.get_string(Key.NAME);
            var count = local_applets.lookup(name);
            count--;
            if (count <= 0)
                local_applets.remove(name);
            else
                local_applets.insert(name,count);
            unowned PluginData data = loaded_types.lookup(name);
            data.count -= 1;
            if (data.count <= 0)
            {
                unowned AppletEngine pl = loaded_types.lookup(name).plugin;
                loaded_types.remove(name);
                unowned Peas.PluginInfo info = pl.plugin_info;
                engine.try_unload_plugin(info);
            }
            else
                loaded_types.insert(name,data);
            core_settings.remove_unit_settings(uuid);
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
        public List<unowned Widget> get_applets_list()
        {
            return box.get_children();
        }
        internal unowned List<Peas.PluginInfo> get_all_types()
        {
            return engine.get_plugin_list();
        }
        internal unowned AppletEngine get_plugin(Applet pl)
        {
//TODO: Correct lookup
            return loaded_types.lookup((core_settings.get_by_uuid(pl.uuid)
                                        .default_settings.get_string(Key.NAME))).plugin;
        }
        internal unowned UnitSettings get_applet_settings(Applet pl)
        {
            return core_settings.get_by_uuid(pl.uuid);
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
        /**************************************************************************************
         * Strut handling
         **************************************************************************************/
        private void update_strut()
        {
            platform.update_strut(this as Gtk.Window);
        }
        /************************************************************************************************
         *  Constructors
         ************************************************************************************************/
        public Toplevel.with_platform(Gtk.Application app, Platform platform, string name)
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
            if (Toplevel.platform == null)
            {
                Toplevel.platform = platform;
                Toplevel.core_settings = platform.get_settings();
            }
            setup(false);
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
            settings = core_settings.get_by_uuid(this.name);
            if (use_internal_values)
            {
                settings.default_settings.set_int(Key.MONITOR, _mon);
                settings.default_settings.set_enum(Key.EDGE, edge);
            }
            settings_as_action(this,settings.default_settings,Key.EDGE);
            settings_as_action(this,settings.default_settings,Key.ALIGNMENT);
            settings_as_action(this,settings.default_settings,Key.HEIGHT);
            settings_as_action(this,settings.default_settings,Key.WIDTH);
            settings_as_action(this,settings.default_settings,Key.DYNAMIC);
            settings_as_action(this,settings.default_settings,Key.AUTOHIDE);
            settings_as_action(this,settings.default_settings,Key.STRUT);
            settings_as_action(this,settings.default_settings,Key.DOCK);
            settings_as_action(this,settings.default_settings,Key.MARGIN);
            settings_bind(this,settings.default_settings,Key.MONITOR);
            settings_as_action(this,settings.default_settings,Key.ICON_SIZE);
            settings_as_action(this,settings.default_settings,Key.BACKGROUND_COLOR);
            settings_as_action(this,settings.default_settings,Key.FOREGROUND_COLOR);
            settings_as_action(this,settings.default_settings,Key.BACKGROUND_FILE);
            settings_as_action(this,settings.default_settings,Key.FONT);
            settings_as_action(this,settings.default_settings,Key.CORNERS_SIZE);
            settings_as_action(this,settings.default_settings,Key.FONT_SIZE_ONLY);
            settings_as_action(this,settings.default_settings,Key.USE_BACKGROUND_COLOR);
            settings_as_action(this,settings.default_settings,Key.USE_FOREGROUND_COLOR);
            settings_as_action(this,settings.default_settings,Key.USE_FONT);
            settings_as_action(this,settings.default_settings,Key.USE_BACKGROUND_FILE);
            if (monitor < Gdk.Screen.get_default().get_n_monitors())
                start_ui();
            unowned Gtk.Application panel_app = get_application();
            if (mon_handler != 0)
                mon_handler = Signal.connect(Gdk.Screen.get_default(),"monitors-changed",
                                            (GLib.Callback)(monitors_changed_cb),panel_app);
        }
        construct
        {
            local_applets = new HashTable<string,int>(str_hash,str_equal);
            unowned Gdk.Visual visual = this.get_screen().get_rgba_visual();
            if (visual != null)
                this.set_visual(visual);
            a = Gtk.Allocation();
            this.notify.connect((s,p)=> {
                if (p.name == Key.EDGE)
                    if (box != null) box.set_orientation(orientation);
                if (p.name == Key.AUTOHIDE && this.ah_rev != null)
                    if (autohide) ah_hide(); else ah_show();
                if (p.name in gnames)
                {
                    this.queue_resize();
                    this.update_strut();
                }
                if (p.name in anames)
                    this.update_appearance();
            });
            this.add_action_entries(panel_entries,this);
            extset.extension_added.connect(on_extension_added);
            engine.load_plugin.connect_after((i)=>
            {
                var ext = extset.get_extension(i);
                on_extension_added(i,ext);
            });
        }
        /***************************************************************************
         * Common UI functions
         ***************************************************************************/
        protected override void destroy()
        {
            stop_ui();
            base.destroy();
        }
        private void stop_ui()
        {
            if (pref_dialog != null)
                pref_dialog.response(Gtk.ResponseType.CLOSE);
            if (initialized)
            {
                Gdk.flush();
                initialized = false;
            }
            if (this.get_child() != null)
            {
                box.destroy();
                box = null;
            }
        }

        private void start_ui()
        {
            a.x = a.y = a.width = a.height = 0;
            set_wmclass("panel","vala-panel");
            PanelCSS.apply_from_resource(this,"/org/vala-panel/lib/style.css","-panel-transparent");
            PanelCSS.toggle_class(this,"-panel-transparent",false);
            this.get_application().add_window(this);
            this.add_events(Gdk.EventMask.BUTTON_PRESS_MASK |
                            Gdk.EventMask.ENTER_NOTIFY_MASK |
                            Gdk.EventMask.LEAVE_NOTIFY_MASK);
            this.realize();
			var r = new Gtk.Revealer();
            var mbox = new Box(this.orientation,0);
            box = mbox;
            this.ah_rev = r;
            r.set_transition_type(RevealerTransitionType.CROSSFADE);
            r.notify.connect((s,p)=>{
                if (p.name == "child-revealed")
                    box.queue_draw();
            });
            r.add(box);
            box.set_baseline_position(Gtk.BaselinePosition.CENTER);
            box.set_border_width(0);
            box.set_hexpand(true);
            box.set_vexpand(true);
            this.add(r);
            r.show();
			box.show();
            this.ah_rev.set_reveal_child(true);
            this.set_type_hint((dock)? Gdk.WindowTypeHint.DOCK : Gdk.WindowTypeHint.NORMAL);
            core_settings.init_toplevel_plugin_list(this.settings);
            foreach(var applet in settings.default_settings.get_strv(Key.APPLETS))
            {
                unowned UnitSettings pl = core_settings.get_by_uuid(applet);
                load_applet(pl);
            }
            this.show();
            this.stick();
            update_applet_positions();
            this.present();
            this.autohide = autohide;
            initialized = true;
        }

        /****************************************************
         *         autohide : new version                   *
         ****************************************************/
        private const int PERIOD = 200;
        private const int GAP = 2;
        private AutohideState ah_state;
        public bool autohide {get; internal set;}
		private void ah_show()
		{
				PanelCSS.toggle_class(this,"-panel-transparent",false);
				this.ah_rev.set_reveal_child(true);
				this.ah_state = AutohideState.VISIBLE;
		}

		private void ah_hide()
		{
			ah_state = AutohideState.WAITING;
			Timeout.add(PERIOD,()=>{
				if(autohide && ah_state == AutohideState.WAITING)
				{
					PanelCSS.toggle_class(this,"-panel-transparent",true);
					this.ah_rev.set_reveal_child(false);
					this.ah_state = AutohideState.HIDDEN;
				}
				return false;
				});
		}

        protected override bool enter_notify_event(EventCrossing event)
        {
            ah_show();
            return false;
        }

        protected override bool leave_notify_event(EventCrossing event)
        {
            if(this.autohide && (event.detail != Gdk.NotifyType.INFERIOR && event.detail != Gdk.NotifyType.VIRTUAL))
                ah_hide();
            return false;
        }

        protected override void grab_notify(bool was_grabbed)
        {
            if(!was_grabbed)
                this.ah_state = AutohideState.GRAB;
            else if (autohide)
                this.ah_hide();
        }

/*
 * Menus stuff
 */

        protected override bool button_release_event(Gdk.EventButton e)
        {
            if (e.button == 3)
            {
                if (context_menu == null)
                {
                    var menu = get_plugin_menu(null);
                    menu.popup(null,null,null,e.button,e.time);
                    return true;
                }
                else
                {
                    context_menu.destroy();
                    context_menu = null;
                    return true;
                }
            }
            return false;
        }

        internal Gtk.Menu get_plugin_menu(Applet? pl)
        {
            var builder = new Builder.from_resource("/org/vala-panel/lib/menus.ui");
            unowned GLib.Menu gmenu = builder.get_object("panel-context-menu") as GLib.Menu;
            if (pl != null)
            {
                var gmenusection = builder.get_object("plugin-section") as GLib.Menu;
                pl.update_context_menu(ref gmenusection);
            }
            if (context_menu != null)
                context_menu.destroy();
            context_menu = new Gtk.Menu.from_model(gmenu as MenuModel);
            if (pl != null)
                context_menu.attach_to_widget(pl,null);
            else
                context_menu.attach_to_widget(this,null);
            context_menu.show_all();
            return context_menu;
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
            var new_name = CoreSettings.get_uuid();
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
                this.stop_ui();
                this.destroy();
                /* delete the config file of this panel */
                core_settings.remove_unit_settings(this.name);
            }
        }
        private void activate_panel_settings(SimpleAction act, Variant? param)
        {
            this.configure(param.get_string());
        }
        public void configure(string page)
        {
            if (pref_dialog == null)
                pref_dialog = new ConfigureDialog(this);
            pref_dialog.prefs_stack.set_visible_child_name(page);
            pref_dialog.present();
            pref_dialog.hide.connect(()=>{
                pref_dialog.destroy();
                pref_dialog = null;
            });
        }
    }
}
