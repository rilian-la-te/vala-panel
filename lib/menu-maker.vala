using GLib;
using Gtk;

namespace MenuMaker
{
	
	public static void append_all_sections(GLib.Menu menu1, GLib.MenuModel menu2)
	{
		for (int i = 0; i< menu2.get_n_items(); i++)
		{
			var link = menu2.get_item_link(i,GLib.Menu.LINK_SECTION);
			var labelv = menu2.get_item_attribute_value(i,"label",
			                                            GLib.VariantType.STRING);
			var label = labelv != null ? labelv.get_string(): null;
			if (link != null)
				menu1.append_section(label,link);
		}
	}

	public static void activate_menu_launch_id(SimpleAction action, Variant? param)
	{
		var id = param.get_string();
		var info = new DesktopAppInfo(id);
		try{
		info.launch(null,Gdk.Display.get_default().get_app_launch_context());
		} catch (GLib.Error e){stderr.printf("%s\n",e.message);}
	}
	
	public static void activate_menu_launch_command(SimpleAction? action, Variant? param)
	{
		var command = param.get_string();
		try{
		GLib.AppInfo info = AppInfo.create_from_commandline(command,null,
					AppInfoCreateFlags.SUPPORTS_STARTUP_NOTIFICATION);
		info.launch(null,Gdk.Display.get_default().get_app_launch_context());
		} catch (GLib.Error e){stderr.printf("%s\n",e.message);}
	}
	
	public static void activate_menu_launch_uri(SimpleAction action, Variant? param)
	{
		var uri = param.get_string();
		try{
		GLib.AppInfo.launch_default_for_uri(uri,
					Gdk.Display.get_default().get_app_launch_context());
		} catch (GLib.Error e){stderr.printf("%s\n",e.message);}
	}
	public static void apply_menu_properties(List<Widget> w, MenuModel menu)
	{
		unowned List<Widget> l = w;
		for(var i = 0; i < menu.get_n_items(); i++)
		{
			var jumplen = 1;
			if (l.data is SeparatorMenuItem) l = l.next;
			var shell = l.data as Gtk.MenuItem;
			string? str = null;
			Variant? val = null;
			MenuAttributeIter attr_iter = menu.iterate_item_attributes(i);
			while(attr_iter.get_next(out str,out val))
			{
				if (str == "icon")
					shell.set("icon",Icon.deserialize(val));
				if (str == "tooltip")
					shell.set_tooltip_text(val.get_string());
			}
			var menuw = shell.submenu;
			MenuLinkIter iter = menu.iterate_item_links(i);
			MenuModel? link_menu;
			while (iter.get_next(out str, out link_menu))
			{
				if (menuw != null && str == "submenu")
					apply_menu_properties(menuw.get_children(),link_menu);
				else if (str == "section")
				{
					jumplen += (link_menu.get_n_items() - 1);
					apply_menu_properties(l,link_menu);
				}
			}
			l = l.nth(jumplen);
			if (l == null) break;
		}
	}
}
