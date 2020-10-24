#include <gio/gio.h>

#include "menu.h"

GMenu * commands_menu_new (void) {
    GMenu * menuFile = g_menu_new();
    g_menu_append(menuFile, "_Quit", "app.quit");

    GMenu * menu = g_menu_new();
    g_menu_append_submenu(menu, "_File", G_MENU_MODEL(menuFile));

    return menu;
}
