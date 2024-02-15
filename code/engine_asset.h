#pragma pack(push, 1)
struct WAVE_header
{
    u32 RIFFID;
    u32 Size;
    u32 WAVEID;
};

#define RIFF_CODE(a, b, c, d) (((u32)(a) << 0) | ((u32)(b) << 8) | ((u32)(c) << 16) | ((u32)(d) << 24))
enum
{
    WAVE_ChunkID_fmt = RIFF_CODE('f', 'm', 't', ' '),
    WAVE_ChunkID_data = RIFF_CODE('d', 'a', 't', 'a'),
    WAVE_ChunkID_RIFF = RIFF_CODE('R', 'I', 'F', 'F'),
    WAVE_ChunkID_WAVE = RIFF_CODE('W', 'A', 'V', 'E'),
};
struct WAVE_chunk
{
    u32 ID;
    u32 Size;
};

struct WAVE_fmt
{
    u16 wFormatTag;
    u16 nChannels;
    u32 nSamplesPerSec;
    u32 nAvgBytesPerSec;
    u16 nBlockAlign;
    u16 wBitsPerSample;
    u16 cbSize;
    u16 wValidBitsPerSample;
    u32 dwChannelMask;
    u8 SubFormat[16];
};
#pragma pack(pop)

struct riff_iterator
{
    u8 *At;
    u8 *Stop;
};

inline riff_iterator
ParseChunkAt(void *At, void *Stop)
{
    riff_iterator Iter;
    
    Iter.At = (u8 *)At;
    Iter.Stop = (u8 *)Stop;
    
    return(Iter);
}

inline riff_iterator
NextChunk(riff_iterator Iter)
{
    WAVE_chunk *Chunk = (WAVE_chunk *)Iter.At;
    u32 Size = (Chunk->Size + 1) & ~1;
    Iter.At += sizeof(WAVE_chunk) + Size;
    
    return(Iter);
}

inline b32
IsValid(riff_iterator Iter)
{    
    b32 Result = (Iter.At < Iter.Stop);
    
    return(Result);
}

inline void *
GetChunkData(riff_iterator Iter)
{
    void *Result = (Iter.At + sizeof(WAVE_chunk));
    
    return(Result);
}

inline u32
GetType(riff_iterator Iter)
{
    WAVE_chunk *Chunk = (WAVE_chunk *)Iter.At;
    u32 Result = Chunk->ID;
    
    return(Result);
}

inline u32
GetChunkDataSize(riff_iterator Iter)
{
    WAVE_chunk *Chunk = (WAVE_chunk *)Iter.At;
    u32 Result = Chunk->Size;
    
    return(Result);
}

loaded_sound
LoadFirstWAV(memory_arena *GameArena, platform_api *Platform)
{
    loaded_sound Result = {};
    u32 FileSize = 0;
    
    platform_file_group *FileGroup = Platform->GetAllFilesOfTypeBegin("wav");
    u32 FileCount = FileGroup->FileCount;
    FileCount = 1;
    //platform_file_handle **FileHandles = (platform_file_handle **)malloc(FileCount * sizeof(platform_file_handle *));
    platform_file_handle *FileHandles = PushArray(GameArena, FileCount, platform_file_handle);
    for(u32 FileIndex = 0;
        FileIndex < FileCount;
        FileIndex++)
    {
        platform_file_handle *FileHandle = FileHandles + FileIndex;
        FileHandle = Platform->OpenNextFile(FileGroup);
        //FileHandles[FileIndex] = Platform->OpenNextFile(FileGroup);
        
        if(FileHandle)
        {
            WAVE_header TmpHeader;
            Platform->ReadDataFromFile(FileHandle, 0, sizeof(WAVE_header), &TmpHeader);
            FileSize = TmpHeader.Size;
            
            //u8 *ReadContents = (u8 *)malloc(FileSize);
            u8 *ReadContents = (u8 *)PushSize(GameArena, FileSize);
            Platform->ReadDataFromFile(FileHandle, 0, FileSize, ReadContents);
            
            Result.Free = ReadContents;
            
            u32 SectionFirstSampleIndex = 0;
            u32 SectionSampleCount = 0;
            
            WAVE_header *Header = (WAVE_header *)ReadContents;
            Assert(Header->RIFFID == WAVE_ChunkID_RIFF);
            Assert(Header->WAVEID == WAVE_ChunkID_WAVE);
            
            u32 ChannelCount = 0;
            u32 SampleDataSize = 0;
            s16 *SampleData = 0;
            for(riff_iterator Iter = ParseChunkAt(Header + 1, (u8 *)(Header + 1) + Header->Size - 4);
                IsValid(Iter);
                Iter = NextChunk(Iter))
            {
                switch(GetType(Iter))
                {
                    case WAVE_ChunkID_fmt:
                    {
                        WAVE_fmt *fmt = (WAVE_fmt *)GetChunkData(Iter);
                        Assert(fmt->wFormatTag == 1); // NOTE(casey): Only support PCM
                        Assert(fmt->nSamplesPerSec == 48000);
                        Assert(fmt->wBitsPerSample == 16);
                        Assert(fmt->nBlockAlign == (sizeof(s16)*fmt->nChannels));
                        ChannelCount = fmt->nChannels;
                    } break;
                    
                    case WAVE_ChunkID_data:
                    {
                        SampleData = (s16 *)GetChunkData(Iter);
                        SampleDataSize = GetChunkDataSize(Iter);
                    } break;
                }
            }
            
            Assert(ChannelCount && SampleData);
            
            Result.ChannelCount = ChannelCount;
            u32 SampleCount = SampleDataSize / (ChannelCount*sizeof(s16));
            if(ChannelCount == 1)
            {
                Result.Samples[0] = SampleData;
                Result.Samples[1] = 0;
            }
            else if(ChannelCount == 2)
            {
                Result.Samples[0] = SampleData;
                Result.Samples[1] = SampleData + SampleCount;
                
#if 0
                for(u32 SampleIndex = 0;
                    SampleIndex < SampleCount;
                    ++SampleIndex)
                {
                    SampleData[2*SampleIndex + 0] = (s16)SampleIndex;
                    SampleData[2*SampleIndex + 1] = (s16)SampleIndex;
                }
#endif
                
                for(u32 SampleIndex = 0;
                    SampleIndex < SampleCount;
                    ++SampleIndex)
                {
                    s16 Source = SampleData[2*SampleIndex];
                    SampleData[2*SampleIndex] = SampleData[SampleIndex];
                    SampleData[SampleIndex] = Source;
                }
            }
            else
            {
                Assert(!"Invalid channel count in WAV file");
            }
            
            // TODO(casey): Load right channels!
            b32 AtEnd = true;
            Result.ChannelCount = 1;
            if(SectionSampleCount)
            {
                Assert((SectionFirstSampleIndex + SectionSampleCount) <= SampleCount);
                AtEnd = ((SectionFirstSampleIndex + SectionSampleCount) == SampleCount);
                SampleCount = SectionSampleCount;
                for(u32 ChannelIndex = 0;
                    ChannelIndex < Result.ChannelCount;
                    ++ChannelIndex)
                {
                    Result.Samples[ChannelIndex] += SectionFirstSampleIndex;
                }
            }
            
            if(AtEnd)
            {
                for(u32 ChannelIndex = 0;
                    ChannelIndex < Result.ChannelCount;
                    ++ChannelIndex)
                {
                    for(u32 SampleIndex = SampleCount;
                        SampleIndex < (SampleCount + 8);
                        ++SampleIndex)
                    {
                        Result.Samples[ChannelIndex][SampleIndex] = 0;
                    }
                }
            }
            
            Result.SampleCount = SampleCount;
        }
    }
    
    return(Result);
}