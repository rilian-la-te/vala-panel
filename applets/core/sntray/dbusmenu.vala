using GLib;
using Gtk;

namespace DBusMenu
{
	[DBus (use_string_marshalling = true)]
	public enum Status
	{
		[DBus (value = "normal")]
		NORMAL,
		[DBus (value = "notice")]
		NOTICE
	}
	[DBus (name = "com.canonical.dbusmenu")]
	public interface Iface : Object
	{
		public abstract uint version {get;}
		public abstract string text_direction {owned get;}
		[DBus (use_string_marshalling = true)]
		public abstract Status status {get;}
		public abstract string[] icon_theme_path {owned get;}
		/* layout signature is "(ia{sv}av)" */
		public abstract void get_layout(int parent_id,
						int recursion_depth,
						string[] property_names,
						out uint revision,
						[DBus (signature = "(ia{sv}av)")] out Variant layout) throws IOError;
		/* properties signature is "a(ia{sv})" */
		public abstract void get_group_properties(
							int[] ids,
							string[] property_names,
							[DBus (signature = "a(ia{sv})")] out Variant properties) throws IOError;
		public abstract void get_property(int id, string name, out Variant value) throws IOError;
		public abstract void event(int id, string event_id, Variant? data, uint timestamp) throws IOError;
		/* events signature is a(isvu) */
		public abstract void event_group( [DBus (signature = "a(isvu)")] Variant events,
										out int[] id_errors) throws IOError;
		public abstract void about_to_show(int id, out bool need_update) throws IOError;
		public abstract void about_to_show_group(int[] ids, out int[] updates_needed, out int[] id_errors) throws IOError;
		/*updated properties signature is a(ia{sv}), removed is a(ias)*/
		public abstract signal void items_properties_updated(
								[DBus (signature = "a(ia{sv})")] Variant updated_props,
								[DBus (signature="a(ias)")] Variant removed_props);
		public abstract signal void layout_updated(uint revision, int parent);
		public abstract signal void item_activation_requested(int id, uint timestamp);
	}

	public class PropertyStore : Object
	{
		VariantDict dict;
		HashTable<string,VariantType> checker;
		public Variant? get_prop(string name)
		{
			var type = checker.lookup(name);
			return (type != null) ? dict.lookup_value(name,type) : null;
		}
		public void set_prop(string name, Variant? val)
		{
			VariantType type = checker.lookup(name);
			if (val == null)
				dict.remove(name);
			else if (type != null && val.is_of_type(type))
				dict.insert_value(name,val);
			init_default();
		}
		construct
		{
			dict = new VariantDict();
			checker = new HashTable<string,VariantType>(str_hash,str_equal);
			checker.insert("visible", VariantType.BOOLEAN);
			checker.insert("enabled", VariantType.BOOLEAN);
			checker.insert("label", VariantType.STRING);
			checker.insert("type", VariantType.STRING);
			checker.insert("children-display", VariantType.STRING);
			checker.insert("toggle-type", VariantType.STRING);
			checker.insert("icon-name", VariantType.STRING);
			checker.insert("accessible-desc", VariantType.STRING);
			checker.insert("shortcut", new VariantType("aas"));
			checker.insert("toggle-state", VariantType.INT32);
			checker.insert("icon-data", new VariantType("ay"));
		}
		public PropertyStore (Variant? props)
		{
			if (props != null)
			{
				VariantIter iter = props.iterator();
				string name;
				Variant v;
				while(iter.next ("{sv}", out name, out v))
					this.set_prop(name,v);
			}
			init_default();
		}
		private void init_default()
		{
			if(!dict.contains("visible"))
				dict.insert_value("visible", new Variant.boolean(true));
			if(!dict.contains("enabled"))
				dict.insert_value("enabled", new Variant.boolean(true));
			if(!dict.contains("type"))
				dict.insert_value("type", new Variant.string("standard"));
			if(!dict.contains("label"))
				dict.insert_value("label", new Variant.string(""));
		}
	}

	public class Item : Object
	{
		private Client client;
		private PropertyStore store;
		private List<int> children_ids;
		public int id
		{get; private set;}
		internal DateTime gc_tag;
		public signal void property_changed(string name, Variant? val);
		public signal void child_added(int id, Item item);
		public signal void child_removed(int id, Item item);
		public signal void child_moved(int oldpos, int newpos, Item item);
		public Item (int id, Client iface, Variant props, List<int> children_ids)
		{
			this.children_ids = children_ids.copy();
			this.client = iface;
			this.store = new PropertyStore(props);
			this.id = id;
		}
		public Variant get_variant_property(string name)
		{
			return store.get_prop(name);
		}
		public string get_string_property(string name)
		{
			return store.get_prop(name).get_string();
		}
		public bool get_bool_property(string name)
		{
			return (store.get_prop(name)!=null) ? store.get_prop(name).get_boolean() : false;
		}
		public int get_int_property(string name)
		{
			return (store.get_prop(name)!=null) ? store.get_prop(name).get_int32() : 0;
		}
		public List<int> get_children_ids()
		{
			return children_ids.copy();
		}
		public void set_variant_property(string name, Variant? val)
		{
			var old_value = this.store.get_prop(name);
			this.store.set_prop(name, val);
			var new_value = this.store.get_prop(name);
			if ((old_value ?? new_value) == null)
				return;
			if (new_value != null && old_value == null
				|| old_value == null && new_value != null
				|| !old_value.equal(new_value))
	            this.property_changed(name,new_value);
		}
		public void add_child(int id, int pos)
		{
			children_ids.insert(id,pos);
			child_added(id,client.get_item(id));
		}
		public void remove_child(int id)
		{
			children_ids.remove(id);
			child_removed(id,client.get_item(id));
		}
		public void move_child(int id, int newpos)
		{
			var oldpos = children_ids.index(id);
			if (oldpos == newpos)
				return;
			children_ids.remove(id);
			children_ids.insert(id,newpos);
			child_moved(oldpos,newpos,client.get_item(id));
		}
		public List<Item> get_children()
		{
			List<Item> ret = new List<Item>();
			foreach (var id in children_ids)
				ret.append(client.get_item(id));
			return ret;
		}
		public int get_child_position(int child_id)
		{
			return children_ids.index(child_id);
		}
		public void handle_event(string event_id, Variant? data, uint timestamp)
		{
			try
			{
				client.iface.event(this.id,event_id,data ?? new Variant.int32(0),timestamp);
			} catch (Error e)
			{
				stderr.printf("%s\n",e.message);
			}
		}
		public void request_about_to_show()
		{
			client.request_about_to_show(this.id);
		}
		public bool wants_separator()
		{
			return get_string_property("type") == "separator";
		}
	}
	public class Client : Object
	{
		private HashTable<int,Item> items;
		private bool layout_update_required;
		private bool layout_update_in_progress;
		private int[] requested_props_ids;
		private uint layout_revision;
		public Iface iface
		{get; private set;}
		construct
		{
			items = new HashTable<int,Item>(direct_hash, direct_equal);
		}
		public Client(string object_name, string object_path)
		{
			layout_revision = 0;
			try{
				this.iface = Bus.get_proxy_sync(BusType.SESSION, object_name, object_path);
			} catch (Error e) {stderr.printf("Cannot get menu! Error: %s",e.message);}
			VariantDict props = new VariantDict();
			variant_dict_insert(props,"children-display","s","submenu");
			var item = new Item(0,this,props.end(),new List<int>());
			items.insert(0,item);
			request_layout_update();
			iface.layout_updated.connect((rev,parent)=>{request_layout_update();});
			iface.items_properties_updated.connect(props_updated_cb);
			requested_props_ids = {};
		}
		public Item? get_root_item()
		{
			return items.lookup(0);
		}
		public Item? get_item(int id)
		{
			return items.lookup(id);
		}
		private void request_layout_update()
		{
			if(layout_update_in_progress)
				layout_update_required = true;
			else layout_update.begin();
		}
		/* the original implementation will only request partial layouts if somehow possible
		/ we try to save us from multiple kinds of race conditions by always requesting a full layout */
		private async void layout_update()
		{
			layout_update_required = false;
			layout_update_in_progress = true;
			string[] props = {"type", "children-display"};
			uint rev;
			Variant layout;
			try{
				iface.get_layout(0,-1,props,out rev, out layout);
			} catch (Error e) {stderr.printf("Cannot update layout. Error: %s",e.message);}
			parse_layout(rev,layout);
			clean_items();
			if (layout_update_required)
				yield layout_update();
			else
				layout_update_in_progress = false;
		}
		private void parse_layout(uint rev, Variant layout)
		{
			/* Removed revision handling because of bug */
	//~ 		if (rev < layout_revision) return;
			/* layout signature must be "(ia{sv}av)" */
			int id = layout.get_child_value(0).get_int32();
			Variant props = layout.get_child_value(1);
			Variant children = layout.get_child_value(2);
			VariantIter chiter = children.iterator();
			List<int> children_ids = new List<int>();
			for(var child = chiter.next_value(); child != null; child = chiter.next_value())
			{
				child = child.get_variant();
				parse_layout(rev,child);
				int child_id = child.get_child_value(0).get_int32();
				children_ids.append(child_id);
			}
			if (id in items)
			{
				unowned Item item = items.lookup(id);
				VariantIter props_iter = props.iterator();
				string name;
				Variant val;
				while(props_iter.next("{sv}",out name, out val))
					item.set_variant_property(name, val);
				/* make sure our children are all at the right place, and exist */
				var old_children_ids = item.get_children_ids();
				int i = 0;
				foreach(var new_id in children_ids)
				{
					var old_child = -1;
					foreach(var old_id in old_children_ids)
						if (new_id == old_id)
						{
							old_child = old_id;
							old_children_ids.remove(old_id);
							break;
						}
					if (old_child < 0)
						item.add_child(new_id,i);
					else
						item.move_child(old_child,i);
					i++;
				}
				foreach (var old_id in old_children_ids)
					item.remove_child(old_id);
			}
			else
			{
				items.insert(id, new Item(id,this,props,children_ids));
				request_properties.begin(id);
			}
			layout_revision = rev;
		}
		private void clean_items()
		{
		/* Traverses the list of cached menu items and removes everyone that is not in the list
		/  so we don't keep alive unused items */
		var tag = new DateTime.now_utc();
		List<int> traverse = new List<int>();
		traverse.append(0);
		while (traverse.length() > 0) {
			var item = this.get_item(traverse.data);
			traverse.delete_link(traverse);
			item.gc_tag = tag;
			traverse.concat(item.get_children_ids());
		}
		SList<int> remover = new SList<int>();
		items.foreach((k,v)=>{if (v.gc_tag != tag) remover.append(k);});
			foreach(var i in remover)
				items.remove(i);
		}
		/* we don't need to cache and burst-send that since it will not happen that frequently */
		public void request_about_to_show(int id)
		{
			var need_update = false;
			try
			{
				iface.about_to_show(id,out need_update);
			} catch (Error e)
			{
				stderr.printf("%s\n",e.message);
			}
			if (need_update)
				request_layout_update();
		}
		private async void request_properties(int id)
		{
			Variant props;
			string[] names = {};
			if (!(id in requested_props_ids))
				requested_props_ids += id;
			try{
				iface.get_group_properties(requested_props_ids,names,out props);
			} catch (GLib.Error e) {stderr.printf("%s\n",e.message);}
			requested_props_ids = {};
			parse_props(props);
		}
	    private void props_updated_cb (Variant updated_props, Variant removed_props)
	    {
			parse_props(updated_props);
			parse_props(removed_props);
	    }
	    private void parse_props(Variant props)
	    {
			/*updated properties signature is a(ia{sv}), removed is a(ias)*/
			var iter = props.iterator();
			for (var props_req = iter.next_value(); props_req!=null; props_req = iter.next_value())
			{
				int req_id = props_req.get_child_value(0).get_int32();
				Variant props_id = props_req.get_child_value(1);
				var ch_iter = props_id.iterator();
				Variant? val;
				for (val = ch_iter.next_value(); val != null; val = ch_iter.next_value())
				{
					if (val.get_type_string() == "{sv}")
					{
						var key = val.get_child_value(0).get_string();
						var prop = val.get_child_value(1).get_variant();
						if (items.lookup(req_id) != null)
							items.lookup(req_id).set_variant_property(key,prop);
					}
					else if (val.get_type_string() == "s")
					{
						var key = val.get_string();
						if (items.lookup(req_id) != null)
							items.lookup(req_id).set_variant_property(key,null);
					}
				}
			}
		}
	}
	public interface GtkItemIface : Object
	{
		public abstract Item item
		{get; protected set;}
	}
	public class GtkMainItem : CheckMenuItem, GtkItemIface
	{
		private static const string[] allowed_properties = {"visible","enabled","label","type",
												"children-display","toggle-type",
												"toggle-state","icon-name","icon-data","accessible-desc"};
		public Item item
		{get; protected set;}
		private bool has_indicator;
		private Box box;
		private Image image;
		private new Label label;
		private ulong activate_handler;
	    private bool is_themed_icon;
		public GtkMainItem(Item item)
		{
			is_themed_icon = false;
			this.item = item;
			box = new Box(Orientation.HORIZONTAL, 5);
			image = new Image();
			label = new Label(null);
			box.add(image);
			box.add(label);
			this.add(box);
			this.show_all();
			this.init();
			item.property_changed.connect(on_prop_changed_cb);
			item.child_added.connect(on_child_added_cb);
			item.child_removed.connect(on_child_removed_cb);
			item.child_moved.connect(on_child_moved_cb);
			activate_handler = this.activate.connect(on_toggled_cb);
			this.select.connect(on_select_cb);
			this.deselect.connect(on_deselect_cb);
			this.notify["visible"].connect(()=>{this.visible=item.get_bool_property("visible");});
		}
		private void init()
		{
			foreach (var prop in allowed_properties)
				on_prop_changed_cb(prop,item.get_variant_property(prop));
		}
		private void on_prop_changed_cb(string name, Variant? val)
		{
			if(activate_handler > 0)
				SignalHandler.block(this,activate_handler);
			switch (name)
			{
				case "visible":
					this.visible = val.get_boolean();
					break;
				case "enabled":
					this.sensitive = val.get_boolean();
					break;
				case "label":
					label.set_text_with_mnemonic(val.get_string());
					break;
				case "children-display":
					if (val != null && val.get_string() == "submenu")
					{
						this.submenu = new Gtk.Menu();
						this.submenu.insert.connect(on_child_insert_cb);
						foreach(var item in this.item.get_children())
							submenu.add(GtkClient.new_item(item));
					}
					else
						this.submenu = null;
					break;
				case "toggle-type":
					if (val == null)
						this.set_toggle_type("normal");
					else
						this.set_toggle_type(val.get_string());
					break;
				case "toggle-state":
					if (val != null && val.get_int32()>0)
						this.active = true;
					else
						this.active = false;
					break;
				case "accessible-desc":
					this.set_tooltip_text(val != null ? val.get_string() : null);
					break;
				case "icon-name":
				case "icon-data":
					update_icon(val);
					break;
				case "shortcut":
				/* FIXME: Accels support*/
					break;
			}
			if(activate_handler > 0)
				SignalHandler.unblock(this,activate_handler);
		}
		private void set_toggle_type(string type)
		{
			if (type=="radio")
			{
				this.set_accessible_role(Atk.Role.RADIO_MENU_ITEM);
				this.has_indicator = true;
				this.draw_as_radio = true;
			}
			else if (type=="checkmark")
			{
				this.set_accessible_role(Atk.Role.CHECK_MENU_ITEM);
				this.has_indicator = true;
				this.draw_as_radio = false;
			}
			else
			{
				this.set_accessible_role(Atk.Role.MENU_ITEM);
				this.has_indicator = false;
			}
		}
		private void update_icon(Variant? val)
		{
			if (val == null)
			{
				var icon = image.gicon;
				if (!(icon != null && icon is ThemedIcon && is_themed_icon))
					is_themed_icon = false;
				return;
			}
			Icon? icon = null;
			if (val.get_type_string() == "s")
			{
				is_themed_icon = true;
				icon = new ThemedIcon.with_default_fallbacks(val.get_string()+"-symbolic");
			}
			else if (!is_themed_icon && val.get_type_string() == "ay")
				icon = new BytesIcon(val.get_data_as_bytes());
			else return;
			image.set_from_gicon(icon,IconSize.MENU);
			image.set_pixel_size(16);
		}
		private void on_child_added_cb(int id,Item item)
		{
			if (this.submenu != null)
				this.submenu.append (GtkClient.new_item(item));
			else
			{
				stderr.printf("Adding new item to item without submenu! Creating new submenu...\n");
				this.submenu = new Gtk.Menu();
				this.submenu.append (GtkClient.new_item(item));
			}
		}
		private void on_child_removed_cb(int id, Item item)
		{
			if (this.submenu != null)
				foreach(var ch in this.submenu.get_children())
					if ((ch as GtkItemIface).item == item)
						ch.destroy();
			else
				stderr.printf("Cannot remove a child from item without submenu!\n");
		}
		private void on_child_moved_cb(int oldpos, int newpos, Item item)
		{
			if (this.submenu != null)
				foreach(var ch in this.submenu.get_children())
					if ((ch as GtkItemIface).item == item)
						this.submenu.reorder_child(ch,newpos);
			else
				stderr.printf("Cannot move a child of item with has no children!\n");
		}
		private void on_toggled_cb()
		{
			item.handle_event("clicked",new Variant.int32(0),get_current_event_time());
		}
		private void on_select_cb()
		{
			if (this.submenu != null)
			{
				item.handle_event("opened",null,0);
				item.request_about_to_show();
			}
		}
		private void on_deselect_cb()
		{
			if (this.submenu != null)
				item.handle_event("closed",null,0);
		}
		private void on_child_insert_cb(Widget w, int pos)
		{
			var ch = w as GtkItemIface;
			this.submenu.reorder_child(w,item.get_child_position(ch.item.id));
			this.submenu.queue_resize();
		}
		protected override void draw_indicator(Cairo.Context cr)
		{
			if (has_indicator)
				base.draw_indicator(cr);
		}
	}
	public class GTKSeparatorItem: SeparatorMenuItem, GtkItemIface
	{
		public Item item
		{get; protected set;}
		public GTKSeparatorItem(Item item)
		{
			this.item = item;
			item.property_changed.connect(on_prop_changed_cb);
			this.show_all();
		}
		private void on_prop_changed_cb(string name, Variant? val)
		{
			switch (name)
			{
				case "visible":
					this.visible = val.get_boolean();
					break;
				case "enabled":
					this.sensitive = val.get_boolean();
					break;
			}
		}
	}
	public class GtkClient : Client
	{
		private Gtk.Menu root_menu;
		public static Gtk.MenuItem new_item(Item item)
		{
			if (item.wants_separator())
				return new GTKSeparatorItem(item);
			return new GtkMainItem(item);
		}
		public GtkClient(string object_name, string object_path)
		{
			base(object_name,object_path);
			this.root_menu = null;
		}
		public void attach_to_menu(Gtk.Menu menu)
		{
			root_menu = menu;
			root_menu.foreach((c)=>{menu.remove(c);});
			root_menu.realize.connect(open_cb);
			root_menu.unrealize.connect(close_cb);
			root_menu.insert.connect(on_child_insert_cb);
			get_root_item().child_added.connect(on_child_added_cb);
			get_root_item().child_moved.connect(on_child_moved_cb);
			get_root_item().child_removed.connect(on_child_removed_cb);
			foreach(var ch in get_root_item().get_children())
				root_menu.append(new_item(ch) as Gtk.MenuItem);
			foreach(var path in iface.icon_theme_path)
				IconTheme.get_default().append_search_path(path);
			root_menu.show_all();
		}
		private void open_cb()
		{
			get_root_item().handle_event("opened",null,0);
			get_root_item().request_about_to_show();
			root_menu.queue_resize();
		}
		private void close_cb()
		{
			get_root_item().handle_event("closed",null,0);
		}
		private void on_child_added_cb(int id, Item item)
		{
			root_menu.append(new_item(item));
		}
		private void on_child_moved_cb(int oldpos, int newpos, Item item)
		{
			foreach(var ch in root_menu.get_children())
				if ((ch as GtkItemIface).item == item)
					root_menu.reorder_child(ch,newpos);
		}
		private void on_child_removed_cb(int id, Item item)
		{
			foreach(var ch in root_menu.get_children())
				if ((ch as GtkItemIface).item == item)
					ch.destroy();
		}
		private void on_child_insert_cb(Widget w, int pos)
		{
			var ch = w as GtkItemIface;
			root_menu.reorder_child(w,get_root_item().get_child_position(ch.item.id));
		}
	}
}
