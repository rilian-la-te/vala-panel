#ifndef GLISTMODELFILTER_H
#define GLISTMODELFILTER_H

#include <gio/gio.h>
#include <glib-object.h>
#include <stdbool.h>

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(ValaPanelListModelFilter, vala_panel_list_model_filter, VALA_PANEL,
                     LIST_MODEL_FILTER, GObject)

typedef bool (*ValaPanelListModelFilterFunc)(gpointer, gpointer);

ValaPanelListModelFilter *vala_panel_list_model_filter_new(GListModel *base_model);
void vala_panel_list_model_filter_set_filter_func(ValaPanelListModelFilter *self,
                                                  ValaPanelListModelFilterFunc func,
                                                  gpointer user_data);
void vala_panel_list_model_filter_invalidate(ValaPanelListModelFilter *self);
void vala_panel_list_model_filter_set_max_results(ValaPanelListModelFilter *self, uint max_results);
G_END_DECLS

#endif // GLISTMODELFILTER_H
