#include "apocalypse_logging.h"

void GetFrameLogPath(char* LogPath, uint32_t LogPathSize)
{
	snprintf(
		LogPath,
		LogPathSize,
		LOGS_PATH "/%d.log",
		GlobalLoggingGameState->LogInUse
	);
}
void WriteFrameLogToDisk()
{
	game_state* GameState = GlobalLoggingGameState;
	ASSERT(GameState->LogInUse == 0 || GameState->LogInUse == 1);
	if(GameState->FrameLogSize == 0)
	{
		return;
	}

	char LogPath[PLATFORM_MAX_PATH];
	GetFrameLogPath(LogPath, sizeof(LogPath));
	
	GameState->FrameLog[GameState->FrameLogSize] = 0;
	PlatformAppendToFile(LogPath, GameState->FrameLog, GameState->FrameLogSize);
	GameState->FrameLogSize = 0;

	uint32_t FileSize = 0;
	PlatformGetFileSize(LogPath, &FileSize);

	if(FileSize >= MAX_LOG_SIZE)
	{
		if(GameState->LogInUse == 0)
		{
			GameState->LogInUse = 1;
		}
		else
		{
			GameState->LogInUse = 0;
		}
		GetFrameLogPath(LogPath, sizeof(LogPath));	
		PlatformWriteEntireFile(LogPath, NULL, 0);
	}
}

void AppendToFrameLog(char* String)
{
	game_state* GameState = GlobalLoggingGameState;
	uint32_t StringLen = (uint32_t) strlen(String);
	// NOTE: + 3 is for the newline and brackets. 
	// NOTE: other + 1s later are for making sure we have room for 0 termination
	uint32_t NewCharCount = StringLen + MAX_FRAME_COUNT_LEN + 3;
	char* CopyTo = NULL;
	if((GameState->FrameLogSize + NewCharCount + 1) > MAX_LOG_SIZE)
	{
		// NOTE: this means we need to write out the frame log
		WriteFrameLogToDisk();
		CopyTo = GameState->FrameLog;
	}
	else
	{
		CopyTo = GameState->FrameLog + GameState->FrameLogSize;
	}
	snprintf(
		CopyTo,
		NewCharCount + 1,
		"[%0" MAX_FRAME_COUNT_LEN_STR "lld]%s\n",
		GameState->FrameCount,
		String
	);
	GameState->FrameLogSize += NewCharCount;
}