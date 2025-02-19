#pragma once

#include "System/_Module/API.h"
#include "System/Drawing/DebugDrawing.h"
#include "System/Threading/Threading.h"

//-------------------------------------------------------------------------

#if EE_DEVELOPMENT_TOOLS
namespace EE::Drawing
{
    class EE_SYSTEM_API DrawingSystem
    {

    public:

        DrawingSystem() = default;
        ~DrawingSystem();

        // Empty all per thread buffers
        void Reset();

        // Returns a per-thread drawing context, this removes the need for constantly calling get thread command buffer
        inline DrawContext GetDrawingContext() { return DrawContext( GetThreadCommandBuffer() ); }

        // Reflects all the individual per-thread buffers into a single supplied frame command buffer. Clears all thread buffers.
        void ReflectFrameCommandBuffer( Seconds const deltaTime, FrameCommandBuffer& reflectedFrameCommands );

    private:

        ThreadCommandBuffer& GetThreadCommandBuffer();

    private:

        TVector<ThreadCommandBuffer*>       m_threadCommandBuffers;
        Threading::Mutex                    m_commandBufferMutex;
    };
}
#endif