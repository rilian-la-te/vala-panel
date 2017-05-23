using GLib;
using Gtk;

namespace DBusMenu
{
    public interface GtkItemIface : Object
    {
        public abstract unowned Item item {get; protected set construct;}
        public static void parse_shortcut_variant(Variant shortcut, out uint key, out Gdk.ModifierType modifier)
        {
            key = 0;
            modifier = 0;
            VariantIter iter = shortcut.iterator();
            string str;
            while(iter.next("s", out str))
            {
                if (str == "Control") {
                    modifier |= Gdk.ModifierType.CONTROL_MASK;
                } else if (str == "Alt") {
                    modifier |= Gdk.ModifierType.MOD1_MASK;
                } else if (str == "Shift") {
                    modifier |= Gdk.ModifierType.SHIFT_MASK;
                } else if (str == "Super") {
                    modifier |= Gdk.ModifierType.SUPER_MASK;
                } else {
                    Gdk.ModifierType tempmod;
                    accelerator_parse(str, out key, out tempmod);
                }
            }
            return;
        }
    }
    public class GtkMainItem : CheckMenuItem, GtkItemIface
    {
        private const string[] allowed_properties = {"visible","enabled","label","type",
                                                "children-display","toggle-type",
                                                "toggle-state","icon-name","icon-data","accessible-desc",
                                                "x-valapanel-icon-size"};
        public unowned Item item {get; protected set construct;}
        public bool always_show_image_placeholder
        {
            get;
            set construct;
            default = true;
        }
        private bool has_indicator;
        private unowned Image image;
        private unowned AccelLabel accel_label;
        private ulong activate_handler;
        private bool is_themed_icon;
        construct
        {
            is_themed_icon = false;
            this.item = item;
            var box = new Box(Orientation.HORIZONTAL, 5);
            var img = new Image();
            image = img;
            var label = new AccelLabel("");
            accel_label = label;
            box.add(image);
            box.add(accel_label);
            this.add(box);
            this.show_all();
            this.init();
            item.property_changed.connect(on_prop_changed_cb);
            item.child_added.connect(on_child_added_cb);
            item.child_removed.connect(on_child_removed_cb);
            item.child_moved.connect(on_child_moved_cb);
            item.removing.connect(()=>{this.destroy();});
            activate_handler = this.activate.connect(on_toggled_cb);
            this.select.connect(on_select_cb);
            this.deselect.connect(on_deselect_cb);
            this.notify["visible"].connect(()=>{this.visible = item.get_bool_property("visible");});
        }
        public GtkMainItem(Item item, bool show_im_pl = true)
        {
            Object(item:item, always_show_image_placeholder: show_im_pl);
        }
        private void init()
        {
            foreach (var prop in allowed_properties)
                on_prop_changed_cb(prop,item.get_variant_property(prop));
        }
        private void on_prop_changed_cb(string name, Variant? val)
        {
            if(activate_handler > 0)
                SignalHandler.block(this,activate_handler);
            switch (name)
            {
                case "visible":
                    this.visible = val.get_boolean();
                    break;
                case "enabled":
                    this.sensitive = val.get_boolean();
                    break;
                case "label":
                    accel_label.set_text_with_mnemonic(val.get_string());
                    break;
                case "children-display":
                    if (this.submenu != null)
                    {
                        this.submenu.destroy();
                        this.submenu = null;
                    }
                    if (val != null && val.get_string() == "submenu")
                    {
                        this.submenu = new Gtk.Menu();
                        this.submenu.insert.connect(on_child_insert_cb);
                        foreach(unowned Item item in this.item.get_children())
                            submenu.add(GtkClient.new_item(item));
                    }
                    break;
                case "toggle-type":
                    if (val == null)
                        this.set_toggle_type("normal");
                    else
                        this.set_toggle_type(val.get_string());
                    break;
                case "toggle-state":
                    if (val != null && val.get_int32()>0)
                        this.active = true;
                    else
                        this.active = false;
                    break;
                case "accessible-desc":
                    this.set_tooltip_text(val != null ? val.get_string() : null);
                    break;
                case "icon-name":
                case "icon-data":
                    update_icon(val);
                    break;
                case "shortcut":
                    update_shortcut(val);
                    break;
            }
            if(activate_handler > 0)
                SignalHandler.unblock(this,activate_handler);
        }
        private void set_toggle_type(string type)
        {
            if (type=="radio")
            {
                this.set_accessible_role(Atk.Role.RADIO_MENU_ITEM);
                this.has_indicator = true;
                this.draw_as_radio = true;
            }
            else if (type=="checkmark")
            {
                this.set_accessible_role(Atk.Role.CHECK_MENU_ITEM);
                this.has_indicator = true;
                this.draw_as_radio = false;
            }
            else
            {
                this.set_accessible_role(Atk.Role.MENU_ITEM);
                this.has_indicator = false;
            }
        }
        private void update_icon(Variant? val)
        {
            if (val == null)
            {
                var icon = image.gicon;
                if (!(icon != null && icon is ThemedIcon && is_themed_icon))
                    is_themed_icon = false;
                return;
            }
            Icon? icon = null;
            if (val.get_type_string() == "s")
            {
                is_themed_icon = true;
                icon = new ThemedIcon.with_default_fallbacks(val.get_string()+"-symbolic");
            }
            else if (!is_themed_icon && val.get_type_string() == "ay")
                icon = new BytesIcon(val.get_data_as_bytes());
            else return;
            image.set_from_gicon(icon,IconSize.MENU);
            image.show();
            image.set_pixel_size(always_show_image_placeholder ? 16 : -1);
        }
        private void update_shortcut(Variant? val)
        {
            if (val == null)
                return;
            uint key;
            Gdk.ModifierType mod;
            parse_shortcut_variant(val, out key, out mod);
            this.accel_label.set_accel(key,mod);
        }
        private void on_child_added_cb(int id,Item item)
        {
            if (this.submenu != null)
                this.submenu.append (GtkClient.new_item(item));
            else
            {
                debug("Adding new item to item without submenu! Creating new submenu...\n");
                this.submenu = new Gtk.Menu();
                this.submenu.append (GtkClient.new_item(item));
            }
        }
        private void on_child_removed_cb(int id, Item item)
        {
            if (this.submenu != null)
                foreach(unowned Widget ch in this.submenu.get_children())
                    if ((ch as GtkItemIface).item == item)
                        ch.destroy();
            else
                debug("Cannot remove a child from item without submenu!\n");
        }
        private void on_child_moved_cb(int oldpos, int newpos, Item item)
        {
            if (this.submenu != null)
                foreach(unowned Widget ch in this.submenu.get_children())
                    if ((ch as GtkItemIface).item == item)
                        this.submenu.reorder_child(ch,newpos);
            else
                debug("Cannot move a child of item with has no children!\n");
        }
        private void on_toggled_cb()
        {
            item.handle_event("clicked",new Variant.int32(0),get_current_event_time());
        }
        private void on_select_cb()
        {
            if (this.submenu != null)
            {
                item.handle_event("opened",null,0);
                item.request_about_to_show();
            }
        }
        private void on_deselect_cb()
        {
            if (this.submenu != null)
                item.handle_event("closed",null,0);
        }
        private void on_child_insert_cb(Widget w, int pos)
        {
            unowned GtkItemIface ch = w as GtkItemIface;
            this.submenu.reorder_child(w,item.get_child_position(ch.item.id));
            this.submenu.queue_resize();
        }
        protected override void draw_indicator(Cairo.Context cr)
        {
            if (has_indicator)
                base.draw_indicator(cr);
        }
        protected override void destroy()
        {
            if (this.submenu != null)
            {
                this.submenu.destroy();
                this.submenu = null;
            }
            base.destroy();
        }
    }
    public class GtkSeparatorItem: SeparatorMenuItem, GtkItemIface
    {
        private const string[] allowed_properties = {"visible","enabled"};
        public unowned Item item {get; protected set construct;}
        public GtkSeparatorItem(Item item)
        {
            this.item = item;
            this.show_all();
            this.init();
            item.property_changed.connect(on_prop_changed_cb);
            item.removing.connect(()=>{this.destroy();});
        }
        private void on_prop_changed_cb(string name, Variant? val)
        {
            switch (name)
            {
                case "visible":
                    this.visible = val.get_boolean();
                    break;
                case "enabled":
                    this.sensitive = val.get_boolean();
                    break;
            }
        }
        private void init()
        {
            foreach (var prop in allowed_properties)
                on_prop_changed_cb(prop,item.get_variant_property(prop));
        }
    }
    public class GtkScaleItem: Gtk.MenuItem, GtkItemIface
    {
        private const string[] allowed_properties = {"visible","enabled","icon-name",
                                                            "x-valapanel-min-value","x-valapanel-current-value","x-valapanel-max-value",
                                                            "x-valapanel-step-increment","x-valapanel-page-increment","x-valapanel-draw-value",
                                                            "x-valapanel-format-value"};
        public unowned Item item {get; protected set construct;}
        private unowned Image primary;
        private unowned Scale scale;
        private string item_format;
        private bool grabbed;
        public GtkScaleItem(Item item)
        {
            this.item = item;
            var box = new Box(Orientation.HORIZONTAL,5);
            var img = new Image();
            primary = img;
            var adj = new Adjustment(0,0,double.MAX,0,0,0);
            var new_scale = new Scale(Orientation.HORIZONTAL,adj);
            scale = new_scale;
            scale.hexpand = true;
            box.add(primary);
            box.add(scale);
            this.add(box);
            this.show_all();
            this.init();
            item.property_changed.connect(on_prop_changed_cb);
            item.removing.connect(()=>{this.destroy();});
            adj.value_changed.connect(on_value_changed_cb);
            scale.format_value.connect(on_value_format_cb);
            scale.value_pos = PositionType.RIGHT;
            this.add_events (Gdk.EventMask.SCROLL_MASK
                            |Gdk.EventMask.POINTER_MOTION_MASK
                            |Gdk.EventMask.BUTTON_MOTION_MASK
                            |Gdk.EventMask.KEY_PRESS_MASK);
            this.set_size_request(200,-1);
        }
        private void on_prop_changed_cb(string name, Variant? val)
        {
            unowned Adjustment adj = scale.adjustment;
            switch (name)
            {
                case "visible":
                    this.visible = val.get_boolean();
                    break;
                case "enabled":
                    this.sensitive = val.get_boolean();
                    break;
                case "icon-name":
                    primary.set_from_gicon (icon_from_name(val),IconSize.MENU);
                    break;
                case "x-valapanel-min-value":
                    adj.lower = val.get_double();
                    break;
                case "x-valapanel-current-value":
                    adj.value = val.get_double();
                    break;
                case "x-valapanel-max-value":
                    adj.upper = val.get_double();
                    break;
                case "x-valapanel-step-increment":
                    adj.step_increment = val.get_double();
                    break;
                case "x-valapanel-page-increment":
                    adj.page_increment = val.get_double();
                    break;
                case "x-valapanel-draw-value":
                    scale.draw_value = val.get_boolean();
                    break;
                case "x-valapanel-format-value":
                    this.item_format = val.get_string();
                    break;
            }
        }
        private Icon icon_from_name(Variant? namev)
        {
            if (namev != null)
            {
                return new ThemedIcon.with_default_fallbacks(namev.get_string() + "-symbolic");
            }
            return new ThemedIcon.with_default_fallbacks("image-missing-symbolic");
        }
        private void on_value_changed_cb()
        {
            var adj = scale.adjustment;
            item.handle_event("value-changed",new Variant.double(adj.value),get_current_event_time());
        }
        private string on_value_format_cb(double val)
        {
            return item_format.printf(val);
        }
        private void init()
        {
            foreach (unowned string prop in allowed_properties)
                on_prop_changed_cb(prop,item.get_variant_property(prop));
        }
        protected override bool button_press_event(Gdk.EventButton event)
        {
            scale.event(event);
            if (!grabbed)
                grabbed = true;
            return true;
        }
        protected override bool button_release_event(Gdk.EventButton event)
        {
            scale.event (event);
            if (grabbed)
            {
                grabbed = false;
                this.grab_broken_event ((Gdk.EventGrabBroken)event);
            }
            return true;
        }
        protected override bool motion_notify_event(Gdk.EventMotion event)
        {
            scale.event (event);
            return true;
        }
        protected override bool scroll_event(Gdk.EventScroll event)
        {
            scale.event (event);
            return true;
        }
        protected override bool key_press_event(Gdk.EventKey event)
        {
            if(event.keyval == Gdk.Key.KP_Prior || event.keyval == Gdk.Key.KP_Next
               || event.keyval == Gdk.Key.Prior || event.keyval == Gdk.Key.Next
               || event.keyval == Gdk.Key.KP_Left || event.keyval == Gdk.Key.KP_Right
               || event.keyval == Gdk.Key.Left || event.keyval == Gdk.Key.Right)
            {
                scale.event (event);
                return true;
            }
            return false;
        }
    }
    public class GtkClient : Client
    {
        private unowned Gtk.MenuShell root_menu;
        public static Gtk.MenuItem new_item(Item item, bool show_im_pl = true)
        {
            if (item.get_string_property("type") == "separator")
                return new GtkSeparatorItem(item);
            else if (item.get_string_property("type") == "slider" || item.get_string_property("type") == "scale")
                return new GtkScaleItem(item);
            return new GtkMainItem(item,show_im_pl);
        }
        public GtkClient(string object_name, string object_path)
        {
            base(object_name,object_path);
            this.root_menu = null;
        }
        public void attach_to_menu(Gtk.MenuShell menu)
        {
            if (iface.icon_theme_path != null && iface.icon_theme_path.length > 0)
                foreach (unowned string path in iface.icon_theme_path)
                    IconTheme.get_default().prepend_search_path(path != null ? path : "");
            root_menu = menu;
            root_menu.foreach((c)=>{menu.remove(c); c.destroy();});
            root_menu.realize.connect(open_cb);
            root_menu.unrealize.connect(close_cb);
            get_root_item().child_added.connect(on_child_added_cb);
            get_root_item().child_moved.connect(on_child_moved_cb);
            get_root_item().child_removed.connect(on_child_removed_cb);
            foreach(unowned Item ch in get_root_item().get_children())
                on_child_added_cb(ch.id,ch);
            root_menu.show();
        }
        public void detach()
        {
            SignalHandler.disconnect_by_data(get_root_item(),this);
            if (root_menu != null)
                root_menu.foreach((c)=>{root_menu.remove(c); c.destroy();});
        }
        private void open_cb()
        {
            get_root_item().handle_event("opened",null,0);
            get_root_item().request_about_to_show();
            root_menu.queue_resize();
        }
        private void close_cb()
        {
            get_root_item().handle_event("closed",null,0);
        }
        private void on_child_added_cb(int id, Item item)
        {
            Gtk.MenuItem menuitem;
            menuitem = new_item(item,!(this.root_menu is Gtk.MenuBar));
            root_menu.insert(menuitem,get_root_item().get_child_position(item.id));
        }
        private void on_child_moved_cb(int oldpos, int newpos, Item item)
        {
            foreach(Widget ch in root_menu.get_children())
                if ((ch as GtkItemIface).item == item)
                {
                    root_menu.remove(ch);
                    root_menu.insert(ch,newpos);
                }
        }
        private void on_child_removed_cb(int id, Item item)
        {
            foreach(unowned Widget ch in root_menu.get_children())
                if ((ch as GtkItemIface).item == item)
                    ch.destroy();
        }
        public static bool check (string bus_name, string object_path)
        {
            Iface? iface = null;
            try
            {
                iface = Bus.get_proxy_sync(BusType.SESSION,bus_name,object_path);
                if (iface.version < 2)
                    return false;
                else
                    return true;
            } catch (Error e){}
            return false;
        }
    }
}
