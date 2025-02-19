#include "Animation_TaskPosePool.h"

//-------------------------------------------------------------------------

namespace EE::Animation
{
    PoseBuffer::PoseBuffer( Skeleton const* pSkeleton )
        : m_pose( pSkeleton )
    {}

    void PoseBuffer::Reset()
    {
        m_pose.Reset( Pose::Type::None );
        m_isUsed = false;
    }

    void PoseBuffer::CopyFrom( PoseBuffer const& rhs )
    {
        EE_ASSERT( m_pose.GetSkeleton() == rhs.m_pose.GetSkeleton() );
        m_pose.CopyFrom( rhs.m_pose );
    }

    //-------------------------------------------------------------------------

    PoseBufferPool::PoseBufferPool( Skeleton const* pSkeleton )
        : m_pSkeleton( pSkeleton )
    {
        EE_ASSERT( m_pSkeleton != nullptr );

        for ( auto i = 0; i < s_numInitialBuffers; i++ )
        {
            m_poseBuffers.emplace_back( PoseBuffer( m_pSkeleton ) );
            m_cachedBuffers.emplace_back( CachedPoseBuffer( m_pSkeleton ) );

            #if EE_DEVELOPMENT_TOOLS
            m_debugBuffers.emplace_back( PoseBuffer( m_pSkeleton ) );
            #endif
        }
    }

    PoseBufferPool::~PoseBufferPool()
    {
        Reset();
    }

    void PoseBufferPool::Reset()
    {
        // Reset all buffers
        for ( auto& PoseBuffer : m_poseBuffers )
        {
            PoseBuffer.Reset();
        }

        m_firstFreeBuffer = 0;

        // Process all cached buffer destruction requests
        int8_t const numCachedBuffers = (int8_t) m_cachedBuffers.size();
        for ( int8_t i = 0; i < numCachedBuffers; i++ )
        {
            if ( m_cachedBuffers[i].m_shouldBeDestroyed )
            {
                m_cachedBuffers[i].Reset();
                m_firstFreeCachedBuffer = Math::Min( m_firstFreeCachedBuffer, i );
            }
        }

        //-------------------------------------------------------------------------

        // Dont reset the actual debug buffers as we want to still access them this frame, only reset the free index
        #if EE_DEVELOPMENT_TOOLS
        m_firstFreeDebugBuffer = 0;
        #endif
    }

    int8_t PoseBufferPool::RequestPoseBuffer()
    {
        if ( m_firstFreeBuffer == m_poseBuffers.size() )
        {
            for ( auto i = 0; i < s_bufferGrowAmount; i++ )
            {
                m_poseBuffers.emplace_back( PoseBuffer( m_pSkeleton ) );
            }
            EE_ASSERT( m_poseBuffers.size() < 255 );
        }

        int8_t const freeBufferIdx = m_firstFreeBuffer;
        EE_ASSERT( !m_poseBuffers[freeBufferIdx].m_isUsed );
        m_poseBuffers[freeBufferIdx].m_isUsed = true;

        // Update free index
        int8_t const numPoseBuffers = (int8_t) m_poseBuffers.size();
        for ( ; m_firstFreeBuffer < numPoseBuffers; m_firstFreeBuffer++ )
        {
            if ( !m_poseBuffers[m_firstFreeBuffer].m_isUsed )
            {
                break;
            }
        }

        return freeBufferIdx;
    }

    void PoseBufferPool::ReleasePoseBuffer( int8_t BufferIdx )
    {
        EE_ASSERT( m_poseBuffers[BufferIdx].m_isUsed );
        m_poseBuffers[BufferIdx].m_isUsed = false;
        m_firstFreeBuffer = Math::Min( BufferIdx, m_firstFreeBuffer );
    }

    UUID PoseBufferPool::CreateCachedPoseBuffer()
    {
        CachedPoseBuffer* pCachedPoseBuffer = nullptr;

        if ( m_firstFreeCachedBuffer == m_cachedBuffers.size() )
        {
            for ( auto i = 0; i < s_bufferGrowAmount; i++ )
            {
                pCachedPoseBuffer = &m_cachedBuffers.emplace_back( CachedPoseBuffer( m_pSkeleton ) );
            }

            EE_ASSERT( m_cachedBuffers.size() < 255 );
        }
        else
        {
            pCachedPoseBuffer = &m_cachedBuffers[m_firstFreeCachedBuffer];
            EE_ASSERT( !pCachedPoseBuffer->m_isUsed );
        }

        //-------------------------------------------------------------------------

        // Create a new ID for the cached pose
        pCachedPoseBuffer->m_ID = UUID::GenerateID();
        pCachedPoseBuffer->m_isUsed = true;

        //-------------------------------------------------------------------------

        // Update free index
        int32_t const numCachedBuffers = (int32_t) m_cachedBuffers.size();
        for ( ; m_firstFreeCachedBuffer < numCachedBuffers; m_firstFreeCachedBuffer++ )
        {
            if ( !m_cachedBuffers[m_firstFreeCachedBuffer].m_isUsed )
            {
                break;
            }
        }

        return pCachedPoseBuffer->m_ID;
    }

    void PoseBufferPool::DestroyCachedPoseBuffer( UUID const& InCachedPoseID )
    {
        EE_ASSERT( InCachedPoseID.IsValid() );

        for ( auto& cachedBuffer : m_cachedBuffers )
        {
            if ( cachedBuffer.m_ID == InCachedPoseID )
            {
                // Cached buffer destruction is deferred to the next frame since we may already have tasks reading from it already
                cachedBuffer.m_shouldBeDestroyed = true;
                return;
            }
        }

        EE_UNREACHABLE_CODE();
    }

    PoseBuffer* PoseBufferPool::GetCachedPoseBuffer( UUID const& cachedPoseID )
    {
        EE_ASSERT( cachedPoseID.IsValid() );
        PoseBuffer* pFoundCachedPoseBuffer = nullptr;

        for ( auto& cachedBuffer : m_cachedBuffers )
        {
            if ( cachedBuffer.m_ID == cachedPoseID )
            {
                pFoundCachedPoseBuffer = &cachedBuffer;
                break;
            }
        }

        EE_ASSERT( pFoundCachedPoseBuffer != nullptr );
        return pFoundCachedPoseBuffer;
    }

    //-------------------------------------------------------------------------

    #if EE_DEVELOPMENT_TOOLS
    void PoseBufferPool::RecordPose( int8_t poseBufferIdx )
    {
        if ( !m_isDebugRecordingEnabled )
        {
            return;
        }

        // If we are out of buffers, add additional debug buffers
        if ( m_firstFreeDebugBuffer == m_debugBuffers.size() )
        {
            for ( auto i = 0; i < s_bufferGrowAmount; i++ )
            {
                m_debugBuffers.emplace_back( PoseBuffer( m_pSkeleton ) );
            }

            EE_ASSERT( m_debugBuffers.size() < 255 );
        }

        EE_ASSERT( m_poseBuffers[poseBufferIdx].m_isUsed );
        m_debugBuffers[m_firstFreeDebugBuffer++].CopyFrom( m_poseBuffers[poseBufferIdx] );
    }

    PoseBuffer const* PoseBufferPool::GetRecordedPose( int8_t debugBufferIdx ) const
    {
        EE_ASSERT( debugBufferIdx >= 0 && debugBufferIdx < m_debugBuffers.size() );
        return &m_debugBuffers[debugBufferIdx];
    }
    #endif
}