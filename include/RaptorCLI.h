// include/RaptorCLI.h
#ifndef RAPTORCLI_H
#define RAPTORCLI_H

// #define USE_DESCRIPTIVE_ERRORS
#ifndef USE_DESCRIPTIVE_ERRORS
#define ERROR_CMD_UNKNOWN "error.cmd.unknown"
#define ERROR_CMD_UNEXPECTED_TOKEN "error.cmd.unexpected_token"
#define ERROR_CMD_DUPLICATE_HELP_FLAG "error.cmd.duplicate_help_flag"
#define ERROR_CMD_MISSING_REQUIRED_ARG "error.cmd.missing_required_arg"
#define ERROR_CMD_TYPE_MISMATCH "error.cmd.type_mismatch"
#define ERROR_CMD_DUPLICATE_NAME "error.cmd.duplicate_name"
#define ERROR_CMD_DUPLICATE_ALIAS "error.cmd.duplicate_alias"
#define ERROR_CMD_NO_CALLBACK "error.cmd.no_callback"
#endif

#include "clioutput.h"
#include "value.h"
#include "argument.h"
#include "command.h"
#include "dispatcher.h"
#include "executable_command.h"

#endif