package com.astrodir2.zombieapoc.client.renderer;

import com.astrodir2.zombieapoc.ZombieApocMod;
import com.astrodir2.zombieapoc.entity.TopHatZombieEntity;
import net.minecraft.client.render.OverlayTexture;
import net.minecraft.client.render.RenderLayer;
import net.minecraft.client.render.VertexConsumer;
import net.minecraft.client.render.VertexConsumerProvider;
import net.minecraft.client.render.entity.feature.FeatureRenderer;
import net.minecraft.client.render.entity.feature.FeatureRendererContext;
import net.minecraft.client.render.entity.model.ZombieEntityModel;
import net.minecraft.client.util.math.MatrixStack;
import net.minecraft.util.Identifier;
import software.bernie.geckolib.cache.GeckoLibCache;
import software.bernie.geckolib.cache.object.BakedGeoModel;
import software.bernie.geckolib.renderer.GeoRenderer;
import net.minecraft.client.render.entity.EntityRendererFactory;

public class TopHatLayerRenderer extends FeatureRenderer<TopHatZombieEntity, ZombieEntityModel<TopHatZombieEntity>> {

    private static final Identifier HAT_TEXTURE = Identifier.of(ZombieApocMod.MOD_ID, "textures/entity/top_hat.png");
    private static final Identifier HAT_MODEL = Identifier.of(ZombieApocMod.MOD_ID, "geo/top_hat_zombie.geo.json");

    private final TopHatGeoRenderer geoRenderer;

    public TopHatLayerRenderer(FeatureRendererContext<TopHatZombieEntity, ZombieEntityModel<TopHatZombieEntity>> context, EntityRendererFactory.Context ctx) {
        super(context);
        this.geoRenderer = new TopHatGeoRenderer(ctx);
    }

    @Override
    public void render(MatrixStack matrices, VertexConsumerProvider vertexConsumers, int light, TopHatZombieEntity entity, float limbAngle, float limbDistance, float tickDelta, float animationProgress, float headYaw, float headPitch) {
        geoRenderer.renderForEntity(entity, HAT_MODEL, HAT_TEXTURE, matrices, vertexConsumers, light, tickDelta);
    }
}
