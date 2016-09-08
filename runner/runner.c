#include "runner.h"
#include <stdbool.h>
#include <string.h>

// namespace Budgie {
///**
// * The meat of the operation
// */
// public class RunDialog : Gtk.ApplicationWindow
//{

//    Settings? settings = null;
//    Gtk.CssProvider? css_provider = null;
//    private string current_theme_uri;
//    Gtk.Revealer bottom_revealer;
//    Gtk.ListBox? app_box;
//    Gtk.SearchEntry entry;

//    string search_text = "";

//    public RunDialog(Gtk.Application app)
//    {
//        Object(application: app);
//        set_keep_above(true);
//        set_skip_pager_hint(true);
//        set_skip_taskbar_hint(true);
//        set_position(Gtk.WindowPosition.CENTER);
//        Gdk.Visual? visual = screen.get_rgba_visual();
//        if (visual != null) {
//            this.set_visual(visual);
//        }

//        var header = new Gtk.EventBox();
//        set_titlebar(header);
//        header.get_style_context().remove_class("titlebar");

//        var gtksettings = Gtk.Settings.get_default();

//        settings = new GLib.Settings("com.solus-project.budgie-panel");
//        settings.bind("dark-theme", gtksettings, "gtk-application-prefer-dark-theme",
//        SettingsBindFlags.GET);
//        settings.changed.connect(on_settings_changed);

//        gtksettings.notify["gtk-theme-name"].connect(on_theme_changed);
//        on_settings_changed("builtin-theme");

//        get_style_context().add_class("budgie-run-dialog");

//        key_release_event.connect(on_key_release);

//        var main_layout = new Gtk.Box(Gtk.Orientation.VERTICAL, 0);
//        add(main_layout);

//        /* Main layout, just a hbox with search-as-you-type */
//        var hbox = new Gtk.Box(Gtk.Orientation.HORIZONTAL, 0);
//        main_layout.pack_start(hbox, false, false, 0);

//        this.entry = new Gtk.SearchEntry();
//        entry.changed.connect(on_search_changed);
//        entry.activate.connect(on_search_activate);
//        entry.get_style_context().set_junction_sides(Gtk.JunctionSides.BOTTOM);
//        hbox.pack_start(entry, true, true, 0);

//        bottom_revealer = new Gtk.Revealer();
//        main_layout.pack_start(bottom_revealer, true, true, 0);
//        app_box = new Gtk.ListBox();
//        app_box.set_selection_mode(Gtk.SelectionMode.SINGLE);
//        app_box.set_activate_on_single_click(true);
//        app_box.row_activated.connect(on_row_activate);
//        app_box.set_filter_func(this.on_filter);
//        var scroll = new Gtk.ScrolledWindow(null, null);
//        scroll.get_style_context().set_junction_sides(Gtk.JunctionSides.TOP);
//        scroll.set_size_request(-1, 300);
//        scroll.add(app_box);
//        scroll.set_policy(Gtk.PolicyType.NEVER, Gtk.PolicyType.AUTOMATIC);
//        bottom_revealer.add(scroll);

//        /* Just so I can debug for now */
//        bottom_revealer.set_reveal_child(false);

//        this.build_app_box();

//        set_size_request(240, -1);
//        main_layout.show_all();
//        set_border_width(0);
//        set_resizable(false);

//        focus_out_event.connect(()=> {
//            this.application.quit();
//            return Gdk.EVENT_STOP;
//        });
//    }

//    /**
//     * Handle click/<enter> activation on the main list
//     */
//    void on_row_activate(Gtk.ListBoxRow row)
//    {
//        var child = (row as Gtk.Bin).get_child() as AppLauncherButton;
//        this.launch_button(child);
//    }

//    /**
//     * Handle <enter> activation on the search
//     */
//    void on_search_activate()
//    {
//        AppLauncherButton? act = null;
//        foreach (var row in app_box.get_children()) {
//            if (row.get_visible() && row.get_child_visible()) {
//                act = (row as Gtk.Bin).get_child() as AppLauncherButton;
//                break;
//            }
//        }
//        if (act != null) {
//            this.launch_button(act);
//        }
//    }

//    /**
//     * Launch the given preconfigured button
//     */
//    void launch_button(AppLauncherButton button)
//    {
//        try {
//            var dinfo = button.app_info as DesktopAppInfo;
//            dinfo.launch_uris_as_manager(null, null,
//                SpawnFlags.SEARCH_PATH,
//                null, null);
//            this.hide();
//            /* Allow dbus activation to happen.. which we'll never be told about. Woo. */
//            Timeout.add(500, ()=> {
//                this.destroy();
//                return false;
//            });
//        } catch (Error e) {
//            message("Error: %s\n", e.message);
//            this.application.quit();
//        }
//    }

//    void on_search_changed()
//    {
//        this.search_text = entry.get_text().down();
//        this.app_box.invalidate_filter();
//        Gtk.Widget? active_row = null;

//        foreach (var row in app_box.get_children()) {
//            if (row.get_visible() && row.get_child_visible()) {
//                active_row = row;
//                break;
//            }
//        }

//        if (active_row == null) {
//            bottom_revealer.set_transition_type(Gtk.RevealerTransitionType.SLIDE_UP);
//            bottom_revealer.set_reveal_child(false);
//        } else {
//            bottom_revealer.set_transition_type(Gtk.RevealerTransitionType.SLIDE_DOWN);
//            bottom_revealer.set_reveal_child(true);
//            app_box.select_row(active_row as Gtk.ListBoxRow);
//        }
//    }

//    /**
//     * Filter the list
//     */
//    bool on_filter(Gtk.ListBoxRow row)
//    {
//        var button = row.get_child() as AppLauncherButton;
//        var disp_name = button.app_info.get_name().down();

//        if (search_text == "") {
//            return false;
//        }

//        if (this.search_text in disp_name) {
//            return true;
//        }
//        return false;
//    }

//    /**
//     * Build the app box in the background
//     */
//    void build_app_box()
//    {
//        var apps = AppInfo.get_all();
//        apps.foreach(this.add_application);
//        app_box.show_all();
//        this.entry.set_text("");
//    }

//    void add_application(AppInfo? app_info)
//    {
//        var dinfo = app_info as DesktopAppInfo;
//        if (dinfo.get_nodisplay()) {
//            return;
//        }
//        var button = new AppLauncherButton(app_info);
//        app_box.add(button);
//        button.show_all();
//    }

//    /**
//     * Be a good citizen and pretend to be a dialog.
//     */
//    bool on_key_release(Gdk.EventKey btn)
//    {
//        if (btn.keyval == Gdk.Key.Escape) {
//            Idle.add(()=> {
//                this.application.quit();
//                return false;
//            });
//            return Gdk.EVENT_STOP;
//        }
//        return Gdk.EVENT_PROPAGATE;
//    }

//    /**
//     * Handle change to builtin-theme
//     */
//    void on_settings_changed(string key)
//    {
//        if (key != "builtin-theme") {
//            return;
//        }
//        if (settings.get_boolean(key)) {
//            this.current_theme_uri = Budgie.form_theme_path("theme.css");
//        } else {
//            this.current_theme_uri = null;
//        }

//        on_theme_changed();
//    }

//    /**
//     * Set the CSS according to the current theme
//     */
//    void set_css_from_uri(string? uri)
//    {
//        var screen = Gdk.Screen.get_default();
//        Gtk.CssProvider? new_provider = null;

//        if (uri == null) {
//            if (this.css_provider != null) {
//                Gtk.StyleContext.remove_provider_for_screen(screen, this.css_provider);
//                this.css_provider = null;
//            }
//            return;
//        }

//        try {
//            var f = File.new_for_uri(uri);
//            new_provider = new Gtk.CssProvider();
//            new_provider.load_from_file(f);
//        } catch (Error e) {
//            warning("Error loading theme: %s", e.message);
//            new_provider = null;
//            return;
//        }

//        if (css_provider != null) {
//            Gtk.StyleContext.remove_provider_for_screen(screen, css_provider);
//            css_provider = null;
//        }

//        css_provider = new_provider;

//        Gtk.StyleContext.add_provider_for_screen(screen, css_provider,
//        Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION);
//    }

//    /**
//     * Update our theming based on the internal theme setting
//     */
//    void on_theme_changed()
//    {
//        var gtksettings = Gtk.Settings.get_default();

//        if (gtksettings.gtk_theme_name == "HighContrast") {
//            set_css_from_uri(this.current_theme_uri == null ? null :
//            Budgie.form_theme_path("theme_hc.css"));
//        } else {
//            /* In future we'll actually support custom themes.. */
//            set_css_from_uri(this.current_theme_uri);
//        }
//    }
//}

///**
// * GtkApplication for single instance wonderness
// */
// public class RunDialogApp : Gtk.Application
//{

//    private RunDialog? rd = null;

//    public RunDialogApp()
//    {
//        Object(application_id: "com.solus_project.BudgieRunDialog", flags: 0);
//    }

//    public override void activate()
//    {
//        if (rd == null) {
//            rd = new RunDialog(this);
//        }
//        rd.present();
//    }
//}

//} /* End namespace */

GtkWidget *create_widget_func(GAppInfo *info, const char *command, bool is_bootstrap)
{
	GtkBox *box = GTK_BOX(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2));
	g_object_set_data_full(G_OBJECT(box),
	                       "button-id",
	                       is_bootstrap ? (gpointer) "bootstrap" : (gpointer)info,
	                       is_bootstrap ? NULL : g_object_unref);

	gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(box)),
	                            "launcher-button");
	GtkImage *image =
	    GTK_IMAGE(gtk_image_new_from_gicon(g_app_info_get_icon(info), GTK_ICON_SIZE_DIALOG));
	gtk_image_set_pixel_size(image, 48);
	gtk_widget_set_margin_start(GTK_WIDGET(image), 8);
	gtk_box_pack_start(box, GTK_WIDGET(image), false, false, 0);

	g_autofree char *nom =
	    g_markup_escape_text(g_app_info_get_name(info), strlen(g_app_info_get_name(info)));
	const char *sdesc =
	    g_app_info_get_description(info) ? g_app_info_get_description(info) : "";
	g_autofree char *desc   = g_markup_escape_text(sdesc, strlen(sdesc));
	g_autofree char *markup = g_strdup_printf("<big>%s</big>\n<small>%s</small>", nom, desc);
	GtkLabel *label         = GTK_LABEL(gtk_label_new(markup));
	gtk_style_context_add_class(gtk_widget_get_style_context(GTK_WIDGET(label)), "dim-label");
	gtk_label_set_line_wrap(label, true);
	g_object_set(label, "xalign", 0.0, NULL);
	gtk_label_set_use_markup(label, true);
	gtk_widget_set_margin_start(GTK_WIDGET(label), 12);
	gtk_label_set_max_width_chars(label, 60);
	gtk_widget_set_halign(GTK_WIDGET(label), GTK_ALIGN_START);
	gtk_box_pack_start(box, GTK_WIDGET(label), false, false, 0);

	gtk_widget_set_hexpand(GTK_WIDGET(box), false);
	gtk_widget_set_vexpand(GTK_WIDGET(box), false);
	gtk_widget_set_halign(GTK_WIDGET(box), GTK_ALIGN_START);
	gtk_widget_set_valign(GTK_WIDGET(box), GTK_ALIGN_START);
	gtk_widget_set_tooltip_text(GTK_WIDGET(box), g_app_info_get_name(info));
	gtk_widget_set_margin_top(GTK_WIDGET(box), 3);
	gtk_widget_set_margin_bottom(GTK_WIDGET(box), 3);
	return GTK_WIDGET(box);
}
