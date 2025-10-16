#include <string>
#include <xmllite.h>
#include <shlwapi.h>

/* 
enum kml_tag
{
    KmlTag_Unknown,
    
    KmlTag_Placemark,
    KmlTag_Name,
    KmlTag_coordinates,
    
    KmlTag_Count,
};

struct xml_tag
{
    char *Name;
    char *Content;
    u32 ContentSize;
};
 */

struct mode_task1
{
    //char *XML;
    //r32 Scale;
    r32 Epsilon;
    
    //u32 TagArrayCount;
    //xml_tag TagArray[20];
    
    /* 
        u32 PointArrayCount;
        v2 *PointArray;
        v2 *PointArrayTmp;
     */
    
    u32 SimplifiedArrayCount;
    v2 SimplifiedArray[1024];
    
    //v2 CameraP;
    
    u32 CoordinateArrayCount;
    v2 CoordinateArray[1024];
    
    b32 IsInitialized;
};