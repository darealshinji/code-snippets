/* possible alternative: call the notify-send command */

#include <libnotify/notify.h>

int send_notification(const char *app_name, const char *summary, const char *body, const char *icon)
{
  if (!app_name || !*app_name || !summary || !*summary || !notify_init(app_name)) {
    return 1;
  }

  if (!icon || !*icon) {
    icon = "dialog-information";
  }

  int rv = 0;
  NotifyNotification *p = notify_notification_new(summary, body, icon);
  if (!notify_notification_show(p, NULL)) rv = 1;
  g_object_unref(G_OBJECT(p));
  notify_uninit();

  return rv;
}

int main(int argc, char **argv)
{
  return send_notification(argv[0], "Hello", "Hi there!", NULL);
}


