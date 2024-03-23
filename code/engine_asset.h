struct bitmap_id
{
    u32 Value;
};

struct sound_id
{
    u32 Value;
};

enum asset_state
{
    AssetState_Unloaded,
    AssetState_Queued,
    AssetState_Loaded,
    AssetState_StateMask = 0xFFF,
    
    AssetState_Lock = 0x10000,
};

struct asset_memory_header
{
    asset_memory_header *Next;
    asset_memory_header *Prev;
    
    u32 AssetIndex;
    u32 TotalSize;
    union
    {
        loaded_bitmap Bitmap;
        loaded_sound Sound;
    };
};

/*struct asset_slot
{
    asset_state State;
    union
    {
        loaded_bitmap *Bitmap;
        loaded_sound *Sound;
    };
};*/

struct asset
{
    u32 State;
    asset_memory_header *Header;
    
    eab_asset EAB;
    u32 FileIndex;
};

struct asset_type
{
    u32 FirstAssetIndex;
    u32 OnePastLastAssetIndex;
};

struct asset_file
{
    platform_file_handle Handle;
    
    // TODO(casey): If we ever do thread stacks, AssetTypeArray
    // doesn't actually need to be kept here probably.
    eab_header Header;
    eab_asset_type *AssetTypeArray;
    
    u32 TagBase;
};

enum asset_memory_block_flags
{
    AssetMemory_Used = 0x1,
};
struct asset_memory_block
{
    asset_memory_block *Prev;
    asset_memory_block *Next;
    u64 Flags;
    memory_index Size;
};

struct game_assets
{
    // TODO(casey): Not thrilled about this back-pointer
    struct tran_state *TranState;
    //memory_arena Arena;
    
    asset_memory_block MemorySentinel;
    asset_memory_header LoadedAssetSentinel;
    
    r32 TagRange[Tag_Count];
    
    /*u64 TargetMemoryUsed;
    u64 TotalMemoryUsed;
    asset_memory_header LoadedAssetSentinel;
    r32 TagRange[Tag_Count];*/
    
    u32 FileCount;
    asset_file *Files;
    
    u32 TagCount;
    eab_tag *Tags;
    
    u32 AssetCount;
    asset *Assets;
    
    asset_type AssetTypes[Asset_Count];
};

inline u32
GetState(asset *Asset)
{
    u32 Result = Asset->State & AssetState_StateMask;
    return(Result);
}

inline b32
IsLocked(asset *Asset)
{
    b32 Result = (Asset->State & AssetState_Lock);
    return(Result);
}

internal void MoveHeaderToFront(game_assets *Assets, asset *Asset);

inline loaded_bitmap *
GetBitmap(game_assets *Assets, bitmap_id ID, b32 MustBeLocked)
{
    Assert(ID.Value <= Assets->AssetCount);
    asset *Asset = Assets->Assets + ID.Value;
    
    loaded_bitmap *Result = 0;
    if(GetState(Asset) >= AssetState_Loaded)
    {
        Assert(!MustBeLocked || IsLocked(Asset));
        CompletePreviousReadsBeforeFutureReads;
        Result = &Asset->Header->Bitmap;
        MoveHeaderToFront(Assets, Asset);
    }    
    
    return(Result);
}

inline loaded_sound *GetSound(game_assets *Assets, sound_id ID)
{
    Assert(ID.Value <= Assets->AssetCount);
    asset *Asset = Assets->Assets + ID.Value;
    
    loaded_sound *Result = 0;
    if(GetState(Asset) >= AssetState_Loaded)
    {
        CompletePreviousReadsBeforeFutureReads;
        Result = &Asset->Header->Sound;
        MoveHeaderToFront(Assets, Asset);
    }
    
    return(Result);
}

inline eab_sound *
GetSoundInfo(game_assets *Assets, sound_id ID)
{
    Assert(ID.Value <= Assets->AssetCount);
    eab_sound *Result = &Assets->Assets[ID.Value].EAB.Sound;
    
    return(Result);
}

inline b32
IsValid(sound_id ID)
{
    b32 Result = (ID.Value != 0);
    
    return(Result);
}

internal void LoadSound(game_assets *Assets, sound_id ID);
inline void PrefetchSound(game_assets *Assets, sound_id ID) {LoadSound(Assets, ID);}

inline sound_id GetNextSoundInChain(game_assets *Assets, sound_id ID)
{
    sound_id Result = {};
    
    eab_sound *Info = GetSoundInfo(Assets, ID);
    switch(Info->Chain)
    {
        case EABSoundChain_None:
        {
            // NOTE(casey): Nothing to do.
        } break;
        
        case EABSoundChain_Loop:
        {
            Result = ID;
        } break;
        
        case EABSoundChain_Advance:
        {
            Result.Value = ID.Value + 1;
        } break;
        
        default:
        {
            InvalidCodePath;
        } break;
    }
    
    return(Result);
}