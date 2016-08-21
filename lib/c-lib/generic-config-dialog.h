#ifndef GENERICCONFIGDIALOG_H
#define GENERICCONFIGDIALOG_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

typedef enum
{
    STR,
    INT,
    BOOL,
    FILE,
    FILE_ENTRY,
    DIRECTORY_ENTRY,
    TRIM,
    EXTERNAL
} GenericConfigType;

G_END_DECLS

#endif // GENERICCONFIGDIALOG_H
