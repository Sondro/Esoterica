#pragma once
#include "Animation_ToolsGraphNode.h"

//-------------------------------------------------------------------------

namespace EE::Animation::GraphNodes
{
    class ConstBoolToolsNode final : public FlowToolsNode
    {
        EE_REGISTER_TYPE( ConstBoolToolsNode );

    public:

        virtual void Initialize( VisualGraph::BaseGraph* pParent ) override;

        virtual char const* GetTypeName() const override { return "Const Bool"; }
        virtual char const* GetCategory() const override { return "Values/Bool"; }
        virtual TBitFlags<GraphType> GetAllowedParentGraphTypes() const override { return TBitFlags<GraphType>( GraphType::BlendTree, GraphType::ValueTree, GraphType::TransitionTree ); }
        virtual int16_t Compile( GraphCompilationContext& context ) const override;
        virtual void DrawExtraControls( VisualGraph::DrawContext const& ctx, VisualGraph::UserContext* pUserContext ) override;

    private:

        EE_EXPOSE bool m_value;
    };

    //-------------------------------------------------------------------------

    class ConstIDToolsNode final : public FlowToolsNode
    {
        EE_REGISTER_TYPE( ConstIDToolsNode );

    public:

        virtual void Initialize( VisualGraph::BaseGraph* pParent ) override;

        virtual char const* GetTypeName() const override { return "Const ID"; }
        virtual char const* GetCategory() const override { return "Values/ID"; }
        virtual TBitFlags<GraphType> GetAllowedParentGraphTypes() const override { return TBitFlags<GraphType>( GraphType::BlendTree, GraphType::ValueTree, GraphType::TransitionTree ); }
        virtual int16_t Compile( GraphCompilationContext& context ) const override;
        virtual void DrawExtraControls( VisualGraph::DrawContext const& ctx, VisualGraph::UserContext* pUserContext ) override;

    private:

        EE_EXPOSE StringID m_value;
    };

    //-------------------------------------------------------------------------
    
    class ConstIntToolsNode final : public FlowToolsNode
    {
        EE_REGISTER_TYPE( ConstIntToolsNode );

    public:

        virtual void Initialize( VisualGraph::BaseGraph* pParent ) override;

        virtual char const* GetTypeName() const override { return "Const Int"; }
        virtual char const* GetCategory() const override { return "Values/Int"; }
        virtual TBitFlags<GraphType> GetAllowedParentGraphTypes() const override { return TBitFlags<GraphType>( GraphType::BlendTree, GraphType::ValueTree, GraphType::TransitionTree ); }
        virtual int16_t Compile( GraphCompilationContext& context ) const override;
        virtual void DrawExtraControls( VisualGraph::DrawContext const& ctx, VisualGraph::UserContext* pUserContext ) override;

    private:

        EE_EXPOSE int32_t m_value;
    };

    //-------------------------------------------------------------------------

    class ConstFloatToolsNode final : public FlowToolsNode
    {
        EE_REGISTER_TYPE( ConstFloatToolsNode );

    public:

        virtual void Initialize( VisualGraph::BaseGraph* pParent ) override;

        virtual char const* GetTypeName() const override { return "Const Float"; }
        virtual char const* GetCategory() const override { return "Values/Float"; }
        virtual TBitFlags<GraphType> GetAllowedParentGraphTypes() const override { return TBitFlags<GraphType>( GraphType::BlendTree, GraphType::ValueTree, GraphType::TransitionTree ); }
        virtual int16_t Compile( GraphCompilationContext& context ) const override;
        virtual void DrawExtraControls( VisualGraph::DrawContext const& ctx, VisualGraph::UserContext* pUserContext ) override;

    private:

        EE_EXPOSE float m_value;
    };

    //-------------------------------------------------------------------------

    class ConstVectorToolsNode final : public FlowToolsNode
    {
        EE_REGISTER_TYPE( ConstVectorToolsNode );

    public:

        virtual void Initialize( VisualGraph::BaseGraph* pParent ) override;

        virtual char const* GetTypeName() const override { return "Const Vector"; }
        virtual char const* GetCategory() const override { return "Values/Vector"; }
        virtual TBitFlags<GraphType> GetAllowedParentGraphTypes() const override { return TBitFlags<GraphType>( GraphType::BlendTree, GraphType::ValueTree, GraphType::TransitionTree ); }
        virtual int16_t Compile( GraphCompilationContext& context ) const override;
        virtual void DrawExtraControls( VisualGraph::DrawContext const& ctx, VisualGraph::UserContext* pUserContext ) override;

    private:

        EE_EXPOSE Float4 m_value;
    };

    //-------------------------------------------------------------------------

    class ConstTargetToolsNode final : public FlowToolsNode
    {
        EE_REGISTER_TYPE( ConstTargetToolsNode );

    public:

        virtual void Initialize( VisualGraph::BaseGraph* pParent ) override;

        virtual char const* GetTypeName() const override { return "Const Target"; }
        virtual char const* GetCategory() const override { return "Values/Target"; }
        virtual TBitFlags<GraphType> GetAllowedParentGraphTypes() const override { return TBitFlags<GraphType>( GraphType::BlendTree, GraphType::ValueTree, GraphType::TransitionTree ); }
        virtual int16_t Compile( GraphCompilationContext& context ) const override;
        virtual void DrawExtraControls( VisualGraph::DrawContext const& ctx, VisualGraph::UserContext* pUserContext ) override;

    private:

        EE_EXPOSE Transform m_value;
    };
}