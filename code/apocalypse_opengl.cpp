// NOTE: this file is a platform layer module. include it after your platform
// CONT: layer headers

#include "apocalypse_math.h"
#include "apocalypse_render_group.h"
#include "apocalypse_memory_arena.h"
#include "apocalypse_vector.h"

// TODO: include other platforms
// TODO: move all of the render stuff to platform layer?
#include <windows.h>

#include <gl/gl.h>

inline void OpenGlRectangle(vector2 MinP, vector2 MaxP, vector4 Color)
{
	glBegin(GL_TRIANGLES);

	glColor4f(Color.R, Color.G, Color.B, Color.A);

	// NOTE: Lower triangle
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(MinP.X, MinP.Y);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(MaxP.X, MinP.Y);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(MaxP.X, MaxP.Y);

	// NOTE: Upper triangle
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(MinP.X, MinP.Y);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(MaxP.X, MaxP.Y);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(MinP.X, MaxP.Y);

	glEnd();
}

// TODO: get rid of this
uint32_t GlobalTextureBindCount = 0;

void RenderGroupToOutput(
	render_group* RenderGroup, uint32_t WindowWidth, uint32_t WindowHeight
)
{
	TIMED_BLOCK();
	memory_arena* RenderArena = RenderGroup->Arena;

	temp_memory TempMemory = BeginTempMemory(RenderArena);
	render_entry_handle* EntryHandles = PushArray(
		RenderArena, RenderGroup->NumEntries, render_entry_handle
	);

	// NOTE: construct the sort space
	uint8_t* CurrentAddress = (
		RenderArena->Base + FindAlignmentOffset(RenderGroup->Arena->Base, 4)
	);
	for(uint32_t Index = 0; Index < RenderGroup->NumEntries; Index++)
	{
		render_entry_header* Header = (render_entry_header*) CurrentAddress; 
		render_entry_handle* EntryHandle = EntryHandles + Index;
		EntryHandle->Header = Header;
		EntryHandle->Layer = Header->Layer;

		switch(Header->Type)
		{
			case(EntryType_Clear):
			{
				render_entry_clear* Entry = (render_entry_clear*) Header;
				CurrentAddress += sizeof(*Entry);
				break;
			}
			case(EntryType_Rectangle):
			{
				render_entry_rectangle* Entry = (render_entry_rectangle*) (
					Header
				);
				CurrentAddress += sizeof(*Entry);
				break;
			}
			case(EntryType_Bitmap):
			{
				render_entry_bitmap* Entry = (render_entry_bitmap*) Header;

				CurrentAddress += sizeof(*Entry);
				break;
			}
			default:
			{
				ASSERT(false);
			}
			CurrentAddress += FindAlignmentOffset(CurrentAddress, 4);
		}
	}

	// NOTE: sort layers here (smallest to largest)
	// TODO: faster sort
	bool Sorted;
	do
	{
		Sorted = true;
		render_entry_handle* LastHandle = EntryHandles;
		for(uint32_t Index = 1; Index < RenderGroup->NumEntries; Index++)
		{
			render_entry_handle* CurrentHandle = EntryHandles + Index;
			if(LastHandle->Layer > CurrentHandle->Layer)
			{
				render_entry_handle Temp = *CurrentHandle;
				*CurrentHandle = *LastHandle;
				*LastHandle = Temp;
				Sorted = false;
			}
			LastHandle = CurrentHandle;
		}
	} while(!Sorted);

	// SECTION START: render to output
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

	glViewport(0, 0, WindowWidth, WindowHeight);

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// NOTE: if we need to render to non-window elements, we can pull out
	// CONT: this code and call it outside of this function 
	glMatrixMode(GL_PROJECTION);
	float A = SafeRatio1(2.0f, (float) WindowWidth);
	float B = SafeRatio1(2.0f, (float) WindowHeight);
	// NOTE: this projection matrix is here to map our 0 to 1 screen space stuff 
	// CONT: to -1 to 1
	float Proj[] =
	{
		A,  0,  0,  0,
		0,  B,  0,  0,
		0,  0,  1,  0,
		-1, -1,  0,  1,
	};
	glLoadMatrixf(Proj);

	uint32_t ClipRectIndex = 0;
	for(uint32_t Index = 0; Index < RenderGroup->NumEntries; Index++)
	{
		render_entry_handle* EntryHandle = EntryHandles + Index;
		render_entry_header* Header = EntryHandle->Header;
		if(Header->ClipRectIndex != ClipRectIndex)
		{
			ASSERT(ClipRectIndex < RenderGroup->NumClipRects);
			ClipRectIndex = Header->ClipRectIndex;
			rectangle ClipRect = RenderGroup->ClipRects[ClipRectIndex];
			glScissor(
				(int) ClipRect.Min.X,
				(int) ClipRect.Min.Y,
				(int) ClipRect.Dim.X,
				(int) ClipRect.Dim.Y
			);
		}

		switch(Header->Type)
		{
			case(EntryType_Clear):
			{
				render_entry_clear* Entry = (render_entry_clear*) Header;
				glClearColor(
					Entry->Color.R,
					Entry->Color.G,
					Entry->Color.B,
					1.0f
				);
				glClear(GL_COLOR_BUFFER_BIT);
				break;
			}
			case(EntryType_Rectangle):
			{
				render_entry_rectangle* Entry = (render_entry_rectangle*) (
					Header
				);
				vector2 MaxP = Entry->Pos + Entry->XAxis + Entry->YAxis;
				glDisable(GL_TEXTURE_2D);
				OpenGlRectangle(Entry->Pos, MaxP, Entry->Color);
				glEnable(GL_TEXTURE_2D);
				break;
			}
			case(EntryType_Bitmap):
			{
				render_entry_bitmap* Entry = (render_entry_bitmap*) Header;
				ASSERT(Entry->Bitmap);

				vector2 MaxP = Entry->Pos + Entry->XAxis + Entry->YAxis;

				if(Entry->Bitmap->GlTextureHandle)
				{
					glBindTexture(
						GL_TEXTURE_2D, Entry->Bitmap->GlTextureHandle
					);
				}
				else
				{
					Entry->Bitmap->GlTextureHandle = ++GlobalTextureBindCount;
					glBindTexture(
						GL_TEXTURE_2D, Entry->Bitmap->GlTextureHandle
					);

					glTexImage2D(
						GL_TEXTURE_2D,
						0,
						GL_RGBA8,
						Entry->Bitmap->Width,
						Entry->Bitmap->Height,
						0,
						GL_BGRA_EXT,
						GL_UNSIGNED_BYTE,
						Entry->Bitmap->Memory
					);
	
					glTexParameteri(
						GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR
					);
					glTexParameteri(
						GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR
					);    
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);    
					glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				}
				OpenGlRectangle(Entry->Pos, MaxP, Entry->Color);
				break;
			}
			default:
			{
				ASSERT(false);
			}
		}
	}
	// SECTION STOP: render to output

	EndTempMemory(TempMemory);
	ResetMemArena(RenderArena);
	RenderGroup->NumEntries = 0;
}