using Gtk;
using GLib;

namespace StatusNotifier
{
	public class ItemBox : FlowBox
	{
		static Host host;
		ulong watcher_registration_handler;
		HashTable<string,Item> items;
		public VariantDict index_override
		{get; set;}
		public bool show_application_status
		{get; set;}
		public bool show_communications
		{get; set;}
		public bool show_system
		{get; set;}
		public bool show_hardware
		{get; set;}
		public bool show_passive
		{get; set;}
		public int icon_size
		{get; set;}
		public unowned MenuPositionFunc? menu_position_func
		{internal get; set;}
		static construct
		{
			host = new Host("org.kde.StatusNotifierHost-itembox%d".printf(Gdk.CURRENT_TIME));
		}
		construct
		{
			items = new HashTable<string,Item>(str_hash, str_equal);
			index_override = new VariantDict();
			show_application_status = true;
			show_communications = true;
			show_system = true;
			show_hardware = true;
			show_passive = false;
			menu_position_func = null;
			child_activated.connect((ch)=>{
				select_child(ch);
				(ch as Item).context_menu();
			});
			notify.connect((pspec)=>{
				if (pspec.name == "index-override")
					invalidate_sort();
				else
					invalidate_filter();
			});
			set_sort_func(sort_cb);
			set_filter_func(filter_cb);
			host.watcher_items_changed.connect(()=>{
					recreate_items();
			});
			watcher_registration_handler = host.notify["watcher-registered"].connect(()=>{
				if (host.watcher_registered)
				{
					recreate_items();
					SignalHandler.disconnect(host,watcher_registration_handler);
				}
			});
			if (host.watcher_registered)
			{
				recreate_items();
				SignalHandler.disconnect(host,watcher_registration_handler);
			}
		}
		public ItemBox()
		{
			Object(orientation: Orientation.HORIZONTAL,
				   selection_mode: SelectionMode.SINGLE,
				   activate_on_single_click: true);
		}
		private void recreate_items()
		{
			string[] new_items = host.watcher_items();
			foreach (var item in new_items)
			{
				string[] np = item.split("/",2);
				if (!items.contains(item))
				{
					var snitem = new Item(np[0],(ObjectPath)("/"+np[1]));
					items.insert(item, snitem);
					this.add(snitem);
				}
			}
		}
		internal void request_remove_item(FlowBoxChild child, string item)
		{
			items.remove(item);
			this.remove(child);
			child.destroy();
		}
		private bool filter_cb(FlowBoxChild ch)
		{
			var item = ch as Item;
			if (!show_passive && item.status == Status.PASSIVE) return false;
			if (show_application_status && item.cat == Category.APPLICATION) return true;
			if (show_communications && item.cat == Category.COMMUNICATIONS) return true;
			if (show_system && item.cat == Category.SYSTEM) return true;
			if (show_hardware && item.cat == Category.HARDWARE) return true;
			return false;
		}
		private int sort_cb(FlowBoxChild ch1, FlowBoxChild ch2)
		{
			var left = ch1 as Item;
			var right = ch2 as Item;
			var lpos = left.ordering_index;
			var rpos = right.ordering_index;
			if (index_override.contains(left.id))
				lpos = index_override.lookup_value(left.id,VariantType.UINT32).get_uint32();
			if (index_override.contains(right.id))
				rpos = index_override.lookup_value(right.id,VariantType.UINT32).get_uint32();
			return (int)lpos - (int)rpos;
		}
	}
}
