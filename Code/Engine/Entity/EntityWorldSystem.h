#pragma once

#include "Engine/_Module/API.h"
#include "Engine/UpdateStage.h"
#include "Engine/Entity/EntityIDs.h"
#include "System/TypeSystem/RegisteredType.h"
#include "System/Types/Arrays.h"
#include "System/Algorithm/Hash.h"

//-------------------------------------------------------------------------
// World Entity System
//-------------------------------------------------------------------------
// This is a global system that exists once per world and tracks/updates all components of certain types in the world!

namespace EE
{
    class SystemRegistry;
    class EntityWorldUpdateContext;
    class Entity;
    class EntityComponent;

    //-------------------------------------------------------------------------

    class EE_ENGINE_API IEntityWorldSystem : public IRegisteredType
    {
        EE_REGISTER_TYPE( IEntityWorldSystem );

        friend class EntityWorld;

    public:

        virtual uint32_t GetSystemID() const = 0;

    protected:

        // Get the required update stages and priorities for this component
        virtual UpdatePriorityList const& GetRequiredUpdatePriorities() = 0;

        // Called when the system is registered with the world - using explicit "EntitySystem" name to allow for a standalone initialize function
        virtual void InitializeSystem( SystemRegistry const& systemRegistry ) {};

        // Called when the system is removed from the world - using explicit "EntitySystem" name to allow for a standalone shutdown function
        virtual void ShutdownSystem() {};

        // System Update - using explicit "EntitySystem" name to allow for a standalone update functions
        virtual void UpdateSystem( EntityWorldUpdateContext const& ctx ) {};

        // Called whenever a new component is activated (i.e. added to the world)
        virtual void RegisterComponent( Entity const* pEntity, EntityComponent* pComponent ) = 0;

        // Called immediately before an component is deactivated
        virtual void UnregisterComponent( Entity const* pEntity, EntityComponent* pComponent ) = 0;
    };
}

//-------------------------------------------------------------------------

#define EE_REGISTER_ENTITY_WORLD_SYSTEM( Type, ... )\
    EE_REGISTER_TYPE( Type );\
    constexpr static uint32_t const s_entitySystemID = Hash::FNV1a::GetHash32( #Type );\
    virtual uint32_t GetSystemID() const override final { return Type::s_entitySystemID; }\
    static UpdatePriorityList const PriorityList;\
    virtual UpdatePriorityList const& GetRequiredUpdatePriorities() override { static UpdatePriorityList const priorityList = UpdatePriorityList( __VA_ARGS__ ); return priorityList; };\
