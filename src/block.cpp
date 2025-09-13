// This file is generated automatically by generate.lua

#define NUM_OF_BLOCK 4

typedef enum {
	BLOCK_STONE,
	BLOCK_COBBLESTONE,
	BLOCK_DIRT,
	BLOCK_GRASS,
	BLOCK_AIR
} Block;

const char* block_to_string(Block b) {
	if (b == BLOCK_AIR) return "air";
	if (b == BLOCK_STONE) return "stone";
	if (b == BLOCK_COBBLESTONE) return "cobblestone";
	if (b == BLOCK_DIRT) return "dirt";
	if (b == BLOCK_GRASS) return "grass";
}

void update_stat_on_dig(Statistics* stats, Block b) {
	if (b == BLOCK_AIR) return;
	else if (b == BLOCK_STONE) stats->stat[COLLECT_COBBLESTONE]++;
	else if (b == BLOCK_COBBLESTONE) stats->stat[COLLECT_COBBLESTONE]++;
	else if (b == BLOCK_DIRT) stats->stat[COLLECT_DIRT]++;
	else if (b == BLOCK_GRASS) stats->stat[COLLECT_DIRT]++;
}

