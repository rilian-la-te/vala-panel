#include "info-data.h"
#include <string.h>
#include <glib/gi18n.h>

/*
 * InfoData struct
 */

static char *generate_markup(const char *name, const char *sdesc)
{
    g_autofree char *nom  = g_markup_escape_text(name, strlen(name));
    g_autofree char *desc = g_markup_escape_text(sdesc, strlen(sdesc));
    return g_strdup_printf("<big>%s</big>\n<small>%s</small>", nom, desc);
}

InfoData *info_data_new_from_info(GAppInfo *info)
{
    if (g_app_info_get_executable(info) == NULL)
        return NULL;
    InfoData *data = (InfoData *)g_malloc0(sizeof(InfoData));
    data->icon     = g_app_info_get_icon(info);
    if (!data->icon)
    {
        data->icon      = g_themed_icon_new_with_default_fallbacks("system-run-symbolic");
        data->free_icon = true;
    }
    else
        data->free_icon = false;
    data->disp_name         = g_strdup(g_app_info_get_display_name(info));
    const char *name =
        g_app_info_get_name(info) ? g_app_info_get_name(info) : g_app_info_get_executable(info);
    const char *sdesc =
        g_app_info_get_description(info) ? g_app_info_get_description(info) : "";
    data->name_markup = generate_markup(name, sdesc);
    data->command     = g_strdup(g_app_info_get_executable(info));
    return data;
}

InfoData *info_data_new_from_command(const char *command)
{
    InfoData *data        = (InfoData *)g_malloc0(sizeof(InfoData));
    data->icon            = g_themed_icon_new_with_default_fallbacks("system-run-symbolic");
    data->free_icon       = true;
    data->disp_name       = g_strdup_printf(_("Run %s"), command);
    g_autofree char *name = g_strdup_printf(_("Run %s"), command);
    const char *sdesc     = _("Run system command");
    data->name_markup     = generate_markup(name, sdesc);
    data->command         = g_strdup(command);
    return data;
}

void info_data_free(InfoData *data)
{
    if (data->free_icon)
        g_object_unref(data->icon);
    g_free(data->disp_name);
    g_free(data->name_markup);
    g_free(data->command);
    g_free(data);
}

/*
 * InfoDataModel GObject
 */

struct _InfoDataModel
{
    GObject __parent__;
    GSequence* seq;
};

static void info_data_model_iface_init(GListModelInterface *iface);

G_DEFINE_TYPE_WITH_CODE(InfoDataModel,info_data_model,G_TYPE_OBJECT,G_IMPLEMENT_INTERFACE(G_TYPE_LIST_MODEL,info_data_model_iface_init))

static void info_data_model_iface_init(GListModelInterface *iface)
{

}

static void info_data_model_init(InfoDataModel* self)
{

}

static void info_data_model_class_init(InfoDataModelClass* klass)
{

}
