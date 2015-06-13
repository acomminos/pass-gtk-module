#include <stdlib.h>
#include <string.h>
#include "pass.h"

gboolean pass_populate_menu(GtkEntry *entry, GFile *dir, GtkMenuShell *menu, GCallback callback) {
  GError *error = NULL;
  GFileEnumerator *dir_enum = g_file_enumerate_children(dir, G_FILE_ATTRIBUTE_STANDARD_DISPLAY_NAME, G_FILE_QUERY_INFO_NONE, NULL, &error);
  if (error) {
    g_error("gtk-module-pass: failed to open password store: %s\n", error->message);
    return FALSE;
  }

  GFileInfo *info;
  while (info = g_file_enumerator_next_file(dir_enum, NULL, &error)) {
    const char *name = g_file_info_get_display_name(info);
    if (g_file_info_get_is_hidden(info) || name[0] == '.')
      continue;
    GFile *file = g_file_enumerator_get_child(dir_enum, info);

    char display_name[strlen(name)];
    strcpy(display_name, name);
    if (g_str_has_suffix(name, ".gpg"))
      display_name[strlen(name) - 4] = '\0';

    GtkWidget *item = gtk_menu_item_new_with_label(display_name);
    GFileType type = g_file_info_get_file_type(info);
    if (type == G_FILE_TYPE_DIRECTORY) {
      // populate submenu
      GtkWidget *subdir_widget = gtk_menu_new();
      GtkMenuShell *subdir_menu = GTK_MENU_SHELL(subdir_widget);
      pass_populate_menu(entry, file, subdir_menu, callback);
      gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), subdir_widget);
    } else if (type == G_FILE_TYPE_REGULAR &&
               g_str_has_suffix(name, ".gpg")) {
      pass_data_t *data = (pass_data_t*) malloc(sizeof(pass_data_t));
      data->file = g_object_ref(file);
      data->entry = g_object_ref(entry);
      g_signal_connect(item, "activate", callback, data);
    } else {
      gtk_widget_set_sensitive(item, FALSE);
    }

    g_object_unref(file);
    gtk_menu_shell_append(menu, item);
    gtk_widget_show(item);
  }

  if (error) {
    g_error("gtk-module-pass: failed to read password store: %s\n", error->message);
    return FALSE;
  }

  g_file_enumerator_close(dir_enum, NULL, NULL);

  return TRUE;
}
