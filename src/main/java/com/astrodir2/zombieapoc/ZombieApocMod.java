package com.astrodir2.zombieapoc;

import com.astrodir2.zombieapoc.entity.ModEntities;
import com.astrodir2.zombieapoc.item.ModItems;
import net.fabricmc.api.ModInitializer;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class ZombieApocMod implements ModInitializer {
    public static final String MOD_ID = "zombieapoc";
    public static final Logger LOGGER = LoggerFactory.getLogger(MOD_ID);

    @Override
    public void onInitialize() {
        LOGGER.info("Zombie Apocalypse Mod loading...");
        ModItems.register();
        ModEntities.register();
        LOGGER.info("Zombie Apocalypse Mod loaded!");
    }
}
