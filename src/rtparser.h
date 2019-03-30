/*
 * xfce4-sntray-plugin
 * Copyright (C) 2015-2019 Konstantin Pugin <ria.freelander@gmail.com>
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
#ifndef RTPARSER_H
#define RTPARSER_H

#include <gio/gio.h>
#include <glib-object.h>
#include <glib.h>
#include <pango/pango.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
	NONE,
	NUM,
	DOT,
} ListType;

struct _QRichTextParser
{
	GHashTable *pango_names;
	GHashTable *division_names;
	GHashTable *span_aliases;
	GHashTable *lists;
	GHashTable *newline_at_end;
	GHashTable *translated_to_pango;
	GHashTable *special_spans;
	GMarkupParseContext *context;
	char *rich_markup;
	GString *pango_markup_builder;
	ListType list_type;
	gint list_order;
	gint table_depth;
	char *pango_markup;
	GIcon *icon;
};

typedef struct _QRichTextParser QRichTextParser;
QRichTextParser *qrich_text_parser_new(const char *markup);
void qrich_text_parser_free(QRichTextParser *self);
void qrich_text_parser_translate_markup(QRichTextParser *self);

#endif
