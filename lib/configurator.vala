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

namespace ValaPanel
{
    internal enum Column
    {
        NAME,
        EXPAND,
        DATA
    }
    [GtkTemplate (ui = "/org/vala-panel/lib/pref.ui")]
    [CCode (cname="ConfigureDialog")]
    internal class ConfigureDialog : Dialog
    {
        public unowned Toplevel toplevel {get; construct;}
        [GtkChild (name="edge-button")]
        MenuButton edge_button;
        [GtkChild (name="alignment-button")]
        MenuButton alignment_button;
        [GtkChild (name="monitors-button")]
        MenuButton monitors_button;
        [GtkChild (name="spin-margin")]
        SpinButton spin_margin;
        [GtkChild (name="spin-iconsize")]
        SpinButton spin_iconsize;
        [GtkChild (name="spin-height")]
        SpinButton spin_height;
        [GtkChild (name="spin-width")]
        SpinButton spin_width;
        [GtkChild (name="spin-corners")]
        SpinButton spin_corners;
        [GtkChild (name="font-selector")]
        FontButton font_selector;
        [GtkChild (name="font-box")]
        Box font_box;
        [GtkChild (name="color-background")]
        ColorButton color_background;
        [GtkChild (name="color-foreground")]
        ColorButton color_foreground;
        [GtkChild (name="chooser-background")]
        FileChooserButton file_background;
        [GtkChild (name="plugin-list")]
        TreeView plugin_list;
        [GtkChild (name="plugin-desc")]
        Label plugin_desc;
        [GtkChild (name="add-button")]
        Button adding_button;
        [GtkChild (name="configure-button")]
        Button configure_button;
        [GtkChild (name="prefs")]
        internal Stack prefs_stack;

        const GLib.ActionEntry[] entries_monitor =
        {
            {"configure-monitors", null,"i","-2" ,state_configure_monitor}
        };

        internal ConfigureDialog(Toplevel top)
        {
            Object(toplevel: top, application: top.get_application(),window_position: WindowPosition.CENTER);
        }
        construct
        {
            var color = Gdk.RGBA();
            var conf = new SimpleActionGroup();
            apply_window_icon(this as Window);
            /* edge */
            edge_button.set_relief(ReliefStyle.NONE);
            switch(toplevel.edge)
            {
                case PositionType.TOP:
                    edge_button.set_label(_("Top"));
                    break;
                case PositionType.BOTTOM:
                    edge_button.set_label(_("Bottom"));
                    break;
                case PositionType.LEFT:
                    edge_button.set_label(_("Left"));
                    break;
                case PositionType.RIGHT:
                    edge_button.set_label(_("Right"));
                    break;
            }
            toplevel.notify["edge"].connect((pspec,data)=>
            {
                switch(toplevel.edge)
                {
                    case PositionType.TOP:
                        edge_button.set_label(_("Top"));
                        break;
                    case PositionType.BOTTOM:
                        edge_button.set_label(_("Bottom"));
                        break;
                    case PositionType.LEFT:
                        edge_button.set_label(_("Left"));
                        break;
                    case PositionType.RIGHT:
                        edge_button.set_label(_("Right"));
                        break;
                }
            });
            /* alignment */
            alignment_button.set_relief(ReliefStyle.NONE);
            switch(toplevel.alignment)
            {
                case AlignmentType.START:
                    alignment_button.set_label(_("Start"));
                    break;
                case AlignmentType.CENTER:
                    alignment_button.set_label(_("Center"));
                    break;
                case AlignmentType.END:
                    alignment_button.set_label(_("End"));
                    break;
            }
            toplevel.notify["alignment"].connect((pspec,data)=>
            {
                switch(toplevel.alignment)
                {
                    case AlignmentType.START:
                        alignment_button.set_label(_("Start"));
                        break;
                    case AlignmentType.CENTER:
                        alignment_button.set_label(_("Center"));
                        break;
                    case AlignmentType.END:
                        alignment_button.set_label(_("End"));
                        break;
                }
                spin_margin.set_sensitive(toplevel.alignment!=AlignmentType.CENTER);
            });
            /* monitors */
            monitors_button.set_relief(ReliefStyle.NONE);
            int monitors;
            unowned Gdk.Screen screen = toplevel.get_screen();
            if(screen != null)
                monitors = screen.get_n_monitors();
            assert(monitors >= 1);
            var menu = new GLib.Menu();
            menu.append(_("All"),"conf.configure-monitors(-1)");
            for (var i = 0; i < monitors; i++)
            {
                var tmp = "conf.configure-monitors(%d)".printf(i);
                var str_num = "%d".printf(i+1);
                menu.append(str_num,tmp);
            }
            monitors_button.set_menu_model(menu as MenuModel);
            monitors_button.set_use_popover(true);
            conf.add_action_entries(entries_monitor,this);
            var v = new Variant.int32(toplevel.monitor);
            conf.change_action_state("configure-monitors",v);
            /* margin */
            toplevel.bind_property(Key.MARGIN,spin_margin,"value",BindingFlags.SYNC_CREATE | BindingFlags.BIDIRECTIONAL);
            spin_margin.set_sensitive(toplevel.alignment != AlignmentType.CENTER);

            /* size */
            toplevel.bind_property(Key.WIDTH,spin_width,"value",BindingFlags.SYNC_CREATE | BindingFlags.BIDIRECTIONAL);
            toplevel.bind_property(Key.DYNAMIC,spin_width,"sensitive",BindingFlags.SYNC_CREATE | BindingFlags.INVERT_BOOLEAN);
            toplevel.bind_property(Key.HEIGHT,spin_height,"value",BindingFlags.SYNC_CREATE | BindingFlags.BIDIRECTIONAL);
            toplevel.bind_property(Key.ICON_SIZE,spin_iconsize,"value",BindingFlags.SYNC_CREATE | BindingFlags.BIDIRECTIONAL);
            toplevel.bind_property(Key.CORNERS_SIZE,spin_corners,"value",BindingFlags.SYNC_CREATE | BindingFlags.BIDIRECTIONAL);
            /* background */
            IconInfo info;
            color.parse(toplevel.background_color);
            color_background.set_rgba(color);
            color_background.set_relief(ReliefStyle.NONE);
            color_background.color_set.connect(()=>{
                toplevel.background_color = color_background.get_rgba().to_string();
            });
            toplevel.bind_property(Key.USE_BACKGROUND_COLOR,color_background,"sensitive",BindingFlags.SYNC_CREATE);
            if (toplevel.background_file != null)
                file_background.set_filename(toplevel.background_file);
            file_background.set_sensitive(toplevel.use_background_file);
            toplevel.bind_property(Key.USE_BACKGROUND_FILE,file_background,"sensitive",BindingFlags.SYNC_CREATE);
            file_background.file_set.connect(()=>{
                toplevel.background_file = file_background.get_filename();
            });
            /* foregorund */
            color.parse(toplevel.foreground_color);
            color_foreground.set_rgba(color);
            color_foreground.set_relief(ReliefStyle.NONE);
            color_foreground.color_set.connect(()=>{
                toplevel.foreground_color = color_foreground.get_rgba().to_string();
            });
            toplevel.bind_property(Key.USE_FOREGROUND_COLOR,color_foreground,"sensitive",BindingFlags.SYNC_CREATE);
            /* fonts */
            toplevel.bind_property(Key.FONT,font_selector,"font",BindingFlags.SYNC_CREATE | BindingFlags.BIDIRECTIONAL);
            font_selector.set_relief(ReliefStyle.NONE);
            toplevel.bind_property(Key.USE_FONT,font_box,"sensitive",BindingFlags.SYNC_CREATE);
            /* plugin list */
            init_plugin_list();
            this.insert_action_group("conf",conf);
            this.insert_action_group("win",toplevel);
            this.insert_action_group("app",toplevel.application);
        }
        private void state_configure_monitor(SimpleAction act, Variant? param)
        {
            int state = act.get_state().get_int32();
            /* change monitor */
            int request_mon = param.get_int32();
            string str = request_mon < 0 ? _("All") : _("%d").printf(request_mon+1);
            PositionType edge = (PositionType) toplevel.edge;
            if(toplevel.panel_edge_available(edge, request_mon,false) || (state<-1))
            {
                toplevel.monitor = request_mon;
                act.set_state(param);
                monitors_button.set_label(str);
            }
        }
        private void on_sel_plugin_changed(TreeSelection tree_sel)
        {
            TreeIter it;
            TreeModel model;
            Applet pl;
            if( tree_sel.get_selected(out model, out it ) )
            {
                model.get(it, Column.DATA, out pl, -1 );
                var desc = toplevel.get_plugin(pl).plugin_info.get_description();
                plugin_desc.set_text(_(desc) );
                configure_button.set_sensitive(pl is AppletConfigurable);
            }
        }
        private void on_plugin_expand_toggled(string path)
        {
            TreeIter it;
            TreePath tp = new TreePath.from_string( path );
            var model = plugin_list.get_model();
            if( model.get_iter(out it, tp) )
            {
                Applet pl;
                bool expand;
                model.get(it, Column.DATA, out pl, Column.EXPAND, out expand, -1 );
                if (toplevel.get_plugin(pl).plugin_info.get_external_data(Data.EXPANDABLE)!=null)
                {
                    expand = !expand;
                    (model as Gtk.ListStore).set(it,Column.EXPAND,expand,-1);
                    unowned PluginSettings s = toplevel.get_applet_settings(pl);
                    s.default_settings.set_boolean(Key.EXPAND,expand);
                }
            }
        }
        private void on_stretch_render(CellLayout layout, CellRenderer renderer, TreeModel model, TreeIter iter)
        {
            /* Set the control visible depending on whether stretch is available for the plugin.
             * The g_object_set method is touchy about its parameter, so we can't pass the boolean directly. */
            Applet pl;
            model.get(iter, Column.DATA, out pl, -1);
            renderer.visible = (toplevel.get_plugin(pl).plugin_info.get_external_data(Data.EXPANDABLE)!=null) ? true : false;
        }
        private void update_plugin_list_model()
        {
            TreeIter it;
            var list = new Gtk.ListStore( 3, typeof(string), typeof(bool), typeof(Applet) );
            var plugins = toplevel.get_applets_list();
            foreach(var widget in plugins)
            {
                var w = widget as Applet;
                var expand = widget.hexpand && widget.vexpand;
                list.append(out it );
                var name = toplevel.get_plugin(w).plugin_info.get_name();
                list.set(it,
                         Column.NAME, _(name),
                         Column.EXPAND, expand,
                         Column.DATA, w,
                         -1);
            }
            plugin_list.set_model(list);
        }
        private void init_plugin_list()
        {
            TreeIter it;
            var textrender = new CellRendererText();
            var col = new TreeViewColumn.with_attributes(
                    _("Currently loaded plugins"),
                    textrender, "text", Column.NAME, null );
            col.expand = true;
            plugin_list.append_column(col );

            var render = new CellRendererToggle();
            render.activatable = true;
            render.toggled.connect(on_plugin_expand_toggled);
            col = new TreeViewColumn.with_attributes(
                    _("Stretch"),
                    render, "active", Column.EXPAND, null );
            col.expand = false;
            col.set_cell_data_func(render, on_stretch_render);
            plugin_list.append_column(col );
            update_plugin_list_model();
            plugin_list.row_activated.connect((path,col)=>{
                TreeSelection tree_sel = plugin_list.get_selection();
                TreeModel model;
                TreeIter iter;
                Applet pl;
                if( ! tree_sel.get_selected(out model, out iter ) )
                    return;
                model.get(iter, Column.DATA, out pl, -1);
                if (pl is AppletConfigurable)
                    pl.show_config_dialog();
            });
            var list = plugin_list.get_model();
            var tree_sel = plugin_list.get_selection();
            tree_sel.set_mode(SelectionMode.BROWSE );
            tree_sel.changed.connect(on_sel_plugin_changed);
            if( list.get_iter_first(out it) )
                tree_sel.select_iter(it);
        }
        [GtkCallback]
        private void on_configure_plugin()
        {
            TreeSelection tree_sel = plugin_list.get_selection();
            TreeModel model;
            TreeIter iter;
            Applet pl;
            if( ! tree_sel.get_selected(out model, out iter ) )
                return;
            model.get(iter, Column.DATA, out pl, -1);
            if (pl is AppletConfigurable)
                pl.show_config_dialog();
        }
        private int sort_by_name(TreeModel model, TreeIter a, TreeIter b)
        {
            string str_a, str_b;
            model.get(a, 0, out str_a, -1);
            model.get(b, 0, out str_b, -1);
            return  str_a.collate(str_b);
        }
        [GtkCallback]
        private void on_add_plugin()
        {
            var dlg = new Popover(adding_button);
            var scroll = new ScrolledWindow( null, null );
            scroll.set_shadow_type(ShadowType.IN );
            scroll.set_policy(PolicyType.AUTOMATIC,
                              PolicyType.AUTOMATIC );
            dlg.add(scroll);
            var view = new TreeView();
            scroll.add(view);
            var tree_sel = view.get_selection();
            tree_sel.set_mode(SelectionMode.BROWSE );

            var render = new CellRendererText();
            var col = new TreeViewColumn.with_attributes(
                                                    _("Available plugins"),
                                                    render, "text", 0, null );
            view.append_column(col);

            var list = new Gtk.ListStore( 2,
                                     typeof(string),
                                     typeof(string));

            /* Populate list of available plugins.
             * Omit plugins that can only exist once per system if it is already configured. */
            foreach(var type in toplevel.get_all_types())
            {
                var once = type.get_external_data(Data.ONE_PER_SYSTEM);
                if (once == null || !type.is_loaded())
                {
                    TreeIter it;
                    list.append( out it );
                    /* it is safe to put classes data here - they will be valid until restart */
                    list.set(it, 0, _(type.get_name()),
                                 1, type.get_module_name(),
                                 -1 );
                }
            }
            list.set_default_sort_func(sort_by_name);
            list.set_sort_column_id(TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
                                                 SortType.ASCENDING);
            view.activate_on_single_click = true;
            view.set_model(list);
            view.row_activated.connect((path,col)=>{
                TreeIter it;
                TreeModel model;
                var sel = view.get_selection();
                if( sel.get_selected(out model, out it ) )
                {
                    string type;
                    list.get(it, 1, out type, -1 );
                    toplevel.add_applet(type);
                    toplevel.update_applet_positions();
                    update_plugin_list_model();
                }
                update_widget_position_keys();
            });

            scroll.set_min_content_width(320);
            scroll.set_min_content_height(200);
            dlg.show_all();
        }
        [GtkCallback]
        private void on_remove_plugin()
        {
            TreeIter it;
            TreeModel model;
            var tree_sel = plugin_list.get_selection();
            Applet pl;
            if( tree_sel.get_selected(out model, out it) )
            {
                var tree_path = model.get_path(it );
                model.get(it, Column.DATA, out pl, -1);
                if( tree_path.get_indices()[0] >= model.iter_n_children(null))
                    tree_path.prev();
#if VALA_0_36
                (model as Gtk.ListStore).remove(ref it);
#else
                (model as Gtk.ListStore).remove(it);
#endif
                tree_sel.select_path(tree_path );
                toplevel.remove_applet(pl);
            }
        }
        private void update_widget_position_keys()
        {
            foreach(var w in toplevel.get_applets_list())
            {
                var applet = w as Applet;
                uint idx = toplevel.get_applet_position(applet);
                unowned PluginSettings s = toplevel.get_applet_settings(applet);
                s.default_settings.set_uint(Key.POSITION,idx);
            }
        }
        [GtkCallback]
        private void on_moveup_plugin()
        {
            TreeIter it;
            TreeIter? prev = null;
            var model = plugin_list.get_model();
            var tree_sel = plugin_list.get_selection();
            if( ! model.get_iter_first( out it ) )
                return;
            if( tree_sel.iter_is_selected(it) )
                return;
            do{
                if( tree_sel.iter_is_selected(it) )
                {
                    Applet pl;
                    model.get(it, Column.DATA, out pl, -1 );
                    (model as Gtk.ListStore).move_before(ref it, prev );

                    var i = toplevel.get_applet_position(pl);
                    /* reorder in config, 0 is Global */
                    i = i > 0 ? i : 0;

                    /* reorder in panel */
                    toplevel.set_applet_position(pl,(int)i-1);
                    update_widget_position_keys();
                    return;
                }
                prev = it;
            }while( model.iter_next(ref it) );
        }
        [GtkCallback]
        private void on_movedown_plugin()
        {
            TreeIter it, next;
            TreeModel model;
            var tree_sel = plugin_list.get_selection();
            Applet pl;

            if(!tree_sel.get_selected(out model, out it))
                return;
            next = it;
            if( !model.iter_next(ref next) )
                return;

            model.get(it, Column.DATA, out pl, -1 );

            (model as Gtk.ListStore).move_after(ref it, next );

            var i = toplevel.get_applet_position(pl);
            /* reorder in panel */
            toplevel.set_applet_position(pl,(int)i+1);
            update_widget_position_keys();
        }
    }
}
