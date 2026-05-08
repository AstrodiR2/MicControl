package com.astrodir2.zombieapoc.client.renderer;

import com.astrodir2.zombieapoc.ZombieApocMod;
import com.astrodir2.zombieapoc.entity.TopHatZombieEntity;
import com.astrodir2.zombieapoc.client.model.TopHatZombieModel;
import net.minecraft.client.render.entity.EntityRendererFactory;
import net.minecraft.util.Identifier;
import software.bernie.geckolib.renderer.GeoEntityRenderer;

public class TopHatGeoRenderer extends GeoEntityRenderer<TopHatZombieEntity> {

    public TopHatGeoRenderer(EntityRendererFactory.Context context) {
        super(context, new TopHatZombieModel());
    }

    @Override
    public Identifier getTextureLocation(TopHatZombieEntity entity) {
        return Identifier.of(ZombieApocMod.MOD_ID, "textures/entity/top_hat.png");
    }
}
