#ifndef APOCALYPSE_UI_H

typedef int32_t ui_id;
struct ui_context
{
	ui_id HotId;
	ui_id ActiveId;
	ui_id NextId;
};

inline void InitUiContext(ui_context* Context)
{
	*Context = {};
	Context->ActiveId = -1;
	Context->HotId = -1;
	Context->NextId = 0;
}

inline ui_context MakeUiContext()
{
	ui_context Result;
	InitUiContext(&Result);
	return Result;
}

inline ui_id GetId(ui_context* UiContext)
{
	return UiContext->NextId++;
}

inline void SetHot(ui_context* UiContext, ui_id Id)
{
	UiContext->HotId = Id;
}

inline void ClearHot(ui_context* UiContext)
{
	UiContext->HotId = -1;
}

inline void SetActive(ui_context* UiContext, ui_id Id)
{
	UiContext->ActiveId = Id;
}

inline void ClearActive(ui_context* UiContext)
{
	UiContext->ActiveId = -1;
}

inline bool IsHot(ui_context* UiContext, ui_id Id)
{
	return UiContext->HotId == Id;
}

inline bool IsActive(ui_context* UiContext, ui_id Id)
{
	return UiContext->ActiveId == Id;
}

#define APOCALYPSE_UI_H
#endif