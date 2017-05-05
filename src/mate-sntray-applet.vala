/*
 * xfce4-sntray-plugin
 * Copyright (C) 2015-2017 Konstantin Pugin <ria.freelander@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

namespace StatusNotifier
{
    private Gtk.Orientation get_orientation(uint orient)
    {
        switch (orient) {
            case MatePanel.AppletOrient.UP:
            case MatePanel.AppletOrient.DOWN:
                return Gtk.Orientation.VERTICAL;
            }
        return Gtk.Orientation.HORIZONTAL;
    }
    private bool factory_callback(MatePanel.Applet applet, string iid)
    {
        if (iid != "SNTrayApplet") {
            return false;
        }

        applet.flags = MatePanel.AppletFlags.HAS_HANDLE | MatePanel.AppletFlags.EXPAND_MINOR;

        var layout = new ItemBox();
        var widget = layout;
        var settings = MatePanel.AppletSettings.new(applet,"org.valapanel.toplevel.sntray-valapanel");
        settings.bind(SHOW_APPS,layout,SHOW_APPS,SettingsBindFlags.DEFAULT);
        settings.bind(SHOW_COMM,layout,SHOW_COMM,SettingsBindFlags.DEFAULT);
        settings.bind(SHOW_SYS,layout,SHOW_SYS,SettingsBindFlags.DEFAULT);
        settings.bind(SHOW_HARD,layout,SHOW_HARD,SettingsBindFlags.DEFAULT);
        settings.bind(SHOW_OTHER,layout,SHOW_OTHER,SettingsBindFlags.DEFAULT);
        settings.bind(SHOW_PASSIVE,layout,SHOW_PASSIVE,SettingsBindFlags.DEFAULT);
        settings.bind(INDICATOR_SIZE,layout,INDICATOR_SIZE,SettingsBindFlags.DEFAULT);
        settings.bind(USE_SYMBOLIC,layout,USE_SYMBOLIC,SettingsBindFlags.DEFAULT);
        settings.bind(USE_LABELS,layout,USE_LABELS,SettingsBindFlags.DEFAULT);
        settings.bind_with_mapping(INDEX_OVERRIDE,layout,INDEX_OVERRIDE,SettingsBindFlags.DEFAULT,
                       (SettingsBindGetMappingShared)get_vardict,
                       (SettingsBindSetMappingShared)set_vardict,
                       (void*)"i",null);
        settings.bind_with_mapping(FILTER_OVERRIDE,layout,FILTER_OVERRIDE,SettingsBindFlags.DEFAULT,
                       (SettingsBindGetMappingShared)get_vardict,
                       (SettingsBindSetMappingShared)set_vardict,
                       (void*)"b",null);

        applet.change_orient.connect((orient) => {
            widget.orientation = get_orientation(orient);
        });
        widget.orientation = get_orientation(applet.orient);
        applet.add(widget);
        applet.show_all();
        var action_group = new Gtk.ActionGroup ("SNTrayApplet Menu Actions");
        action_group.set_translation_domain (Config.GETTEXT_PACKAGE);
        Gtk.Action a = new Gtk.Action("SNTrayPreferences",N_("_Preferences"),null,Gtk.Stock.PREFERENCES);
        a.activate.connect(()=>
        {
                var dlg = ConfigWidget.get_config_dialog(layout,true);
                dlg.show();
                dlg.response.connect(()=>{
                    dlg.destroy();
                });
        });
        action_group.add_action (a);
        applet.setup_menu("""<menuitem name="SNTray Preferences Item" action="SNTrayPreferences" />""",action_group);
        return true;
    }
    private static bool get_vardict(Value val, Variant variant,void* data)
    {
        var iter = variant.iterator();
        string name;
        Variant inner_val;
        var dict = new HashTable<string,Variant?>(str_hash,str_equal);
        while(iter.next("{sv}",out name, out inner_val))
        dict.insert(name,inner_val);
        val.set_boxed((void*)dict);
        return true;
    }
    private static Variant set_vardict(Value val, VariantType type,void* data)
    {
        var builder = new VariantBuilder(type);
        unowned HashTable<string,Variant?> table = (HashTable<string,Variant?>)val.get_boxed();
        table.foreach((k,v)=>{
        builder.add("{sv}",k,v);
        });
        return builder.end();
    }
}

//int _mate_panel_applet_shlib_factory()
//{
//    GLib.Intl.bindtextdomain(Config.GETTEXT_PACKAGE,Config.LOCALE_DIR);
//    GLib.Intl.bind_textdomain_codeset(Config.GETTEXT_PACKAGE,"UTF-8");
//    return MatePanel.Applet.factory_main("SNTrayAppletFactory", false, typeof (MatePanel.Applet), StatusNotifier.factory_callback);
//}

void main(string[] args) {
    Gtk.init(ref args);
    MatePanel.Applet.factory_main("SNTrayAppletFactory", true, typeof (MatePanel.Applet), StatusNotifier.factory_callback);
}
