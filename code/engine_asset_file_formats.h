// EAB - ENGINE ASSET BUILDER FILE
/*
     EAB FILE STRUCTURE
- Header
- Tags array
- AssetTypes array
- Assets array
- AssetSources array
	
struct name	 variable name	      struct size   start pos in file
----------------------------------------------------------------------------------------------------------
eab_header  	Header				 44			0
eab_tag		 Assets->Tags		   8			 prev + sizeof(Header)
eab_asset_type  Assets->AssetTypes	 12			prev + sizeof(tag) * Header.TagCount
eab_asset   	Assets->Assets	     44			prev + sizeof(eab_asset_type) * Header.AssetTypeCount
asset_source	Assets->AssetSources   24			prev + sizeof(eab_asset) * Header.AssetCount


*/

#define EAB_CODE(a, b, c, d) (((u32)(a) << 0) | ((u32)(b) << 8) | ((u32)(c) << 16) | ((u32)(d) << 24))

#pragma pack(push, 1)
struct eab_header
{
#define EAB_MAGIC_VALUE EAB_CODE('e','a','b','f')
    u32 MagicValue;
    
#define EAB_VERSION 0
    u32 Version;
    
    u32 TagCount;
    u32 AssetTypeCount;
    u32 AssetCount;
    
    u64 Tags; // eab_tag[TagCount]
    u64 AssetTypes; // eab_asset_type[AssetTypeCount]
    u64 Assets; // eab_asset[AssetCount]
};

struct eab_tag
{
    u32 ID;
    r32 Value;
};

struct eab_asset_type
{
    u32 TypeID;
    u32 FirstAssetIndex;
    u32 OnePastLastAssetIndex;
};

struct eab_bitmap
{
    u32 Dim[2];
    r32 AlignPercentage[2];
};
struct eab_sound
{
    u32 FirstSampleIndex;
    u32 SampleCount;
    u32 NextIDToPlay;
};
struct eab_asset
{
    u64 DataOffset;
    u32 FirstTagIndex;
    u32 OnePastLastTagIndex;
    union
    {
        eab_bitmap Bitmap;
        eab_sound Sound;
    };
};

#pragma pack(pop)
