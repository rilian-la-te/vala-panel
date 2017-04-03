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

using GLib;
using Gtk;

namespace LaunchBar
{
    [GtkTemplate (ui = "/org/vala-panel/launchbar/config.ui"), CCode (cname = "LaunchBarConfig")]
    public class ConfigDialog : Dialog
    {
        Bar launchbar;
        [GtkChild (name = "liststore-currentitems")]
        Gtk.ListStore current_items;
        [GtkChild (name = "selection-items")]
        Gtk.TreeSelection selection_items;
        [GtkChild (name = "button-add")]
        MenuButton button_add;
        [GtkChild (name = "choose-desktop")]
        AppChooserWidget choose_desktop;
        [GtkChild (name = "choose-file")]
        FileChooserWidget choose_file;
        [GtkChild (name = "box-popover")]
        Box box_popover;
        public ConfigDialog(Bar launchbar)
        {
            ValaPanel.apply_window_icon(this);
            TreeIter iter;
            this.launchbar = launchbar;
            foreach(var id in launchbar.ids)
            {
                current_items.append(out iter);
                current_items.set(iter, 0, id,
                                        1, launchbar.get_icon_from_id(id),
                                        2, launchbar.get_display_name_from_id(id));
            }
            launchbar.settings.changed[BUTTONS].connect(check_widgets_from_ids);
            var popover = new Popover(button_add);
            popover.add(box_popover);
            popover.set_size_request(760,360);
            button_add.popover = popover;
            box_popover.show();
            this.show_all();
        }
        private void add_uri(string uri, bool show_err = true)
        {
            if (!(uri in launchbar.ids))
            {
                launchbar.ids += uri;
                launchbar.commit_ids();
                return;
            }
            else if (show_err)
                show_error(_("Quicklaunch already contains this URI.\n"));
        }
        [GtkCallback]
        private void on_file_activated()
        {
                var uri = choose_file.get_uri();
                add_uri(uri);
        }
        [GtkCallback]
        private void on_application_activated()
        {
            var info = choose_desktop.get_app_info();
            if (info != null)
            {
                unowned string id = info.get_id();
                if (id == null)
                    id = (info as DesktopAppInfo).get_filename();
                if (!(id in launchbar.ids))
                {
                    launchbar.ids += id;
                    launchbar.commit_ids();
                }
                else
                    show_error(_("Quicklaunch already contains this application.\n"));
            }
        }
        [GtkCallback]
        private void on_remove_button_clicked()
        {
            TreeIter sel_iter;
            if(selection_items.get_selected(null, out sel_iter))
            {
                string id;
                current_items.get(sel_iter,0,out id);
                launchbar.request_remove_id(id);
                launchbar.commit_ids();
            }
        }
        [GtkCallback]
        private void on_up_button_clicked()
        {
            TreeIter sel_iter;
            TreeIter? prev_iter = null;
            if(selection_items.get_selected(null, out sel_iter))
            {
                TreePath path = current_items.get_path(sel_iter);
                if(path.prev() && current_items.get_iter(out prev_iter,path))
                {
                    current_items.swap(sel_iter, prev_iter);
                    update_ids_from_widget();
                }
            }
        }
        [GtkCallback]
        private void on_down_button_clicked()
        {
            TreeIter sel_iter;
            TreeIter? prev_iter = null;
            if(selection_items.get_selected(null, out sel_iter))
            {
                prev_iter = sel_iter;
                if(current_items.iter_next(ref prev_iter))
                {
                    current_items.swap(sel_iter, prev_iter);
                    update_ids_from_widget();
                }
            }
        }
        [GtkCallback]
        private void on_add_all_files_clicked()
        {
            var uri = choose_file.get_current_folder_uri();
            var path = choose_file.get_current_folder();
            if (uri == null || path == null)
                return;
            try
            {
                Dir dir = Dir.open(path);
                for (var ch = dir.read_name(); ch!= null; ch = dir.read_name())
                {
                        var ch_uri = Filename.to_uri(path+"/"+ch);
                        add_uri(ch_uri,false);
                }
                return;
            } catch (GLib.Error e)
            {
                print("%s %s\n",e.message, path);
                show_error(_("Failed to add directory content.\n Adding current directory as launcher."));
                add_uri(uri);
            }
        }
        private void update_ids_from_widget()
        {
            launchbar.ids = populate_ids_from_widget();
            launchbar.commit_ids();
        }
        private string[] populate_ids_from_widget()
        {
            string[] ids = {};
            current_items.foreach((model,path,iter)=>{
                string id;
                model.get(iter,0,out id);
                ids += id;
                return false;
            });
            return ids;
        }
        private void check_widgets_from_ids()
        {
            var rows_num = current_items.iter_n_children(null);
            var len = launchbar.ids.length;
            TreeIter iter;
            if (rows_num < len)
                for(int i = 0; i < len - rows_num; i++)
                    current_items.append(out iter);
            else if (rows_num > len)
            {
                current_items.iter_nth_child(out iter, null, len);
#if VALA_0_36
                while(current_items.remove(ref iter));
#else
                while(current_items.remove(iter));
#endif
            }
            for (int i = 0; i < len; i++)
            {
                current_items.iter_nth_child(out iter, null, i);
                string id;
                current_items.get(iter,0,out id);
                if (id != launchbar.ids[i])
                    current_items.set(iter,0,launchbar.ids[i],
                                           1,launchbar.get_icon_from_id(launchbar.ids[i]),
                                           2,launchbar.get_display_name_from_id(launchbar.ids[i]));
            }
        }
        private void show_error(string error_i18n)
        {
            var msg = new MessageDialog
                    (this,
                     DialogFlags.DESTROY_WITH_PARENT,
                     MessageType.ERROR,ButtonsType.CLOSE,
                     error_i18n);
            ValaPanel.apply_window_icon(msg as Gtk.Window);
            msg.set_title(_("Error"));
            msg.run();
            msg.destroy();
            return;
        }
    }
}
