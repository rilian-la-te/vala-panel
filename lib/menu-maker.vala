using GLib;

namespace MenuMaker
{
	private static void do_app_info(GLib.DesktopAppInfo info, Gtk.Builder builder)
	{
		GLib.Menu menu_link;
		if (info.should_show())
		{
			var found = false;
			var action = "app.launch_id('%s')".printf(info.get_id());
			var item = new GLib.MenuItem(info.get_name(),action);
			var missing = "application-x-executable";
			item.set_attribute("icon","s", info.get_icon() != null ?
			                   info.get_icon().serialize() : missing);
			if(info.get_description() != null)
				item.set_attribute("tooltip","s",info.get_description());
			var cats = info.get_categories().split_set(";");
			foreach (var cat in cats)
			{
				menu_link = (GLib.Menu)builder.get_object(cat.down());
				if (menu_link != null)
				{
					found = true;
					menu_link.append_item(item);
					break;
				}
			}
			if (!found)
			{
				menu_link = (GLib.Menu)builder.get_object("other");
				menu_link.append_item(item);
			}
		}
	}

	public static GLib.MenuModel applications_model(string[] cats)
	{
		var builder = new Gtk.Builder.from_resource("/org/vala/panel/lib/system-menus.ui");
		var menu = (GLib.Menu) builder.get_object("categories");
		foreach (var info in GLib.AppInfo.get_all ())
			do_app_info((GLib.DesktopAppInfo)info,builder);
		for(int i = 0; i < menu.get_n_items(); i++)
		{
			i = (i < 0) ? 0 : i;
			var submenu = (GLib.Menu)menu.get_item_link(i,GLib.Menu.LINK_SUBMENU);
			if (submenu.get_n_items() <= 0)
			{
				menu.remove(i);
				i--;
			}
			var j = (i < 0) ? 0 : i;
    		if ((string)menu.get_item_attribute_value(j,"x-cat",GLib.VariantType.STRING) in cats) 
			{
        		menu.remove(j);
        		i--;
    		}
		}
		return (GLib.MenuModel) menu;
	}

	public static static GLib.MenuModel do_applications(bool do_settings)
	{
		string[] apps_cats = {"audiovideo","education","game","graphics",
							"network","office","utility","development","other"};
		string[] settings_cats = {"settings"};
		if (do_settings)
			return applications_model (apps_cats);
		else
			return applications_model(settings_cats);
	}

	public static GLib.MenuModel do_places()
	{
		var builder = new Gtk.Builder.from_resource ("/org/vala/panel/lib/system-menus.ui");
		var menu = (GLib.Menu) builder.get_object("places-menu");
		var section = (GLib.Menu) builder.get_object("folders-section");
		var item = new GLib.MenuItem(_("Home"),null);
		item.set_attribute("icon","s","user-home");
		string path=null;
		try 
		{
			path = GLib.Filename.to_uri (GLib.Environment.get_home_dir ());
		} 
		catch (GLib.ConvertError e){}
		item.set_action_and_target("app.launch-uri","s",path);
		section.append_item(item);
		item = new GLib.MenuItem(_("Desktop"),null);
		try 
		{
			path = GLib.Filename.to_uri (GLib.Environment.get_user_special_dir(
			                               GLib.UserDirectory.DESKTOP));
		} 
		catch (GLib.ConvertError e){}
		item.set_attribute("icon","s","user-desktop");
		item.set_action_and_target("app.launch-uri","s",path);
		section.append_item(item);
		section = (GLib.Menu) builder.get_object("recent-section");
		var info = new GLib.DesktopAppInfo("gnome-search-tool.desktop");
		if (info == null)
			info = new GLib.DesktopAppInfo("mate-search-tool.desktop");
		if (info != null)
		{
			item = new GLib.MenuItem(_("Search..."),null);
			item.set_attribute("icon","s","system-search");
			item.set_action_and_target("app.launch-id","s",info.get_id());
			section.prepend_item(item);
		}
		section.remove(1);
		return (GLib.MenuModel) menu;
	}

	public static GLib.MenuModel do_system()
	{
		var builder = new Gtk.Builder.from_resource ("/org/vala/panel/lib/system-menus.ui");
		var menu = (GLib.Menu) builder.get_object("settings-section");
		menu.append_section(null,MenuMaker.do_applications (true));
		var info = new GLib.DesktopAppInfo("gnome-control-center.desktop");
		if (info == null)
			info = new GLib.DesktopAppInfo("matecc.desktop");
		if (info == null)
			info = new GLib.DesktopAppInfo("cinnamon-settings.desktop");
		if (info == null)
			info = new GLib.DesktopAppInfo("xfce4-settings-manager.desktop");
		if (info == null)
			info = new GLib.DesktopAppInfo("kdesystemsettings.desktop");
		if (info != null)
		{
			var item = new GLib.MenuItem(_("Control center"),null);
			item.set_attribute("icon","s","preferences-system");
			item.set_action_and_target("app.launch-id","s",info.get_id());
			menu.append_item(item);
		}
		menu = (GLib.Menu) builder.get_object("system-menu");
		return (GLib.MenuModel) menu;                    
	}

	public static void append_all_sections(GLib.Menu menu1, GLib.MenuModel menu2)
	{
		for (int i = 0; i< menu2.get_n_items(); i++)
		{
			var link = menu2.get_item_link(i,GLib.Menu.LINK_SECTION);
			string label = (string)menu2.get_item_attribute_value(i,"label",
			                                                      GLib.VariantType.STRING);
			if (link != null)
				menu1.append_section(label,link);
		}
	}

	public static GLib.MenuModel do_default(bool submenus, string? icon)
	{
		var menu = new GLib.Menu();
		if (submenus)
		{
			var item = new GLib.MenuItem.submenu (_("Applications"),
			                                      MenuMaker.do_applications (false));
			item.set_attribute("icon","s",icon);
			menu.append_item(item);
			menu.append_submenu(_("Places"),MenuMaker.do_places());
			menu.append_submenu(_("System"),MenuMaker.do_system());
		}
		else
		{
			menu.append(_("Vala ValaPanel"),null);
			menu.append_section(null,MenuMaker.do_applications (false));
			var section = new GLib.Menu();
			section.append_submenu(_("Places"),MenuMaker.do_places());
			menu.append_section(null,section);
			MenuMaker.append_all_sections(menu,MenuMaker.do_system());
		}
		return (GLib.MenuModel) menu;
	}
}
