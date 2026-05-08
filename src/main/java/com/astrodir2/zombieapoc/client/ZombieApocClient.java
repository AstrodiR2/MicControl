package com.astrodir2.zombieapoc.client;

import com.astrodir2.zombieapoc.entity.ModEntities;
import com.astrodir2.zombieapoc.client.renderer.TopHatZombieRenderer;
import net.fabricmc.api.ClientModInitializer;
import net.fabricmc.fabric.api.client.rendering.v1.EntityRendererRegistry;

public class ZombieApocClient implements ClientModInitializer {
    @Override
    public void onInitializeClient() {
        EntityRendererRegistry.register(ModEntities.TOP_HAT_ZOMBIE, TopHatZombieRenderer::new);
    }
}
