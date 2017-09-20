namespace DBusMenu
{
    [DBus (use_string_marshalling = true)]
    public enum Status
    {
        [DBus (value = "normal")]
        NORMAL,
        [DBus (value = "notice")]
        NOTICE
    }
}
