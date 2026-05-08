package com.astrodir2.zombieapoc.client.renderer;

import com.astrodir2.zombieapoc.ZombieApocMod;
import com.astrodir2.zombieapoc.entity.TopHatZombieEntity;
import com.astrodir2.zombieapoc.item.ModItems;
import net.minecraft.client.render.OverlayTexture;
import net.minecraft.client.render.VertexConsumerProvider;
import net.minecraft.client.render.entity.EntityRendererFactory;
import net.minecraft.client.render.entity.ZombieEntityRenderer;
import net.minecraft.client.render.entity.model.EntityModelLayers;
import net.minecraft.client.util.math.MatrixStack;
import net.minecraft.item.ItemStack;
import net.minecraft.util.Identifier;
import net.minecraft.util.math.RotationAxis;
import software.bernie.geckolib.cache.object.BakedGeoModel;
import software.bernie.geckolib.model.GeoModel;
import software.bernie.geckolib.renderer.GeoRenderer;
import software.bernie.geckolib.renderer.GeoArmorRenderer;

public class TopHatZombieRenderer extends ZombieEntityRenderer {

    private final TopHatLayerRenderer hatLayer;

    public TopHatZombieRenderer(EntityRendererFactory.Context context) {
        super(context);
        this.hatLayer = new TopHatLayerRenderer(this, context);
        this.addFeature(hatLayer);
    }
}
