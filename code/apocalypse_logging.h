#ifndef APOCALYPSE_LOGGING_H

#include <string.h>

#include "apocalypse_platform.h"

// TODO: probably get LOGS_PATH at run time
#define LOGS_PATH "./logs"
#define ASSERT_LOG_PATH (LOGS_PATH "/assert.log")
#define PERM_COREDUMP_LOG_PATH (LOGS_PATH "/perm_coredump.bin")
#define TRANS_COREDUMP_LOG_PATH (LOGS_PATH "/trans_coredump.bin")

#if APOCALYPSE_SLOW
// TODO: Complete assertion macro
#define ASSERT(Expression) if(!(Expression)) {PlatformWriteEntireFile(ASSERT_LOG_PATH, "At " __FILE__ ": " __FUNCTION__ ": " LINE_STRING, sizeof("At " __FILE__ ": " __FUNCTION__ ": " LINE_STRING) - 1);PlatformWriteEntireFile(PERM_COREDUMP_LOG_PATH, GlobalGameMemoryPtr->PermanentStorage, (uint32_t) GlobalGameMemoryPtr->PermanentStorageSize);PlatformWriteEntireFile(TRANS_COREDUMP_LOG_PATH, GlobalGameMemoryPtr->TransientStorage, (uint32_t) GlobalGameMemoryPtr->TransientStorageSize);*(int*) 0 = 0;}
#else
#define ASSERT(Expression)
#endif

#define MAX_LOG_SIZE (MEGABYTES(1))
// NOTE: MAX_FRAME_COUNT_LEN is 22 since it is assumed to be a uint64
// NOTE: do not change one of these without changing the other
#define MAX_FRAME_COUNT_LEN 20
#define MAX_FRAME_COUNT_LEN_STR "20"

struct game_state;
game_state* GlobalLoggingGameState = NULL;
struct game_memory;
game_memory* GlobalGameMemoryPtr = NULL;
void WriteFrameLogToDisk();
void AppendToFrameLog(char* String);

#define APOCALYPSE_LOGGING_H
#endif