using GLib;
using Gtk;

namespace ValaPanel
{
	[GtkTemplate (ui = "/org/vala-panel/lib/pref.ui")]
	[CCode (cname="ConfigureDialog")]
	internal class ConfigureDialog : Dialog
	{
		public Toplevel toplevel
		{get; construct;}
		private GLib.Settings settings
		{get {return toplevel.settings.settings;}}
		[GtkChild (name="edge-button")]
		MenuButton edge_button;
		[GtkChild (name="alignment-button")]
		MenuButton alignment_button;
		[GtkChild (name="monitors-button")]
		MenuButton monitors_button;		
		[GtkChild (name="spin-margin")]
		SpinButton spin_margin;
		[GtkChild (name="spin-iconsize")]
		SpinButton spin_iconsize;
		[GtkChild (name="spin-height")]
		SpinButton spin_height;
		[GtkChild (name="spin-width")]		
		SpinButton spin_width;
		[GtkChild (name="spin-corners")]		
		SpinButton spin_corners;
		[GtkChild (name="font-selector")]	
		FontButton font_selector;	
		[GtkChild (name="font-box")]	
		Box font_box;	
		[GtkChild (name="color-background")]
		ColorButton color_background;
		[GtkChild (name="color-foreground")]
		ColorButton color_foreground;		
		[GtkChild (name="chooser-background")]	
		FileChooserButton file_background;	
		[GtkChild (name="plugin-list")]	
		TreeView plugin_list;	
		[GtkChild (name="plugin-desc")]
		Label plugin_desc;
		[GtkChild (name="add-button")]
		MenuButton adding_button;
		[GtkChild (name="remove-button")]
		Button remove_button;		
		[GtkChild (name="configure-button")]
		Button configure_button;		
		[GtkChild (name="up-button")]
		Button up_button;
		[GtkChild (name="down-button")]
		Button down_button;	
		[GtkChild (name="prefs")]
		internal Stack prefs_stack;	
		
		const GLib.ActionEntry[] entries_monitor =
		{
			{"configure-monitors", null,"i","-2" ,state_configure_monitor}
		};
		
		internal ConfigureDialog(Toplevel top)
		{
			Object(toplevel: top, application: top.get_application(),window_position: WindowPosition.CENTER);
		}
		construct
		{
			Gdk.RGBA color = Gdk.RGBA();
			SimpleActionGroup conf = new SimpleActionGroup();
			apply_window_icon(this as Window);
			/* edge */
			edge_button.set_relief(ReliefStyle.NONE);
			switch(toplevel.edge)
			{
				case PositionType.TOP:
					edge_button.set_label(_("Top"));
					break;
				case PositionType.BOTTOM:
					edge_button.set_label(_("Bottom"));
					break;
				case PositionType.LEFT:
					edge_button.set_label(_("Left"));
					break;
				case PositionType.RIGHT:
					edge_button.set_label(_("Right"));
					break;
			}
			toplevel.notify["edge"].connect((pspec,data)=>
			{
				switch(toplevel.edge)
				{
					case PositionType.TOP:
						edge_button.set_label(_("Top"));
						break;
					case PositionType.BOTTOM:
						edge_button.set_label(_("Bottom"));
						break;
					case PositionType.LEFT:
						edge_button.set_label(_("Left"));
						break;
					case PositionType.RIGHT:
						edge_button.set_label(_("Right"));
						break;
				}
			});
			/* alignment */
			alignment_button.set_relief(ReliefStyle.NONE);
			switch(toplevel.alignment)
			{
				case AlignmentType.START:
					alignment_button.set_label(_("Start"));
					break;
				case AlignmentType.CENTER:
					alignment_button.set_label(_("Center"));
					break;
				case AlignmentType.END:
					alignment_button.set_label(_("End"));
					break;
			}
			toplevel.notify["alignment"].connect((pspec,data)=>
			{
				switch(toplevel.alignment)
				{
					case AlignmentType.START:
						alignment_button.set_label(_("Start"));
						break;
					case AlignmentType.CENTER:
						alignment_button.set_label(_("Center"));
						break;
					case AlignmentType.END:
						alignment_button.set_label(_("End"));
						break;
				}
				spin_margin.set_sensitive(toplevel.alignment!=AlignmentType.CENTER);
			});
			/* monitors */
			monitors_button.set_relief(ReliefStyle.NONE);
			int monitors;
			var screen = toplevel.get_screen();
		    if(screen != null)
				monitors = screen.get_n_monitors();
		    assert(monitors >= 1);
		    var menu = new GLib.Menu();
		    menu.append(_("All"),"conf.configure-monitors(-1)");
		    for (var i = 0; i < monitors; i++)
		    {
		        var tmp = "conf.configure-monitors(%d)".printf(i);
		        var str_num = "%d".printf(i+1);
		        menu.append(str_num,tmp);
		    }
		    monitors_button.set_menu_model(menu as MenuModel);
		    monitors_button.set_use_popover(true);
		    conf.add_action_entries(entries_monitor,this);
		    var v = new Variant.int32(settings.get_int(Key.MONITOR));
		    conf.change_action_state("configure-monitors",v);
		    /* margin */
		    settings.bind(Key.MARGIN,spin_margin,"value",SettingsBindFlags.DEFAULT);
		    spin_margin.set_sensitive(toplevel.alignment != AlignmentType.CENTER);
		    
		    /* size */
		    settings.bind(Key.WIDTH,spin_width,"value",SettingsBindFlags.DEFAULT);
		    spin_width.set_sensitive(!toplevel.is_dynamic);
		    settings.bind(Key.HEIGHT,spin_height,"value",SettingsBindFlags.DEFAULT);
		    settings.bind(Key.ICON_SIZE,spin_iconsize,"value",SettingsBindFlags.DEFAULT);
		    settings.bind(Key.CORNERS_SIZE,spin_corners,"value",SettingsBindFlags.DEFAULT);
			/* background */
		    IconInfo info;
		    color.parse(toplevel.background_color);
	        color_background.set_rgba(color);
			color_background.set_relief(ReliefStyle.NONE);
	        color_background.color_set.connect(()=>{
				settings.set_string(Key.BACKGROUND_COLOR,color_background.get_rgba().to_string());
			});
			settings.bind(Key.USE_BACKGROUND_COLOR,color_background,"sensitive",SettingsBindFlags.GET);
			if (toplevel.background_file != null)
				file_background.set_filename(toplevel.background_file);
	        else if ((info = IconTheme.get_default().lookup_icon("lxpanel-background", 0, 0)) != null)
	            file_background.set_filename(info.get_filename());
	        file_background.set_sensitive(toplevel.use_background_file);
			settings.bind(Key.USE_BACKGROUND_FILE,file_background,"sensitive",SettingsBindFlags.GET);
			file_background.file_set.connect(()=>{
				settings.set_string(Key.BACKGROUND_FILE,file_background.get_filename());
			});
			/* foregorund */
			color.parse(toplevel.foreground_color);
	        color_foreground.set_rgba(color);
	        color_foreground.set_relief(ReliefStyle.NONE);
	        color_foreground.color_set.connect(()=>{
				settings.set_string(Key.FOREGROUND_COLOR,color_foreground.get_rgba().to_string());
			});
			settings.bind(Key.USE_FOREGROUND_COLOR,color_foreground,"sensitive",SettingsBindFlags.GET);
			/* fonts */
			settings.bind(Key.FONT,font_selector,"font",SettingsBindFlags.DEFAULT);
			font_selector.set_relief(ReliefStyle.NONE);
			settings.bind(Key.USE_FONT,font_box,"sensitive",SettingsBindFlags.GET);
			/* plugin list */
			init_plugin_list();
			this.insert_action_group("conf",conf);
			this.insert_action_group("win",toplevel);
			this.insert_action_group("app",toplevel.application);
    	}
		private void state_configure_monitor(SimpleAction act, Variant? param)
		{
			int state = act.get_state().get_int32();
		    /* change monitor */
		    int request_mon = param.get_int32();
		    string str = request_mon < 0 ? _("All") : _("%d").printf(request_mon+1);
		    PositionType edge = (PositionType)settings.get_enum(Key.EDGE);
		    if(toplevel.panel_edge_available(edge, request_mon,false) || (state<-1))
		    {
		        settings.set_int(Key.MONITOR,request_mon);
		        act.set_state(param);
		        monitors_button.set_label(str);
		    }
		}
		private void init_plugin_list()
		{
//FIXME: Plugins handling.			
		}
	}
}
