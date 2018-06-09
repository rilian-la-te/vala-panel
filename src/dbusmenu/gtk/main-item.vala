using GLib;
using Gtk;

namespace DBusMenu
{
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
            construct;
            default = true;
        }
        private bool has_indicator = false;
        private unowned Image image;
        private unowned AccelLabel accel_label;
        private ulong activate_handler;
        private bool is_themed_icon = false;
        construct
        {
            this.item = item;
            var box = new Box(Orientation.HORIZONTAL, 5);
            var img = new Image();
            image = img;
            var label = new AccelLabel("");
            accel_label = label;
            box.pack_start(image,false,false,2);
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
        protected override void toggle_size_request(void* req)
        {
            if(has_indicator)
                base.toggle_size_request(req);
            else
            {
                int* req_int = req;
                *req_int = 0;
            }
        }
        protected override void toggle_size_allocate(int alloc)
        {
            base.toggle_size_allocate(has_indicator ? alloc : 0);
        }
        private void update_icon(Variant? val)
        {
            if (val == null)
            {
                var icon = image.gicon;
                if (has_indicator || (icon == null && !always_show_image_placeholder))
                    image.hide();
                else if (!(icon != null && icon is ThemedIcon && is_themed_icon))
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
            image.set_pixel_size(16);
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
}
