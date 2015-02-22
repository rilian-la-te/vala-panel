using GLib;
using Gtk;

namespace StatusNotifier
{
	public class Item : FlowBoxChild
	{
		public ObjectPath object_path
		{private get; internal construct;}
		public string object_name
		{private get; internal construct;}
		public Status status
		{get; private set;}
		public uint ordering_index
		{get {return proxy.index;}}
		public Category cat
		{get {return proxy.category;}}
		public string id
		{owned get {return proxy.id;}}
		private ItemProxy proxy;
		private EventBox ebox;
		private Box box;
		private Label label;
		private Image image;
		private bool is_attention_icon;
		DBusMenu.GtkClient? client;
		Gtk.Menu menu;
		public Item (string n, ObjectPath p)
		{
			Object(object_path: p, object_name: n);
		}
		construct
		{
			is_attention_icon = false;
			PanelCSS.apply_from_resource(this,"/org/vala-panel/lib/style.css","grid-child");
			proxy = new ItemProxy(object_name,object_path);
			proxy.bind_property("has-tooltip",this,"has-tooltip",BindingFlags.SYNC_CREATE);
			client = null;
			ebox = new EventBox();
			box = new Box(Orientation.HORIZONTAL,0);
			label = new Label(null);
			image = new Image();
			box.add(image);
			box.add(label);
			ebox.add(box);
			this.add(ebox);
			init_all();
			ebox.add_events(Gdk.EventMask.SCROLL_MASK);
			ebox.scroll_event.connect((e)=>{
				switch (e.direction)
				{
					case Gdk.ScrollDirection.LEFT:
						scroll(-120, "horizontal");
						break;
					case Gdk.ScrollDirection.RIGHT:
						scroll(120, "horizontal");
						break;
					case Gdk.ScrollDirection.DOWN:
						scroll(-120, "vertical");
						break;
					case Gdk.ScrollDirection.UP:
						scroll(120, "vertical");
	                break;
	                case Gdk.ScrollDirection.SMOOTH:
						double dx,dy;
						e.get_scroll_deltas(out dx, out dy);
						var x = (int) Math.round(dx);
						var y = (int) Math.round(dy);
						if (x.abs() > y.abs())
							scroll(x,"horizontal");
						else if (y.abs() > x.abs())
							scroll(y,"vertical");
						else
							info("Scroll value very small\n");
						break;
				}
				return false;
			});
			ebox.button_press_event.connect((e)=>{
				if (e.type == Gdk.EventType.DOUBLE_BUTTON_PRESS)
					this.primary_activate();
				if (e.button == 2)
					this.primary_activate();
				return false;
			});
			ebox.enter_notify_event.connect((e)=>{
				this.get_style_context().add_class("-panel-launch-button-selected");
			});
			ebox.leave_notify_event.connect((e)=>{
				this.get_style_context().remove_class("-panel-launch-button-selected");
			});
			proxy.notify.connect((pspec)=>{
				if (pspec.name == "status")
					iface_new_status_cb();
				if (pspec.name == "icon")
					iface_new_icon_cb();
				if (pspec.name == "icon-theme-path")
					iface_new_path_cb();
				if (pspec.name == "label")
					iface_new_label_cb();
				if (pspec.name == "category" || pspec.name == "status")
					this.changed();
			});
			proxy.proxy_destroyed.connect(()=>{
				this.get_applet().request_remove_item(this,object_name+(string)object_path);
			});
			this.query_tooltip.connect(query_tooltip_cb);
			this.popup_menu.connect(context_menu);
			IconTheme.get_default().changed.connect(()=>{
				iface_new_icon_cb();
			});
			this.parent_set.connect((prev)=>{
				if (get_applet() != null)
					get_applet().bind_property("icon-size",image,"pixel-size",BindingFlags.SYNC_CREATE);
			});
			this.show_all();
		}
		private void init_all()
		{
			if (proxy.items_in_menu || proxy.menu != null)
				setup_inner_menu();
			iface_new_status_cb();
			iface_new_path_cb();
			iface_new_label_cb();
		}
		private void iface_new_path_cb()
		{
			IconTheme.get_default().prepend_search_path(proxy.icon_theme_path);
			iface_new_icon_cb();
		}
		private void iface_new_status_cb()
		{
			this.status = proxy.status;
			switch(this.status)
			{
				case Status.PASSIVE:
				case Status.ACTIVE:
					this.get_style_context().remove_class(STYLE_CLASS_NEEDS_ATTENTION);
					break;
				case Status.NEEDS_ATTENTION:
					this.get_style_context().add_class(STYLE_CLASS_NEEDS_ATTENTION);
					break;
			}
		}
		private bool query_tooltip_cb(int x, int y, bool keyboard, Tooltip tip)
		{
			tip.set_icon_from_gicon(proxy.tooltip_icon ?? proxy.icon,IconSize.DIALOG);
			tip.set_markup(proxy.tooltip_markup ?? proxy.accessible_desc ?? proxy.title);
			return true;
		}
		private void iface_new_icon_cb()
		{
			if (proxy.icon != null)
			{
				image.set_from_gicon(proxy.icon,IconSize.INVALID);
				image.show();
			}
			else
				image.hide();

		}
		private void iface_new_label_cb()
		{
			if (proxy.label != null)
			{
				this.label.set_text(proxy.label);
				this.label.show();
			}
			else
				this.label.hide();
			/* FIXME: Guided labels */
		}
		private ItemBox get_applet()
		{
			return this.get_parent() as ItemBox;
		}
		private void setup_inner_menu()
		{
			menu = new Gtk.Menu();
			menu.attach_to_widget(this,null);
			menu.vexpand = true;
			/*FIXME: MenuModel support */
			if (client == null)
			{
				client = new DBusMenu.GtkClient(object_name,proxy.menu);
				client.attach_to_menu(menu);
			}

		}
		public void primary_activate()
		{
			int x,y;
			ebox.get_window().get_origin(out x, out y);
			try
			{
				proxy.activate(x,y);
				return;
			}
			catch (Error e) {}
			try
			{
				proxy.secondary_activate(x,y);
			}
			catch (Error e) {
					stderr.printf("%s\n",e.message);
			}
		}
		public bool context_menu()
		{
			int x,y;
			if (proxy.items_in_menu || proxy.menu != null)
			{
				menu.hide.connect(()=>{(this.get_parent() as FlowBox).unselect_child(this);});
				menu.popup(null,null,get_applet().menu_position_func,0,get_current_event_time());
				menu.reposition();
				return true;
			}
			ebox.get_window().get_origin(out x, out y);
			try
			{
				proxy.context_menu(x,y);
				return true;
			}
			catch (Error e) {
					stderr.printf("%s\n",e.message);
			}
			return false;
		}
		public void scroll (int delta, string direction)
		{
			try
			{
				proxy.scroll(delta,direction);
			}
			catch (Error e) {
					stderr.printf("%s\n",e.message);
			}
		}
	}
}
