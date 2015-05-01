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

using ValaPanel;
using Gtk;
public class ButtonsApplet : AppletPlugin, Peas.ExtensionBase
{
    public Applet get_applet_widget(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    uint number)
    {
        return new Buttons(toplevel,settings,number);
    }
}
public class Buttons: Applet
{
    Button minimize;
    Button maximize;
    Button close;
    Box box;
    ulong handler;
    ulong state;
    public Buttons(ValaPanel.Toplevel toplevel,
                                    GLib.Settings? settings,
                                    uint number)
    {
        base(toplevel,settings,number);
    }
    public override void create()
    {
        Wnck.Screen.get_default().force_update();
        handler = Wnck.Screen.get_default().active_window_changed.connect(update_buttons_sensitivity);
        state = Wnck.Screen.get_default().get_active_window().state_changed.connect((m,n)=>{
            var image = maximize.get_image() as Gtk.Image;
            if (Wnck.Screen.get_default().get_active_window().is_maximized())
                image.set_from_icon_name("window-restore-symbolic",IconSize.MENU);
            else
                image.set_from_icon_name("window-maximize-symbolic",IconSize.MENU);
        });
        box = new Box(Orientation.HORIZONTAL,0);
        var settings = this.get_settings();
        settings.notify["gtk-decoration-layout"].connect(()=>{
            update_window_buttons(settings.gtk_decoration_layout);
        });
        update_window_buttons(settings.gtk_decoration_layout);
        this.add(box);
        this.show_all();
    }
    private void update_buttons_sensitivity(Wnck.Window? prev = null)
    {
        var window = Wnck.Screen.get_default().get_active_window();
        if (window == null)
        {
            minimize.sensitive = maximize.sensitive = close.sensitive = false;
            return;
        }
        var actions = window.get_actions();
        minimize.sensitive = ((actions & Wnck.WindowActions.MINIMIZE) > 0);
        maximize.sensitive = ((actions & Wnck.WindowActions.MAXIMIZE) > 0 ||
                              (actions & Wnck.WindowActions.UNMAXIMIZE) > 0);
        close.sensitive = ((actions & Wnck.WindowActions.CLOSE) > 0);
        if (state > 0 && prev != null)
            prev.disconnect(state);
        update_maximize_image();
        state = Wnck.Screen.get_default().get_active_window().state_changed.connect((m,n)=>{
            update_maximize_image();
        });
    }
    private void update_maximize_image()
    {
        if (maximize == null)
            return;
        var image = maximize.get_image() as Gtk.Image;
        if (Wnck.Screen.get_default().get_active_window().is_maximized())
            image.set_from_icon_name("window-restore-symbolic",IconSize.MENU);
        else
            image.set_from_icon_name("window-maximize-symbolic",IconSize.MENU);
    }
    private void update_window_buttons(string decoration_layout)
    {
        if (minimize != null)
        {
            minimize.destroy();
            minimize = null;
        }
        if (maximize != null)
        {
            maximize.destroy();
            maximize = null;
        }
        if (close != null)
        {
            close.destroy();
            close = null;
        }
        var tokens = decoration_layout.split(":",2);
        var window = Wnck.Screen.get_default().get_active_window();
        if (tokens == null)
            tokens = {"close,minimize,maximize","menu"};
        for (var i = 0; i < 2; i++)
        {
            if (tokens[i] == null)
                break;

            var t = tokens[i].split (",", -1);
            for (var j = 0; t[j] != null; j++)
            {
                if (t[j] == "minimize")
                {
                    var button = new Button();
                    button.set_valign(Align.CENTER);
                    button.get_style_context().add_class("titlebutton");
                    button.get_style_context().add_class("minimize");
                    setup_window_button(button,new ThemedIcon.with_default_fallbacks("window-minimize-symbolic"),null,this.toplevel);
                    button.can_focus = false;
                    button.show_all();
                    button.clicked.connect(()=>{
                        Wnck.Screen.get_default().get_active_window().minimize();
                    });
                    var accessible = button.get_accessible();
                    if (accessible is Gtk.Accessible)
                        accessible.set_name (dgettext("gtk30","Minimize"));
                    minimize = button;
                    box.add(minimize);
                }
                else if (t[j] == "maximize")
                {
                    var button = new Button();
                    bool max = window.is_maximized();
                    button.set_valign(Align.CENTER);
                    button.get_style_context().add_class("titlebutton");
                    button.get_style_context().add_class("maximize");
                    var icon = new ThemedIcon.with_default_fallbacks(max ? "window-restore-symbolic" :"window-maximize-symbolic");
                    setup_window_button(button,icon,null,this.toplevel);
                    button.can_focus = false;
                    button.show_all();
                    button.clicked.connect(()=>{
                        var win = Wnck.Screen.get_default().get_active_window();
                        if (win.is_maximized())
                            win.unmaximize();
                        else
                            win.maximize();
                    });
                    var accessible = button.get_accessible();
                    if (accessible is Gtk.Accessible)
                        accessible.set_name (max ? dgettext("gtk30","Restore") : dgettext("gtk30","Maximize"));
                    maximize = button;
                    box.add(maximize);
                }
                else if (t[j] == "close")
                {
                    var button = new Button();
                    button.set_valign(Align.CENTER);
                    button.get_style_context().add_class("titlebutton");
                    button.get_style_context().add_class("close");
                    setup_window_button(button,new ThemedIcon.with_default_fallbacks("window-close-symbolic"),null,this.toplevel);
                    button.can_focus = false;
                    button.show_all();
                    button.clicked.connect(()=>{
                        Wnck.Screen.get_default().get_active_window().close(get_current_event_time());
                    });
                    var accessible = button.get_accessible();
                    if (accessible is Gtk.Accessible)
                        accessible.set_name (dgettext("gtk30","Close"));
                    close = button;
                    box.add(close);
                }
            }
        }
        update_buttons_sensitivity();
    }
    private void setup_window_button(Button btn, Icon? icon = null, string? label = null, ValaPanel.Toplevel? top = null)
    {
        btn.relief = Gtk.ReliefStyle.NONE;
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
} // End class

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(ValaPanel.AppletPlugin), typeof(ButtonsApplet));
}
