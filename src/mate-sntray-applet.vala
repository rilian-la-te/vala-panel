namespace StatusNotifier {
    private Gtk.Orientation get_orientation(uint orient) {
    switch (orient) {
    case MatePanel.AppletOrient.UP:
    case MatePanel.AppletOrient.DOWN:
        return Gtk.Orientation.HORIZONTAL;
    }
    return Gtk.Orientation.VERTICAL;
    }

    private bool factory_callback(MatePanel.Applet applet, string iid) {
    if (iid != "SNTrayApplet") {
        return false;
    }

    applet.flags = MatePanel.AppletFlags.HAS_HANDLE;

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

    applet.change_size.connect((size) => {
        widget.indicator_size = size;
    });

    applet.add(widget);
    applet.show_all();

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

void main(string[] args) {
    Gtk.init(ref args);
    MatePanel.Applet.factory_main("SNTrayAppletFactory", true, typeof (MatePanel.Applet), StatusNotifier.factory_callback);
}
