using GLib;
using Gtk;

namespace XEmbed
{
    public class Socket : Gtk.Socket
    {
        public X.Window window {get
             {return (X.Window)uint_window;}
             set {uint_window = (uint32)value;}}
        public uint32 uint_window {internal get; internal set construct;}
        public int icon_size {get; set; default = 16;}
        private string window_name;
        public Socket(Gdk.Screen screen, uint32 window)
        {
            Object(uint_window: window);
            this.set_has_window(true);
            this.notify["icon-size"].connect(()=>{
                this.queue_resize();
            });
        }
        protected override void realize()
        {
            Gdk.RGBA transparent = { 0, 0, 0, 0 };
            Gdk.Window window = this.get_window();
            base.realize();
            if (window is Gdk.Window)
            {
                window.set_background_rgba(transparent);
                window.set_composited(true);
            }
            this.set_app_paintable(true);
        }
        protected override void size_allocate(Allocation alloc)
        {
            Allocation prev_alloc;
            this.get_allocation(out prev_alloc);
            alloc.height = icon_size;
            alloc.width = icon_size;
            var moved = alloc.x != prev_alloc.x || alloc.y != prev_alloc.y;
            var resized = alloc.width != prev_alloc.width || alloc.height != prev_alloc.height;
            if ((moved || resized) && this.get_mapped())
                this.get_window().get_parent().invalidate_rect((Gdk.Rectangle)prev_alloc,false);
            base.size_allocate(alloc);
            if ((moved || resized) && this.get_mapped())
                this.get_window().get_parent().invalidate_rect((Gdk.Rectangle)prev_alloc,false);
        }
        protected override bool draw(Cairo.Context cr)
        {
            Gtk.Allocation allocation;
            this.get_allocation (out allocation);
            cr.save ();
            Gdk.cairo_set_source_window (cr,
                                    this.get_window(),
                                    allocation.x,
                                    allocation.y);
            cr.rectangle (allocation.x, allocation.y, allocation.width, allocation.height);
            cr.clip ();
            cr.paint ();
            cr.restore ();
            return base.draw(cr);
        }
        private string? get_name_prop(string prop_name, string type_name)
        {
            X.Atom type;
            int format;
            ulong nitems;
            ulong bytes_after;
            void* val_void;
            string val;
            string? name = null;
            var display = this.get_display() as Gdk.X11.Display;
            var req_type = Gdk.X11.get_xatom_by_name_for_display(display,type_name);
            Gdk.error_trap_push();
            var result = display.get_xdisplay().get_window_property(window,
                                                                    Gdk.X11.get_xatom_by_name_for_display(display,prop_name),
                                                                    0,long.MAX, false, req_type,
                                                                    out type, out format, out nitems,
                                                                    out bytes_after, out val_void);
            val = (string)val_void;
            if (Gdk.error_trap_pop() != 0 || result != 0 || val == null)
                return null;
            /* check the returned data */
            if (type == req_type && format == 8 && nitems > 0 && val.validate((ssize_t)nitems))
                /* lowercase the result */
                name = val.down((ssize_t)nitems);
            X.free ((void*)val);
            return name;
        }
        public unowned string get_name()
        {
            if (window_name != null)
                return window_name;
            /* try _NET_WM_NAME first, for gtk icon implementations, fall back to
            * WM_NAME for qt icons */
            window_name = get_name_prop("_NET_WM_NAME", "UTF8_STRING");
            if (window_name == null)
                window_name = get_name_prop("WM_NAME", "STRING");
            return window_name;
        }
    }
}
