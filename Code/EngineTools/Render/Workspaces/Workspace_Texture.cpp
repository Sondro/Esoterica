#include "Workspace_Texture.h"
#include "EngineTools/Core/Widgets/InterfaceHelpers.h"
#include "Engine/Entity/EntityWorld.h"

//-------------------------------------------------------------------------

namespace EE::Render
{
    EE_RESOURCE_WORKSPACE_FACTORY( TextureWorkspaceFactory, Texture, TextureWorkspace );

    //-------------------------------------------------------------------------

    void TextureWorkspace::Initialize( UpdateContext const& context )
    {
        TWorkspace<Texture>::Initialize( context );
        m_previewWindowName.sprintf( "Preview##%u", GetID() );
        m_infoWindowName.sprintf( "Info##%u", GetID() );
    }

    void TextureWorkspace::InitializeDockingLayout( ImGuiID dockspaceID ) const
    {
        ImGuiID topDockID = 0;
        ImGuiID bottomDockID = ImGui::DockBuilderSplitNode( dockspaceID, ImGuiDir_Down, 0.5f, nullptr, &topDockID );

        // Dock windows
        ImGui::DockBuilderDockWindow( m_previewWindowName.c_str(), topDockID );
        ImGui::DockBuilderDockWindow( m_descriptorWindowName.c_str(), bottomDockID );
        ImGui::DockBuilderDockWindow( m_infoWindowName.c_str(), bottomDockID );
    }

    void TextureWorkspace::Update( UpdateContext const& context, ImGuiWindowClass* pWindowClass, bool isFocused )
    {
        ImGui::SetNextWindowClass( pWindowClass );
        if ( ImGui::Begin( m_previewWindowName.c_str() ) )
        {
            if ( IsResourceLoaded() )
            {
                ImTextureID const textureID = (void*) &m_pResource->GetShaderResourceView();
                ImGui::Image( textureID, Float2( m_pResource->GetDimensions() ) );
            }
        }
        ImGui::End();

        //-------------------------------------------------------------------------

        ImGui::SetNextWindowClass( pWindowClass );
        if ( ImGui::Begin( m_infoWindowName.c_str() ) )
        {
            if ( IsWaitingForResource() )
            {
                ImGui::Text( "Loading:" );
                ImGui::SameLine();
                ImGuiX::DrawSpinner( "Loading" );
            }
            else if ( HasLoadingFailed() )
            {
                ImGui::Text( "Loading Failed: %s", m_pResource.GetResourceID().c_str() );
            }
            else
            {
                ImGui::Text( "Dimensions: %d x %d", m_pResource->GetDimensions().m_x, m_pResource->GetDimensions().m_y );
            }
        }
        ImGui::End();

        //-------------------------------------------------------------------------

        DrawDescriptorEditorWindow( context, pWindowClass );
    }
}