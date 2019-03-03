using GLib;
using Gtk;
using ValaDBusMenu;

namespace ValaDBusMenu
{
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
            primary.show();
            scale.show();
            box.add(primary);
            box.add(scale);
            this.add(box);
            box.show();
            this.show();
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
}
