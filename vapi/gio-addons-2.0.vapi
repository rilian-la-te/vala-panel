[CCode (cprefix = "G", lower_case_cprefix = "g_")]
namespace GLib
{
    [CCode (cname = "GSettingsBackend")]
    public class KeyfileSettingsBackend : GLib.SettingsBackend
    {
        [CCode (cname = "g_keyfile_settings_backend_new", cheader_filename = "gio/gsettingsbackend.h")]
        public KeyfileSettingsBackend(string filename, string root_path, string? root_group);
    }
}
