package com.astrodir2.zombieapoc.client.model;

import com.astrodir2.zombieapoc.ZombieApocMod;
import com.astrodir2.zombieapoc.entity.TopHatZombieEntity;
import net.minecraft.util.Identifier;
import software.bernie.geckolib.model.GeoModel;

public class TopHatZombieModel extends GeoModel<TopHatZombieEntity> {

    @Override
    public Identifier getModelResource(TopHatZombieEntity entity) {
        return Identifier.of(ZombieApocMod.MOD_ID, "geo/top_hat_zombie.geo.json");
    }

    @Override
    public Identifier getTextureResource(TopHatZombieEntity entity) {
        return Identifier.of(ZombieApocMod.MOD_ID, "textures/entity/top_hat.png");
    }

    @Override
    public Identifier getAnimationResource(TopHatZombieEntity entity) {
        return Identifier.of(ZombieApocMod.MOD_ID, "animations/top_hat_zombie.animation.json");
    }
}
