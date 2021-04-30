#include "apocalypse_deck_selector.h"
#include "apocalypse_deck_editor.h"
#include "apocalypse_card_game.h"
#include "apocalypse_platform.h"
#include "apocalypse.h"
#include "apocalypse_deck_storage.h"
#include "apocalypse_render_group.h"
#include "apocalypse_assets.h"
#include "apocalypse_scroll.h"

void DeckSelectorPrepNextScene(
	game_state* GameState, deck_selector_state* SceneState, bool AlreadyExists
)
{
	switch(SceneState->ToStart)
	{
		case(SceneType_CardGame):
		{
			if(AlreadyExists)
			{
				if(SceneState->NetworkGame)
				{
					StartCardGamePrep(
						GameState,
						SceneState->P1Deck,
						SceneState->P2Deck,
						SceneState->IsLeader,
						&SceneState->ListenSocket,
						&SceneState->ConnectSocket
					);
				}
				else
				{
					char* P2Deck = "P2Deck";
					StartCardGamePrep(GameState, SceneState->DeckName, P2Deck);
				}
			}
			else
			{
				DisplayMessageFor(
					GameState,
					&SceneState->Alert,
					"Cannot load non-existent deck",
					1.0f
				);
			}
			break;
		}
		case(SceneType_DeckEditor):
		{
			StartDeckEditorPrep(GameState, SceneState->DeckName, AlreadyExists);
			break;
		}
		default:
		{
			ASSERT(false);
			break;
		}
	}
}

float GetDeckScrollBoxLeft(rectangle DeckNameInputRectangle)
{
	return GetRight(DeckNameInputRectangle) + 10.0f;
}

rectangle MakeDeleteRect(rectangle ButtonRect, vector2 DeleteButtonDim)
{
	return MakeRectangle(
		GetTopRight(ButtonRect) - 0.5f * DeleteButtonDim, DeleteButtonDim
	);
}

void StartWaiting(
	game_state* GameState, deck_selector_state* SceneState
)
{
	SceneState->WaitingForOpponent = true;
	
	// NOTE: send opponent our deck data
	{
		deck_data_packet* Packet = PushStruct(
			&GameState->FrameArena, deck_data_packet
		);
		*Packet = {};
		
		packet_header* Header = &Packet->Header;
		deck_data_payload* Payload = &Packet->Payload;

		loaded_deck* Deck = &Payload->Deck;
		char Buffer[PLATFORM_MAX_PATH];
		FormatDeckPath(Buffer, sizeof(Buffer), SceneState->DeckName);
		*Deck = LoadDeck(Buffer);
		
		SceneState->P1Deck = *Deck;

		Header->DataSize = sizeof(deck_data_packet);
		InitPacketHeader(
			GameState, Header, Packet_DeckData, (uint8_t*) Payload
		);
		SocketSendErrorCheck(
			GameState,
			&SceneState->ConnectSocket,
			&SceneState->ListenSocket,
			Header
		);
	}

	// NOTE: tell opponent that we're ready
	{
		packet_header* Packet = PushStruct(
			&GameState->FrameArena, packet_header
		);
		*Packet = {};
		Packet->DataSize = sizeof(packet_header);
		InitPacketHeader(GameState, Packet, Packet_Ready, NULL);
		SocketSendErrorCheck(
			GameState,
			&SceneState->ConnectSocket,
			&SceneState->ListenSocket,
			Packet
		);
	}
}

void StartDeckSelectorPrep(
	game_state* GameState,
	scene_type ToStart,
	bool NetworkGame,
	bool IsLeader,
	platform_socket* ListenSocket,
	platform_socket* ConnectSocket
)
{
	start_deck_selector_args* SceneArgs = PushStruct(
		&GameState->SceneArgsArena, start_deck_selector_args
	);

	SceneArgs->ToStart = ToStart;

	SceneArgs->NetworkGame = NetworkGame;
	SceneArgs->IsLeader = IsLeader;

	SceneArgs->ListenSocket = {};
	SceneArgs->ConnectSocket = {};
	if(ListenSocket)
	{
		SceneArgs->ListenSocket = *ListenSocket;
	}
	else
	{
		SceneArgs->ListenSocket.IsValid = false; 
	}
	if(ConnectSocket)
	{
		SceneArgs->ConnectSocket = *ConnectSocket;
	}
	else
	{
		SceneArgs->ConnectSocket.IsValid = false;
	}

	GameState->SceneArgs = SceneArgs;
	GameState->Scene = SceneType_DeckSelector; 
}

void StartDeckSelector(
	game_state* GameState, uint32_t WindowWidth, uint32_t WindowHeight
)
{
	start_deck_selector_args* Args = (start_deck_selector_args*) (
		GameState->SceneArgs
	);
	scene_type ToStart = Args->ToStart;

	bool IsLeader = Args->IsLeader;
	bool NetworkGame = Args->NetworkGame;
	platform_socket ListenSocket = Args->ListenSocket;
	platform_socket ConnectSocket = Args->ConnectSocket;

	GameState->SceneState = PushStruct(
		&GameState->TransientArena, deck_selector_state
	);
	deck_selector_state* SceneState = (deck_selector_state*) (
		GameState->SceneState
	);
	*SceneState = {};
	SceneState->ToStart = ToStart;

	SceneState->CanDelete = SceneState->ToStart == SceneType_DeckEditor;

	SceneState->ScreenDimInWorld = TransformVectorToBasis(
		&GameState->WorldToCamera,
		Vector2(WindowWidth, WindowHeight)
	);

	InitUiContext(&SceneState->UiContext);
	ui_context* UiContext = &SceneState->UiContext;

	SceneState->DeckNameBufferSize = PLATFORM_MAX_PATH;
	SceneState->DeckName = PushArray(
		&GameState->TransientArena, SceneState->DeckNameBufferSize, char
	);
	memset(
		SceneState->DeckName, 0, SceneState->DeckNameBufferSize * sizeof(char)
	);

	InitTextInput(
		UiContext,
		&SceneState->DeckNameInput,
		20.0f,
		SceneState->DeckName,
		SceneState->DeckNameBufferSize
	);
	SceneState->DeckNameInputRectangle = MakeRectangleCentered(
		Vector2(WindowWidth / 2.0f, WindowHeight / 2.0f),
		Vector2(WindowWidth / 5.0f, SceneState->DeckNameInput.FontHeight)
	);
	rectangle DeckNameInputRectangle = SceneState->DeckNameInputRectangle;
	SetActive(UiContext, SceneState->DeckNameInput.UiId);

	SceneState->DeckNamesSize = PLATFORM_MAX_PATH * MAX_DECKS_SAVED;
	SceneState->DeckNames = PushArray(
		&GameState->TransientArena,
		SceneState->DeckNamesSize,
		char
	);
	memset(SceneState->DeckNames, 0, SceneState->DeckNamesSize);
	GetAllDeckPaths(SceneState->DeckNames, SceneState->DeckNamesSize);
	SceneState->LoadDeckButtonDim = Vector2(160.0f, 30.0f);
	SceneState->LoadDeckButtonsYStart = (float) WindowHeight;
	SceneState->LoadDeckButtonsYMargin = 0.1f * SceneState->LoadDeckButtonDim.Y;
	load_deck_button* LoadDeckButtons = SceneState->LoadDeckButtons;
	for(
		uint32_t ButtonIndex = 0;
		ButtonIndex < ARRAY_COUNT(SceneState->LoadDeckButtons);
		ButtonIndex++
	)
	{
		load_deck_button* Button = LoadDeckButtons + ButtonIndex;
		Button->UiId = GetId(UiContext);
	}

	float DeleteButtonDim = 0.4f * SceneState->LoadDeckButtonDim.Y;
	SceneState->DeleteButtonDim = Vector2(DeleteButtonDim, DeleteButtonDim);

	SceneState->Alert = MakeAlert();

	rectangle DeckScrollBox = MakeRectangle(
		Vector2(GetDeckScrollBoxLeft(DeckNameInputRectangle), 0.0f), 
		Vector2(SceneState->LoadDeckButtonDim.X, (float) WindowHeight)
	);
	scroll_bar* DeckScrollBar = &SceneState->DeckScrollBar;
	InitScrollBar(
		UiContext,
		DeckScrollBar,
		30.0f,
		DeckScrollBox
	);

	SceneState->IsLeader = IsLeader;
	SceneState->NetworkGame = NetworkGame;
	SceneState->ListenSocket = ListenSocket;
	SceneState->ConnectSocket = ConnectSocket;
	SceneState->WaitingForOpponent = false;
	SceneState->PacketReader = {};
	SceneState->PacketReader.NetworkArena = &GameState->NetworkArena; 
}

void UpdateAndRenderDeckSelector(
	game_state* GameState,
	deck_selector_state* SceneState,
	uint32_t WindowWidth,
	uint32_t WindowHeight,
	game_mouse_events* MouseEvents,
	game_keyboard_events* KeyboardEvents,
	float DtForFrame
)
{
	vector2 ScreenDimInWorld = TransformVectorToBasis(
		&GameState->WorldToCamera,
		Vector2(WindowWidth, WindowHeight)
	);

	memory_arena* FrameArena = &GameState->FrameArena;
	rectangle* DeckNameInputRectangle = &SceneState->DeckNameInputRectangle;

	// NOTE: at start, we calculate the height of our stack of deck buttons
	// TODO: consider making all the rectangles ONCE and then reusing the array 
	// CONT: of rects throughout this function call
	float AllElementsHeight = 0.0f;
	{
		char* CurrentDeckName = SceneState->DeckNames;
		flat_string_array_reader FlatArrayReader;
		InitFlatStringArrayReader(
			&FlatArrayReader,
			CurrentDeckName,
			SceneState->DeckNamesSize
		);
		rectangle Rectangle = MakeNextRectangle(
			GetDeckScrollBoxLeft(*DeckNameInputRectangle),
			SceneState->LoadDeckButtonsYStart,
			SceneState->LoadDeckButtonsYMargin,
			SceneState->LoadDeckButtonDim,
			0
		);
		float Top = GetTop(Rectangle);
		for(
			uint32_t ButtonIndex = 1;
			ButtonIndex < ARRAY_COUNT(SceneState->LoadDeckButtons);
			ButtonIndex++
		)
		{
			if(CurrentDeckName == NULL || *CurrentDeckName == 0)
			{
				break;
			}
			Rectangle = MakeNextRectangle(
				GetDeckScrollBoxLeft(*DeckNameInputRectangle),
				SceneState->LoadDeckButtonsYStart,
				SceneState->LoadDeckButtonsYMargin,
				SceneState->LoadDeckButtonDim,
				ButtonIndex
			);
			CurrentDeckName = GetNextString(&FlatArrayReader);
		}
		float Bottom = GetBottom(Rectangle);
		AllElementsHeight = Top - Bottom;
		// TODO: make a correction for the height calculation (it goes one past)
	}
	UpdateScrollBarPosDim(
		&SceneState->DeckScrollBar,
		SceneState->LoadDeckButtonsYStart,
		AllElementsHeight
	);
	
	user_event_index UserEventIndex = 0;
	int MouseEventIndex = 0;
	int KeyboardEventIndex = 0;

	ui_context* UiContext = &SceneState->UiContext;
	if(!(SceneState->NetworkGame && SceneState->WaitingForOpponent))
	{
		while(
			(MouseEventIndex < MouseEvents->Length) ||
			(KeyboardEventIndex < KeyboardEvents->Length)
		)
		{
			for(; MouseEventIndex < MouseEvents->Length; MouseEventIndex++)
			{
				game_mouse_event* MouseEvent = (
					&MouseEvents->Events[MouseEventIndex]
				);

				if(MouseEvent->UserEventIndex != UserEventIndex)
				{
					break;
				}

				vector2 MouseEventWorldPos = TransformPosFromBasis(
					&GameState->WorldToCamera,
					TransformPosFromBasis(
						&GameState->CameraToScreen, 
						Vector2(MouseEvent->XPos, MouseEvent->YPos)
					)
				);
				
				TextInputHandleMouse(
					UiContext,
					&SceneState->DeckNameInput,
					*DeckNameInputRectangle,
					MouseEvent,
					MouseEventWorldPos
				);

				vector2 Dim = SceneState->LoadDeckButtonDim;
				load_deck_button* LoadDeckButtons = SceneState->LoadDeckButtons;
				char* CurrentDeckName = SceneState->DeckNames;
				flat_string_array_reader FlatArrayReader;
				InitFlatStringArrayReader(
					&FlatArrayReader,
					CurrentDeckName,
					SceneState->DeckNamesSize
				);
				for(
					uint32_t ButtonIndex = 0;
					ButtonIndex < ARRAY_COUNT(SceneState->LoadDeckButtons);
					ButtonIndex++
				)
				{
					if(CurrentDeckName == NULL || *CurrentDeckName == 0)
					{
						break;
					}
					load_deck_button* LoadDeckButton = (
						LoadDeckButtons + ButtonIndex
					);
					rectangle Rectangle = MakeNextRectangle(
						GetDeckScrollBoxLeft(*DeckNameInputRectangle),
						SceneState->LoadDeckButtonsYStart,
						SceneState->LoadDeckButtonsYMargin,
						SceneState->LoadDeckButtonDim,
						ButtonIndex
					);

					button_handle_event_result Result = ButtonHandleEvent(
						UiContext,
						LoadDeckButton->UiId,
						Rectangle,
						MouseEvent->Type,
						MouseEventWorldPos
					);
					if(Result == ButtonHandleEvent_TakeAction)
					{
						rectangle DeleteButtonRect = MakeDeleteRect(
							Rectangle, SceneState->DeleteButtonDim
						);
						if(
							SceneState->CanDelete &&
							PointInRectangle(
								MouseEventWorldPos, DeleteButtonRect
							)
						)
						{
							// TODO: start delete confirmation process
							char* PathToDeck = PushArray(
								FrameArena, PLATFORM_MAX_PATH, char
							);
							snprintf(
								PathToDeck,
								PLATFORM_MAX_PATH,
								"%s%s",
								DECKS_PATH,
								CurrentDeckName
							);
							DeleteDeck(PathToDeck);
							memset(
								SceneState->DeckNames,
								0,
								SceneState->DeckNamesSize
							);
							GetAllDeckPaths(
								SceneState->DeckNames,
								SceneState->DeckNamesSize
							);
						}
						else
						{
							uint32_t DotIndex = FindIndex(
								CurrentDeckName,
								'.',
								FlatArrayReader.BytesRemaining
							);
							strcpy_s(
								SceneState->DeckName,
								SceneState->DeckNameBufferSize,
								CurrentDeckName
							);
							SceneState->DeckName[DotIndex] = 0;

							if(SceneState->NetworkGame)
							{
								StartWaiting(GameState, SceneState);
							}
							else
							{
								DeckSelectorPrepNextScene(
									GameState, SceneState, true
								);
							}
						}
						break;
					}
					else
					{
						CurrentDeckName = GetNextString(&FlatArrayReader); 
					}
				}

				float MinY = 0.0f;
				scroll_handle_mouse_code ScrollResult = ScrollHandleMouse(
					UiContext,
					&SceneState->DeckScrollBar,
					MouseEvent,
					MouseEventWorldPos,
					MinY, 
					SceneState->DeckScrollBar.Trough.Dim.Y
				);
				if(ScrollResult == ScrollHandleMouse_Moved)
				{
					SceneState->LoadDeckButtonsYStart = GetElementsYStart(
						&SceneState->DeckScrollBar, AllElementsHeight
					);
				}

				UserEventIndex++;
			}

			for(
				;
				KeyboardEventIndex < KeyboardEvents->Length;
				KeyboardEventIndex++
			)
			{
				game_keyboard_event* KeyboardEvent = (
					&KeyboardEvents->Events[KeyboardEventIndex]
				);
				if(KeyboardEvent->UserEventIndex != UserEventIndex)
				{
					break;
				}

				if(KeyboardEvent->IsDown != KeyboardEvent->WasDown)
				{
					// NOTE: :common keyboard actions across all states
					switch(KeyboardEvent->Code)
					{
						case(0x1B): // NOTE: Escape V-code
						{
							GameState->Scene = SceneType_MainMenu; 
							break;
						}
					}

					text_input_kb_result KeyboardResult = (
						TextInputHandleKeyboard(
							UiContext, &SceneState->DeckNameInput, KeyboardEvent
						)
					);
					if(KeyboardResult == TextInputKbResult_Submit)
					{
						bool AlreadyExists = false;
						char* CurrentDeckName = SceneState->DeckNames;
						flat_string_array_reader FlatArrayReader;
						InitFlatStringArrayReader(
							&FlatArrayReader,
							CurrentDeckName,
							SceneState->DeckNamesSize
						);
						for(
							uint32_t ButtonIndex = 0;
							(
								ButtonIndex < 
								ARRAY_COUNT(SceneState->LoadDeckButtons)
							);
							ButtonIndex++
						)
						{
							if(CurrentDeckName == NULL)
							{
								break;
							}

							int32_t StopAt = FindIndex(
								CurrentDeckName, 
								'.', 
								FlatArrayReader.BytesRemaining
							);

							if(
								StopAt != -1 &&
								StrCmp(								
									CurrentDeckName,
									SceneState->DeckName,
									StopAt
								)
							)
							{
								// NOTE: the name the user typed in is already 
								// CONT: a deck. load that deck
								AlreadyExists = true;
								break;
							}

							CurrentDeckName = GetNextString(&FlatArrayReader);
						}
						if(SceneState->NetworkGame)
						{
							if(AlreadyExists)
							{
								StartWaiting(GameState, SceneState);
							}
							else
							{
								DisplayMessageFor(
									GameState,
									&SceneState->Alert,
									"Cannot load non-existent deck",
									1.0f
								);
							}
						}
						else
						{
							DeckSelectorPrepNextScene(
								GameState, SceneState, AlreadyExists
							);
						}
					}
				}

				UserEventIndex++;
			}
		}
	}

	if(SceneState->NetworkGame && SceneState->WaitingForOpponent)
	{
		uint32_t BytesRead = 0;
		packet_reader_data* PacketReader = &SceneState->PacketReader;
		while(true)
		{
			read_packet_result ReadResult = ReadPacket(
				GameState,
				&SceneState->ConnectSocket,
				&SceneState->ListenSocket,
				PacketReader
			);

			if(ReadResult == ReadPacketResult_Complete)
			{
				packet_header* Header = SceneState->PacketReader.Header;
				void* Payload = SceneState->PacketReader.Payload;
				if(Header->Type == Packet_DeckData)
				{
					deck_data_payload* DeckDataPayload = (
						(deck_data_payload*) Payload
					);
					SceneState->P2Deck = DeckDataPayload->Deck;
				}
				else if(Header->Type == Packet_Ready)
				{
					DeckSelectorPrepNextScene(GameState, SceneState, true);
				}

				ReadPacketEnd(PacketReader);
			}
			else if(
				ReadResult == ReadPacketResult_Incomplete ||
				ReadResult == ReadPacketResult_PeerReset
			)
			{
				break;
			}
			else if(ReadResult == ReadPacketResult_Error)
			{
				ReadPacketEnd(PacketReader);
			}
			else
			{
				ASSERT(false);
			}
		}
	}
	else
	{
		UpdateTextInput(UiContext, &SceneState->DeckNameInput, DtForFrame);
	}

	render_group* DefaultRenderGroup = &GameState->RenderGroup;
	assets* Assets = &GameState->Assets;

	PushClear(DefaultRenderGroup, Vector4(0.25f, 0.25f, 0.25f, 1.0f));
	
	vector4 White = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	vector4 Black = Vector4(0.0f, 0.0f, 0.0f, 1.0f);

	vector4 FontColor = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	vector4 BackgroundColor = Vector4(0.1f, 0.1f, 0.1f, 1.0f);
	if(SceneState->NetworkGame && SceneState->WaitingForOpponent)
	{
		PushTextCentered(
			DefaultRenderGroup,
			Assets,
			FontHandle_TestFont,
			"Waiting for opponent",
			64,
			50.0f,
			SceneState->ScreenDimInWorld / 2.0f,
			Vector4(1.0f, 1.0f, 1.0f, 1.0f),
			&GameState->FrameArena
		);
	}
	else
	{
		bitmap_handle Background = BitmapHandle_TestCard2;
		PushTextInput(
			UiContext,
			&SceneState->DeckNameInput,
			FontColor,
			*DeckNameInputRectangle,
			Background,
			BackgroundColor,
			Assets,
			DefaultRenderGroup,
			&GameState->FrameArena
		);

		vector2 Dim = SceneState->LoadDeckButtonDim;
		load_deck_button* LoadDeckButtons = SceneState->LoadDeckButtons;
		flat_string_array_reader FlatArrayReader;
		char* CurrentDeckName = SceneState->DeckNames;
		InitFlatStringArrayReader(
			&FlatArrayReader, CurrentDeckName, SceneState->DeckNamesSize
		);
		uint32_t ButtonLayer = 1;
		for(
			uint32_t ButtonIndex = 0;
			ButtonIndex < ARRAY_COUNT(SceneState->LoadDeckButtons);
			ButtonIndex++
		)
		{
			if(CurrentDeckName == NULL || *CurrentDeckName == 0)
			{
				break;
			}
			load_deck_button* LoadDeckButton = (
				LoadDeckButtons + ButtonIndex
			);
			rectangle Rectangle = MakeNextRectangle(
				GetDeckScrollBoxLeft(*DeckNameInputRectangle),
				SceneState->LoadDeckButtonsYStart,
				SceneState->LoadDeckButtonsYMargin,
				SceneState->LoadDeckButtonDim,
				ButtonIndex
			);

			uint32_t StopAt = FindIndex(
				CurrentDeckName, '.', FlatArrayReader.BytesRemaining
			);
			PushButtonToRenderGroup(
				Rectangle,
				BitmapHandle_TestCard2,
				DefaultRenderGroup,
				Assets, 
				CurrentDeckName,
				StopAt,
				FontHandle_TestFont,
				Black,
				&GameState->FrameArena,
				ButtonLayer
			);

			CurrentDeckName = GetNextString(&FlatArrayReader);

			if(IsHot(UiContext, LoadDeckButton->UiId) && SceneState->CanDelete)
			{
				rectangle DeleteButtonRect = MakeDeleteRect(
					Rectangle, SceneState->DeleteButtonDim
				);

				PushSizedBitmap(
					DefaultRenderGroup,
					Assets,
					BitmapHandle_X,
					GetBottomLeft(DeleteButtonRect),
					Vector2(DeleteButtonRect.Dim.X, 0.0f),
					Vector2(0.0f, DeleteButtonRect.Dim.Y),
					Vector4(1.0f, 0.0f, 0.0f, 1.0f),
					ButtonLayer + 1
				);
			}
		}

		PushCenteredAlert(&SceneState->Alert, GameState, ScreenDimInWorld);

		if(CanScroll(&SceneState->DeckScrollBar))
		{
			PushScrollBarToRenderGroup(
				SceneState->DeckScrollBar.Rect,
				BitmapHandle_TestCard2,
				DefaultRenderGroup,
				Assets
			);
		}
	}
}