#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>
#include "pass.h"

// The absolute path to the password store; typically ~/.password-store.
static gchar* STORE_PATH;
// The user's GPG id.
static gchar* GPG_ID;

void dialog_entry_activate_cb(GtkEntry *entry, gpointer data) {
  GtkDialog *dialog = GTK_DIALOG(data);
  gtk_dialog_response(dialog, GTK_RESPONSE_ACCEPT);
}

void selection_cb(GtkMenuItem *item, gpointer data) {
  pass_data_t *pass_data = (pass_data_t*) data;

  GtkWidget *dialog = gtk_dialog_new_with_buttons("Enter Passphrase",
      GTK_WINDOW(gtk_widget_get_toplevel(GTK_WIDGET(pass_data->entry))),
      GTK_DIALOG_MODAL | GTK_DIALOG_USE_HEADER_BAR, NULL);

  GtkWidget *entry = gtk_entry_new();
  gtk_entry_set_visibility(GTK_ENTRY(entry), FALSE);
  gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))),
                    entry);
  g_signal_connect(G_OBJECT(entry), "activate", G_CALLBACK(dialog_entry_activate_cb), dialog);
  gtk_widget_show(entry);

  if (gtk_dialog_run(GTK_DIALOG(dialog)) != GTK_RESPONSE_ACCEPT)
    goto cleanup;

  const gchar *passphrase = gtk_entry_get_text(GTK_ENTRY(entry));
  // FIXME(acomminos): this is really gross that we call to gpg from exec.
  gchar *command = g_strconcat("gpg --yes --quiet --compress-algo=none",
      " --passphrase '", passphrase, "' -r ",
      GPG_ID, " -d ", g_file_get_path(pass_data->file),
      NULL);

  gchar *output;
  gint exit_status;
  GError *error = NULL;
  if (!g_spawn_command_line_sync(command, &output, NULL, &exit_status, &error)) {
    g_error("Error decrypting: %s", error->message);
    goto cleanup;
  }

  if (exit_status != 0) {
    // TODO: notify user of failure
    g_warning("got gpg exit status %d\n", exit_status);
    goto cleanup;
  }

  int len = strlen(output);
  // remove newline
  if (output[len - 1] == '\n')
    output[len - 1] = '\0';

  gtk_entry_set_text(pass_data->entry, output);

cleanup:
  gtk_widget_destroy(dialog);
  g_object_unref(pass_data->file);
  g_object_unref(pass_data->entry);
  free(pass_data);
}

void populate_popup_cb(GtkEntry *entry, GtkWidget *popup) {
  GtkMenuShell *menu = GTK_MENU_SHELL(popup);
  GtkWidget *item = gtk_menu_item_new_with_label("Password");

  GtkWidget *pass_menu = gtk_menu_new();
  GtkMenuShell *pass_menu_shell = GTK_MENU_SHELL(pass_menu);

  // populate submenu
  GFile *pass_dir = g_file_new_for_path(STORE_PATH);
  pass_populate_menu(entry, pass_dir, pass_menu_shell, G_CALLBACK(selection_cb));

  gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), pass_menu);
  gtk_menu_shell_append(menu, item);
  gtk_widget_show(item);
  g_signal_chain_from_overridden_handler(entry, popup);
}

void gtk_module_init(gint *argc, gchar ***argv[]) {
  GError *error = NULL;
  STORE_PATH = g_build_filename(g_get_home_dir(), ".password-store", NULL);
  gchar *gpgid_path = g_build_filename(STORE_PATH, ".gpg-id", NULL);
  g_file_get_contents(gpgid_path, &GPG_ID, NULL, &error);
  if (error) {
    g_warning("Failed to load GPG id.");
    return;
  }
  // remove newline
  if (g_str_has_suffix(GPG_ID, "\n")) {
    GPG_ID[strlen(GPG_ID) - 1] = '\0';
  }

  GType entry_type = gtk_entry_get_type();
  // The type class must be instantiated to attach a handler.
  gpointer entry_type_class = g_type_class_ref(entry_type);
  g_signal_override_class_handler("populate-popup", entry_type,
                                  G_CALLBACK(populate_popup_cb));
  g_type_class_unref(entry_type_class);
}
