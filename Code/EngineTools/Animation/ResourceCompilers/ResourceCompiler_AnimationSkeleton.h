#pragma once

#include "EngineTools/_Module/API.h"
#include "EngineTools/Resource/ResourceCompiler.h"

//-------------------------------------------------------------------------

namespace EE::Animation
{
    class SkeletonCompiler : public Resource::Compiler
    {
        EE_REGISTER_TYPE( SkeletonCompiler );
        static const int32_t s_version = 2;

    public:

        SkeletonCompiler();
        virtual Resource::CompilationResult Compile( Resource::CompileContext const& ctx ) const final;
    };
}