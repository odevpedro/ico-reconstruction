#ifndef ICO_ENTITY_TYPES_H
#define ICO_ENTITY_TYPES_H

enum entity_type {
    ENTITY_NONE      = 0,
    ENTITY_BOY       = 1,
    ENTITY_GIRL      = 2,
    ENTITY_ENEMY1    = 4,
    ENTITY_WOODBOX0  = 17,
    ENTITY_BARREL    = 19,
    ENTITY_ROPE      = 20,
    ENTITY_CHAIN     = 21,
    ENTITY_FLEVER    = 22,
    ENTITY_BGA       = 30,
    ENTITY_BIRD      = 32,
    ENTITY_QUEEN     = 46,
    ENTITY_DEVIL_GI  = 48,
    ENTITY_AP1       = 61,
    ENTITY_ATTACKCH  = 62,
    ENTITY_ATTACKCH2 = 63,
    ENTITY_BOSS_CTR  = 64,
    ENTITY_COUNT     = 68
};

enum phy_type {
    PHY_NONE     = 0x00,
    PHY_LEVER    = 0xF1,
    PHY_WOODBOX  = 0xF2,
    PHY_BARREL   = 0xF3
};

#endif
