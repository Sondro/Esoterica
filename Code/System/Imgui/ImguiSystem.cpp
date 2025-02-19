#include "ImguiSystem.h"

#if EE_DEVELOPMENT_TOOLS
#include "ImguiFont.h"
#include "ImguiStyle.h"
#include "System/Fonts/FontDecompressor.h"
#include "System/Fonts/FontData_Lexend.h"
#include "System/Fonts/FontData_MaterialDesign.h"

//-------------------------------------------------------------------------

#if _WIN32
#include "Platform/ImguiPlatform_Win32.h"
#include "System/ThirdParty/imgui/misc/freetype/imgui_freetype.h"
#endif

//-------------------------------------------------------------------------

namespace EE::ImGuiX
{
    bool ImguiSystem::Initialize( String const& iniFilename, Render::RenderDevice* pRenderDevice, bool enableViewports )
    {
        ImGui::CreateContext();

        //-------------------------------------------------------------------------

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigWindowsMoveFromTitleBarOnly = true;

        if ( enableViewports )
        {
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        }

        // Set render device in the user data
        EE_ASSERT( pRenderDevice != nullptr );
        io.BackendRendererUserData = pRenderDevice;

        //-------------------------------------------------------------------------

        m_iniFilename = iniFilename;
        if ( !m_iniFilename.empty() )
        {
            io.IniFilename = m_iniFilename.c_str();
        }

        //-------------------------------------------------------------------------

        Platform::InitializePlatform();
        InitializeFonts();

        //-------------------------------------------------------------------------

        Style::Apply();

        return true;
    }

    void ImguiSystem::Shutdown()
    {
        for ( int i = 0; i < (int8_t) Font::NumFonts; i++ )
        {
            SystemFonts::s_fonts[i] = nullptr;
        }

        Platform::ShutdownPlatform();
        ImGui::DestroyContext();
    }

    void ImguiSystem::InitializeFonts()
    {
        ImGuiIO& io = ImGui::GetIO();

        // Decompress fonts
        //-------------------------------------------------------------------------

        Blob fontData, boldFontData;
        Fonts::GetDecompressedFontData( Fonts::Lexend::Regular::GetData(), fontData );
        Fonts::GetDecompressedFontData( Fonts::Lexend::Bold::GetData(), boldFontData );

        ImWchar const icons_ranges[] = { EE_ICONRANGE_MIN, EE_ICONRANGE_MAX, 0 };
        Blob iconFontData;
        Fonts::GetDecompressedFontData( (uint8_t const*) Fonts::MaterialDesignIcons::GetData(), iconFontData );

        // Base font configs
        //-------------------------------------------------------------------------

        ImFontConfig fontConfig;
        fontConfig.FontDataOwnedByAtlas = false;

        ImFontConfig iconFontConfig;
        iconFontConfig.FontDataOwnedByAtlas = false;
        iconFontConfig.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_LoadColor | ImGuiFreeTypeBuilderFlags_Bitmap;
        iconFontConfig.MergeMode = true;
        iconFontConfig.PixelSnapH = true;
        iconFontConfig.RasterizerMultiply = 1.5f;

        auto CreateFont = [&] ( Blob& fontData, float fontSize, float iconFontSize, Font fontID, char const* pName, ImVec2 const& glyphOffset )
        {
            Printf( fontConfig.Name, 40, pName );
            ImFont* pFont = io.Fonts->AddFontFromMemoryTTF( fontData.data(), (int32_t) fontData.size(), fontSize, &fontConfig );
            SystemFonts::s_fonts[(uint8_t) fontID] = pFont;

            iconFontConfig.GlyphOffset = glyphOffset;
            iconFontConfig.GlyphMinAdvanceX = iconFontSize;
            io.Fonts->AddFontFromMemoryTTF( iconFontData.data(), (int32_t) iconFontData.size(), iconFontSize, &iconFontConfig, icons_ranges );
        };

        CreateFont( fontData, 12, 14, Font::Tiny, "Tiny", ImVec2( 0, 2 ) );
        CreateFont( boldFontData, 12, 14, Font::TinyBold, "Tiny Bold", ImVec2( 0, 2 ) );

        CreateFont( fontData, 14, 16, Font::Small, "Small", ImVec2( 0, 2 ) );
        CreateFont( boldFontData, 14, 16, Font::SmallBold, "Small Bold", ImVec2( 0, 2 ) );

        CreateFont( fontData, 16, 18, Font::Medium, "Medium", ImVec2( 0, 2 ) );
        CreateFont( boldFontData, 16, 18, Font::MediumBold, "Medium Bold", ImVec2( 0, 2 ) );

        CreateFont( fontData, 24, 24, Font::Large, "Large", ImVec2( 0, 2 ) );
        CreateFont( boldFontData, 24, 24, Font::LargeBold, "Large Bold", ImVec2( 0, 2 ) );

        CreateFont( fontData, 36, 36, Font::Huge, "Huge", ImVec2( 0, 2 ) );
        CreateFont( boldFontData, 36, 36, Font::HugeBold, "Huge Bold", ImVec2( 0, 2 ) );

        // Build font atlas
        //-------------------------------------------------------------------------

        io.Fonts->Build();
        EE_ASSERT( io.Fonts->IsBuilt() );

        #if EE_DEVELOPMENT_TOOLS
        EE_ASSERT( SystemFonts::s_fonts[(uint8_t) Font::Small]->IsLoaded() );
        EE_ASSERT( SystemFonts::s_fonts[(uint8_t) Font::SmallBold]->IsLoaded() );
        EE_ASSERT( SystemFonts::s_fonts[(uint8_t) Font::Medium]->IsLoaded() );
        EE_ASSERT( SystemFonts::s_fonts[(uint8_t) Font::MediumBold]->IsLoaded() );
        EE_ASSERT( SystemFonts::s_fonts[(uint8_t) Font::Large]->IsLoaded() );
        EE_ASSERT( SystemFonts::s_fonts[(uint8_t) Font::LargeBold]->IsLoaded() );
        EE_ASSERT( SystemFonts::s_fonts[(uint8_t) Font::Huge]->IsLoaded() );
        EE_ASSERT( SystemFonts::s_fonts[(uint8_t) Font::HugeBold]->IsLoaded() );
        #endif

        io.FontDefault = SystemFonts::s_fonts[(uint8_t) Font::Medium];
    }

    //-------------------------------------------------------------------------

    void ImguiSystem::StartFrame( float deltaTime )
    {
        ImGuiIO& io = ImGui::GetIO();
        io.DeltaTime = deltaTime;

        Platform::UpdateDisplayInformation();
        Platform::UpdateInputInformation();

        ImGui::NewFrame();
    }

    void ImguiSystem::EndFrame()
    {
        ImGui::EndFrame();
    }
}
#endif