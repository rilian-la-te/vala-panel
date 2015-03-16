using GLib;
using Gtk;

namespace StatusNotifier
{
	[GtkTemplate (ui = "/org/vala-panel/sntray/snconfig.ui"), CCode (cname = "StatusNotifierConfig")]
	public class ConfigDialog : Dialog
	{
		private static const int COLUMN_ID = 0;
		private static const int COLUMN_NAME = 1;
		private static const int COLUMN_OVERRIDE_INDEX = 2;
		private static const int COLUMN_INDEX = 3;
		private static const int COLUMN_OVERRIDE_VISIBLE = 4;
		private static const int COLUMN_VISIBLE = 5;
		private static const int COLUMN_ICON = 6;

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
		[GtkChild (name = "store")]
		ListStore store;
		[GtkChild (name = "box-indicator")]
		Box box_indicator;
		[GtkChild (name = "scale-indicator")]
		Scale scale_indicator;
		ItemBox layout;
		public bool configure_icon_size {get; set;}
		public ConfigDialog(ItemBox box)
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
			layout.bind_property("icon-size",scale_indicator.adjustment,"value",BindingFlags.BIDIRECTIONAL|BindingFlags.SYNC_CREATE);
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
						store.remove(iter);
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
			var name = v.title;
			var id = v.id;
			var icon = v.icon;
			var over_index = layout.index_override.contains(v.id);
			int index = (int)v.ordering_index;
			if (over_index)
				index = layout.index_override.lookup(v.id).get_int32();
			var over_filter = layout.filter_override.contains(v.id);
			bool filter = true;
			if (over_filter)
				filter = layout.filter_override.lookup(v.id).get_boolean();
			TreeIter iter;
			store.append(out iter);
			store.set(iter,COLUMN_ID,id,COLUMN_NAME,name,COLUMN_OVERRIDE_INDEX,over_index,COLUMN_INDEX,index.to_string(),
														 COLUMN_OVERRIDE_VISIBLE,over_filter,COLUMN_VISIBLE,filter,
														 COLUMN_ICON,icon);
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
            if (over)
			{
				store.set(iter,COLUMN_INDEX,"0");
				layout.index_override.insert(id,new Variant.int32(0));
			}
			else
				layout.index_override.remove(id);
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
            if (over)
			{
				store.set(iter,COLUMN_VISIBLE,true);
				layout.filter_override.insert(id,new Variant.boolean(true));
			}
			else
				layout.filter_override.remove(id);
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
            var over_dict = layout.index_override;
            layout.index_override = over_dict;
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
            var over_dict = layout.filter_override;
            layout.filter_override = over_dict;
		}
	}
}
