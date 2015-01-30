using GLib;

namespace ValaPanel
{
	private static const string PLUGIN_SCHEMA = "org.simple.panel.toplevel.plugin";
	private static const string PANEL_SCHEMA = "org.simple.panel.toplevel";
	private static const string PANEL_PATH = "/org/simple/panel/toplevel/";
	private static const string ROOT_NAME = "toplevel-settings";

	namespace Key
	{
		public static const string NAME = "plugin-type";
		private static const string SCHEMA = "has-schema";
		public static const string EXPAND = "is-expanded";
		public static const string CAN_EXPAND = "can-expand";
		public static const string PADDING = "padding";
		public static const string BORDER = "border";
		public static const string POSITION = "position";
	}
	
	public class PluginSettings : GLib.Object
	{
		internal string path_append;
		internal GLib.Settings default_settings;
		internal GLib.Settings config_settings;
		internal uint number;

		public PluginSettings(ToplevelSettings settings, string name, uint num)
		{
			this.number = num;
			this.path_append = name;
			var path = "%s%u/".printf(settings.root_path,this.number); 
			this.default_settings = new GLib.Settings.with_backend_and_path(
				                                PLUGIN_SCHEMA, settings.backend, path);
		}
		public void init_configuration(ToplevelSettings settings, bool has_config)
		{
			default_settings.set_boolean(Key.SCHEMA,has_config);
			if (has_config)
			{
				var id = "%s.%s".printf(settings.root_schema, this.path_append);
				var path = "%s%u/".printf(settings.root_path,this.number);
				this.config_settings = new GLib.Settings.with_backend_and_path(
				                                id, settings.backend, path);
			}
		}
	}

	public class ToplevelSettings : GLib.Object
	{
		public unowned GLib.SList<PluginSettings> plugins {
			public get; private set;
		}
		public GLib.Settings settings {
			public get; private set;
		}
		internal GLib.SettingsBackend backend {
			internal get; private set;
		}
		public string filename {
			public get; private set;
		}
		internal string root_name {
			internal get; private set;
		}		
		internal string root_schema {
			internal get; private set;
		}		
		internal string root_path {
			internal get; private set;
		}
		
		internal ToplevelSettings.full(string file, string schema, string path, string? root)
		{
			this.filename = file;
			this.root_name = root;
			this.root_path = path;
			this.root_schema = schema;
			backend = GLib.keyfile_settings_backend_new(file,path,root);
			settings = new GLib.Settings.with_backend_and_path (schema,backend,path);
		}

		public ToplevelSettings (string file)
		{
			ToplevelSettings.full(file,PANEL_SCHEMA,PANEL_PATH,ROOT_NAME);
		}

		public uint find_free_num()
		{
			var f = new GLib.KeyFile();
			try{
			f.load_from_file(this.filename,GLib.KeyFileFlags.KEEP_COMMENTS);
			} catch (GLib.KeyFileError e) {} catch (GLib.FileError e) {}
			var numtable = new GLib.HashTable<int,int>(direct_hash,direct_equal);
			var len = f.get_groups().length;
			foreach (var grp in f.get_groups())
			{
				if (grp == this.root_name)
					continue;
				numtable.insert(int.parse(grp),int.parse(grp));
			}
			for (var i = 0; i < len; i++)
				if (!numtable.contains(i))
					return i;
			return len+1;
		}
		
		internal PluginSettings add_plugin_settings_full(string name, uint num)
		{
			var settings = new PluginSettings(this,name,num);
			plugins.append(settings);
			return settings;
		}

		public PluginSettings add_plugin_settings(string name)
		{
			var num = find_free_num ();
			var settings = new PluginSettings(this,name,num);
			plugins.append(settings);
			return settings;
		}
		
		public void remove_plugin_settings(uint num)
		{
			foreach (var tmp in plugins)
			{
				if (tmp.number == num)
				{
					plugins.remove(tmp);
					var f = new GLib.KeyFile();
					try
					{
						f.load_from_file(this.filename,GLib.KeyFileFlags.KEEP_COMMENTS);
						if (f.has_group(num.to_string()))
						{
							f.remove_group(num.to_string());
							f.save_to_file(this.filename);
						}
					}
					catch (GLib.KeyFileError e) {} catch (GLib.FileError e) {}
				}
			}
		}
		public bool init_plugin_list()
		{
			if (plugins != null)
				return false;
			var f = new GLib.KeyFile();
			try {
				f.load_from_file(this.filename,GLib.KeyFileFlags.KEEP_COMMENTS);
			}
			catch (Error e)
			{
				stderr.printf("Cannot load config file: %s\n",e.message);
				return false;
			}
			var groups = f.get_groups();
			foreach (var group in groups)
			{
				try 
				{
					var name = f.get_string(group,Key.NAME);
					name = name._delimit("'",' ')._strip();
					var config = f.get_boolean(group,Key.SCHEMA);
					var s = add_plugin_settings_full(name,int.parse(group));
					s.init_configuration(this,config);
				}
				catch (GLib.KeyFileError e)
				{
					try{
						f.remove_group(group);
					} catch (GLib.KeyFileError e) {}
				}
			}
			return true;
		}
		internal PluginSettings? get_settings_by_num(uint num)
		{
			foreach (var pl in plugins)
				if (pl.number == num)
					return pl;
			return null;
		}
	}
}
