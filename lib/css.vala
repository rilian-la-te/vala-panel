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
				context.add_provider(provider,1000);
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
			context.add_provider(provider,1000);
			context.add_class(klass);
			return provider;
		} catch (GLib.Error e) {}
		return null;
	}
	internal string generate_background(string? name, Gdk.RGBA color)
	{
		if (name != null)
			return ".-vala-panel-background{\n
			 background-color: transparent;\n
			 background-image: url('%s');\n
			}".printf(name);
		return ".-vala-panel-background{\n
			 background-color: %s;\n
			 background-image: none;\n
			}".printf(color.to_string());
	}
	internal string generate_font_size(int size)
	{
		return ".-vala-panel-font-size{\n
				font-size: %dpx;\n
				}".printf(size);
	}
	internal string generate_font_color(Gdk.RGBA color)
	{
		return ".-vala-panel-font-size{\n
				color: %s;\n
				}".printf(color.to_string());
	}
	internal string generate_font_label(double size ,bool bold)
	{
		int size_factor = (int)size * 100;
		return ".-vala-panel-font-label{\n
				 font-size: %d%%;\n
				 font-weight: %s;\n
				}".printf(size_factor,bold ? "bold" : "normal");
	}
	public string generate_flat_button(Gtk.Widget w, Gtk.PositionType e)
	{
		var flags = w.get_state_flags();
		var pass = w.get_style_context().get_color(flags);
		var act = pass.copy();
		pass.alpha = 0.5;
		act.alpha = 0.8;
		string edge = "0px 0px 0px 0px";
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
		return ".-panel-flat-button {\n
               padding: 0px;\n
                -GtkWidget-focus-line-width: 0px;\n
                -GtkWidget-focus-padding: 0px;\n
               border-style: solid;
               border-color: transparent;
               border-width: %s;
               }\n
#if HAVE_GTK313
               .-panel-flat-button:checked,
#endif
               .-panel-flat-button:active {\n
               border-style: solid;
               border-width: %s;
               border-color: %s;
               }\n
               .-panel-flat-button:hover,
               .-panel-flat-button.highlight,
               .-panel-flat-button:active:hover {\n
               border-style: solid;
               border-width: %s;
               border-color: %s;
               }\n".printf(edge,edge,act.to_string(),edge,pass.to_string());
	}
}