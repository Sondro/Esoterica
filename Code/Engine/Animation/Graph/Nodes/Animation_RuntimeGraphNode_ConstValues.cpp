#include "Animation_RuntimeGraphNode_ConstValues.h"

//-------------------------------------------------------------------------

namespace EE::Animation::GraphNodes
{
    void ConstBoolNode::Settings::InstantiateNode( InstantiationContext const& context, InstantiationOptions options ) const
    {
        auto pNode = CreateNode<ConstBoolNode>( context, options );
    }

    void ConstBoolNode::GetValueInternal( GraphContext& context, void* pOutValue )
    {
        *( (bool*) pOutValue ) = GetSettings<ConstBoolNode>()->m_value;
    }

    //-------------------------------------------------------------------------

    void ConstIDNode::Settings::InstantiateNode( InstantiationContext const& context, InstantiationOptions options ) const
    {
        auto pNode = CreateNode<ConstIDNode>( context, options );
    }

    void ConstIDNode::GetValueInternal( GraphContext& context, void* pOutValue )
    {
        *( (StringID*) pOutValue ) = GetSettings<ConstIDNode>()->m_value;
    }

    //-------------------------------------------------------------------------

    void ConstIntNode::Settings::InstantiateNode( InstantiationContext const& context, InstantiationOptions options ) const
    {
        auto pNode = CreateNode<ConstIntNode>( context, options );
    }

    void ConstIntNode::GetValueInternal( GraphContext& context, void* pOutValue )
    {
        *( (int32_t*) pOutValue ) = GetSettings<ConstIntNode>()->m_value;
    }

    //-------------------------------------------------------------------------

    void ConstFloatNode::Settings::InstantiateNode( InstantiationContext const& context, InstantiationOptions options ) const
    {
        auto pNode = CreateNode<ConstFloatNode>( context, options );
    }

    void ConstFloatNode::GetValueInternal( GraphContext& context, void* pOutValue )
    {
        *( (float*) pOutValue ) = GetSettings<ConstFloatNode>()->m_value;
    }

    //-------------------------------------------------------------------------

    void ConstVectorNode::Settings::InstantiateNode( InstantiationContext const& context, InstantiationOptions options ) const
    {
        auto pNode = CreateNode<ConstVectorNode>( context, options );
    }

    void ConstVectorNode::GetValueInternal( GraphContext& context, void* pOutValue )
    {
        *( (Vector*) pOutValue ) = GetSettings<ConstVectorNode>()->m_value;
    }

    //-------------------------------------------------------------------------

    void ConstTargetNode::Settings::InstantiateNode( InstantiationContext const& context, InstantiationOptions options ) const
    {
        auto pNode = CreateNode<ConstTargetNode>( context, options );
    }

    void ConstTargetNode::GetValueInternal( GraphContext& context, void* pOutValue )
    {
        *( (Target*) pOutValue ) = GetSettings<ConstTargetNode>()->m_value;
    }
}