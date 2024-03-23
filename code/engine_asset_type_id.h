enum asset_tag_id
{
    //Tag_Smoothness,
    //Tag_Flatness,
    //Tag_FacingDirection, // NOTE(casey): Angle in radians off of due right
    Tag_Opacity,
    Tag_Face, // NOTE(ezexff): 0 - right, 1 - left, 2 - top, 3 - bottom, 4 - front, 5 - back
    
    Tag_Count,
};

enum asset_type_id
{
    Asset_None,
    
    //
    // NOTE(casey): Bitmaps!
    //
    
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
    
    //
    // NOTE(casey): Sounds!
    //
    
    Asset_Bloop,
    //Asset_Crack,
    //Asset_Drop,
    //Asset_Glide,
    Asset_Music,
    //Asset_Puhp,
    
    //
    //
    //
    
    Asset_Count,
};