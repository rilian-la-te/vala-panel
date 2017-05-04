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

using Gtk;
using Config;

namespace ValaPanel
{
    [CCode (cname = "PanelApplet")]
    public abstract class Applet : Gtk.Bin
    {
        private Dialog? dialog;
        const GLib.ActionEntry[] remove_entry =
        {
            {"menu",activate_menu,null,null,null},
            {"configure",activate_configure,null,null,null},
            {"remove",activate_remove,null,null,null}
        };
        public unowned Gtk.Widget background_widget {get; set;}
        public unowned ValaPanel.Toplevel toplevel {get; construct;}
        public unowned GLib.Settings? settings {get; construct;}
        public string uuid {get; construct;}
        public virtual void update_context_menu(ref GLib.Menu parent_menu){}
        public SimpleActionGroup grp {get; private set;}
        public Applet(ValaPanel.Toplevel top, GLib.Settings? s, string uuid)
        {
            Object(toplevel: top, settings: s, uuid: uuid);
        }
        construct
        {
            grp = new SimpleActionGroup();
            this.set_has_window(false);
            this.border_width = 0;
            this.button_release_event.connect((b)=>
            {
                if (b.button == 3 &&
                    ((b.state & Gtk.accelerator_get_default_mod_mask ()) == 0))
                {
                    toplevel.get_plugin_menu(this).popup_at_widget(this,Gdk.Gravity.NORTH, Gdk.Gravity.NORTH,b);
                    return true;
                }
                return false;
            });
            grp.add_action_entries(remove_entry,this);
            this.insert_action_group("applet",grp);
            var cnf = grp.lookup_action("configure") as SimpleAction;
            var mn = grp.lookup_action("menu") as SimpleAction;
            cnf.set_enabled(false);
            cnf.set_enabled(false);
            set_actions();
        }
        protected override void parent_set(Gtk.Widget? prev_parent)
        {
            if (prev_parent == null)
            {
                if (background_widget == null)
                    background_widget = this;
                init_background();
            }
        }
        public void init_background()
        {
            var color = Gdk.RGBA();
            color.parse ("transparent");
            PanelCSS.apply_with_class(background_widget,
                                      PanelCSS.generate_background(null,color),
                                      "-vala-panel-background",
                                      false);
        }
        private void activate_configure(SimpleAction act, Variant? param)
        {
            show_config_dialog();
        }
        protected virtual void activate_menu(SimpleAction act, Variant? param)
        {
        }
        public void show_config_dialog()
        {
            if (dialog == null)
            {
                var dlg = this.get_config_dialog();
                this.destroy.connect(()=>{dlg.response(Gtk.ResponseType.CLOSE);});
                dlg.set_transient_for(toplevel);
                dialog = dlg;
                dialog.hide.connect(()=>{dialog.destroy(); dialog = null;});
                dialog.response.connect(()=>{dialog.destroy(); dialog = null;});
            }
            dialog.present();
        }
        public bool is_configurable()
        {
        return grp.get_action_enabled("configure");
        }
        private void activate_remove(SimpleAction act, Variant? param)
        {
            /* If the configuration dialog is open, there will certainly be a crash if the
             * user manipulates the Configured Plugins list, after we remove this entry.
             * Close the configuration dialog if it is open. */
            if (toplevel.pref_dialog != null)
            {
                toplevel.pref_dialog.destroy();
                toplevel.pref_dialog = null;
            }
            toplevel.remove_applet(this);
        }
        private void measure(Orientation orient, int for_size, out int min, out int nat, out int base_min, out int base_nat)
        {
            if(toplevel.orientation != orient)
            {
                min = (int)toplevel.icon_size;
                nat = toplevel.height;
            }
            else
            {
                if (orient == Gtk.Orientation.HORIZONTAL)
                    base.get_preferred_width_for_height_internal(for_size, out min, out nat);
                else
                    base.get_preferred_height_for_width_internal(for_size, out min, out nat);
            }
            base_min=base_nat = -1;
        }

        protected override void get_preferred_height_for_width(int width,out int min, out int nat)
        {
            int x,y;
            measure(Orientation.VERTICAL,width,out min,out nat,out x, out y);
        }
        protected override void get_preferred_width_for_height(int height, out int min, out int nat)
        {
            int x,y;
            measure(Orientation.HORIZONTAL,height,out min,out nat,out x, out y);
        }
        protected override SizeRequestMode get_request_mode()
        {
            return (toplevel.orientation == Orientation.HORIZONTAL) ? SizeRequestMode.WIDTH_FOR_HEIGHT : SizeRequestMode.HEIGHT_FOR_WIDTH;
        }
        protected override void get_preferred_width(out int min, out int nat)
        {
            min = (int)toplevel.icon_size;
            nat = toplevel.height;
        }
        protected override void get_preferred_height(out int min, out int nat)
        {
            min = (int)toplevel.icon_size;
            nat = toplevel.height;
        }
        [CCode (returns_floating_reference = true)]
        public virtual Gtk.Dialog? get_config_dialog()
        {
            return null;
        }
        public virtual void set_actions()
        {

        }
    }
}
