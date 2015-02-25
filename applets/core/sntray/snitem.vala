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
		{get; private set;}
		public Category cat
		{get; private set;}
		public string id
		{get; private set;}
		internal bool use_symbolic
		{get; set;}
		public Item (string n, ObjectPath p)
		{
			Object(object_path: p, object_name: n);
		}
		construct
		{
			var context = this.get_style_context();
			this.reset_style();
			var provider = new Gtk.CssProvider();
			File ruri = File.new_for_uri("resource://org/vala-panel/sntray/style.css");
			try
			{
				provider.load_from_file(ruri);
				context.add_provider(provider,STYLE_PROVIDER_PRIORITY_APPLICATION);
			} catch (GLib.Error e) {}
			try
			{
				uint id;
				id = Bus.watch_name(BusType.SESSION,object_name,BusNameWatcherFlags.NONE,
					null,
					() => {Bus.unwatch_name(id); get_applet().request_remove_item(this,object_name+(string)object_path);}
					);
				init_proxy.begin();
			} catch (IOError e) {stderr.printf ("%s\n", e.message); this.destroy();}
			client = null;
			this.has_tooltip = true;
			icon_theme = IconTheme.get_default();
			ebox = new EventBox();
			box = new Box(Orientation.HORIZONTAL,0);
			label = new Label(null);
			image = new Image();
			box.add(image);
			box.add(label);
			ebox.add(box);
			this.add(ebox);
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
			ebox.button_press_event.connect(button_press_event_cb);
			ebox.enter_notify_event.connect((e)=>{
				this.get_style_context().add_class("-panel-launch-button-selected");
			});
			ebox.leave_notify_event.connect((e)=>{
				this.get_style_context().remove_class("-panel-launch-button-selected");
			});

			this.query_tooltip.connect(query_tooltip_cb);
			this.popup_menu.connect(context_menu);
			icon_theme.changed.connect(()=>{
				image.set_from_gicon(image.gicon,IconSize.INVALID);
			});
			this.parent_set.connect((prev)=>{
				if (get_applet() != null)
				{
					get_applet().bind_property("icon-size",image,"pixel-size",BindingFlags.SYNC_CREATE);
					get_applet().bind_property("symbolic-icons",this,"use-symbolic",BindingFlags.SYNC_CREATE);
				}
			});
			ebox.show_all();
		}
		private async void init_proxy() throws Error
		{
			this.iface = yield Bus.get_proxy(BusType.SESSION,object_name,object_path);
			if (iface.items_in_menu || iface.menu != null)
				setup_inner_menu();
			title = iface.title;
			this.ordering_index = iface.x_ayatana_ordering_index;
			this.cat = iface.category;
			this.id = iface.id;
			iface_new_status_cb(iface.status);
			iface_new_path_cb(iface.icon_theme_path);
			iface_new_label_cb(iface.x_ayatana_label,iface.x_ayatana_label_guide);
			unbox_tooltip(iface.tool_tip,out tooltip_icon, out markup);
			iface.new_status.connect(iface_new_status_cb);
			iface.new_icon.connect(iface_new_icon_cb);
			iface.new_overlay_icon.connect(iface_new_icon_cb);
			iface.new_attention_icon.connect(iface_new_icon_cb);
			iface.new_icon_theme_path.connect(iface_new_path_cb);
			iface.x_ayatana_new_label.connect(iface_new_label_cb);
			iface.new_tool_tip.connect(iface_new_tooltip_cb);
			iface.new_title.connect(iface_new_title_cb);
			this.notify["use-symbolic"].connect(()=>{
				iface_new_icon_cb();
			});
			this.changed();
			this.show();
		}
		private bool button_press_event_cb(Gdk.EventButton e)
		{
				if (e.button == 3)
				{
					try
					{
						iface.activate((int)Math.round(e.x_root),(int)Math.round(e.y_root));
						return true;
					}
					catch (Error e) {stderr.printf("%s\n",e.message);}
				}
				else if (e.button == 2)
				{
					try
					{
						iface.x_ayatana_secondary_activate(e.time);
						return true;
					} catch (Error e){/* This only means that method not supported*/}
					try
					{
						iface.secondary_activate((int)Math.round(e.x_root),(int)Math.round(e.y_root));
						return true;
					}
					catch (Error e) {stderr.printf("%s\n",e.message);}
				}
				return false;
		}
		private void iface_new_path_cb(string? path)
		{
			if (path != null)
				IconTheme.get_default().prepend_search_path(path);
			iface_new_icon_cb();
		}
		private void iface_new_status_cb(Status status)
		{
			switch(status)
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
			tip.set_icon_from_gicon(tooltip_icon ?? image.gicon,IconSize.DIALOG);
			tip.set_markup(markup ?? accessible_desc ?? title);
			return true;
		}
		private void iface_new_label_cb(string? label, string? guide)
		{
			if (label != null)
			{
				this.label.set_text(label);
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
			if (client == null && remote_menu_model == null)
			{
				use_menumodel = !DBusMenu.GtkClient.check(object_name,iface.menu);
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
				else
				{
					client = new DBusMenu.GtkClient(object_name,iface.menu);
					client.attach_to_menu(menu);
				}
			}

		}
		public bool context_menu()
		{
			int x,y;
			if (iface.items_in_menu || iface.menu != null)
			{
				menu.hide.connect(()=>{(this.get_parent() as FlowBox).unselect_child(this);});
				menu.popup(null,null,get_applet().menu_position_func,0,get_current_event_time());
				menu.reposition();
				return true;
			}
			ebox.get_window().get_origin(out x, out y);
			try
			{
				iface.context_menu(x,y);
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
				iface.scroll(delta,direction);
			}
			catch (Error e) {
					stderr.printf("%s\n",e.message);
			}
		}
		private void iface_new_title_cb()
		{
			try
			{
				ItemIface item = Bus.get_proxy_sync(BusType.SESSION,object_name, object_path);
				this.title = item.title;
			} catch (Error e) {stderr.printf("Cannot set title: %s\n",e.message);}
		}
		private void unbox_tooltip(ToolTip tooltip, out Icon? icon, out string? markup)
		{
			var raw_text = tooltip.title + "\n" + tooltip.description;
			var is_pango_markup = true;
			if (raw_text != null)
			{
				try
				{
					Pango.parse_markup(raw_text,-1,'\0',null,null,null);
				} catch (Error e){is_pango_markup = false;}
			}
			if (!is_pango_markup)
			{
				var str = "<markup>";
				if (tooltip.title.length > 0)
					str += tooltip.title;
				if (tooltip.description.length > 0)
					str += (str.length > 8) ? "<br/>" + tooltip.description : tooltip.description;
				str += "</markup>";
				if (str.contains("&"))
					str = str.replace("&","&amp;");
				var markup_parser = new QRichTextParser(str);
				markup_parser.translate_markup();
				markup = (markup_parser.pango_markup.length > 0) ? markup_parser.pango_markup: tooltip_markup;
				var res_icon = change_icon(tooltip.icon_name, tooltip.pixmap);
				icon = (markup_parser.icon != null)	? markup_parser.icon: res_icon;
			}
			else
			{
				markup = raw_text;
				icon = change_icon(tooltip.icon_name, tooltip.pixmap);
			}
		}
		private Icon? change_icon(string? icon_name, IconPixmap[] pixmaps)
		{
			var new_name = (use_symbolic) ? icon_name+"-symbolic" : icon_name;
			if (icon_name != null && icon_name.length > 0)
			{
				if (icon_name[0] == '/')
					return new FileIcon(File.new_for_path(icon_name));
				else if (icon_theme.has_icon(icon_name)
						|| icon_theme.has_icon(new_name)
						|| iface.icon_theme_path == null
						|| iface.icon_theme_path.length == 0)
					return new ThemedIcon.with_default_fallbacks(new_name);
				else return find_file_icon(icon_name,iface.icon_theme_path);
			}
			/* FIXME: Choose pixmap size */
			else if (pixmaps.length > 0 && pixmaps[0].bytes.length > 0)
				return new BytesIcon(new Bytes(pixmaps[0].bytes));
			return null;
		}
		private Icon? find_file_icon(string? icon_name, string? path)
		{
			if (path == null || path.length == 0)
				return null;
			try
			{
				var dir = Dir.open(path);
				for (var ch = dir.read_name(); ch!= null; ch = dir.read_name())
				{
					var f = File.new_for_path(path+"/"+ch);
					if (ch[0:ch.last_index_of(".")] == icon_name)
						return new FileIcon(f);
					var t = f.query_file_type(FileQueryInfoFlags.NONE);
					Icon? ret = null;
					if (t == FileType.DIRECTORY)
						ret = find_file_icon(icon_name,path+"/"+ch);
					if (ret != null)
						return ret;
				}
			} catch (Error e) {stderr.printf("%s\n",e.message);}
			return null;
		}
		private void iface_new_tooltip_cb()
		{
			try{
				ItemIface item = Bus.get_proxy_sync(BusType.SESSION, object_name, object_path);
				unbox_tooltip(item.tool_tip,out tooltip_icon, out markup);
			} catch (Error e){stderr.printf("Cannot set tooltip:%s\n",e.message);}
		}
		private void iface_new_icon_cb()
		{
			try
			{
				ItemIface item = Bus.get_proxy_sync(BusType.SESSION, object_name, object_path);
				var main_icon = change_icon(item.icon_name,item.icon_pixmap);
				var attention_icon = change_icon(item.attention_icon_name,item.attention_icon_pixmap);
				var overlay_icon = change_icon(item.overlay_icon_name,item.overlay_icon_pixmap);
				if (overlay_icon != null)
					overlay_icon = new Emblem(overlay_icon);
				var icon = new EmblemedIcon(attention_icon ?? main_icon ?? (image.gicon as EmblemedIcon).gicon,overlay_icon as Emblem);
				if (icon != null)
				{
					image.set_from_gicon(icon,IconSize.INVALID);
					image.show();
				}
				else
					image.hide();
				if (item.attention_accessible_desc != null && item.attention_accessible_desc.length > 0)
					accessible_desc = item.attention_accessible_desc;
				else if (item.icon_accessible_desc != null && item.icon_accessible_desc.length > 0)
					accessible_desc = item.icon_accessible_desc;
				else accessible_desc = null;
			} catch (Error e) {stderr.printf("%s\n",e.message);}
		}
		ItemIface iface;
		EventBox ebox;
		Box box;
		Label label;
		Image image;
		Icon? tooltip_icon;
		string markup;
		string accessible_desc;
		string title;
		bool use_menumodel;
		DBusMenu.GtkClient? client;
		MenuModel remote_menu_model;
		GLib.ActionGroup remote_action_group;
		Gtk.Menu menu;
		IconTheme icon_theme;
	}
}
