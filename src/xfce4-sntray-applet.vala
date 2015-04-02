using GLib;
using StatusNotifier;
using Xfce;

public class StatusNotifierPlugin : Xfce.PanelPlugin {

    public override void @construct() {
        GLib.Intl.setlocale(LocaleCategory.CTYPE,"");
        GLib.Intl.bindtextdomain(Config.GETTEXT_PACKAGE,Config.LOCALE_DIR);
        GLib.Intl.bind_textdomain_codeset(Config.GETTEXT_PACKAGE,"UTF-8");
        GLib.Intl.textdomain(Config.GETTEXT_PACKAGE);
        widget = new ItemBox();
        add(widget);
        add_action_widget(widget);
        widget.menu_position_func = (menu,ref x,ref y,out push)=>{Xfce.PanelPlugin.position_menu(menu, ref x, ref y, out push, this);};
        widget.icon_size = (int)this.size/(int)this.nrows - 2;
        widget.show_passive = true;
        widget.orientation = (this.mode != Xfce.PanelPluginMode.DESKBAR) ? Gtk.Orientation.VERTICAL : Gtk.Orientation.HORIZONTAL;
        this.width_request = -1;
        try{
            Xfconf.init();
            wrapper = new ItemBoxWrapper(widget);
            channel = this.get_channel();
            Xfconf.Property.bind(channel,this.get_property_base()+"/"+SHOW_APPS,typeof(bool),widget,SHOW_APPS);
            Xfconf.Property.bind(channel,this.get_property_base()+"/"+SHOW_COMM,typeof(bool),widget,SHOW_COMM);
            Xfconf.Property.bind(channel,this.get_property_base()+"/"+SHOW_SYS,typeof(bool),widget,SHOW_SYS);
            Xfconf.Property.bind(channel,this.get_property_base()+"/"+SHOW_HARD,typeof(bool),widget,SHOW_HARD);
            Xfconf.Property.bind(channel,this.get_property_base()+"/"+SHOW_OTHER,typeof(bool),widget,SHOW_OTHER);
            Xfconf.Property.bind(channel,this.get_property_base()+"/"+SHOW_PASSIVE,typeof(bool),widget,SHOW_PASSIVE);
            Xfconf.Property.bind(channel,this.get_property_base()+"/"+USE_SYMBOLIC,typeof(bool),widget,USE_SYMBOLIC);
            Xfconf.Property.bind(channel,this.get_property_base()+"/"+INDEX_OVERRIDE,typeof(string),wrapper,INDEX_OVERRIDE);
            Xfconf.Property.bind(channel,this.get_property_base()+"/"+FILTER_OVERRIDE,typeof(string),wrapper,FILTER_OVERRIDE);
            this.menu_show_configure();
        } catch (Xfconf.Error e) {
            stderr.printf("Xfconf init failed. Configuration will not be saved.\n");
        }
        this.notify.connect((pspec)=>{
            if (pspec.name == "mode")
                 widget.orientation = (this.mode != Xfce.PanelPluginMode.DESKBAR) ? Gtk.Orientation.VERTICAL : Gtk.Orientation.HORIZONTAL;
            if (pspec.name == "size" || pspec.name == "nrows") {
                 widget.icon_size = (int)this.size/(int)this.nrows - 8;
                 this.width_request = -1;
            }
        });
        this.shrink = true;
        widget.show_all();
    }
    public override void configure_plugin()
    {
        var dlg = new ConfigDialog(widget);
        dlg.configure_icon_size = false;
        dlg.present();
    }
    Xfconf.Channel channel;
    ItemBox widget;
    ItemBoxWrapper wrapper;
}

internal class ItemBoxWrapper: Object
{
    internal string index_override
    {
        owned get {
            return hashtable_to_string(layout.index_override);
        }
        set {
            layout.index_override = string_to_hashtable(value);
        }
    }
    internal string filter_override
    {
        owned get {
            return hashtable_to_string(layout.filter_override);
        }
        set {
            layout.filter_override = string_to_hashtable(value);
        }
    }
    internal ItemBoxWrapper(ItemBox box)
    {
        this.layout = box;
        layout.notify.connect((pspec)=>{
            if (pspec.name == INDEX_OVERRIDE)
                this.notify_property(INDEX_OVERRIDE);
            if (pspec.name == FILTER_OVERRIDE)
                this.notify_property(FILTER_OVERRIDE);
        });
    }
    ItemBox layout;
    private string hashtable_to_string(HashTable<string,Variant?> table)
    {
        var builder = new VariantBuilder(VariantType.VARDICT);
        table.foreach((k,v)=>{
            builder.add("{sv}",k,v);
        });
        var val = builder.end();
        return val.print(false);
    }
    private HashTable<string,Variant?> string_to_hashtable(string str)
    {
        Variant variant;
        try
        {
            variant = Variant.parse(VariantType.VARDICT,str);
            var iter = variant.iterator();
            string name;
            Variant inner_val;
            var dict = new HashTable<string,Variant?>(str_hash,str_equal);
            while(iter.next("{sv}",out name, out inner_val))
                dict.insert(name,inner_val);
            return dict;
        } catch (Error e) {
            stderr.printf("Cannot convert string\n");
        }
        return new HashTable<string,Variant?>(str_hash,str_equal);
    }
}

[ModuleInit]
public Type xfce_panel_module_init (TypeModule module) {
    return typeof (StatusNotifierPlugin);
}
