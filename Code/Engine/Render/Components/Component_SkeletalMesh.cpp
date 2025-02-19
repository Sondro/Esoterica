#include "Component_SkeletalMesh.h"
#include "Engine/Animation/AnimationPose.h"
#include "System/Drawing/DebugDrawing.h"
#include "System/Profiling.h"

//-------------------------------------------------------------------------

namespace EE::Render
{
    void SkeletalMeshComponent::Initialize()
    {
        MeshComponent::Initialize();

        if ( HasMeshResourceSet() )
        {
            EE_ASSERT( m_pMesh.IsLoaded() );
            SetLocalBounds( m_pMesh->GetBounds() );

            if ( HasSkeletonResourceSet() )
            {
                GenerateAnimationBoneMap();
            }

            // Set mesh to reference pose
            //-------------------------------------------------------------------------

            m_boneTransforms.resize( m_pMesh->GetNumBones() );
            ResetPose();

            //-------------------------------------------------------------------------

            // Allocate skinning transforms and calculate initial values
            m_skinningTransforms.resize( m_boneTransforms.size() );
            FinalizePose();
        }
    }

    void SkeletalMeshComponent::Shutdown()
    {
        m_boneTransforms.clear();
        m_skinningTransforms.clear();
        m_animToMeshBoneMap.clear();
        MeshComponent::Shutdown();
    }

    TVector<TResourcePtr<Render::Material>> const& SkeletalMeshComponent::GetDefaultMaterials() const
    {
        EE_ASSERT( IsInitialized() && HasMeshResourceSet() );
        return m_pMesh->GetMaterials();
    }

    bool SkeletalMeshComponent::TryFindAttachmentSocketTransform( StringID socketID, Transform& outSocketWorldTransform ) const
    {
        EE_ASSERT( socketID.IsValid() );

        outSocketWorldTransform = GetWorldTransform();

        if ( m_pMesh.IsValid() && m_pMesh.IsLoaded() )
        {
            auto const boneIdx = m_pMesh->GetBoneIndex( socketID );
            if ( boneIdx != InvalidIndex )
            {
                if ( IsInitialized() )
                {
                    outSocketWorldTransform = m_boneTransforms[boneIdx] * outSocketWorldTransform;
                }
                else
                {
                    outSocketWorldTransform = m_pMesh->GetBindPose()[boneIdx] * outSocketWorldTransform;
                }

                return true;
            }
        }

        return false;
    }

    bool SkeletalMeshComponent::HasSocket( StringID socketID ) const
    {
        EE_ASSERT( socketID.IsValid() );

        if ( m_pMesh.IsValid() && m_pMesh.IsLoaded() )
        {
            int32_t boneIdx = m_pMesh->GetBoneIndex( socketID );
            return boneIdx != InvalidIndex;
        }

        return false;
    }

    //-------------------------------------------------------------------------

    void SkeletalMeshComponent::SetSkeleton( ResourceID skeletonResourceID )
    {
        EE_ASSERT( IsUnloaded() );
        EE_ASSERT( skeletonResourceID.IsValid() );
        m_pSkeleton = skeletonResourceID;
    }

    void SkeletalMeshComponent::SetPose( Animation::Pose const* pPose )
    {
        EE_PROFILE_FUNCTION_RENDER();
        EE_ASSERT( IsInitialized() );
        EE_ASSERT( HasMeshResourceSet() && HasSkeletonResourceSet() );
        EE_ASSERT( !m_animToMeshBoneMap.empty() );
        EE_ASSERT( pPose != nullptr && pPose->HasGlobalTransforms() );

        int32_t const numAnimBones = pPose->GetNumBones();
        for ( auto animBoneIdx = 0; animBoneIdx < numAnimBones; animBoneIdx++ )
        {
            int32_t const meshBoneIdx = m_animToMeshBoneMap[animBoneIdx];
            if ( meshBoneIdx != InvalidIndex )
            {
                Transform const boneTransform = pPose->GetGlobalTransform( animBoneIdx );
                m_boneTransforms[meshBoneIdx] = boneTransform;
            }
        }
    }

    void SkeletalMeshComponent::ResetPose()
    {
        EE_ASSERT( IsInitialized() );

        if ( HasSkeletonResourceSet() )
        {
            Animation::Pose referencePose( m_pSkeleton.GetPtr() );
            referencePose.CalculateGlobalTransforms();
            SetPose( &referencePose );
        }
        else
        {
            m_boneTransforms = m_pMesh->GetBindPose();
        }
    }

    void SkeletalMeshComponent::FinalizePose()
    {
        EE_PROFILE_FUNCTION_RENDER();
        EE_ASSERT( m_pMesh.IsValid() && m_pMesh.IsLoaded() );

        NotifySocketsUpdated();
        UpdateBounds();
        UpdateSkinningTransforms();
    }

    //-------------------------------------------------------------------------

    void SkeletalMeshComponent::UpdateBounds()
    {
        EE_ASSERT( m_pMesh.IsValid() && m_pMesh.IsLoaded() );

        AABB newBounds;
        for ( auto const& boneTransform : m_boneTransforms )
        {
            newBounds.AddPoint( boneTransform.GetTranslation() );
        }

        SetLocalBounds( OBB( newBounds ) );
    }

    void SkeletalMeshComponent::UpdateSkinningTransforms()
    {
        EE_ASSERT( m_pMesh.IsValid() && m_pMesh.IsLoaded() );

        auto const numBones = m_boneTransforms.size();
        EE_ASSERT( m_skinningTransforms.size() == numBones );

        auto const& inverseBindPose = m_pMesh->GetInverseBindPose();
        for ( auto i = 0; i < numBones; i++ )
        {
            Transform const skinningTransform = inverseBindPose[i] * m_boneTransforms[i];
            m_skinningTransforms[i] = ( skinningTransform ).ToMatrix();
        }
    }

    void SkeletalMeshComponent::GenerateAnimationBoneMap()
    {
        EE_ASSERT( m_pMesh != nullptr && m_pSkeleton != nullptr );

        auto const pMesh = GetMesh();

        auto const numBones = m_pSkeleton->GetNumBones();
        m_animToMeshBoneMap.resize( numBones, InvalidIndex );

        for ( auto boneIdx = 0; boneIdx < numBones; boneIdx++ )
        {
            auto const& boneID = m_pSkeleton->GetBoneID( boneIdx );
            m_animToMeshBoneMap[boneIdx] = pMesh->GetBoneIndex( boneID );
        }
    }

    //-------------------------------------------------------------------------

    #if EE_DEVELOPMENT_TOOLS
    void SkeletalMeshComponent::DrawPose( Drawing::DrawContext& drawingContext ) const
    {
        EE_ASSERT( IsInitialized() );

        if ( !m_pMesh.IsValid() || !m_pMesh.IsLoaded() )
        {
            return;
        }

        //-------------------------------------------------------------------------

        Transform const& worldTransform = GetWorldTransform();
        auto const numBones = m_boneTransforms.size();

        Transform boneWorldTransform = m_boneTransforms[0] * worldTransform;
        drawingContext.DrawBox( boneWorldTransform, Float3( 0.005f ), Colors::Orange );
        drawingContext.DrawAxis( boneWorldTransform, 0.05f );

        for ( auto i = 1; i < numBones; i++ )
        {
            boneWorldTransform = m_boneTransforms[i] * worldTransform;

            auto const parentBoneIdx = m_pMesh->GetParentBoneIndex( i );
            Transform const parentBoneWorldTransform = m_boneTransforms[parentBoneIdx] * worldTransform;

            drawingContext.DrawLine( parentBoneWorldTransform.GetTranslation(), boneWorldTransform.GetTranslation(), Colors::Orange );
            drawingContext.DrawAxis( boneWorldTransform, 0.03f, 2.0f );
        }
    }
    #endif
}
