#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#include <gtk/gtk.h>
#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif
#include <libappindicator/app-indicator.h>


int indicator(const char *appID, const char *icon, void (*callback)())
{
  GtkWidget *window;
  GtkMenu *menu;
  GtkActionGroup *action_group;
  GtkUIManager *uim;
  AppIndicator *indicator;

  if (!appID || !*appID) {
    appID = "my-app";
  }

  if (!icon || !*icon) {
    icon = "dialog-information";
  }

  const GtkActionEntry entries[] = {
    { "Action", "dialog-information", "_Action", NULL, NULL, callback },
    { "Close",  "dialog-close",       "_Close",  NULL, NULL, gtk_main_quit },
  };

  const gchar *ui_info =
    "<ui>"
      "<popup name='IndicatorPopup'>"
        "<menuitem action='Action' />"
        "<menuitem action='Close' />"
      "</popup>"
    "</ui>";

  gtk_init(NULL, NULL);
  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

  action_group = gtk_action_group_new ("AppActions");
  gtk_action_group_add_actions (action_group, entries, 2, window);

  uim = gtk_ui_manager_new();
  g_object_set_data (G_OBJECT(window), "ui-manager", uim);
  gtk_ui_manager_insert_action_group (uim, action_group, 0);

  if (!gtk_ui_manager_add_ui_from_string (uim, ui_info, -1, NULL)) {
    return 1;
  }

  indicator = app_indicator_new (appID, icon, APP_INDICATOR_CATEGORY_OTHER);
  menu = (GtkMenu *)gtk_ui_manager_get_widget (uim, "/ui/IndicatorPopup");
  app_indicator_set_status (indicator, APP_INDICATOR_STATUS_ACTIVE);
  app_indicator_set_menu (indicator, menu);

  gtk_main();

  return 0;
}

static void hello_cb() {
  printf("Hello World!\n");
}

int main()
{
  return indicator(NULL, NULL, hello_cb);
}


