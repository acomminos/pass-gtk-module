#include <gtk/gtk.h>

typedef struct {
  GtkEntry *entry;
  GFile *file;
} pass_data_t;

gboolean pass_populate_menu(GtkEntry *entry, GFile *dir, GtkMenuShell *menu,
                            GCallback callback);
