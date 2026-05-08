package com.astrodir2.zombieapoc.item;

import net.minecraft.entity.effect.StatusEffectInstance;
import net.minecraft.entity.effect.StatusEffects;
import net.minecraft.entity.player.PlayerEntity;
import net.minecraft.item.Item;
import net.minecraft.item.ItemStack;
import net.minecraft.item.equipment.EquipmentType;
import net.minecraft.world.World;

public class TopHatItem extends Item {

    public TopHatItem(Settings settings) {
        super(settings);
    }

    @Override
    public void inventoryTick(ItemStack stack, World world, net.minecraft.entity.Entity entity, int slot, boolean selected) {
        if (!world.isClient && entity instanceof PlayerEntity player) {
            ItemStack helmet = player.getInventory().getArmorStack(3);
            if (helmet.getItem() == this) {
                if (!player.hasStatusEffect(StatusEffects.SPEED)) {
                    player.addStatusEffect(new StatusEffectInstance(StatusEffects.SPEED, 40, 0, false, false, true));
                }
                if (!player.hasStatusEffect(StatusEffects.REGENERATION)) {
                    player.addStatusEffect(new StatusEffectInstance(StatusEffects.REGENERATION, 40, 0, false, false, true));
                }
                if (!player.hasStatusEffect(StatusEffects.HEALTH_BOOST)) {
                    player.addStatusEffect(new StatusEffectInstance(StatusEffects.HEALTH_BOOST, 40, 1, false, false, true));
                }
            }
        }
    }

    @Override
    public EquipmentType getEquipmentType() {
        return EquipmentType.HELMET;
    }
}
