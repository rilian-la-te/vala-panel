/*
 * xfce4-sntray-plugin
 * Copyright (C) 2015-2017 Konstantin Pugin <ria.freelander@gmail.com>
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

namespace StatusNotifier
{
    public class Item : FlowBoxChild
    {
        public ObjectPath object_path {private get; internal construct;}
        public string object_name {private get; internal construct;}
        public Status status {get; private set;}
        public uint ordering_index {get; private set;}
        public Category cat {get; private set;}
        public string id {get; private set;}
        internal string title {get; private set;}
        internal Icon icon
        {owned get {
            return proxy.icon;
            }
        }
        Label label;
        Image image;
        EventBox ebox;
        ValaDBusMenu.GtkClient? client;
        Gtk.Menu menu;
        StatusNotifier.Proxy proxy;
        public Item (string n, ObjectPath p)
        {
            Object(object_path: p, object_name: n);
        }
        public override void destroy()
        {
            if (menu != null)
                menu.destroy();
            if (client != null)
                client = null;
            base.destroy();
        }
        construct
        {
            unowned StyleContext context = this.get_style_context();
            this.reset_style();
            var provider = new Gtk.CssProvider();
            provider.load_from_resource("/org/vala-panel/sntray/style.css");
            context.add_provider(provider,STYLE_PROVIDER_PRIORITY_APPLICATION);
            context.add_class("-panel-launch-button");
            proxy = new StatusNotifier.Proxy(object_name,object_path);
            client = null;
            this.has_tooltip = true;
            ebox = new EventBox();
            var box = new Box(Orientation.HORIZONTAL,0);
            label = new Label(null);
            image = new Image();
            box.add(image);
            image.valign = Align.CENTER;
            image.show();
            box.add(label);
            label.valign = Align.CENTER;
            label.show();
            ebox.add(box);
            box.show();
            this.add(ebox);
            ebox.add_events(Gdk.EventMask.SCROLL_MASK);
            ebox.scroll_event.connect((e)=>{
                switch (e.direction)
                {
                    case Gdk.ScrollDirection.LEFT:
                        proxy.scroll(-120, 0);
                        break;
                    case Gdk.ScrollDirection.RIGHT:
                        proxy.scroll(120, 0);
                        break;
                    case Gdk.ScrollDirection.DOWN:
                        proxy.scroll(0, -120);
                        break;
                    case Gdk.ScrollDirection.UP:
                        proxy.scroll(0, 120);
                    break;
                    case Gdk.ScrollDirection.SMOOTH:
                        double dx,dy;
                        e.get_scroll_deltas(out dx, out dy);
                        var x = (int) Math.round(dx);
                        var y = (int) Math.round(dy);
                        proxy.scroll(x,y);
                        break;
                }
                return false;
            });
            ebox.button_release_event.connect(button_press_event_cb);
            ebox.enter_notify_event.connect((e)=>{
                this.get_style_context().add_class("-panel-launch-button-selected");
                return false;
            });
            ebox.leave_notify_event.connect((e)=>{
                this.get_style_context().remove_class("-panel-launch-button-selected");
                return false;
            });
            this.query_tooltip.connect(query_tooltip_cb);
            this.popup_menu.connect(context_menu);
            this.parent_set.connect((prev)=>{
                if (get_applet() != null)
                {
                    get_applet().bind_property(INDICATOR_SIZE,image,"pixel-size",BindingFlags.SYNC_CREATE);
                    get_applet().bind_property(INDICATOR_SIZE,proxy,"icon-size",BindingFlags.SYNC_CREATE);
                    get_applet().bind_property(USE_SYMBOLIC,proxy,"use-symbolic",BindingFlags.SYNC_CREATE);
                    get_applet().bind_property(USE_LABELS,label,"visible",BindingFlags.SYNC_CREATE);
                }
            });
            ebox.show();
            proxy.initialized.connect(()=>{
                init_proxy();
            });
            proxy.start();
        }
        private void init_proxy()
        {
            if (proxy.menu != null)
                setup_inner_menu();
            title = proxy.title;
            this.ordering_index = proxy.x_ayatana_ordering_index;
            this.cat = proxy.category;
            this.id = proxy.id;
            this.title = proxy.title;
            iface_new_status_cb();
            proxy.notify.connect((pspec)=>{
                if(pspec.name == "status")
                    iface_new_status_cb();
                if(pspec.name == "icon")
                    iface_new_icon_cb();
                if(pspec.name == "tooltip-text" || pspec.name == "tooltip-title")
                    this.trigger_tooltip_query();
                if(pspec.name == "x-ayatana-label" || pspec.name == "x-ayatana-label-guide")
                    this.label.set_text(proxy.x_ayatana_label);             /* FIXME: Guided labels */
            });
            this.changed();
            this.show();
            get_applet().item_added(object_name+(string)object_path);
        }
        private bool button_press_event_cb(Gdk.EventButton e)
        {
                if (e.button == 3)
                {
                    proxy.activate((int)Math.round(e.x_root),(int)Math.round(e.y_root));
                    return true;
                }
                else if (e.button == 2)
                {
                    try
                    {
                        proxy.ayatana_secondary_activate(e.time);
                        return true;
                    } catch (Error e){/* This only means than method not supported*/}
                    try
                    {
                        proxy.secondary_activate((int)Math.round(e.x_root),(int)Math.round(e.y_root));
                        return true;
                    }
                    catch (Error e) {stderr.printf("%s\n",e.message);}
                }
                return false;
        }
        private void iface_new_status_cb()
        {
            this.label.set_text(proxy.x_ayatana_label);
            iface_new_icon_cb();
            this.trigger_tooltip_query();
            this.status = proxy.status;
            switch(this.status)
            {
                case Status.PASSIVE:
                case Status.ACTIVE:
                    this.get_style_context().remove_class(STYLE_CLASS_NEEDS_ATTENTION);
                    break;
                case Status.ATTENTION:
                    this.get_style_context().add_class(STYLE_CLASS_NEEDS_ATTENTION);
                    break;
            }
        }
        private bool query_tooltip_cb(int x, int y, bool keyboard, Tooltip tip)
        {
            tip.set_icon_from_gicon(proxy.tooltip_icon,IconSize.DIALOG);
            tip.set_markup(proxy.tooltip_text);
            return true;
        }
        private unowned ItemBox get_applet()
        {
            return this.get_parent() as ItemBox;
        }
        private void setup_inner_menu()
        {
            menu = new Gtk.Menu();
            menu.attach_to_widget(this,null);
            menu.vexpand = true;
            /*FIXME: MenuModel support */
            /* if (client == null && remote_menu_model == null)
            {
                use_menumodel = !ValaDBusMenu.GtkClient.check(object_name,iface.menu);
                if (use_menumodel)
                {
                    try
                    {
                        var connection = Bus.get_sync(BusType.SESSION);
                        remote_action_group = DBusActionGroup.get(connection,object_name,iface.x_valapanel_action_group);
                        remote_menu_model = DBusMenuModel.get(connection,object_name,iface.menu);
                        this.insert_action_group("indicator",remote_action_group);
                    } catch (Error e) {stderr.printf("Cannot create GMenuModel: %s",e.message);}
                }
                else */
                {
                    client = new ValaDBusMenu.GtkClient(object_name,proxy.menu);
                    client.attach_to_menu(menu);
                }
            }

 //       }
        public bool context_menu()
        {
            int x,y;
            if (proxy.menu != null)
            {
                menu.hide.connect(()=>{(this.get_parent() as FlowBox).unselect_child(this);});
                menu.popup_at_widget(get_applet(),Gdk.Gravity.NORTH,Gdk.Gravity.NORTH,null);
                menu.reposition();
                return true;
            }
            ebox.get_window().get_origin(out x, out y);
            proxy.context_menu(x,y);
            return true;
        }
        private void iface_new_icon_cb()
        {
            var icon = proxy.icon;
            if(icon != null)
            {
                if(icon is Gdk.Pixbuf)
                    image.set_from_pixbuf(icon as Gdk.Pixbuf);
                else
                    image.set_from_gicon(icon,Gtk.IconSize.INVALID);
                image.show();
            }
            else
                image.hide();
        }
    }
}
