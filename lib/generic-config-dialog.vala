using GLib;
using Gtk;
using Config;

namespace ValaPanel.Configurator
{
	public enum GenericConfigType
	{
		STR,
	    INT,
	    BOOL,
	    FILE,
	    FILE_ENTRY,
	    DIRECTORY_ENTRY,
	    TRIM,
	    EXTERNAL
	}
	private Dialog generic_config_dlg_internal(string title, Toplevel p,
											   Applet plugin,
	                                           va_list args)
	{
	    Dialog dlg = new Dialog.with_buttons( title, p, DialogFlags.DESTROY_WITH_PARENT,
	                                         _("_Close"),
	                                          ResponseType.CLOSE,
	                                          null );
	    Box dlg_vbox = dlg.get_content_area() as Box;
	
	    apply_window_icon(dlg as Window);
	
	    dlg_vbox.spacing = 4;
	
	    while( true )
	    {
			string? name = args.arg();
			if (name == null) break;
	        var label = new Label(name);
	        Widget? entry = null;
	        Widget? w = null;
	        void* arg = args.arg();
	        string? key = null;
			GenericConfigType type = args.arg();
	        if (type == GenericConfigType.EXTERNAL)
				w = (Widget)arg;
			else
				key = (string)arg;
			if (type != GenericConfigType.TRIM && type != GenericConfigType.EXTERNAL && key == null)
	            critical("NULL pointer for generic config dialog");
	        else switch( type )
	        {
	            case GenericConfigType.STR:
	            case GenericConfigType.FILE_ENTRY: /* entry with a button to browse for files. */
	            case GenericConfigType.DIRECTORY_ENTRY: /* entry with a button to browse for directories. */
	                entry = new Entry();
	                (entry as Entry).set_width_chars(40);
	                plugin.settings.bind(key,entry,"text",SettingsBindFlags.DEFAULT);
	                break;
	            case GenericConfigType.INT:
	            {
	                /* FIXME: the range shouldn't be hardcoded */
	                entry = new SpinButton.with_range( 0, 1000, 1 );
					plugin.settings.bind(key,entry,"value",SettingsBindFlags.DEFAULT);
	                break;
	            }
	            case GenericConfigType.BOOL:
	                entry = new CheckButton();
	                (entry as Container).add(label);
	                plugin.settings.bind(key,entry,"active",SettingsBindFlags.DEFAULT);
	                break;
	            case GenericConfigType.FILE:
	                entry = new FileChooserButton(_("Select a file"), FileChooserAction.OPEN);
					(entry as FileChooser).set_filename(plugin.settings.get_string(key));
					(entry as FileChooserButton).file_set.connect(()=>{
						plugin.settings.set_string(key,(entry as FileChooser).get_filename());
					});
	                break;
	            case GenericConfigType.TRIM:
	                entry = new Label(null);
	                string markup = Markup.printf_escaped ("<span style=\"italic\">%s</span>", name);
	                (entry as Label).set_markup (markup);
	                break;
	            case GenericConfigType.EXTERNAL:
	                if (w is Widget)
	                    dlg_vbox.pack_start(w, false, false, 2);
	                else
	                    critical("value for GenericConfigType.EXTERNAL is not a Gtk.Widget");
	                break;
	        }
	        if( entry != null)
	        {
	            if(( type == GenericConfigType.BOOL ) || ( type == GenericConfigType.TRIM ))
	                dlg_vbox.pack_start(entry, false, false, 2);
	            else
	            {
	                Box hbox = new Box(Orientation.HORIZONTAL, 2 );
	                hbox.pack_start(label, false, false, 2 );
	                hbox.pack_start(entry, true, true, 2 );
	                dlg_vbox.pack_start(hbox, false, false, 2 );
	                if ((type == GenericConfigType.FILE_ENTRY) || (type == GenericConfigType.DIRECTORY_ENTRY))
	                {
	                    Button browse = new Button.with_mnemonic(_("_Browse"));
	                    hbox.pack_start(browse, true, true, 2 );
	                    var txtentry = entry as Entry;
						var action = (type == GenericConfigType.FILE_ENTRY)
										? FileChooserAction.OPEN
										: FileChooserAction.SELECT_FOLDER;
						browse.clicked.connect(()=>{
							var fc = new FileChooserDialog(
											(action == FileChooserAction.SELECT_FOLDER) ? _("Select a directory") : _("Select a file"),
											dlg,
											action,
											_("_Cancel"), ResponseType.CANCEL,
											_("_OK"), ResponseType.OK,
											null);
							if (txtentry.text != null) fc.set_filename(txtentry.text);
							var res = fc.run();
							if (res == ResponseType.OK)
								txtentry.text = fc.get_filename();
						});
	                }
	            }
	        }
	    }
		dlg.response.connect((id)=>{
			dlg.destroy();
		});
	    dlg.border_width = 8;
	    dlg_vbox.show_all();
	    return dlg;
	}
	
	public static Dialog generic_config_dlg(string title, Toplevel toplevel,
									Applet applet, ...)
	{
		Dialog dlg;
		var l = va_list();
		dlg = generic_config_dlg_internal(title,toplevel,applet,l);
		return dlg;
	}	
}
