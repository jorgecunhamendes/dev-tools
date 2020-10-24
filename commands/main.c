#include <glib.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <vte/vte.h>

#include "command.h"
#include "menu.h"

const char * APP_NAME = "dev-tools.commands";
const char * APP_TITLE = "Commands";

struct State {
    GtkWidget * notebook;
    GtkTextBuffer * infoPageBuffer;
};

int
main (int argc, char ** argv);

static void
app_quit (GSimpleAction * action, GVariant * parameter, gpointer app);

static void
app_startup (GtkApplication * app, gpointer user_data);

static void
app_activate (GtkApplication * app, gpointer user_data);

static gboolean
stdin_watch (GIOChannel * channel, GIOCondition cond, gpointer user_data);

void
app_append_command (GtkNotebook * notebook, struct Command * command);

/**
 * Application entry point.
 */
int main (int argc, char ** argv) {
    GtkApplication * app;
    int status;

    app = gtk_application_new(APP_NAME, G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "startup", G_CALLBACK(app_startup), NULL);
    g_signal_connect(app, "activate", G_CALLBACK(app_activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}

static void app_quit (GSimpleAction * action, GVariant * parameter, gpointer app) {
    g_application_quit(G_APPLICATION(app));
}

/**
 * Handler for the application 'startup' signal.
 */
static void app_startup (GtkApplication * app, gpointer user_data) {
    GActionEntry app_actions[] = {
      { "quit", app_quit, NULL, NULL, NULL }
    };
    const gchar * app_quit_accels[2] = { "<Ctrl>Q", NULL };

    g_action_map_add_action_entries(G_ACTION_MAP(app),
            app_actions,
            G_N_ELEMENTS(app_actions),
            app);
    gtk_application_set_accels_for_action(GTK_APPLICATION(app),
            "app.quit",
            app_quit_accels);

    GMenu * menu = commands_menu_new();
    gtk_application_set_menubar(app, G_MENU_MODEL(menu));
}

/**
 * Handler for the application 'activate' signal.
 */
static void app_activate (GtkApplication * app, gpointer user_data) {
    GtkWidget * window;
    GtkWidget * notebook;
    GtkWidget * infoPageLabel;
    GtkWidget * infoPage;
    GtkTextBuffer * infoPageBuffer;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), APP_TITLE);
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 400);

    notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(window), notebook);

    // info page: label
    infoPageLabel = gtk_label_new("Info");
    // info page: text view
    infoPage = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(infoPage), FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(infoPage), FALSE);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), infoPage, infoPageLabel);
    // info page: text view buffer
    infoPageBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(infoPage));

    struct State * state = g_new(struct State, 1);
    state->notebook = notebook;
    state->infoPageBuffer = infoPageBuffer;

    GIOChannel * channel = g_io_channel_unix_new(STDIN_FILENO);
    g_io_add_watch(channel, G_IO_IN, stdin_watch, state);

    gtk_widget_show_all(window);
}

/**
 * Handler for the STDIN channel.
 */
static gboolean stdin_watch (GIOChannel * channel, GIOCondition cond, gpointer user_data) {
    GIOStatus status;
    gchar * line = NULL;
    gsize length;
    GError * error = NULL;
    struct State * state = (struct State *) user_data;

    status = g_io_channel_read_line(channel, &line, &length, NULL, &error);
    switch(status) {
        case G_IO_STATUS_ERROR:
        {
            g_warning("g_io_channel_read_line error");
            if (error != NULL) {
                g_warning(error->message);
            }
            break;
        }
        case G_IO_STATUS_NORMAL:
        {
            if (line == NULL) {
                g_info("g_io_channel_read_line empty line");
            } else {
                // add STDIN line to the info page
                GtkTextIter iter;
                gtk_text_buffer_get_end_iter(state->infoPageBuffer, &iter);
                gtk_text_buffer_insert(state->infoPageBuffer, &iter, line, length);
                // parse the command line
                gchar * cleaned = g_strstrip(g_strdup(line));
                struct Command * command = command_parse_line(cleaned);
                if (command) {
                    app_append_command(GTK_NOTEBOOK(state->notebook), command);
                    command_free(command);
                } else {
                    GString * messageBuilder = g_string_new("Invalid command: ");
                    gchar * escaped = g_strescape(cleaned, NULL);
                    g_string_append(messageBuilder, escaped);
                    g_free(escaped);
                    gchar * message = g_string_free(messageBuilder, FALSE);
                    g_warning(message);
                    g_free(message);
                }
                g_free(cleaned);
            }
            break;
        }
        case G_IO_STATUS_EOF:
        {
            break;
        }
        case G_IO_STATUS_AGAIN:
        {
            g_message("resource temporarily unavailable");
            break;
        }
        default:
            g_critical("invalid g_io_channel_read_line status");
    }

    if (line != NULL) {
        g_free(line);
    }

    return TRUE;
}

void
app_append_command (GtkNotebook * notebook, struct Command * command) {
    GtkWidget * vteTerminal = vte_terminal_new();
    vte_terminal_spawn_async(VTE_TERMINAL(vteTerminal),
            VTE_PTY_DEFAULT,
            NULL, // working directory
            command->parts, // command
            NULL,
            0,
            NULL,
            NULL,
            NULL,
            -1,
            NULL,
            NULL,
            NULL
            );
    gtk_widget_show_all(vteTerminal);
    // append the tab to the notebook
    int idx = gtk_notebook_append_page(notebook, vteTerminal, NULL);
    if (idx == -1) {
        g_warning("Couldn't append notebook page");
    }
}
