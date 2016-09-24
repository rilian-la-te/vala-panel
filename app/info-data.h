#ifndef INFODATA_H
#define INFODATA_H

#include <gio/gio.h>
#include <glib-object.h>
#include <stdbool.h>

G_BEGIN_DECLS

typedef struct info_data
{
	GIcon *icon;
	char *name_markup;
	char *disp_name;
	char *command;
	bool free_icon;
} InfoData;

InfoData *info_data_new_from_info(GAppInfo *info);
InfoData *info_data_new_from_command(const char *command);
InfoData *info_data_new_bootstrap();
void info_data_free(InfoData *data);

G_DECLARE_FINAL_TYPE(InfoDataModel, info_data_model, VALA_PANEL, INFO_DATA_MODEL, GObject)

InfoDataModel *info_data_model_new();
G_GNUC_INTERNAL GSequence *info_data_model_get_sequence(InfoDataModel *model);

G_END_DECLS

#endif // INFODATA_H
