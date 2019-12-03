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
using Gtk;
using GLib;
using Budgie;
using StatusNotifier;

public class SNApplet : Budgie.Plugin, Peas.ExtensionBase
{
    public Applet get_panel_widget(string uid)
    {
        return new SNTray(uid);
    }
}
public class SNTray: Budgie.Applet
{
    unowned ItemBox widget;
    GLib.Settings settings;
    public string uuid { public set ; public get; }
    public override Gtk.Widget? get_settings_ui()
    {
        var dlg = new ConfigWidget(widget);
        dlg.configure_icon_size = false;
        return dlg;
    }

    public override bool supports_settings()
    {
        return true;
    }
    public SNTray (string uuid)
    {
        Object(uuid: uuid);
        var layout = new ItemBox();
        widget = layout;
        settings_schema = "org.valapanel.sntray";
        settings_prefix = "/com/solus-project/budgie-panel/instance/sntray";
        settings = get_applet_settings(uuid);
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
        layout.orientation = Orientation.VERTICAL; //Just setup
        panel_size_changed.connect((p,i,s)=> {
            layout.indicator_size = (int)s;
        });
        this.add(layout);
        show_all();
    }
    protected override void panel_position_changed(Budgie.PanelPosition position)
    {
        if (position == Budgie.PanelPosition.LEFT || position == Budgie.PanelPosition.RIGHT)
            widget.orientation = Gtk.Orientation.HORIZONTAL;
        else
            widget.orientation = Gtk.Orientation.VERTICAL;
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
} // End class

[ModuleInit]
public void peas_register_types(TypeModule module)
{
    // boilerplate - all modules need this
    var objmodule = module as Peas.ObjectModule;
    objmodule.register_extension_type(typeof(Budgie.Plugin), typeof(SNApplet));
}
