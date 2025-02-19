#pragma once
#include "Animation_ToolsGraphNode.h"
#include "Engine/Animation/Graph/Nodes/Animation_RuntimeGraphNode_Blends.h"

//-------------------------------------------------------------------------

namespace EE::Animation::GraphNodes
{
    class ParameterizedBlendToolsNode : public FlowToolsNode
    {
        EE_REGISTER_TYPE( ParameterizedBlendToolsNode );

    public:

        virtual void Initialize( VisualGraph::BaseGraph* pParent ) override;

        virtual char const* GetCategory() const override { return "Blends"; }
        virtual TBitFlags<GraphType> GetAllowedParentGraphTypes() const override final { return TBitFlags<GraphType>( GraphType::BlendTree ); }
        virtual bool SupportsDynamicInputPins() const override { return true; }
        virtual TInlineString<100> GetNewDynamicInputPinName() const override { return "Input"; }
        virtual uint32_t GetDynamicInputPinValueType() const override { return (uint32_t) GraphValueType::Pose; }

    protected:

        bool CompileParameterAndSourceNodes( GraphCompilationContext& context, ParameterizedBlendNode::Settings* pSettings ) const;

    protected:

        EE_EXPOSE bool                 m_isSynchronized = false;
    };

    //-------------------------------------------------------------------------

    class RangedBlendToolsNode final : public ParameterizedBlendToolsNode
    {
        EE_REGISTER_TYPE( RangedBlendToolsNode );

        virtual void Initialize( VisualGraph::BaseGraph* pParent ) override;

        virtual char const* GetTypeName() const override { return "Ranged Blend"; }
        virtual int16_t Compile( GraphCompilationContext& context ) const override;
        virtual bool DrawPinControls( VisualGraph::UserContext* pUserContext, VisualGraph::Flow::Pin const& pin ) override;
        virtual void OnDynamicPinCreation( UUID pinID ) override;
        virtual void OnDynamicPinDestruction( UUID pinID ) override;

    private:

        EE_REGISTER TVector<float>     m_parameterValues;
    };

    //-------------------------------------------------------------------------

    class VelocityBlendToolsNode final : public ParameterizedBlendToolsNode
    {
        EE_REGISTER_TYPE( VelocityBlendToolsNode );

        virtual char const* GetTypeName() const override { return "Velocity Blend"; }
        virtual int16_t Compile( GraphCompilationContext& context ) const override;
        virtual bool IsValidConnection( UUID const& inputPinID, Node const* pOutputPinNode, UUID const& outputPinID ) const override;
    };
}