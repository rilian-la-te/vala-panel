[CCode (prefix = "", lower_case_cprefix = "", cheader_filename = "config.h")]
namespace Config
{
/* Package information */
public const string PACKAGE_NAME;
public const string PACKAGE_STRING;
public const string PACKAGE_VERSION;
public const string VERSION;
/* Gettext package */
public const string GETTEXT_PACKAGE;
/* Configured paths - these variables are not present in config.h, they are
* passed to underlying C code as cmd line macros. */
public const string PACKAGE_DATA_DIR; 
public const string PACKAGE_LOCALE_DIR; 
public const string LOCALEDIR; 
public const string PKGDATADIR; 
public const string PKGLIBDIR; 
}
