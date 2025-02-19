#include "EditorContext.h"
#include "RenderingSystem.h"
#include "EngineTools/Entity/Workspaces/Workspace_MapEditor.h"
#include "EngineTools/Entity/Workspaces/Workspace_GamePreviewer.h"
#include "EngineTools/Core/Workspace.h"
#include "EngineTools/ThirdParty/pfd/portable-file-dialogs.h"
#include "Engine/Entity/EntityWorld.h"
#include "Engine/Entity/EntityWorldManager.h"
#include "Engine/UpdateContext.h"
#include "System/TypeSystem/TypeRegistry.h"
#include "System/Resource/ResourceSettings.h"
#include "System/Resource/ResourceSystem.h"
#include "System/IniFile.h"

//-------------------------------------------------------------------------

namespace EE
{
    EditorContext::~EditorContext()
    {
        EE_ASSERT( m_workspaces.empty() );
    }

    void EditorContext::Initialize( UpdateContext const& context )
    {
        m_pTypeRegistry = context.GetSystem<TypeSystem::TypeRegistry>();
        m_pResourceSystem = context.GetSystem<Resource::ResourceSystem>();
        m_pWorldManager = context.GetSystem<EntityWorldManager>();
        m_pRenderingSystem = context.GetSystem<Render::RenderingSystem>();

        m_resourceDB.Initialize( m_pTypeRegistry, m_pResourceSystem->GetSettings().m_rawResourcePath, m_pResourceSystem->GetSettings().m_compiledResourcePath );
        m_pResourceDatabase = &m_resourceDB;

        // Create map editor workspace
        //-------------------------------------------------------------------------

        // Destroy the default created game world
        m_pWorldManager->DestroyWorld( m_pWorldManager->GetWorlds()[0] );

        // Create the map editor world
        auto pMapEditorWorld = m_pWorldManager->CreateWorld( EntityWorldType::Tools );
        m_pRenderingSystem->CreateCustomRenderTargetForViewport( pMapEditorWorld->GetViewport(), true );
        m_pMapEditor = EE::New<EntityModel::EntityMapEditor>( this, pMapEditorWorld );
        m_pMapEditor->Initialize( context );
        m_workspaces.emplace_back( m_pMapEditor );

        // Create bindings to start/stop game preview
        m_gamePreviewStartedEventBindingID = m_pMapEditor->OnGamePreviewStartRequested().Bind( [this] ( UpdateContext const& context ) { StartGamePreview( context ); } );
        m_gamePreviewStoppedEventBindingID = m_pMapEditor->OnGamePreviewStopRequested().Bind( [this] ( UpdateContext const& context ) { StopGamePreview( context ); } );
    }

    void EditorContext::Shutdown( UpdateContext const& context )
    {
        m_pGamePreviewer = nullptr;

        EE_ASSERT( m_pMapEditor != nullptr );
        m_pMapEditor->OnGamePreviewStartRequested().Unbind( m_gamePreviewStartedEventBindingID );
        m_pMapEditor->OnGamePreviewStopRequested().Unbind( m_gamePreviewStoppedEventBindingID );
        m_pMapEditor = nullptr;

        while( !m_workspaces.empty() )
        {
            DestroyWorkspaceInternal( context, m_workspaces[0] );
        }

        m_workspaces.clear();

        //-------------------------------------------------------------------------

        m_pWorldManager = nullptr;
        m_pRenderingSystem = nullptr;
        m_pResourceSystem = nullptr;
        m_pTypeRegistry = nullptr;
        m_pResourceDatabase = nullptr;

        m_resourceDB.Shutdown();
    }

    void EditorContext::Update( UpdateContext const& context )
    {
        // Update the resource database
        m_resourceDB.Update();

        // Destroy all required workspaces
        // We needed to defer this to the start of the update since we may have references resources that we might unload (i.e. textures)
        for ( auto pWorkspaceToDestroy : m_workspaceDestructionRequests )
        {
            DestroyWorkspace( context, pWorkspaceToDestroy );
        }
        m_workspaceDestructionRequests.clear();

        // Create all workspaces
        for ( auto const& resourceID : m_workspaceCreationRequests )
        {
            TryCreateWorkspace( context, resourceID );
        }
        m_workspaceCreationRequests.clear();
    }

    //-------------------------------------------------------------------------

    bool EditorContext::TryCreateWorkspace( UpdateContext const& context, ResourceID const& resourceID )
    {
        ResourceTypeID const resourceTypeID = resourceID.GetResourceTypeID();

        // Don't try to open invalid resource IDs
        if ( !m_resourceDB.DoesResourceExist( resourceID ) )
        {
            return false;
        }

        // Handle maps explicitly
        //-------------------------------------------------------------------------

        if ( resourceTypeID == EntityModel::SerializedEntityMap::GetStaticResourceTypeID() )
        {
            m_pMapEditor->LoadMap( resourceID );
            ImGuiX::MakeTabVisible( m_pMapEditor->GetWorkspaceWindowID() );
            return true;
        }

        // Other resource types
        //-------------------------------------------------------------------------

        // Check if we already have a workspace open for this resource, if so then switch focus to it
        for ( auto pWorkspace : m_workspaces )
        {
            if ( pWorkspace->IsEditingResource( resourceID ) )
            {
                ImGuiX::MakeTabVisible( pWorkspace->GetWorkspaceWindowID() );
                return true;
            }
        }

        // Check if we can create a new workspace
        if ( !ResourceWorkspaceFactory::CanCreateWorkspace( this, resourceID ) )
        {
            return false;
        }

        // Create preview world
        auto pPreviewWorld = m_pWorldManager->CreateWorld( EntityWorldType::Tools );
        pPreviewWorld->LoadMap( ResourcePath( "data://Editor/EditorMap.map" ) );
        m_pRenderingSystem->CreateCustomRenderTargetForViewport( pPreviewWorld->GetViewport() );

        // Create workspace
        auto pCreatedWorkspace = ResourceWorkspaceFactory::CreateWorkspace( this, pPreviewWorld, resourceID );
        pCreatedWorkspace->Initialize( context );
        m_workspaces.emplace_back( pCreatedWorkspace );

        return true;
    }

    void EditorContext::QueueCreateWorkspace( ResourceID const& resourceID )
    {
        m_workspaceCreationRequests.emplace_back( resourceID );
    }

    void EditorContext::DestroyWorkspace( UpdateContext const& context, Workspace* pWorkspace )
    {
        EE_ASSERT( m_pMapEditor != pWorkspace );
        DestroyWorkspaceInternal( context, pWorkspace );
    }

    void EditorContext::QueueDestroyWorkspace( Workspace* pWorkspace )
    {
        EE_ASSERT( m_pMapEditor != pWorkspace );
        m_workspaceDestructionRequests.emplace_back( pWorkspace );
    }

    void EditorContext::DestroyWorkspaceInternal( UpdateContext const& context, Workspace* pWorkspace )
    {
        EE_ASSERT( pWorkspace != nullptr );

        auto foundWorkspaceIter = eastl::find( m_workspaces.begin(), m_workspaces.end(), pWorkspace );
        EE_ASSERT( foundWorkspaceIter != m_workspaces.end() );

        if ( pWorkspace->IsDirty() )
        {
            auto messageDialog = pfd::message( "Unsaved Changes", "You have unsaved changes!\nDo you wish to save these changes before closing?", pfd::choice::yes_no_cancel );
            switch ( messageDialog.result() )
            {
                case pfd::button::yes:
                {
                    if ( !pWorkspace->Save() )
                    {
                        return;
                    }
                }
                break;

                case pfd::button::cancel:
                {
                    return;
                }
                break;
            }
        }

        //-------------------------------------------------------------------------

        bool const isGamePreviewerWorkspace = m_pGamePreviewer == pWorkspace;

        // Destroy the custom viewport render target
        auto pPreviewWorld = pWorkspace->GetWorld();
        m_pRenderingSystem->DestroyCustomRenderTargetForViewport( pPreviewWorld->GetViewport() );

        // Destroy workspace
        pWorkspace->Shutdown( context );
        EE::Delete( pWorkspace );
        m_workspaces.erase( foundWorkspaceIter );

        // Clear the game previewer workspace ptr if we just destroyed it
        if ( isGamePreviewerWorkspace )
        {
            m_pMapEditor->NotifyGamePreviewEnded();
            m_pGamePreviewer = nullptr;
        }

        // Destroy preview world
        m_pWorldManager->DestroyWorld( pPreviewWorld );
    }

    void EditorContext::LoadMap( ResourceID const& mapResourceID ) const
    {
        EE_ASSERT( m_pMapEditor != nullptr );
        EE_ASSERT( mapResourceID.GetResourceTypeID() == EntityModel::SerializedEntityMap::GetStaticResourceTypeID() );
        m_pMapEditor->LoadMap( mapResourceID );
    }

    bool EditorContext::IsMapEditorWorkspace( Workspace const* pWorkspace ) const
    {
        return m_pMapEditor == pWorkspace;
    }

    char const* EditorContext::GetMapEditorWindowName() const
    {
        return m_pMapEditor->GetWorkspaceWindowID();
    }

    bool EditorContext::IsGamePreviewWorkspace( Workspace const* pWorkspace ) const
    {
        return m_pGamePreviewer == pWorkspace;
    }

    void* EditorContext::GetViewportTextureForWorkspace( Workspace* pWorkspace ) const
    {
        EE_ASSERT( pWorkspace != nullptr );
        auto pWorld = pWorkspace->GetWorld();
        return (void*) &m_pRenderingSystem->GetRenderTargetTextureForViewport( pWorld->GetViewport() );
    }

    Render::PickingID EditorContext::GetViewportPickingID( Workspace* pWorkspace, Int2 const& pixelCoords ) const
    {
        EE_ASSERT( pWorkspace != nullptr );
        auto pWorld = pWorkspace->GetWorld();
        return m_pRenderingSystem->GetViewportPickingID( pWorld->GetViewport(), pixelCoords );
    }

    //-------------------------------------------------------------------------

    void EditorContext::StartGamePreview( UpdateContext const& context )
    {
        EE_ASSERT( m_pGamePreviewer == nullptr );

        auto pPreviewWorld = m_pWorldManager->CreateWorld( EntityWorldType::Game );
        m_pRenderingSystem->CreateCustomRenderTargetForViewport( pPreviewWorld->GetViewport() );
        m_pGamePreviewer = EE::New<GamePreviewer>( this, pPreviewWorld );
        m_pGamePreviewer->Initialize( context );
        m_pGamePreviewer->LoadMapToPreview( m_pMapEditor->GetLoadedMap() );
        m_workspaces.emplace_back( m_pGamePreviewer );

        m_pMapEditor->NotifyGamePreviewStarted();
    }

    void EditorContext::StopGamePreview( UpdateContext const& context )
    {
        EE_ASSERT( m_pGamePreviewer != nullptr );
        QueueDestroyWorkspace( m_pGamePreviewer );
    }
}