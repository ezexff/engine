enum finalize_asset_operation
{
    FinalizeAsset_None,
    FinalizeAsset_Font,
};
struct load_asset_work
{
    task_with_memory *Task;
    asset *Asset;    
    
    platform_file_handle *Handle;
    u64 Offset;
    u64 Size;
    void *Destination;
    
    finalize_asset_operation FinalizeOperation;
    u32 FinalState;
};
internal void
LoadAssetWorkDirectly(load_asset_work *Work)
{
    Platform.ReadDataFromFile(Work->Handle, Work->Offset, Work->Size, Work->Destination);
    if(PlatformNoFileErrors(Work->Handle))
    {
        switch(Work->FinalizeOperation)
        {
            case FinalizeAsset_None:
            {
                // NOTE(casey): Nothing to do.
            } break;
            
            case FinalizeAsset_Font:
            {
                loaded_font *Font = &Work->Asset->Header->Font;
                eab_font *EAB = &Work->Asset->EAB.Font;
                for(u32 GlyphIndex = 1;
                    GlyphIndex < EAB->GlyphCount;
                    ++GlyphIndex)
                {
                    eab_font_glyph *Glyph = Font->Glyphs + GlyphIndex;
                    
                    Assert(Glyph->UnicodeCodePoint < EAB->OnePastHighestCodepoint);
                    Assert((u32)(u16)GlyphIndex == GlyphIndex);
                    Font->UnicodeMap[Glyph->UnicodeCodePoint] = (u16)GlyphIndex;
                }
            } break;
        }
    }
    
    CompletePreviousWritesBeforeFutureWrites;
    
    if(!PlatformNoFileErrors(Work->Handle))
    {
        ZeroSize(Work->Size, Work->Destination);
    }
    
    Work->Asset->State = Work->FinalState;
}
internal PLATFORM_WORK_QUEUE_CALLBACK(LoadAssetWork)
{
    load_asset_work *Work = (load_asset_work *)Data;
    
    LoadAssetWorkDirectly(Work);
    
    EndTaskWithMemory(Work->Task);
}

inline asset_file *
GetFile(game_assets *Assets, u32 FileIndex)
{
    Assert(FileIndex < Assets->FileCount);
    asset_file *Result = Assets->Files + FileIndex;
    
    return(Result);
}

inline platform_file_handle *
GetFileHandleFor(game_assets *Assets, u32 FileIndex)
{
    platform_file_handle *Result = &GetFile(Assets, FileIndex)->Handle;
    
    return(Result);
}

internal asset_memory_block *
InsertBlock(asset_memory_block *Prev, u64 Size, void *Memory)
{
    Assert(Size > sizeof(asset_memory_block));
    asset_memory_block *Block = (asset_memory_block *)Memory;
    Block->Flags = 0;
    Block->Size = Size - sizeof(asset_memory_block);
    Block->Prev = Prev;
    Block->Next = Prev->Next;
    Block->Prev->Next = Block;
    Block->Next->Prev = Block;
    return(Block);
}

internal asset_memory_block *
FindBlockForSize(game_assets *Assets, memory_index Size)
{
    asset_memory_block *Result = 0;
    
    // TODO(casey): This probably will need to be accelerated in the
    // future as the resident asset count grows.
    
    // TODO(casey): Best match block!
    for(asset_memory_block *Block = Assets->MemorySentinel.Next;
        Block != &Assets->MemorySentinel;
        Block = Block->Next)
    {
        if(!(Block->Flags & AssetMemory_Used))
        {
            if(Block->Size >= Size)
            {
                Result = Block;
                break;
            }
        }
    }
    
    return(Result);
}

internal b32
MergeIfPossible(game_assets *Assets, asset_memory_block *First, asset_memory_block *Second)
{
    b32 Result = false;
    
    if((First != &Assets->MemorySentinel) &&
       (Second != &Assets->MemorySentinel))
    {
        if(!(First->Flags & AssetMemory_Used) &&
           !(Second->Flags & AssetMemory_Used))
        {
            u8 *ExpectedSecond = (u8 *)First + sizeof(asset_memory_block) + First->Size;
            if((u8 *)Second == ExpectedSecond)
            {
                Second->Next->Prev = Second->Prev;
                Second->Prev->Next = Second->Next;
                
                First->Size += sizeof(asset_memory_block) + Second->Size;
                
                Result = true;
            }
        }
    }
    
    return(Result);
}

internal b32
GenerationHasCompleted(game_assets *Assets, u32 CheckID)
{
    b32 Result = true;
    
    for(u32 Index = 0;
        Index < Assets->InFlightGenerationCount;
        ++Index)
    {
        if(Assets->InFlightGenerations[Index] == CheckID)
        {
            Result = false;
            break;
        }
    }
    
    return(Result);
}

internal asset_memory_header *
AcquireAssetMemory(game_assets *Assets, u32 Size, u32 AssetIndex)
{
    asset_memory_header *Result = 0;
    
    BeginAssetLock(Assets);
    
    asset_memory_block *Block = FindBlockForSize(Assets, Size);
    for(;;)
    {
        if(Block && (Size <= Block->Size))
        {
            Block->Flags |= AssetMemory_Used;
            
            Result = (asset_memory_header *)(Block + 1);
            
            memory_index RemainingSize = Block->Size - Size;
            memory_index BlockSplitThreshold = 4096; // TODO(casey): Set this based on the smallest asset?
            if(RemainingSize > BlockSplitThreshold)
            {
                Block->Size -= RemainingSize;
                InsertBlock(Block, RemainingSize, (u8 *)Result + Size);
            }
            else
            {
                // TODO(casey): Actually record the unused portion of the memory
                // in a block so that we can do the merge on blocks when neighbors
                // are freed.
            }
            
            break;
        }
        else
        {
            for(asset_memory_header *Header = Assets->LoadedAssetSentinel.Prev;
                Header != &Assets->LoadedAssetSentinel;
                Header = Header->Prev)
            {
                asset *Asset = Assets->Assets + Header->AssetIndex;
                if((Asset->State >= AssetState_Loaded) &&
                   (GenerationHasCompleted(Assets, Asset->Header->GenerationID)))
                {
                    u32 AssetIndex1 = Header->AssetIndex;
                    asset *Asset = Assets->Assets + AssetIndex1;
                    
                    Assert(Asset->State == AssetState_Loaded);
                    
                    RemoveAssetHeaderFromList(Header);
                    
                    Block = (asset_memory_block *)Asset->Header - 1;
                    Block->Flags &= ~AssetMemory_Used;
                    
                    if(MergeIfPossible(Assets, Block->Prev, Block))
                    {
                        Block = Block->Prev;
                    }
                    
                    MergeIfPossible(Assets, Block, Block->Next);
                    
                    Asset->State = AssetState_Unloaded;
                    Asset->Header = 0;    
                    break;
                }
            }
        }
    }
    
    if(Result)
    {
        Result->AssetIndex = AssetIndex;
        Result->TotalSize = Size;
        InsertAssetHeaderAtFront(Assets, Result);
    }
    
    EndAssetLock(Assets);
    
    return(Result);
}

struct asset_memory_size
{
    u32 Total;
    u32 Data;
    u32 Section;
};

internal void
LoadBitmap(game_assets *Assets, bitmap_id ID, b32 Immediate)
{
    asset *Asset = Assets->Assets + ID.Value;        
    if(ID.Value)
    {
        if(AtomicCompareExchangeUInt32((u32 *)&Asset->State, AssetState_Queued, AssetState_Unloaded) ==
           AssetState_Unloaded)
        {
            task_with_memory *Task = 0;
            
            if(!Immediate)
            {
                Task = BeginTaskWithMemory(Assets->TranState);
            }
            
            if(Immediate || Task)        
            {
                asset *Asset = Assets->Assets + ID.Value;
                eab_bitmap *Info = &Asset->EAB.Bitmap;
                
                asset_memory_size Size = {};
                u32 Width = Info->Dim[0];
                u32 Height = Info->Dim[1];
                //Size.Section = 4*Width;
                Size.Section = Info->BytesPerPixel*Width;
                Size.Data = Height*Size.Section;
                Size.Total = Size.Data + sizeof(asset_memory_header);
                
                Asset->Header = AcquireAssetMemory(Assets, Size.Total, ID.Value);
                
                loaded_bitmap *Bitmap = &Asset->Header->Bitmap;            
                //Bitmap->AlignPercentage = V2(Info->AlignPercentage[0], Info->AlignPercentage[1]);
                //Bitmap->WidthOverHeight = (r32)Info->Dim[0] / (r32)Info->Dim[1];
                Bitmap->Width = Info->Dim[0];
                Bitmap->Height = Info->Dim[1];
                //Bitmap->Pitch = Size.Section;
                Bitmap->BytesPerPixel = Info->BytesPerPixel;
                Bitmap->Memory = (Asset->Header + 1);
                
                load_asset_work Work;
                Work.Task = Task;
                Work.Asset = Assets->Assets + ID.Value;
                Work.Handle = GetFileHandleFor(Assets, Asset->FileIndex);
                Work.Offset = Asset->EAB.DataOffset;
                Work.Size = Size.Data;
                Work.Destination = Bitmap->Memory;
                Work.FinalizeOperation = FinalizeAsset_None;
                Work.FinalState = AssetState_Loaded;            
                if(Task)
                {
                    load_asset_work *TaskWork = PushStruct(&Task->Arena, load_asset_work);
                    *TaskWork = Work;
                    Platform.AddEntry(Assets->TranState->LowPriorityQueue, LoadAssetWork, TaskWork);
                }
                else
                {
                    LoadAssetWorkDirectly(&Work);
                }
            }
            else
            {
                Asset->State = AssetState_Unloaded;
            }
        }
        else if(Immediate)
        {
            // TODO(casey): Do we want to have a more coherent story here
            // for what happens when two force-load people hit the load
            // at the same time?
            asset_state volatile *State = (asset_state volatile *)&Asset->State;
            while(*State == AssetState_Queued) {}
        }
    }    
}

internal void
LoadSound(game_assets *Assets, sound_id ID)
{
    asset *Asset = Assets->Assets + ID.Value;        
    if(ID.Value &&
       (AtomicCompareExchangeUInt32((u32 *)&Asset->State, AssetState_Queued, AssetState_Unloaded) ==
        AssetState_Unloaded))
    {    
        task_with_memory *Task = BeginTaskWithMemory(Assets->TranState);
        if(Task)        
        {
            asset *Asset = Assets->Assets + ID.Value;
            eab_sound *Info = &Asset->EAB.Sound;
            
            asset_memory_size Size = {};
            Size.Section = Info->SampleCount*sizeof(s16);
            Size.Data = Info->ChannelCount*Size.Section;
            Size.Total = Size.Data + sizeof(asset_memory_header);
            
            Asset->Header = (asset_memory_header *)AcquireAssetMemory(Assets, Size.Total, ID.Value);
            loaded_sound *Sound = &Asset->Header->Sound;
            
            Sound->SampleCount = Info->SampleCount;
            Sound->ChannelCount = Info->ChannelCount;
            u32 ChannelSize = Size.Section;
            
            void *Memory = (Asset->Header + 1);
            s16 *SoundAt = (s16 *)Memory;
            for(u32 ChannelIndex = 0;
                ChannelIndex < Sound->ChannelCount;
                ++ChannelIndex)
            {
                Sound->Samples[ChannelIndex] = SoundAt;
                SoundAt += ChannelSize;
            }
            
            load_asset_work *Work = PushStruct(&Task->Arena, load_asset_work);
            Work->Task = Task;
            Work->Asset = Assets->Assets + ID.Value;
            Work->Handle = GetFileHandleFor(Assets, Asset->FileIndex);
            Work->Offset = Asset->EAB.DataOffset;
            Work->Size = Size.Data;
            Work->Destination = Memory;
            Work->FinalizeOperation = FinalizeAsset_None;
            Work->FinalState = (AssetState_Loaded);
            
            Platform.AddEntry(Assets->TranState->LowPriorityQueue, LoadAssetWork, Work);
        }
        else
        {
            Assets->Assets[ID.Value].State = AssetState_Unloaded;
        }
    }
}

internal void
LoadFont(game_assets *Assets, font_id ID, b32 Immediate)
{
    // TODO(casey): Merge all this boilerplate!!!!  Same between LoadBitmap, LoadSound, and LoadFont
    asset *Asset = Assets->Assets + ID.Value;        
    if(ID.Value)
    {
        if(AtomicCompareExchangeUInt32((u32 *)&Asset->State, AssetState_Queued, AssetState_Unloaded) ==
           AssetState_Unloaded)
        {
            task_with_memory *Task = 0;
            
            if(!Immediate)
            {
                Task = BeginTaskWithMemory(Assets->TranState);
            }
            
            if(Immediate || Task)        
            {
                asset *Asset = Assets->Assets + ID.Value;
                eab_font *Info = &Asset->EAB.Font;
                
                /*u32 HorizontalAdvanceSize = sizeof(r32)*Info->GlyphCount*Info->GlyphCount;
                u32 GlyphsSize = Info->GlyphCount*sizeof(eab_font_glyph);
                u32 UnicodeMapSize = sizeof(u16)*Info->OnePastHighestCodepoint;*/
                //u32 SizeData = GlyphsSize + HorizontalAdvanceSize;
                //u32 SizeTotal = SizeData + sizeof(asset_memory_header) + UnicodeMapSize;
                
                u32 GlyphsSize = Info->GlyphCount * sizeof(eab_font_glyph);
                u32 AdvancesSize = Info->OnePastHighestCodepoint * sizeof(s32);
                u32 LSBsSize = Info->OnePastHighestCodepoint * sizeof(s32);
                u32 GlyphOffsetsSize = Info->OnePastHighestCodepoint * sizeof(v2);
                u32 KerningTableSize = Info->KerningTableLength * sizeof(kerning_entry);
                u32 UnicodeMapSize = sizeof(u16) * Info->OnePastHighestCodepoint;
                u32 SizeData = GlyphsSize + AdvancesSize + LSBsSize + GlyphOffsetsSize 
                    + KerningTableSize + UnicodeMapSize;
                u32 SizeTotal = SizeData + sizeof(asset_memory_header);
                
                Asset->Header = AcquireAssetMemory(Assets, SizeTotal, ID.Value);
                
                loaded_font *Font = &Asset->Header->Font;
                Font->BitmapIDOffset = GetFile(Assets, Asset->FileIndex)->FontBitmapIDOffset;
                Font->Glyphs = (eab_font_glyph *)(Asset->Header + 1);
                Font->Advances = (s32 *)((u8 *)Font->Glyphs + GlyphsSize);
                Font->LSBs = (s32 *)((u8 *)Font->Advances + AdvancesSize);
                Font->GlyphOffsets = (v2 *)((u8 *)Font->LSBs + LSBsSize);
                Font->KerningTable = (kerning_entry *)((u8 *)Font->GlyphOffsets + GlyphOffsetsSize);
                //Font->HorizontalAdvance = (r32 *)((u8 *)Font->Glyphs + GlyphsSize);
                Font->UnicodeMap = (u16 *)((u8 *)Font->KerningTable + KerningTableSize);
                ZeroSize(UnicodeMapSize, Font->UnicodeMap);
                
                load_asset_work Work;
                Work.Task = Task;
                Work.Asset = Assets->Assets + ID.Value;
                Work.Handle = GetFileHandleFor(Assets, Asset->FileIndex);
                Work.Offset = Asset->EAB.DataOffset;
                Work.Size = SizeData - UnicodeMapSize;
                Work.Destination = Font->Glyphs;
                Work.FinalizeOperation = FinalizeAsset_Font;
                Work.FinalState = AssetState_Loaded;            
                if(Task)
                {
                    load_asset_work *TaskWork = PushStruct(&Task->Arena, load_asset_work);
                    *TaskWork = Work;
                    Platform.AddEntry(Assets->TranState->LowPriorityQueue, LoadAssetWork, TaskWork);
                }
                else
                {
                    LoadAssetWorkDirectly(&Work);
                }
            }
            else
            {
                Asset->State = AssetState_Unloaded;
            }
        }
        else if(Immediate)
        {
            // TODO(casey): Do we want to have a more coherent story here
            // for what happens when two force-load people hit the load
            // at the same time?
            asset_state volatile *State = (asset_state volatile *)&Asset->State;
            while(*State == AssetState_Queued) {}
        }
    }    
}

internal u32
GetFirstAssetFrom(game_assets *Assets, asset_type_id TypeID)
{
    u32 Result = 0;
    
    asset_type *Type = Assets->AssetTypes + TypeID;
    if(Type->FirstAssetIndex != Type->OnePastLastAssetIndex)
    {
        Result = Type->FirstAssetIndex;
    }
    
    return(Result);
}

inline bitmap_id
GetFirstBitmapFrom(game_assets *Assets, asset_type_id TypeID)
{
    bitmap_id Result = {GetFirstAssetFrom(Assets, TypeID)};
    return(Result);
}

inline sound_id
GetFirstSoundFrom(game_assets *Assets, asset_type_id TypeID)
{
    sound_id Result = {GetFirstAssetFrom(Assets, TypeID)};
    return(Result);
}

inline font_id
GetFirstFontFrom(game_assets *Assets, asset_type_id TypeID)
{
    font_id Result = {GetFirstAssetFrom(Assets, TypeID)};
    return(Result);
}

inline u32
GetGlyphFromCodePoint(eab_font *Info, loaded_font *Font, u32 CodePoint)
{
    u32 Result = 0;
    if(CodePoint < Info->OnePastHighestCodepoint)
    {
        Result = Font->UnicodeMap[CodePoint];
        Assert(Result < Info->GlyphCount);
    }
    
    return(Result);
}

internal bitmap_id
GetBitmapForGlyph(game_assets *Assets, eab_font *Info, loaded_font *Font, u32 DesiredCodePoint)
{
    u32 Glyph = GetGlyphFromCodePoint(Info, Font, DesiredCodePoint);    
    bitmap_id Result = Font->Glyphs[Glyph].BitmapID;
    Result.Value += Font->BitmapIDOffset;
    
    return(Result);
}

internal r32
GetHorizontalAdvanceForPair(eab_font *Info, loaded_font *Font, u32 DesiredPrevCodePoint, u32 DesiredCodePoint)
{
    u32 PrevGlyph = GetGlyphFromCodePoint(Info, Font, DesiredPrevCodePoint);
    u32 Glyph = GetGlyphFromCodePoint(Info, Font, DesiredCodePoint);
    
    r32 Result = 0.0f;
    //r32 Result = Font->HorizontalAdvance[PrevGlyph*Info->GlyphCount + Glyph];
    //r32 Advance = 50.0f;
    r32 Advance = 0.0f;
    
    if((DesiredCodePoint != 0))
    {
        if(DesiredPrevCodePoint != 0)
        {
            /*for(s32 Index = 0;
                Index < Info->KerningTableLength;
                ++Index)
            {
                kerning_entry *Entry = Font->KerningTable + Index;
                if(Entry->Glyph1 == (s32)Glyph)
                {
                    for(s32 PrevIndex = 0;
                        PrevIndex < Info->KerningTableLength;
                        ++PrevIndex, ++Index)
                    {
                        if(Entry->Glyph2 == (s32)PrevGlyph)
                        {
                            Advance = (r32)Entry->Advance;
                            int tewasdad = 042432;
                        }
                    }
                }
            }*/
            
            //Advance = Font->Advances[];
            
        }
        //s32 PrevAdvance = Font->Advances[DesiredPrevCodePoint];
        //s32 Advance = Font->Advances[DesiredCodePoint];
        Result = (r32)Advance;
    }
    
    
    return(Result);
}

game_assets *
AllocateGameAssets(memory_arena *Arena, memory_index Size, tran_state *TranState)
{
    game_assets *Assets = PushStruct(Arena, game_assets);
    /*SubArena(&Assets->Arena, Arena, Size);
    Assets->TranState = TranState;
    */
    
    Assets->MemorySentinel.Flags = 0;
    Assets->MemorySentinel.Size = 0;
    Assets->MemorySentinel.Prev = &Assets->MemorySentinel;
    Assets->MemorySentinel.Next = &Assets->MemorySentinel;
    
    InsertBlock(&Assets->MemorySentinel, Size, PushSize(Arena, Size));
    
    Assets->TranState = TranState;
    
    Assets->LoadedAssetSentinel.Next = 
        Assets->LoadedAssetSentinel.Prev =
        &Assets->LoadedAssetSentinel;
    
    for(u32 TagType = 0;
        TagType < Tag_Count;
        ++TagType)
    {
        Assets->TagRange[TagType] = 1000000.0f;
    }
    
    
    Assets->TagCount = 1;
    Assets->AssetCount = 1;
    
    platform_file_group FileGroup = Platform.GetAllFilesOfTypeBegin(PlatformFileType_AssetFile);
    Assets->FileCount = FileGroup.FileCount;
    Assets->Files = PushArray(Arena, Assets->FileCount, asset_file);
    for(u32 FileIndex = 0;
        FileIndex < Assets->FileCount;
        ++FileIndex)
    {
        asset_file *File = Assets->Files + FileIndex;
        
        File->FontBitmapIDOffset = 0;
        File->TagBase = Assets->TagCount;
        
        ZeroStruct(File->Header);
        File->Handle = Platform.OpenNextFile(&FileGroup);
        Platform.ReadDataFromFile(&File->Handle, 0, sizeof(File->Header), &File->Header);
        
        u32 AssetTypeArraySize = File->Header.AssetTypeCount*sizeof(eab_asset_type);
        File->AssetTypeArray = (eab_asset_type *)PushSize(Arena, AssetTypeArraySize);
        Platform.ReadDataFromFile(&File->Handle, File->Header.AssetTypes,
                                  AssetTypeArraySize, File->AssetTypeArray);
        
        if(File->Header.MagicValue != EAB_MAGIC_VALUE)
        {
            Platform.FileError(&File->Handle, "EAB file has an invalid magic value.");
        }
        
        if(File->Header.Version > EAB_VERSION)
        {
            Platform.FileError(&File->Handle, "EAB file is of a later version.");
        }
        
        if(PlatformNoFileErrors(&File->Handle))
        {
            // NOTE(casey): The first asset and tag slot in every
            // EAB is a null (reserved) so we don't count it as
            // something we will need space for!
            Assets->TagCount += (File->Header.TagCount - 1);
            Assets->AssetCount += (File->Header.AssetCount - 1);
        }
        else
        {
            // TODO(casey): Eventually, have some way of notifying users of bogus files?
            InvalidCodePath;
        }
    }
    Platform.GetAllFilesOfTypeEnd(&FileGroup);
    
    // NOTE(casey): Allocate all metadata space
    Assets->Assets = PushArray(Arena, Assets->AssetCount, asset);
    Assets->Tags = PushArray(Arena, Assets->TagCount, eab_tag);
    
    // NOTE(casey): Reserve one null tag at the beginning
    ZeroStruct(Assets->Tags[0]);
    
    // NOTE(casey): Load tags
    for(u32 FileIndex = 0;
        FileIndex < Assets->FileCount;
        ++FileIndex)
    {
        asset_file *File = Assets->Files + FileIndex;
        if(PlatformNoFileErrors(&File->Handle))
        {
            // NOTE(casey): Skip the first tag, since it's null
            u32 TagArraySize = sizeof(eab_tag)*(File->Header.TagCount - 1);
            Platform.ReadDataFromFile(&File->Handle, File->Header.Tags + sizeof(eab_tag),
                                      TagArraySize, Assets->Tags + File->TagBase);
        }
    }
    
    // NOTE(casey): Reserve one null asset at the beginning
    u32 AssetCount = 0;
    ZeroStruct(*(Assets->Assets + AssetCount));
    ++AssetCount;
    
    // TODO(casey): Excersize for the reader - how would you do this in a way
    // that scaled gracefully to hundreds of asset pack files?  (or more!)
    for(u32 DestTypeID = 0;
        DestTypeID < Asset_Count;
        ++DestTypeID)
    {
        asset_type *DestType = Assets->AssetTypes + DestTypeID;
        DestType->FirstAssetIndex = AssetCount;
        
        for(u32 FileIndex = 0;
            FileIndex < Assets->FileCount;
            ++FileIndex)
        {
            asset_file *File = Assets->Files + FileIndex;
            if(PlatformNoFileErrors(&File->Handle))
            {
                for(u32 SourceIndex = 0;
                    SourceIndex < File->Header.AssetTypeCount;
                    ++SourceIndex)
                {
                    eab_asset_type *SourceType = File->AssetTypeArray + SourceIndex;
                    
                    if(SourceType->TypeID == DestTypeID)
                    {
                        if(SourceType->TypeID == Asset_FontGlyph)
                        {
                            File->FontBitmapIDOffset = AssetCount - SourceType->FirstAssetIndex;
                        }
                        
                        u32 AssetCountForType = (SourceType->OnePastLastAssetIndex -
                                                 SourceType->FirstAssetIndex);
                        
                        temporary_memory TempMem = BeginTemporaryMemory(&TranState->TranArena);
                        eab_asset *EABAssetArray = PushArray(&TranState->TranArena,
                                                             AssetCountForType, eab_asset);
                        Platform.ReadDataFromFile(&File->Handle,
                                                  File->Header.Assets +
                                                  SourceType->FirstAssetIndex*sizeof(eab_asset),
                                                  AssetCountForType*sizeof(eab_asset),
                                                  EABAssetArray);
                        for(u32 AssetIndex = 0;
                            AssetIndex < AssetCountForType;
                            ++AssetIndex)
                        {
                            eab_asset *EABAsset = EABAssetArray + AssetIndex;
                            
                            Assert(AssetCount < Assets->AssetCount);
                            asset *Asset = Assets->Assets + AssetCount++;
                            
                            Asset->FileIndex = FileIndex;
                            Asset->EAB = *EABAsset;
                            if(Asset->EAB.FirstTagIndex == 0)
                            {
                                Asset->EAB.FirstTagIndex = Asset->EAB.OnePastLastTagIndex = 0;
                            }
                            else
                            {
                                Asset->EAB.FirstTagIndex += (File->TagBase - 1);
                                Asset->EAB.OnePastLastTagIndex += (File->TagBase - 1);
                            }
                        }
                        
                        EndTemporaryMemory(TempMem);
                    }
                }
            }
        }
        
        DestType->OnePastLastAssetIndex = AssetCount;
    }
    
    return(Assets);
}

// TODO(ezexff): Think about how keep shaders on storage (asset file or smth else)
internal void
DEBUGLoadShaders(memory_arena *ConstArena, renderer_shaders *Shaders)
{
    platform_file_group FileGroup = Platform.GetAllFilesOfTypeBegin(PlatformFileType_VertFile);
    u32 FileCount = FileGroup.FileCount;
    platform_file_handle *FileHandles = PushArray(ConstArena, FileCount, platform_file_handle);
    for(u32 FileIndex = 0;
        FileIndex < FileCount;
        FileIndex++)
    {
        platform_file_handle *FileHandle = FileHandles + FileIndex;
        *FileHandle = Platform.OpenNextFile(&FileGroup);
        
        if(FileHandle->NoErrors)
        {
            if(FileHandle->Size < ArrayCount(Shaders->FrameVert.Text))
            {
                u8 *Text = 0;
                if(FileHandle->Name == L"frame.vert")
                {
                    Text = Shaders->FrameVert.Text;
                }
                else if(FileHandle->Name == L"skybox.vert")
                {
                    Text = Shaders->SkyboxVert.Text;
                }
                else if(FileHandle->Name == L"scene.vert")
                {
                    Text = Shaders->SceneVert.Text;
                }
                else if(FileHandle->Name == L"shadowmap.vert")
                {
                    Text = Shaders->ShadowMapVert.Text;
                }
                else if(FileHandle->Name == L"water.vert")
                {
                    Text = Shaders->WaterVert.Text;
                }
                else
                {
                    InvalidCodePath;
                }
                if(Text)
                {
                    Platform.ReadDataFromFile(FileHandle, 0, FileHandle->Size, Text);
#if ENGINE_INTERNAL
                    Log->Add("[asset] loaded text from %ls\n", FileHandle->Name.Data);
#endif
                }
                else
                {
                    InvalidCodePath;
                }
            }
            else
            {
                InvalidCodePath;
            }
        }
        else
        {
            InvalidCodePath;
        }
    }
    Platform.GetAllFilesOfTypeEnd(&FileGroup);
    
    FileGroup = Platform.GetAllFilesOfTypeBegin(PlatformFileType_FragFile);
    FileCount = FileGroup.FileCount;
    FileHandles = PushArray(ConstArena, FileCount, platform_file_handle);
    for(u32 FileIndex = 0;
        FileIndex < FileCount;
        FileIndex++)
    {
        platform_file_handle *FileHandle = FileHandles + FileIndex;
        *FileHandle = Platform.OpenNextFile(&FileGroup);
        
        if(FileHandle->NoErrors)
        {
            if(FileHandle->Size < ArrayCount(Shaders->FrameFrag.Text))
            {
                u8 *Text = 0;
                if(FileHandle->Name == L"frame.frag")
                {
                    Text = Shaders->FrameFrag.Text;
                }
                else if(FileHandle->Name == L"skybox.frag")
                {
                    Text = Shaders->SkyboxFrag.Text;
                }
                else if(FileHandle->Name == L"scene.frag")
                {
                    Text = Shaders->SceneFrag.Text;
                }
                else if(FileHandle->Name == L"shadowmap.frag")
                {
                    Text = Shaders->ShadowMapFrag.Text;
                }
                else if(FileHandle->Name == L"water.frag")
                {
                    Text = Shaders->WaterFrag.Text;
                }
                else
                {
                    InvalidCodePath;
                }
                if(Text)
                {
                    Platform.ReadDataFromFile(FileHandle, 0, FileHandle->Size, Text);
#if ENGINE_INTERNAL
                    Log->Add("[asset] loaded text from %ls\n", FileHandle->Name.Data);
#endif
                }
                else
                {
                    InvalidCodePath;
                }
            }
            else
            {
                InvalidCodePath;
            }
        }
        else
        {
            InvalidCodePath;
        }
    }
    Platform.GetAllFilesOfTypeEnd(&FileGroup);
}