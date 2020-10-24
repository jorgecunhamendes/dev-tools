#include "command.h"

struct Command *
command_parse_line (const gchar * line) {
    struct Command * result = NULL;
    if (g_str_has_prefix(line, "exec:")) {
        result = g_new(struct Command, 1);
        result->type = COMMAND_EXEC;
        result->line = g_strdup(line + 5 * sizeof(gchar));
        result->parts = g_strsplit_set(result->line, " ", -1);
    }
    return result;
}

void
command_free (struct Command * command) {
    g_strfreev(command->parts);
    g_free(command->line);
    g_free(command);
}
