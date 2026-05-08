package com.astrodir2.zombieapoc.entity;

import com.astrodir2.zombieapoc.item.ModItems;
import net.minecraft.entity.EntityType;
import net.minecraft.entity.LivingEntity;
import net.minecraft.entity.damage.DamageSource;
import net.minecraft.entity.mob.ZombieEntity;
import net.minecraft.entity.player.PlayerEntity;
import net.minecraft.server.world.ServerWorld;
import net.minecraft.util.math.BlockPos;
import net.minecraft.util.math.Vec3d;
import net.minecraft.world.World;

public class TopHatZombieEntity extends ZombieEntity {

    private int summonCooldown = 0;
    private int totalSummoned = 0;
    private static final int SUMMON_COOLDOWN = 20 * 60 * 3;
    private static final int MAX_SUMMONS = 5;
    private boolean wantsToSummon = false;

    public TopHatZombieEntity(EntityType<? extends ZombieEntity> entityType, World world) {
        super(entityType, world);
    }

    public static net.minecraft.entity.attribute.DefaultAttributeContainer.Builder createAttributes() {
        return ZombieEntity.createZombieAttributes()
                .add(net.minecraft.entity.attribute.EntityAttributes.MAX_HEALTH, 30.0)
                .add(net.minecraft.entity.attribute.EntityAttributes.MOVEMENT_SPEED, 0.26f)
                .add(net.minecraft.entity.attribute.EntityAttributes.ATTACK_DAMAGE, 4.0);
    }

    @Override
    public void tick() {
        super.tick();
        if (!this.getWorld().isClient && this.isAlive()) {
            if (summonCooldown > 0) summonCooldown--;
            if (summonCooldown == 0 && totalSummoned < MAX_SUMMONS) {
                if (this.getRandom().nextInt(100) < 30) {
                    wantsToSummon = true;
                }
            }
            if (wantsToSummon && summonCooldown == 0) {
                PlayerEntity target = this.getWorld().getClosestPlayer(this, 32);
                if (target != null && isPlayerLookingAway(target)) {
                    summonZombieHorde();
                    wantsToSummon = false;
                    summonCooldown = SUMMON_COOLDOWN;
                }
            }
        }
    }

    private boolean isPlayerLookingAway(PlayerEntity player) {
        Vec3d playerLook = player.getRotationVec(1.0f);
        Vec3d toZombie = this.getPos().subtract(player.getPos()).normalize();
        double dot = playerLook.dotProduct(toZombie);
        return dot < -0.3;
    }

    private void summonZombieHorde() {
        if (!(this.getWorld() instanceof ServerWorld serverWorld)) return;
        int toSummon = Math.min(this.getRandom().nextInt(3) + 1, MAX_SUMMONS - totalSummoned);
        for (int i = 0; i < toSummon; i++) {
            ZombieEntity zombie = EntityType.ZOMBIE.create(serverWorld);
            if (zombie == null) continue;
            double offsetX = (this.getRandom().nextDouble() - 0.5) * 6;
            double offsetZ = (this.getRandom().nextDouble() - 0.5) * 6;
            BlockPos spawnPos = this.getBlockPos().add((int)offsetX, 0, (int)offsetZ);
            zombie.refreshPositionAndAngles(spawnPos.getX(), spawnPos.getY(), spawnPos.getZ(), 0, 0);
            if (this.getTarget() != null) {
                zombie.setTarget((LivingEntity) this.getTarget());
            }
            serverWorld.spawnEntity(zombie);
            totalSummoned++;
        }
    }

    @Override
    public void onDeath(DamageSource damageSource) {
        super.onDeath(damageSource);
        if (!this.getWorld().isClient) {
            this.getWorld().createExplosion(this, this.getX(), this.getY(), this.getZ(), 2.0f, false, World.ExplosionSourceType.NONE);
        }
    }

    @Override
    protected void dropLoot(ServerWorld world, DamageSource damageSource, boolean causedByPlayer) {
        super.dropLoot(world, damageSource, causedByPlayer);
        this.dropItem(world, ModItems.TOP_HAT);
    }
}
