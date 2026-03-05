/** Provides shared layout information regarding the Height uint */
#define MAX_VALUE 10
#define LAYOUT_SELECTION 31
#define LAYOUT_TEX0 (LAYOUT_SELECTION - 1)
#define LAYOUT_TEX1 (LAYOUT_TEX0 - 1)
#define LAYOUT_TEX2 (LAYOUT_TEX1 - 1)
#define LAYOUT_GRASS (LAYOUT_TEX2 - 1)
#define LAYOUT_FLOWER (LAYOUT_GRASS - 1)
#define LAYOUT_WATER (LAYOUT_FLOWER - 1)

// keep this the same as @TerrainManager::..
#define LAYOUT_HEIGHT_MASK 0xFFu
#define LAYOUT_HEIGHT_OFFSET 0u
#define LAYOUT_WATER_MASK 0xFF00u
#define LAYOUT_WATER_OFFSET 8u

#define DECORATION_TYPE_COUNT 2
#define DECORATION_CHANCE_GRASS 1
#define DECORATION_CHANCE_FLOWER 0.3
#define DECORATION_GRASS 1
#define DECORATION_FLOWER 2

/**
 * SSBO bindings, have to be the same as @TerrainSSBO!
 * Can start at 0 as this refers to bindings, not IDs!
 */
#define SSBO_LAYOUT_VERTICES 0
#define SSBO_LAYOUT_NORMALS 1
#define SSBO_LAYOUT_HEIGHTS 2
#define SSBO_LAYOUT_VERTEX_OFFSETS 3
#define SSBO_LAYOUT_VERTICAL_QUADS 4
#define SSBO_LAYOUT_VERTICAL_QUAD_LENGTHS 5
#define SSBO_LAYOUT_HORIZONTAL_QUADS 6
#define SSBO_LAYOUT_COUNT 7
#define SSBO_LAYOUT_POSITIONS 8


/** Which LAYOUT_X should we write to? */
#define TARGET_SELECTION 0
#define TARGET_TEX0 1
#define TARGET_TEX1 2
#define TARGET_TEX2 3
#define TARGET_GRASS 4
#define TARGET_FLOWER 5
#define TARGET_WATER 6