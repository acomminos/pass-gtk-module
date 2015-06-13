#include <gtk/gtk.h>

void populate_popup_cb(GtkEntry *entry, GtkWidget *popup) {
  GtkMenuShell *menu = GTK_MENU_SHELL(popup);
  GtkWidget *item = gtk_menu_item_new_with_label("Keepass");

  GtkWidget *keepass_menu = gtk_menu_new();
  GtkMenuShell *keepass_menu_shell = GTK_MENU_SHELL(keepass_menu);
  // TODO(acomminos): populate submenu
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), keepass_menu);

  gtk_menu_shell_append(menu, item);
  gtk_widget_show(item);
  g_signal_chain_from_overridden_handler(entry, popup);
}

void gtk_module_init(gint *argc, gchar ***argv[]) {
  GType entry_type = gtk_entry_get_type();
  // The type class must be instantiated to attach a handler.
  gpointer entry_type_class = g_type_class_ref(entry_type);
  g_signal_override_class_handler("populate-popup", entry_type,
                                  G_CALLBACK(populate_popup_cb));
  g_type_class_unref(entry_type_class);
}
