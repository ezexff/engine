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

struct bitmap_id
{
    u32 Value;
};

struct sound_id
{
    u32 Value;
};

struct font_id
{
    u32 Value;
};

enum asset_font_type
{
    FontType_Default = 0,
    FontType_Debug = 10,
};

enum asset_tag_id
{
    //Tag_Smoothness,
    //Tag_Flatness,
    //Tag_FacingDirection, // NOTE(casey): Angle in radians off of due right
    Tag_Opacity,
    Tag_Face, // NOTE(ezexff): 0 - right, 1 - left, 2 - top, 3 - bottom, 4 - front, 5 - back
    Tag_UnicodeCodepoint,
    Tag_FontType, // NOTE(casey): 0 - Default Game Font, 10 - Debug Font?
    
    Tag_Count,
};

enum asset_type_id
{
    Asset_None,
    
    //~ NOTE(ezexff): Bitmap!
    //Asset_Shadow,
    //Asset_Tree,
    //Asset_Sword,
    //    Asset_Stairwell,
    //Asset_Rock,
    
    Asset_Grass,
    Asset_Tuft,
    Asset_Stone,
    
    //Asset_Head,
    //Asset_Cape,
    //Asset_Torso,
    Asset_Clip,
    Asset_Ground,
    Asset_Skybox,
    Asset_Terrain,
    Asset_DuDvMap,
    Asset_NormalMap,
    
    //~ NOTE(ezexff): Sounds
    Asset_Bloop,
    //Asset_Crack,
    //Asset_Drop,
    //Asset_Glide,
    Asset_Music,
    //Asset_Puhp,
    
    //~ NOTE(ezexff): Fonts
    
    Asset_Font,
    Asset_FontGlyph,
    
    //~
    Asset_Count,
};

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
    u32 BytesPerPixel;
};
enum hha_sound_chain
{
    EABSoundChain_None,
    EABSoundChain_Loop,
    EABSoundChain_Advance,
};
struct eab_sound
{
    u32 SampleCount;
    u32 ChannelCount;
    u32 Chain; // NOTE(casey): hha_sound_chain
};
struct eab_font_glyph
{
    u32 UnicodeCodePoint;
    bitmap_id BitmapID;
};
struct eab_font
{
    u32 OnePastHighestCodepoint;
    u32 GlyphCount;
    /*r32 AscenderHeight;
    r32 DescenderHeight;    
    r32 ExternalLeading;*/
    r32 Scale; // NOTE(ezexff): scale = pixels / (ascent - descent)
    s32 Ascent, Descent, LineGap;
    s32 KerningTableLength;
    
    /* NOTE(casey): Data is:

       hha_font_glyph CodePoints[GlyphCount];
       r32 HorizontalAdvance[GlyphCount][GlyphCount];
    */
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
        eab_font Font;
    };
};

#pragma pack(pop)
