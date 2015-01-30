using Gtk;
using Gdk;

namespace PanelCSS
{
	public void apply_with_class(Gtk.Widget w, string css, string klass, bool add)
	{
		var context = w.get_style_context();
		w.reset_style();
		if (!add)
		{
			context.remove_class(klass);
		}
		else
		{
			var provider = new Gtk.CssProvider();
			try 
			{
				provider.load_from_data(css,css.length);
				context.add_provider(provider,Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);
				context.add_class(klass);
			} catch (GLib.Error e) {}
		}
	}
	public Gtk.CssProvider? apply_with_provider(Gtk.Widget w, string css, string klass)
	{
		var context = w.get_style_context();
		w.reset_style();
		var provider = new Gtk.CssProvider();
		try 
		{
			provider.load_from_data(css,css.length);
			context.add_provider(provider,Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);
			context.add_class(klass);
			return provider;
		} catch (GLib.Error e) {}
		return null;
	}
	
	public Gtk.CssProvider? apply_from_file_to_app_with_provider(string file)
	{
		var provider = new Gtk.CssProvider();
		try 
		{
			provider.load_from_path(file);
			Gtk.StyleContext.add_provider_for_screen(Gdk.Screen.get_default(),provider,Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);
			return provider;
		} catch (GLib.Error e)
		{
			stderr.printf("Cannot apply custom style: %s\n",e.message);
		}
		return null;
	}
	
	public void apply_from_resource(Gtk.Widget w, string file, string klass)
	{
		var context = w.get_style_context();
		w.reset_style();
		var provider = new Gtk.CssProvider();
		File ruri = File.new_for_uri("resource:/%s".printf(file));
		try 
		{
			provider.load_from_file(ruri);
			context.add_provider(provider,Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);
			context.add_class(klass);
		} catch (GLib.Error e) {}
	}
	
	public Gtk.CssProvider? apply_from_resource_with_provider(Gtk.Widget w, string file, string klass)
	{
		var context = w.get_style_context();
		w.reset_style();
		var provider = new Gtk.CssProvider();
		File ruri = File.new_for_uri("resource:/%s".printf(file));
		try 
		{
			provider.load_from_file(ruri);
			context.add_provider(provider,Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);
			context.add_class(klass);
			return provider;
		} catch (GLib.Error e) {}
		return null;
	}
	
	public string generate_background(string? name, Gdk.RGBA color)
	{
		if (name != null)
			return ".-vala-panel-background{
			 background-color: transparent;
			 background-image: url('%s');
			}".printf(name);
		return ".-vala-panel-background{
			 background-color: %s;
			 background-image: none;
			}".printf(color.to_string());
	}
	public string generate_font_size(int size)
	{
		return ".-vala-panel-font-size{
				font-size: %dpx;
				}".printf(size);
	}
	public string generate_font_color(Gdk.RGBA color)
	{
		return ".-vala-panel-font-color{
				color: %s;
				}".printf(color.to_string());
	}
	public string generate_font_label(double size ,bool bold)
	{
		int size_factor = (int)size * 100;
		return ".-vala-panel-font-label{
				 font-size: %d%%;
				 font-weight: %s;
				}".printf(size_factor,bold ? "bold" : "normal");
	}
	public string generate_flat_button(Gtk.Widget w, Gtk.PositionType e)
	{
		var flags = w.get_state_flags();
		var pass = w.get_style_context().get_color(flags);
		var act = w.get_style_context().get_color(flags);
		pass.alpha = 0.7;
		act.alpha = 0.9;
		string? edge = null;
		switch (e)
		{
			case Gtk.PositionType.TOP:
				edge = "2px 0px 0px 0px";
				break;
			case Gtk.PositionType.BOTTOM:
				edge = "0px 0px 2px 0px";
				break;
			case Gtk.PositionType.LEFT:
				edge = "0px 0px 0px 2px";
				break;
			case Gtk.PositionType.RIGHT:
				edge = "0px 2px 0px 0px";
				break;
		}
#if HAVE_GTK313
		var checked = ".-panel-flat-button:checked,";
#else
		var checked = "";
#endif
		return ".-panel-flat-button {
               padding: 0px;
                -GtkWidget-focus-line-width: 0px;
                -GtkWidget-focus-padding: 0px;
               border-style: solid;
               border-color: transparent;
               border-width: %s;
               }\n

               %s
               .-panel-flat-button:active {
               border-style: solid;
               border-width: %s;
               border-color: %s;
               }
               .-panel-flat-button:hover,
               .-panel-flat-button.highlight,
               .-panel-flat-button:active:hover {
               border-style: solid;
               border-width: %s;
               border-color: %s;
               }".printf(edge,checked,edge,pass.to_string(),edge,act.to_string());
	}
}
