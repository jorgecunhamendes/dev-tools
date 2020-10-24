#ifndef COMMANDS_COMMAND_H
#define COMMANDS_COMMAND_H

#include <glib.h>

// Command format:
//   - run a command: "exec:<command_line>"

enum CommandType {
    COMMAND_INVALID = 0,
    COMMAND_EXEC = 1,
};

struct Command {
    enum CommandType type;
    gchar * line;
    gchar ** parts;
};

struct Command *
command_parse_line (const gchar * line);

void
command_free (struct Command * command);

#endif // COMMANDS_COMMAND_H
