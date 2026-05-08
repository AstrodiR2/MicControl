package com.astrodir2.zombieapoc.entity;

import com.astrodir2.zombieapoc.ZombieApocMod;
import net.fabricmc.fabric.api.biome.v1.BiomeModifications;
import net.fabricmc.fabric.api.biome.v1.BiomeSelectors;
import net.fabricmc.fabric.api.object.builder.v1.entity.FabricEntityTypeBuilder;
import net.minecraft.entity.EntityDimensions;
import net.minecraft.entity.EntityType;
import net.minecraft.entity.SpawnGroup;
import net.minecraft.entity.attribute.DefaultAttributeContainer;
import net.minecraft.registry.Registries;
import net.minecraft.registry.Registry;
import net.minecraft.util.Identifier;

public class ModEntities {

    public static final EntityType<TopHatZombieEntity> TOP_HAT_ZOMBIE = Registry.register(
            Registries.ENTITY_TYPE,
            Identifier.of(ZombieApocMod.MOD_ID, "top_hat_zombie"),
            FabricEntityTypeBuilder.create(SpawnGroup.MONSTER, TopHatZombieEntity::new)
                    .dimensions(EntityDimensions.fixed(1.2f, 3.9f))
                    .build()
    );

    public static void register() {
        net.fabricmc.fabric.api.entity.event.v1.ServerEntityCombatEvents.AFTER_KILLED_OTHER_ENTITY.register((world, entity, killedEntity) -> {});

        BiomeModifications.addSpawn(
                BiomeSelectors.foundInOverworld(),
                SpawnGroup.MONSTER,
                TOP_HAT_ZOMBIE,
                5, 1, 3
        );

        net.fabricmc.fabric.api.object.builder.v1.entity.FabricDefaultAttributeRegistry.register(
                TOP_HAT_ZOMBIE,
                TopHatZombieEntity.createAttributes()
        );
    }
}
