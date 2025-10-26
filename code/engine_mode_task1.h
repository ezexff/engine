#include <string>
#include <xmllite.h>
#include <shlwapi.h>

//#include "httplib.h"
//#include <tls.h>
#include <curl/curl.h>

#define STB_IMAGE_IMPLEMENTATION  
#include "stb_image.h"

/* 
#include <ws2tcpip.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
 */


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
    v2d SimplifiedArray[1024];
    
    //v2 CameraP;
    
    u32 CoordinateArrayCount;
    v2d CoordinateArray[1024];
    
    b32 IsInitialized;
};