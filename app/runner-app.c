/*
 * vala-panel
 * Copyright (C) 2015-2017 Konstantin Pugin <ria.freelander@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <locale.h>
#include "runner-app.h"
#include "lib/config.h"
#include "lib/definitions.h"
#include "runner.h"

struct _ValaPanelRunApplication
{
	GtkApplication __parent__;
	GtkWindow *run_dialog;
};

G_DEFINE_TYPE(ValaPanelRunApplication, vala_panel_run_application, GTK_TYPE_APPLICATION)

static ValaPanelRunApplication *vala_panel_run_application_new()
{
	return VALA_PANEL_RUN_APPLICATION(g_object_new(vala_panel_run_application_get_type(),
	                                               "application-id",
	                                               "org.valapanel.extras.Runner",
	                                               "flags",
	                                               0,
	                                               NULL));
}

static void vala_panel_run_application_activate(GApplication *application)
{
	ValaPanelRunApplication *app = VALA_PANEL_RUN_APPLICATION(application);
	if (app->run_dialog == NULL)
		app->run_dialog = vala_panel_runner_new(GTK_APPLICATION(app));
	gtk_run(VALA_PANEL_RUNNER(app->run_dialog));
}

static void vala_panel_run_application_finalize(GObject *app)
{
	gtk_widget_destroy0(VALA_PANEL_RUN_APPLICATION(app)->run_dialog);
	(*G_OBJECT_CLASS(vala_panel_run_application_parent_class)->finalize)(app);
}
static void vala_panel_run_application_init(ValaPanelRunApplication *self)
{
	setlocale(LC_CTYPE, "");
	bindtextdomain(CONFIG_GETTEXT_PACKAGE, CONFIG_LOCALE_DIR);
	bind_textdomain_codeset(CONFIG_GETTEXT_PACKAGE, "UTF-8");
	textdomain(CONFIG_GETTEXT_PACKAGE);
}

static void vala_panel_run_application_class_init(ValaPanelRunApplicationClass *klass)
{
	GApplicationClass *parent = G_APPLICATION_CLASS(klass);
	parent->activate          = vala_panel_run_application_activate;
	GObjectClass *obj_class   = G_OBJECT_CLASS(klass);
	obj_class->finalize       = vala_panel_run_application_finalize;
}

int main(int argc, char *argv[])
{
	ValaPanelRunApplication *rd = vala_panel_run_application_new();
	return g_application_run(G_APPLICATION(rd), argc, argv);
}
