[CCode (cprefix = "G", lower_case_cprefix = "g_")]
namespace GLib 
{
	[CCode (cname = "g_keyfile_settings_backend_new", cheader_filename = "gio/gsettingsbackend.h")]
	public static GLib.SettingsBackend keyfile_settings_backend_new(string filename, string root_path, string? root_group);
	namespace FileUtils
	{
		[CCode (cname = "g_mkdir_with_parents", cheader_filename = "glib.h")]
		public static int mkdir_with_parents(string name, int mode);
	}
}
