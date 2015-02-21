using Gtk;
using GLib;

namespace StatusNotifier
{
	public class ItemBox : FlowBox
	{
		static Host host;
		ulong watcher_registration_handler;
		HashTable<string,Item> items;
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
			set_sort_func((ch1,ch2)=>{return (int)(ch1 as Item).ordering_index - (int)(ch2 as Item).ordering_index;});
			set_filter_func((ch)=>{
				var item = ch as Item;
				if (!show_passive && item.status == Status.PASSIVE) return false;
				if (show_application_status && item.cat == Category.APPLICATION) return true;
				if (show_communications && item.cat == Category.COMMUNICATIONS) return true;
				if (show_system && item.cat == Category.SYSTEM) return true;
				if (show_hardware && item.cat == Category.HARDWARE) return true;
				return false;
			});
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
	}
}
