using GLib;
using Gtk;

namespace ValaPanel
{
    internal static string user_config_file_name(string name1, string profile, string? name2)
    {
        return GLib.Path.build_filename(GLib.Environment.get_user_config_dir(),
                                Config.GETTEXT_PACKAGE,profile,name1,name2);
    }
    public static void apply_window_icon(Window w)
    {
        try{
            var icon = new Gdk.Pixbuf.from_resource("/org/vala-panel/lib/panel.png");
            w.set_icon(icon);
        } catch (Error e)
        {
            stderr.printf("Unable to load icon: %s. Trying fallback...\n",e.message);
            w.set_icon_name("start-here-symbolic");
        }
    }
    public void activate_panel_preferences(SimpleAction simple, Variant? param, void* data)
    {
        Gtk.Application app = data as Gtk.Application;
        foreach(var win in app.get_windows())
        {
            if (win is Toplevel)
            {
                var p = win as Toplevel;
                if (p.panel_name == param.get_string())
                {
                    p.configure("position");
                    break;
                }
            }
            stderr.printf("No panel with this name found.\n");
        }
    }
    public void activate_menu(SimpleAction simple, Variant? param, void* data)
    {
        Gtk.Application app = data as Gtk.Application;
        foreach(var win in app.get_windows())
        {
            if (win is Toplevel)
            {
                var p = win as Toplevel;
                foreach(var pl in p.get_applets_list())
                {
                    if (pl is AppletMenu)
                        (pl as AppletMenu).show_system_menu();
                }
            }
        }
    }
    public static void start_panels_from_dir(Gtk.Application app, string dirname)
    {
        Dir dir;
        try
        {
            dir = Dir.open(dirname,0);
        } catch (FileError e)
        {
            stdout.printf("Cannot load directory: %s\n",e.message);
            return;
        }
        string? name;
        while ((name = dir.read_name()) != null)
        {
            string cfg = GLib.Path.build_filename(dirname,name);
            if (!(cfg.contains("~") && cfg[0] !='.'))
            {
                var panel = Toplevel.load(app,cfg,name);
                if (panel != null)
                    app.add_window(panel);
            }
        }
    }
    public static void settings_as_action(ActionMap map, GLib.Settings settings, string prop)
    {
        settings.bind(prop,map,prop,SettingsBindFlags.GET|SettingsBindFlags.SET|SettingsBindFlags.DEFAULT);
        var action = settings.create_action(prop);
        map.add_action(action);
    }
    public static void settings_bind(Object map, GLib.Settings settings, string prop)
    {
        settings.bind(prop,map,prop,SettingsBindFlags.GET|SettingsBindFlags.SET|SettingsBindFlags.DEFAULT);
    }

    public static void setup_button(Button b, Image? img = null, string? label = null)
    {
        PanelCSS.apply_from_resource(b,"/org/vala-panel/lib/style.css","-panel-button");
/* Children hierarhy: button => alignment => box => (label,image) */
        b.notify.connect((a,b)=>{
            if (b.name == "label" || b.name == "image")
            {
                var B = a as Bin;
                var w = B.get_child();
                if (w is Container)
                {
                    Bin? bin;
                    Widget ch;
                    if (w is Bin)
                    {
                        bin = w as Bin;
                        ch = bin.get_child();
                    }
                    else
                        ch = w;
                    if (ch is Container)
                    {
                        var cont = ch as Container;
                        cont.forall((c)=>{
                            if (c is Widget){
                            c.set_halign(Gtk.Align.FILL);
                            c.set_valign(Gtk.Align.FILL);
                            }});
                    }
                    ch.set_halign(Gtk.Align.FILL);
                    ch.set_valign(Gtk.Align.FILL);
                }
            }
        });
        if (img != null)
        {
            b.set_image(img);
            b.set_always_show_image(true);
        }
        if (label != null)
            b.set_label(label);
        b.set_relief(Gtk.ReliefStyle.NONE);
    }
    public static void setup_icon(Image img, Icon icon, Toplevel? top = null, int size = -1)
    {
        img.set_from_gicon(icon,IconSize.INVALID);
        if (top != null)
            top.bind_property(Key.ICON_SIZE,img,"pixel-size",BindingFlags.DEFAULT|BindingFlags.SYNC_CREATE);
        else if (size > 0)
            img.set_pixel_size(size);
        Gtk.IconTheme.get_default().changed.connect(()=>{
            Icon i;
            img.get_gicon(out i, IconSize.INVALID);
            img.set_from_gicon(i,IconSize.INVALID);
        });
    }
    public static void setup_icon_button(Button btn, Icon? icon = null, string? label = null, Toplevel? top = null)
    {
        PanelCSS.apply_from_resource(btn,"/org/vala-panel/lib/style.css","-panel-icon-button");
        PanelCSS.apply_with_class(btn,"",Gtk.STYLE_CLASS_BUTTON,false);
        Image? img = null;
        if (icon != null)
        {
            img = new Image.from_gicon(icon,IconSize.INVALID);
            setup_icon(img,icon,top);
        }
        setup_button(btn, img, label);
        btn.set_border_width(0);
        btn.set_can_focus(false);
        btn.set_has_window(false);
    }
    /* Draw text into a label, with the user preference color and optionally bold. */
    public static void setup_label(Label label, string text, bool bold, double factor)
    {
        label.set_text(text);
        var css = PanelCSS.generate_font_label(factor,bold);
        PanelCSS.apply_with_class(label as Widget,css,"-vala-panel-font-label",true);
    }
    public static void scale_button_set_range(ScaleButton b, int lower, int upper)
    {
        var a = b.get_adjustment();
        a.set_lower(lower);
        a.set_upper(upper);
        a.set_step_increment(1);
        a.set_page_increment(5);
    }
    public static void scale_button_set_value_labeled(ScaleButton b, int val)
    {
        b.set_value(val);
        b.set_label("%d".printf(val));
    }
}
