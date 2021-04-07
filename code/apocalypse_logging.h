#ifndef APOCALYPSE_LOGGING_H

#include <string.h>

#include "apocalypse_platform.h"

// TODO: probably get LOGS_PATH at run time
#define LOGS_PATH "./logs"
#define ASSERT_LOG_PATH (LOGS_PATH "/assert.log")

#define MAX_LOG_SIZE (MEGABYTES(1))
// NOTE: MAX_FRAME_COUNT_LEN is 22 since it is assumed to be a uint64
// NOTE: do not change one of these without changing the other
#define MAX_FRAME_COUNT_LEN 20
#define MAX_FRAME_COUNT_LEN_STR "20"

struct game_state;
game_state* GlobalLoggingGameState = NULL;
void WriteFrameLogToDisk();
void AppendToFrameLog(char* String);

#define APOCALYPSE_LOGGING_H
#endif