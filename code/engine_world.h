/*
    первый чанк мира (центр) в точке v2(max_int / 2, max_int /2)
    к центру мира можно добавить ещё 8 чанков (слева, справа, снизу, сверху и по диагоналям)
    определить размер чанка? быть может 1х1, чтобы float был в пределах единицы?

    256x256
    uint32 GroundBufferWidth = 256;
    uint32 GroundBufferHeight = 256;
    GameState->MetersToPixels = 42.0f;
    GameState->PixelsToMeters = 1.0f / GameState->MetersToPixels; //
    v3 WorldChunkDimInMeters = {GameState->PixelsToMeters*(real32)GroundBufferWidth,
                                    GameState->PixelsToMeters*(real32)GroundBufferHeight,
                                    GameState->TypicalFloorHeight};

    GameState->PixelsToMeters = 0,0238095238095238
    WorldChunkDimInMeters.x = 6,095238095238095
    WorldChunkDimInMeters.y = 6,095238095238095

    // Offset_ используем в физике и т.д., а после рассчётов физики (мувмента, коллизий и т.д.)
    находим новую world_position (если вышли за пределы текущего чанка)?


    структура entity:
        1. позиция
        2. набор флагов (обрабатывать коллизиии и т.д.)
        3. clip width & height

    тайлы для камеры?
*/

struct world_position
{
    // TODO(casey): It seems like we have to store ChunkX/Y/Z with each
    // entity because even though the sim region gather doesn't need it
    // at first, and we could get by without it, entity references pull
    // in entities WITHOUT going through their world_chunk, and thus
    // still need to know the ChunkX/Y/Z

    // 24 bits chunk in world and 8 bits - tile 256x256
    s32 ChunkX;
    s32 ChunkY;
    // s32 ChunkZ;

    // NOTE(casey): These are the offsets from the chunk center
    // v3 Offset_;
    v2 Offset_;
};

// TODO(casey): Could make this just tile_chunk and then allow multiple tile chunks per X/Y/Z
/*struct world_entity_block
{
    u32 EntityCount;
    u32 LowEntityIndex[16];
    world_entity_block *Next;
};*/

struct world_chunk
{
    s32 X;
    s32 Y;
    // s32 ChunkZ;

    // TODO(casey): Profile this and determine if a pointer would be better here!
    // world_entity_block FirstBlock;

    // world_chunk *NextInHash;
};

struct world
{
    s32 ChunkDimInMeters;

    u32 ChunksCount;
    world_chunk *Chunks;
    // v3 ChunkDimInMeters;

    // world_entity_block *FirstFree;

    // TODO(casey): WorldChunkHash should probably switch to pointers IF
    // tile entity blocks continue to be stored en masse directly in the tile chunk!
    // NOTE(casey): A the moment, this must be a power of two!
    // world_chunk ChunkHash[4096];
};

/*
inline void
RecanonicalizeCoord(world *World, s32 Chunk, r32 *ChunkRel)
{
    int32 Offset = FloorReal32ToInt32(*ChunkRel / World->ChunkDimInMeters);
    *Tile += Offset;
    *TileRel -= Offset*World->TileSideInMeters;

    Assert(*TileRel >= 0);
    // TODO(casey): Fix floating point math so this can be <
    Assert(*TileRel <= World->TileSideInMeters);

    if(*Tile < 0)
    {
        *Tile = TileCount + *Tile;
        --*TileMap;
    }

    if(*Tile >= TileCount)
    {
        *Tile = *Tile - TileCount;
        ++*TileMap;
    }
}

inline world_position
RecanonicalizePosition(world *World, world_position Pos)
{
    world_position Result = Pos;

    //RecanonicalizeCoord(World, World->CountX, &Result.TileMapX, &Result.TileX, &Result.TileRelX);
    //RecanonicalizeCoord(World, World->CountY, &Result.TileMapY, &Result.TileY, &Result.TileRelY);

    return(Result);
}
*/

void InitializeWorld(memory_arena *WorldArena, world *World, s32 WorldChunkDimInMeters, s32 ChunksRadius)
{
    World->ChunkDimInMeters = WorldChunkDimInMeters;

    /*World->Chunks[1].X = 1;

    World->Chunks[2].X = -1;

    World->Chunks[3].Y = 1;

    World->Chunks[4].Y = -1;

    World->Chunks[5].X = -1;
    World->Chunks[5].Y = 1;

    World->Chunks[6].X = 1;
    World->Chunks[6].Y = 1;

    World->Chunks[7].X = -1;
    World->Chunks[7].Y = -1;

    World->Chunks[8].X = 1;
    World->Chunks[8].Y = -1;*/

    for(s32 x = -ChunksRadius; x <= ChunksRadius; x++)
    {
        for(s32 y = -ChunksRadius; y <= ChunksRadius; y++)
        {
            World->ChunksCount++;
        }
    }

    World->Chunks = PushArray(WorldArena, World->ChunksCount, world_chunk);
    u32 TmpIndex = 0;
    for(s32 x = -ChunksRadius; x <= ChunksRadius; x++)
    {
        for(s32 y = -ChunksRadius; y <= ChunksRadius; y++)
        {
            World->Chunks[TmpIndex].X = x;
            World->Chunks[TmpIndex].Y = y;
            TmpIndex++;
        }
    }
}