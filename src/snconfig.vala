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
    [GtkTemplate (ui = "/org/vala-panel/sntray/snconfig.ui"), CCode (cname = "StatusNotifierConfig")]
    public class ConfigWidget : Box
    {
        private const int COLUMN_ID = 0;
        private const int COLUMN_NAME = 1;
        private const int COLUMN_OVERRIDE_INDEX = 2;
        private const int COLUMN_INDEX = 3;
        private const int COLUMN_OVERRIDE_VISIBLE = 4;
        private const int COLUMN_VISIBLE = 5;
        private const int COLUMN_ICON = 6;

        [GtkChild (name = "check-application")]
        CheckButton check_application;
        [GtkChild (name = "check-communications")]
        CheckButton check_communications;
        [GtkChild (name = "check-system")]
        CheckButton check_system;
        [GtkChild (name = "check-hardware")]
        CheckButton check_hardware;
        [GtkChild (name = "check-other")]
        CheckButton check_other;
        [GtkChild (name = "check-passive")]
        CheckButton check_passive;
        [GtkChild (name = "check-symbolic")]
        CheckButton check_symbolic;
        [GtkChild (name = "check-labels")]
        CheckButton check_labels;
        [GtkChild (name = "store")]
        Gtk.ListStore store;
        [GtkChild (name = "box-indicator")]
        Box box_indicator;
        [GtkChild (name = "scale-indicator")]
        Scale scale_indicator;
        unowned ItemBox layout;
        public bool configure_icon_size {get; set;}
        public static Gtk.Dialog get_config_dialog(ItemBox layout, bool configure_icon_size)
        {
            var widget = new ConfigWidget(layout);
            widget.configure_icon_size = configure_icon_size;
            var dlg = new Dialog();
            dlg.set_title(_("StatusNotifier Configuration"));
            dlg.get_content_area().add(widget);
            return dlg;
        }
        public ConfigWidget(ItemBox box)
        {
            layout = box;
            this.bind_property("configure-icon-size",box_indicator,"visible",BindingFlags.SYNC_CREATE);
            layout.bind_property(SHOW_APPS,check_application,"active",BindingFlags.BIDIRECTIONAL|BindingFlags.SYNC_CREATE);
            layout.bind_property(SHOW_COMM,check_communications,"active",BindingFlags.BIDIRECTIONAL|BindingFlags.SYNC_CREATE);
            layout.bind_property(SHOW_SYS,check_system,"active",BindingFlags.BIDIRECTIONAL|BindingFlags.SYNC_CREATE);
            layout.bind_property(SHOW_HARD,check_hardware,"active",BindingFlags.BIDIRECTIONAL|BindingFlags.SYNC_CREATE);
            layout.bind_property(SHOW_OTHER,check_other,"active",BindingFlags.BIDIRECTIONAL|BindingFlags.SYNC_CREATE);
            layout.bind_property(SHOW_PASSIVE,check_passive,"active",BindingFlags.BIDIRECTIONAL|BindingFlags.SYNC_CREATE);
            layout.bind_property(USE_SYMBOLIC,check_symbolic,"active",BindingFlags.BIDIRECTIONAL|BindingFlags.SYNC_CREATE);
            layout.bind_property(USE_LABELS,check_labels,"active",BindingFlags.BIDIRECTIONAL|BindingFlags.SYNC_CREATE);
            layout.bind_property(INDICATOR_SIZE,scale_indicator.adjustment,"value",BindingFlags.BIDIRECTIONAL|BindingFlags.SYNC_CREATE);
            layout.item_added.connect((id)=>{
                item_to_store(layout.items.lookup(id));
            });
            layout.item_removed.connect((id)=>{
                TreeIter iter;
                for(store.get_iter_first(out iter);store.iter_next(ref iter);)
                {
                    string inner_id;
                    store.get(iter, COLUMN_ID, out inner_id);
                    if (id == inner_id)
#if VALA_0_36
                        store.remove(ref iter);
#else
                        store.remove(iter);
#endif
                }
            });
            build_stores();
        }
        private void build_stores()
        {
            layout.items.foreach((k,v)=>{
                item_to_store(v);
            });
        }
        private void item_to_store(Item v)
        {
            unowned string name = v.title;
            unowned string id = v.id;
            Icon icon = v.icon;
            var over_index = layout.index_override.contains(v.id);
            var index = layout.get_index(v);
            var over_filter = layout.filter_override.contains(v.id);
            bool filter = layout.filter_cb(v);
            TreeIter iter;
            store.append(out iter);
            store.set(iter,COLUMN_ID,id,COLUMN_NAME,name,COLUMN_OVERRIDE_INDEX,over_index,COLUMN_INDEX,index.to_string(),
                                                         COLUMN_OVERRIDE_VISIBLE,over_filter,COLUMN_VISIBLE,filter,
                                                         COLUMN_ICON,icon);
        }
        private void layout_notify_by_pspec(string prop)
        {
            Type type = typeof (ItemBox);
            ObjectClass ocl = (ObjectClass) type.class_ref ();
            unowned ParamSpec? spec = ocl.find_property (prop);
            layout.notify(spec);
        }
        [GtkCallback]
        private void on_index_override(string path)
        {
            TreeIter iter;
            store.get_iter_from_string(out iter, path);
            bool over;
            string id;
            store.get(iter,COLUMN_ID,out id, COLUMN_OVERRIDE_INDEX, out over);
            over = !over;
            var index = layout.get_index(layout.get_item_by_id(id));
            if (over)
            {
                store.set(iter,COLUMN_INDEX,"%d".printf(index));
                layout.index_override.insert(id,new Variant.int32(index));
            }
            else
            {
                layout.index_override.remove(id);
                var new_index = layout.get_index(layout.get_item_by_id(id));
                store.set(iter,COLUMN_INDEX,"%d".printf(new_index));
            }
            store.set(iter,COLUMN_OVERRIDE_INDEX,over);
            var over_dict = layout.index_override;
            layout.index_override = over_dict;
        }
        [GtkCallback]
        private void on_filter_override(string path)
        {
            TreeIter iter;
            store.get_iter_from_string(out iter, path);
            bool over;
            string id;
            store.get(iter,COLUMN_ID,out id, COLUMN_OVERRIDE_VISIBLE, out over);
            over = !over;
            var filter = layout.filter_cb(layout.get_item_by_id(id));
            if (over)
            {
                store.set(iter,COLUMN_VISIBLE,filter);
                layout.filter_override.insert(id,new Variant.boolean(filter));
            }
            else
            {
                layout.filter_override.remove(id);
                var new_filter = layout.filter_cb(layout.get_item_by_id(id));
                store.set(iter,COLUMN_VISIBLE,new_filter);
            }
            store.set(iter,COLUMN_OVERRIDE_VISIBLE,over);
            var over_dict = layout.filter_override;
            layout.filter_override = over_dict;
        }
        [GtkCallback]
        private void on_index_index(string path, string val)
        {
            TreeIter iter;
            store.get_iter_from_string(out iter, path);
            string id;
            store.get(iter,COLUMN_ID,out id);
            store.set(iter,COLUMN_INDEX,val);
            layout.index_override.insert(id,new Variant.int32(int.parse(val)));
            layout_notify_by_pspec("index-override");
        }
        [GtkCallback]
        private void on_filter_visible(string path)
        {
            TreeIter iter;
            store.get_iter_from_string(out iter, path);
            bool over;
            string id;
            store.get(iter,COLUMN_ID,out id, COLUMN_VISIBLE, out over);
            over = !over;
            store.set(iter,COLUMN_VISIBLE,over);
            layout.filter_override.insert(id,new Variant.boolean(over));
            layout_notify_by_pspec("filter-override");
        }
    }
}
