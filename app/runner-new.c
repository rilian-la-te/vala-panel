#include <gtk/gtk.h>
#include <gio/gdesktopappinfo.h>
#include <glib/gi18n.h>
#include <string.h>
#include <unistd.h>

#include "css.h"
static GtkWidget* win = NULL; /* the run dialog */
static GAppInfoMonitor* monitor = NULL; /* apps monitor*/
static GList* app_list = NULL; /* all known apps in cache */
static GAppInfoCreateFlags terminal;
static GtkApplication* application;

const gchar* css = ".-panel-run-dialog {"
        "border-radius: 6px;"
        "border: none;"
        "}"
        ".-panel-run-header {"
        "background-image: none;"
        "background-color: transparent;"
        "}";

typedef struct _ThreadData
{
    gboolean cancel; /* is the loading cancelled */
    GSList* files; /* all executable files found */
    GtkEntry* entry;
}ThreadData;

typedef struct {
    pid_t pid;
} SpawnData;

static ThreadData* thread_data = NULL; /* thread data used to load availble programs in PATH */
static void child_spawn_func(void* data)
{
    SpawnData* d = (SpawnData*)data;
    setpgid(0,d->pid);
}

static GDesktopAppInfo* match_app_by_exec(const char* exec)
{
    GList* l;
    GDesktopAppInfo* ret = NULL;
    char* exec_path = g_find_program_in_path(exec);
    const char* pexec;
    int path_len, exec_len, len;

    if( ! exec_path )
        return NULL;

    path_len = strlen(exec_path);
    exec_len = strlen(exec);

    for( l = app_list; l; l = l->next )
    {
        GAppInfo* app = G_APP_INFO(l->data);
        const char* app_exec = g_app_info_get_executable(app);
        if ( ! app_exec)
            continue;
        if( g_path_is_absolute(app_exec) )
        {
            pexec = exec_path;
            len = path_len;
        }
        else
        {
            pexec = exec;
            len = exec_len;
        }

        if( strncmp(app_exec, pexec, len) == 0 )
        {
            /* exact match has the highest priority */
            if( app_exec[len] == '\0' )
            {
                ret = G_DESKTOP_APP_INFO(app);
                break;
            }
        }
    }

    /* if this is a symlink */
    if( ! ret && g_file_test(exec_path, G_FILE_TEST_IS_SYMLINK) )
    {
        char target[512]; /* FIXME: is this enough? */
        len = readlink( exec_path, target, sizeof(target) - 1);
        if( len > 0 )
        {
            target[len] = '\0';
            ret = match_app_by_exec(target);
            if( ! ret )
            {
                /* FIXME: Actually, target could be relative paths.
                 *        So, actually path resolution is needed here. */
                char* basename = g_path_get_basename(target);
                char* locate = g_find_program_in_path(basename);
                if( locate && strcmp(locate, target) == 0 )
                {
                    ret = match_app_by_exec(basename);
                    g_free(locate);
                }
                g_free(basename);
            }
        }
    }
    g_free(exec_path);
    return ret;
}

static void setup_auto_complete_with_data(ThreadData* data)
{
    GSList *l;
    g_autoptr(GtkEntryCompletion) comp = gtk_entry_completion_new();
    gtk_entry_completion_set_minimum_key_length( comp, 2 );
    gtk_entry_completion_set_inline_completion( comp, TRUE );
    gtk_entry_completion_set_popup_set_width( comp, TRUE );
    gtk_entry_completion_set_popup_single_match( comp, FALSE );
    g_autoptr(GtkListStore) store = gtk_list_store_new( 1, G_TYPE_STRING );

    for( l = data->files; l; l = l->next )
    {
        const char *name = (const char*)l->data;
        GtkTreeIter it;
        gtk_list_store_append( store, &it );
        gtk_list_store_set( store, &it, 0, name, -1 );
    }

    gtk_entry_completion_set_model( comp, (GtkTreeModel*)store );
    gtk_entry_completion_set_text_column( comp, 0 );
    gtk_entry_set_completion( (GtkEntry*)data->entry, comp );

    /* trigger entry completion */
    gtk_entry_completion_complete(comp);
}

static void thread_data_free(ThreadData* data)
{
    g_slist_foreach(data->files, (GFunc)g_free, NULL);
    g_slist_free(data->files);
    g_slice_free(ThreadData, data);
}

static gboolean on_thread_finished(ThreadData* data)
{
    /* don't setup entry completion if the thread is already cancelled. */
    if( !data->cancel )
        setup_auto_complete_with_data(thread_data);
    thread_data_free(data);
    thread_data = NULL; /* global thread_data pointer */
    return FALSE;
}

static gpointer thread_func(ThreadData* data)
{
    GSList *list = NULL;
    gchar **dirname;
    gchar **dirnames = g_strsplit( g_getenv("PATH"), ":", 0 );

    for( dirname = dirnames; !thread_data->cancel && *dirname; ++dirname )
    {
        GDir *dir = g_dir_open( *dirname, 0, NULL );
        const char *name;
        if( ! dir )
            continue;
        while( !thread_data->cancel && (name = g_dir_read_name(dir)) )
        {
            char* filename = g_build_filename( *dirname, name, NULL );
            if( g_file_test( filename, G_FILE_TEST_IS_EXECUTABLE ) )
            {
                if(thread_data->cancel)
                    break;
                if( !g_slist_find_custom( list, name, (GCompareFunc)strcmp ) )
                    list = g_slist_prepend( list, g_strdup( name ) );
            }
            g_free( filename );
        }
        g_dir_close( dir );
    }
    g_strfreev( dirnames );

    data->files = list;
    /* install an idle handler to free associated data */
    g_idle_add((GSourceFunc)on_thread_finished, data);

    return NULL;
}

static void setup_auto_complete( GtkEntry* entry )
{
    gboolean cache_is_available = FALSE;
    /* FIXME: consider saving the list of commands as on-disk cache. */
    if( cache_is_available )
    {
        /* load cached program list */
    }
    else
    {
        /* load in another working thread */
        thread_data = g_slice_new0(ThreadData); /* the data will be freed in idle handler later. */
        thread_data->entry = entry;
        g_thread_new("Autocompletion",(GThreadFunc)thread_func, thread_data);
    }
}

static void reload_apps(GAppInfoMonitor* cache, gpointer user_data)
{
    g_debug("reload apps!");
    if(app_list)
        g_list_free_full(app_list,(GDestroyNotify)g_object_unref);
    app_list = g_app_info_get_all();
}

static void on_response( GtkDialog* dlg, gint response, gpointer user_data )
{
    GtkEntry* entry = (GtkEntry*)user_data;
    if( G_LIKELY(response == GTK_RESPONSE_OK) )
    {
        g_autoptr(GError) err = NULL;
        g_autoptr(GAppInfo) app_info = g_app_info_create_from_commandline(gtk_entry_get_text(entry),NULL,terminal,&err);
        if (err)
        {
            g_signal_stop_emission_by_name( dlg, "response" );
            g_clear_error(&err);
            return;
        }
        SpawnData data;
        data.pid = getpgid(getppid());
        gboolean launch = g_desktop_app_info_launch_uris_as_manager(G_DESKTOP_APP_INFO(app_info),NULL,
                                            G_APP_LAUNCH_CONTEXT(gdk_display_get_app_launch_context(gdk_display_get_default())),
                                                                    G_SPAWN_SEARCH_PATH,
                                                                    child_spawn_func,
                                                                    &data,NULL,NULL,
                                                                    &err);
        if (!launch || err)
        {
            g_signal_stop_emission_by_name( dlg, "response" );
            return;
        }
    }

    /* cancel running thread if needed */
    if( thread_data ) /* the thread is still running */
        thread_data->cancel = TRUE; /* cancel the thread */

    gtk_widget_destroy( (GtkWidget*)dlg );
    win = NULL;

    /* free app list */
    g_list_free_full(app_list,(GDestroyNotify)g_object_unref);
    app_list = NULL;

    /* free menu cache */
    g_signal_handlers_disconnect_matched(monitor,G_SIGNAL_MATCH_FUNC,0,0,NULL,reload_apps,NULL);
    g_object_unref(monitor);
    monitor = NULL;
}

static void on_entry_changed( GtkEntry* entry, gpointer user_data)
{
    const char* str = gtk_entry_get_text(entry);
    GDesktopAppInfo* app = NULL;
    if( str && *str )
        app = match_app_by_exec(str);

    if( app )
    {
        gtk_entry_set_icon_from_gicon(entry,GTK_ENTRY_ICON_PRIMARY,g_app_info_get_icon(G_APP_INFO(app)));
    }
    else
    {
        gtk_entry_set_icon_from_icon_name(entry,GTK_ENTRY_ICON_PRIMARY, "application-x-executable");
    }
}

static void on_entry_activated(GtkEntry* entry, GtkDialog* dlg)
{
    gtk_dialog_response(dlg,GTK_RESPONSE_OK);
}

static void on_icon_activated(GtkEntry* entry, GtkEntryIconPosition pos, GdkEventButton* event, GtkDialog* dlg)
{
    if (pos == GTK_ENTRY_ICON_SECONDARY)
    {
        if (event->button == 1)
            gtk_dialog_response(dlg, GTK_RESPONSE_OK);
    }
}

static void on_terminal_toggled (GtkToggleButton* btn, gpointer user_data)
{
    terminal = gtk_toggle_button_get_active(btn) ? G_APP_INFO_CREATE_NEEDS_TERMINAL : 0;
}

void gtk_run(GtkApplication * app)
{
    GtkWidget *entry ,*h;
    g_autoptr(GtkBuilder) builder;
    g_autoptr (GError) err = NULL;

    if(!win)
    {
        builder = gtk_builder_new();
        gtk_builder_add_from_resource(builder,"/org/simple/panel/app/run.ui",NULL);
        win = GTK_WIDGET(gtk_builder_get_object(builder,"app-run"));
        GdkScreen *screen = gtk_widget_get_screen(GTK_WIDGET(win));
        GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
        gtk_widget_set_visual(GTK_WIDGET(win), visual);
        gtk_dialog_set_default_response( (GtkDialog*)win, GTK_RESPONSE_OK );
        gtk_window_set_keep_above(GTK_WINDOW(win),TRUE);
        css_apply_with_class(win,css,"-panel-run-dialog",FALSE);
        entry = GTK_WIDGET(gtk_builder_get_object(builder,"main-entry"));

        g_signal_connect( win, "response", G_CALLBACK(on_response), entry );
        h = GTK_WIDGET(gtk_builder_get_object(builder,"main-box"));
        css_apply_with_class(h,css,"-panel-run-dialog",FALSE);
        css_apply_with_class(h,css,"-panel-run-header",FALSE);
        gtk_widget_show_all( win );

        setup_auto_complete( (GtkEntry*)entry );
        gtk_widget_show(win);

        g_signal_connect(entry ,"changed", G_CALLBACK(on_entry_changed), NULL);
        g_signal_connect(entry, "activate", G_CALLBACK(on_entry_activated), GTK_DIALOG(win));
        g_signal_connect(entry, "icon-press", G_CALLBACK(on_icon_activated), GTK_DIALOG(win));
        h = GTK_WIDGET(gtk_builder_get_object(builder,"terminal-button"));
        g_signal_connect(h ,"toggled", G_CALLBACK(on_terminal_toggled), NULL);

        application = app;
        on_terminal_toggled(GTK_TOGGLE_BUTTON(h),NULL);

        /* get all apps */
        monitor = g_app_info_monitor_get();
        app_list = g_app_info_get_all();
    }

    gtk_window_present(GTK_WINDOW(win));
}


/* vim: set sw=4 et sts=4 ts=4 : */
