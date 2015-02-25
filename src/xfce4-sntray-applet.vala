using GLib;
using StatusNotifier;

public class StatusNotifierPlugin : Xfce.PanelPlugin {

    public override void @construct() {
        widget = new ItemBox();
        add(widget);
        add_action_widget(widget);
        widget.menu_position_func = (menu,out x,out y,out push)=>{Xfce.PanelPlugin.position_menu(menu, out x, out y, out push, this);};
	widget.icon_size = (int)this.size/(int)this.nrows - 2;
        widget.show_passive = true;
	widget.orientation = (this.mode != Xfce.PanelPluginMode.DESKBAR) ? Gtk.Orientation.VERTICAL : Gtk.Orientation.HORIZONTAL;
	this.width_request = -1;
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

    ItemBox widget;
}

[ModuleInit]
public Type xfce_panel_module_init (TypeModule module) {
    return typeof (StatusNotifierPlugin);
}
