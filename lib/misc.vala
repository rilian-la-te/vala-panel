using GLib;
using Gtk;

namespace ValaPanel
{
	public static void apply_window_icon(Window w)
	{
		try{
			var icon = new Gdk.Pixbuf.from_resource("/org/vala-panel/lib/panel.png");
			w.set_icon(icon);	
		} catch (Error e)
		{
			stderr.printf("Unable to load icon: %s. Trying fallback...\n",e.message);
			w.set_icon_name("start-here-symbolic");
		}
	}
	public static void start_panels_from_dir(Gtk.Application app, string dirname)
	{
		Dir dir;
		try
		{
			dir = Dir.open(dirname,0);
		} catch (FileError e)
		{
			stdout.printf("Cannot load directory: %s\n",e.message);
			return;
		}
		string? name;
		while ((name = dir.read_name()) != null)
		{
			string cfg = GLib.Path.build_filename(dirname,name);
			if (!(cfg.contains("~") || cfg[0] =='.'))
			{
				var panel = Compat.Toplevel.load(app,cfg,name);
				if (panel != null)
					app.add_window(panel);
			}
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
	public static int apply_menu_properties(List<Widget> widgets, MenuModel menu)
	{
		var len = menu.get_n_items();
		unowned List<Widget> l = widgets;
		var i = 0;
		for(i = 0, l = widgets;(i<len)&&(l!=null);i++,l=l.next)
		{
			while (l.data is SeparatorMenuItem) l = l.next;
			var shell = l.data as Gtk.MenuItem;
			var menuw = shell.get_submenu() as Gtk.Menu;
			var menu_link = menu.get_item_link(i,"submenu");
			if (menuw !=null && menu_link !=null)
				apply_menu_properties(menuw.get_children(),menu_link);
			menu_link = menu.get_item_link(i,"section");
			if (menu_link != null)
			{
				var ret = apply_menu_properties(l,menu_link);
				for (int j=0;j<ret;j++) l = l.next;
			}
			string? str = null;
			menu.get_item_attribute(i,"icon","s",out str);
			if(str != null)
			{
				try
				{
					var icon = Icon.new_for_string(str);
					shell.set("icon",icon);
				} catch (Error e)
				{
					stderr.printf("Incorrect menu icon:%s\n", e.message);
				}
			}
			str=null;
			menu.get_item_attribute(i,"tooltip","s",out str);
			if (str != null)
				shell.set_tooltip_text(str);
		}
		return i-1;
	}
	public static void settings_as_action(ActionMap map, GLib.Settings settings, string prop)
	{
		settings.bind(prop,map,prop,SettingsBindFlags.GET|SettingsBindFlags.SET|SettingsBindFlags.DEFAULT);
		var action = settings.create_action(prop);
		map.add_action(action);
	}
	public static void settings_bind(Object map, GLib.Settings settings, string prop)
	{
		settings.bind(prop,map,prop,SettingsBindFlags.GET|SettingsBindFlags.SET|SettingsBindFlags.DEFAULT);
	}
	
	public static void setup_button(Button b, Image? img = null, string? label = null)
	{
		PanelCSS.apply_from_resource(b,"/org/vala-panel/lib/style.css","-panel-button");
//Children hierarhy: button => alignment => box => (label,image)
		b.notify.connect((a,b)=>{
			if (b.name == "label" || b.name == "image")
			{
				var B = a as Bin;
				var w = B.get_child();
				if (w is Container)
				{
					Bin? bin;
					Widget ch;
					if (w is Bin)
					{
						bin = w as Bin;
						ch = bin.get_child();
					}
					else
						ch = w;
					if (ch is Container)
					{
						var cont = ch as Container;
						cont.forall((c)=>{
							if (c is Widget){
							c.set_halign(Gtk.Align.FILL);
							c.set_valign(Gtk.Align.FILL);
							}});
					}
					ch.set_halign(Gtk.Align.FILL);
					ch.set_valign(Gtk.Align.FILL);
				}
			}
		});
		if (img != null)
		{
			b.set_image(img);
			b.set_always_show_image(true);
		}
		if (label != null)
			b.set_label(label);
		b.set_relief(Gtk.ReliefStyle.NONE);
	}
	public static void setup_icon(Image img, Icon icon, Compat.Toplevel? top = null, int size = -1)
	{
		img.set_from_gicon(icon,IconSize.INVALID);
		if (top != null)
			top.bind_property(Compat.Key.ICON_SIZE,img,"pixel-size",BindingFlags.DEFAULT|BindingFlags.SYNC_CREATE);
		else if (size > 0)
			img.set_pixel_size(size);
		Gtk.IconTheme.get_default().changed.connect(()=>{
			Icon i;
			img.get_gicon(out i, IconSize.INVALID);
			img.set_from_gicon(i,IconSize.INVALID);
		});
	}
	public static void setup_icon_button(Button btn, Icon? icon = null, string? label = null, Compat.Toplevel? top = null)
	{
		PanelCSS.apply_from_resource(btn,"/org/vala-panel/lib/style.css","-panel-icon-button");
		PanelCSS.apply_with_class(btn,"",Gtk.STYLE_CLASS_BUTTON,false);
		Image? img = null;
		if (icon != null)
		{
			img = new Image.from_gicon(icon,IconSize.INVALID);
			setup_icon(img,icon,top);
		}
		setup_button(btn, img, label);
		btn.set_border_width(0);
		btn.set_can_focus(false);
		btn.set_has_window(false);
	}
	/* Draw text into a label, with the user preference color and optionally bold. */
	public static void setup_label(Label label, string text, bool bold, double factor)
	{
		label.set_text(text);
		var css = PanelCSS.generate_font_label(factor,bold);
		PanelCSS.apply_with_class(label as Widget,css,"-vala-panel-font-label",true);
	}
	public static void scale_button_set_range(ScaleButton b, int lower, int upper)
	{
		var a = b.get_adjustment();
		a.set_lower(lower);
		a.set_upper(upper);
		a.set_step_increment(1);
		a.set_page_increment(5);
	}
	public static void scale_button_set_value_labeled(ScaleButton b, int val)
	{
		b.set_value(val);
		b.set_label("%d".printf(val));
	}
}
