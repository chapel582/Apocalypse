#ifndef WIN32_APOCALYPSE_H

typedef enum
{
	WindowsSuccess,

	// InitDSound
	DSoundLibLoad,
	DSoundCreate,
	DSoundCooperative,
	DSoundPrimary,
	DSoundSecondary
} windows_result_code;

#define WIN32_APOCALYPSE_H
#endif