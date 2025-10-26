internal1 bool DisableButton = false;
internal1 bool BeginDisabledButton = false;
internal1 r64 GlobalMaxDistance = 0.0f;
internal1 v2d GlobalMaxDistanceArray[2] = {};
internal1 loaded_bitmap GlobalTileBitmapArray[9] = {};
internal1 rectangle2 GlobalTileRect = {};
internal1 v2 GlobalFirstP = {};
internal1 v2d GlobalTileOffset = {};

struct mercator_pyramid
{
    s32 TileSize;
    r64 InitialResolution;
    r64 OriginShift;
};

internal1 mercator_pyramid MercatorPyramid = {};

internal1 void
InitMercatorPyramid(mercator_pyramid *MercatorPyramid1, s32 TileSize)
{
    MercatorPyramid1->TileSize = TileSize;
    MercatorPyramid1->InitialResolution = 2.0f * Pi32 * 6378137.0f / TileSize;
    MercatorPyramid1->OriginShift = 2.0f * Pi32 * 6378137.0f / 2.0f;
}

internal1 r64
Resolution(r32 Zoom)
{
    // Resolution (meters/pixel) for given zoom level (measured at Equator)
    r64 Result = MercatorPyramid.InitialResolution / pow(2, Zoom);
    return(Result);
}

internal1 v2d
LatLonToMeters(r64 lat, r64 lon)
{
    // Converts given lat/lon in WGS84 Datum to XY in Spherical Mercator EPSG:900913
    v2d Result = {};
    Result.x = lon * MercatorPyramid.OriginShift / 180.0f;
    Result.y = log(tan((90.0f + lat) * Pi32 / 360.0f)) / (Pi32 / 180.0f);
    
    Result.y = Result.y * MercatorPyramid.OriginShift / 180.0f;
    return(Result);
}

internal1 v2d
MetersToPixels(r64 mx, r64 my, r32 zoom)
{
    // Converts EPSG:900913 to pyramid pixel coordinates in given zoom level
    r64 res = Resolution(zoom);
    v2d Result = {};
    Result.x = (mx + MercatorPyramid.OriginShift) / res;
    Result.y = (my + MercatorPyramid.OriginShift) / res;
    return(Result);
}

internal1 v2s
PixelsToTile(v2d Pixels)
{
    //Returns a tile covering region in given pixel coordinates
    v2s Result = {};
    
    Result.x = (s32)(ceil(Pixels.x / (r64)MercatorPyramid.TileSize) - 1);
    Result.y = (s32)(ceil(Pixels.y / (r64)MercatorPyramid.TileSize) - 1);
    
    return(Result);
}

internal1 v2s
MetersToTile(r64 mx, r64 my, r32 zoom)
{
    // Returns tile for given mercator coordinates
    v2s Result = {};
    
    v2d Pixels = MetersToPixels(mx, my, zoom);
    Result = PixelsToTile(Pixels);
    return(Result);
}

internal1 v2
PixelsToMeters(r32 px, r32 py, r32 zoom)
{
    // Converts pixel coordinates in given zoom level of pyramid to EPSG:900913
    v2 Result = {};
    
    r64 res = Resolution(zoom);
    Result.x = px * (r32)res; // - (r32)MercatorPyramid.OriginShift;
    Result.y = py * (r32)res; // - (r32)MercatorPyramid.OriginShift;
    return(Result);
}

internal1 void
PrintLastError()
{
    int ErrorCode = WSAGetLastError();
    LPSTR ErrorString = 0;
    int Size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                             0,
                             ErrorCode,
                             MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                             (LPSTR)&ErrorString, 0, 0);
    printf("\n\n\nError code %d. Message(%d): %s", ErrorCode, Size, ErrorString);
    //printf("\n\n\nError code %d. Message(%d): %s", ErrorCode, Size, ErrorString);
    LocalFree(ErrorString);
}

/* 
void handle_tls_error(tls *ctx, const std::string& message)
{
//std::cerr << message << ": " << tls_error(ctx) << std::endl;
Log->Add("%s : %s", message.c_str(), tls_error(ctx));
tls_free(ctx);
ExitProcess(EXIT_FAILURE);
}
*/

v2s LatLngToCoords(r64 Zoom, r64 Lat, r64 Lon)
{
    v2s Result = {};
    r64 n = pow(2, Zoom);
    r64 LatRad = Lat * (Pi32 / 180.0f);
    r64 X = n * ((Lon + 180) / 360);
    r64 Y = 0.5f * n * (1 - (log(tan(LatRad) + (1 / cos(LatRad))) / Pi32));
    Result.x = (s32)floor(X);
    Result.y = (s32)floor(Y);
    
    GlobalTileOffset.x = X - floor(X);
    GlobalTileOffset.y = 1.0f - (Y - floor(Y));
    
    return(Result);
}

// Callback function to write received data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output)
{
    output->append((char*)contents, size * nmemb);
    return size * nmemb;
}

internal1 void
TestRequest3(renderer_frame *Frame, long double X, long double Y, r32 Zoom, r64 X2, r64 Y2)
{
#if 1
    v2s Tile = LatLngToCoords(Zoom, (r64)Y, (r64)X);
    Log->Add("GoogleTile = %d %d\n", Tile.x, Tile.y);
    
    v2d Meters1 = LatLonToMeters((r64)Y, (r64)X);
    v2s Tile1 = MetersToTile(Meters1.x, Meters1.y, Zoom);
    Log->Add("TmsTile = %d %d\n", Tile1.x, Tile1.y);
    
    
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    
    curl = curl_easy_init();
    if(curl)
    {
#if GEOAPIFY
        std::string Request = "https://maps.geoapify.com/v1/tile/carto/11/1283/671.png?&apiKey=e836c30db6244e57966e3f11521b889d";
#else
        std::string Request = "https://api.maptiler.com/tiles/satellite-v2/";
        
        Request += std::to_string((s32)floor(Zoom));
        Request += "/";
        Request += std::to_string(Tile.x);
        Request += "/";
        Request += std::to_string(Tile.y);
        Request += "?key=0319xaMoopMsP55uY26S";
#endif
        
        curl_easy_setopt(curl, CURLOPT_URL, Request.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        
        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
        {
            //std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            Log->Add("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        else
        {
            //std::cout << "Received: " << readBuffer << std::endl;
            Log->Add("Received: %s\n", readBuffer.c_str());
            
            // NOTE(ezexff): load png
            stbi_set_flip_vertically_on_load(1);
            const char *png_data = readBuffer.c_str();
            int data_size = (int)readBuffer.length();
            int width, height, channels;
            void *pixels = stbi_load_from_memory((unsigned char *)png_data, data_size, &width, &height, &channels, 0);
            GlobalTileBitmapArray[0].Width = width;
            GlobalTileBitmapArray[0].Height = height;
            GlobalTileBitmapArray[0].BytesPerPixel = channels;
            GlobalTileBitmapArray[0].Memory = pixels;
        }
        curl_easy_cleanup(curl);
    }
#endif
}

internal1 void
TestRequest2()
{
#if 0
    // 1. Initialize LibreSSL
    if(tls_init() == -1)
    {
        //std::cerr << "tls_init failed" << std::endl;
        //return EXIT_FAILURE;
        Log->Add("tls_init failed");
        ExitProcess(0);
    }
    
    // 2. Create a client context
    tls *ctx = nullptr;
    if((ctx = tls_client()) == nullptr)
    {
        //std::cerr << "tls_client failed" << std::endl;
        //return EXIT_FAILURE;
        Log->Add("tls_client failed");
        ExitProcess(0);
    }
    
    // 3. Configure the context (optional, but good practice for security)
    // For example, to set certificate verification:
    // tls_config_insecure_no_verify_cert(tls_get_config(ctx)); // WARNING: Disables cert verification!
    // For production, you would typically load trusted CA certificates.
    
    // 4. Connect to the server
    const char *hostname = "example.com";
    const char *port = "443"; // HTTPS default port
    tls *client_ctx = nullptr;
    
    //if(tls_connect_socket(ctx, -1, hostname, port) == -1)
    if(tls_connect_socket(ctx, -1, hostname) == -1)
    {
        handle_tls_error(ctx, "tls_connect failed");
    }
    
    // 5. Perform the TLS handshake
    if(tls_handshake(ctx) == -1)
    {
        handle_tls_error(ctx, "tls_handshake failed");
    }
    
    // 6. Construct the HTTP GET request
    std::string request = "GET / HTTP/1.1\r\n";
    request += "Host: " + std::string(hostname) + "\r\n";
    request += "Connection: close\r\n"; // Close connection after response
    request += "\r\n"; // End of headers
    
    // 7. Send the request
    ssize_t sent_bytes = tls_write(ctx, request.data(), request.size());
    if(sent_bytes == -1)
    {
        handle_tls_error(ctx, "tls_write failed");
    }
    if(static_cast<size_t>(sent_bytes) != request.size())
    {
        //std::cerr << "Warning: Not all data sent" << std::endl;
        Log->Add("Warning: Not all data sent");
    }
    
    // 8. Read the response
    char Buffer[4096];
    ssize_t received_bytes;
    while((received_bytes = tls_read(ctx, Buffer, sizeof(Buffer))) > 0)
    {
        //std::cout.write(buffer.data(), received_bytes);
        Log->Add("%s\n", Buffer);
    }
    
    if(received_bytes == -1)
    {
        // Check for expected end-of-file condition
        if(tls_error(ctx) != nullptr && std::string(tls_error(ctx)) == "handshake failed: closed")
        {
            // This can happen when the server closes the connection cleanly
            //std::cout << "\nServer closed connection cleanly." << std::endl;
            Log->Add("\nServer closed connection cleanly.");
        }
        else
        {
            handle_tls_error(ctx, "tls_read failed");
        }
    }
    
    // 9. Clean up
    tls_close(ctx);
    tls_free(ctx);
#endif
}

internal1 void
TestRequest1()
{
    /* 
    httplib::Client cli("https://api.maptiler.com");
    
    if(auto res = cli.Get("/maps/satellite/?key=QIjbJMNE3luox1FRle1Y"))
    {
        Log->Add("Status: %s\n", res->status);
        Log->Add("Body: %s\n", res->body);
    }
    else
    {
        Log->Add("Error: %s\n", cli.error());
    }
 */
    
#if 0
    const char* version_str = OpenSSL_version(OPENSSL_VERSION);
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    
    SOCKET Sock;
    s32 Result = 0;
    
    WSAData lpWSAData;
    WORD DLLVersion = MAKEWORD(2, 2);
    Result = WSAStartup(DLLVersion, &lpWSAData);
    if(Result != 0)
    {
        PrintLastError();
        WSACleanup();
        ExitProcess(0);
    }
    
    Sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(Sock == INVALID_SOCKET)
    {
        PrintLastError();
        WSACleanup();
        ExitProcess(0);
    }
    
    // Set the socket to non-blocking mode
    /* 
    u_long iMode = 1; // 1 for non-blocking, 0 for blocking
    if(ioctlsocket(Sock, FIONBIO, &iMode) != 0)
    {
        PrintLastError();
        WSACleanup();
        ExitProcess(0);
    }
 */
    
    
#if 0
    SOCKADDR_IN Addr;
    int sizeofaddr = sizeof(Addr);
    Addr.sin_addr.s_addr = inet_addr("104.17.245.40");
    Addr.sin_port = htons(443);
    Addr.sin_family = AF_INET;
    
    Result = connect(Sock, (SOCKADDR*)&Addr, sizeof(Addr));
    if(Result == SOCKET_ERROR)
    {
        PrintLastError();
        WSACleanup();
        ExitProcess(0);
    }
    
#else
    addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    //hints.ai_protocol = IPPROTO_TCP;
    //getaddrinfo("google.com", "443", &hints, &res); // Use port 443 for HTTPS
    //getaddrinfo("example.com", "443", &hints, &res); // Use port 443 for HTTPS
    getaddrinfo("maptiler.com", "443", &hints, &res); // Use port 443 for HTTPS
    //getaddrinfo("maps.geoapify.com", "443", &hints, &res); // Use port 443 for HTTPS
    connect(Sock, res->ai_addr, (int)res->ai_addrlen);
    freeaddrinfo(res);
#endif
    
    SSL_CTX* ssl_ctx = SSL_CTX_new(TLS_client_method());
    if(!ssl_ctx)
    {
        ERR_print_errors_fp(stderr);
        ExitProcess(0);
    }
    
    //SSL_CTX_set_min_proto_version(ssl_ctx, TLS1_2_VERSION);
    //SSL_CTX_set_max_proto_version(ssl_ctx, TLS1_2_VERSION);
    
    SSL* ssl = SSL_new(ssl_ctx);
    if(!ssl)
    {
        ERR_print_errors_fp(stderr);
        ExitProcess(0);
    }
    SSL_set_fd(ssl, (int)Sock);
    if(SSL_connect(ssl) <= 0)
    {
        ERR_print_errors_fp(stderr);
        ExitProcess(0);
    }
    
    const char* version_str1 = SSL_get_version(ssl);
    
#if 0
    // NOTE(ezexff): send message
    //std::string request = "GET /maps/satellite/?key=QIjbJMNE3luox1FRle1Y#1/0/0 HTTP/1.1\r\n";
    //request += "Host: api.maptiler.com\r\n";
    //std::string request = "GET / HTTP/1.1\r\n";
    //request += "Host: maptiler.com\r\n";
    std::string request = "GET / HTTP/1.1\r\n";
    request += "Host: www.google.com\r\n";
    //request += "Host: example.com\r\n";
    request += "User-Agent: curl/8.16.0\r\n";
    request += "Accept: */*\r\n\r\n";
    
    SSL_write(ssl, request.c_str(), (int)request.length());
    char Buffer[4096];
    int bytesRead = SSL_read(ssl, Buffer, sizeof(Buffer) - 1);
    if(bytesRead > 0)
    {
        Buffer[bytesRead] = '\0';
        Log->Add("Response: %s\n", Buffer);
    }
#else
    std::string request = "GET /resources/logo.svg HTTP/1.1\r\n";
    request += "Host: api.maptiler.com\r\n";
    //std::string request = "GET /v1/tile/carto/1/1/1.png?&apiKey=cda6cb9607da4e98b1ad9b86d015156c HTTP/1.1\r\n";
    //request += "Host: maps.geoapify.com\r\n";
    //request += "User-Agent: curl/8.16.0\r\n";
    request += "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/141.0.0.0 Safari/537.36\r\n";
    //request += "Cookie: \r\n";
    request += "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7\r\n\r\n";
    SSL_write(ssl, request.c_str(), (int)request.length());
    char Buffer2[4096];
    int bytesRead = SSL_read(ssl, Buffer2, sizeof(Buffer2) - 1);
    if(bytesRead > 0)
    {
        Buffer2[bytesRead] = '\0';
        Log->Add("Response: %s\n", Buffer2);
    }
#endif
    
    
    
    
    //Result = send(Connection, request, sizeof(request), 0);
    /* 
    if(Result == SOCKET_ERROR)
    {
        PrintLastError();
        WSACleanup();
        ExitProcess(0);
    }
    else
    {
        char Response[256] = "";
        //printf("[thread] tick\n");
        s32 Result = 0;
        Result = recv(Connection, Response, sizeof(Response), 0);
        if(Result > 0)
        {
            printf(Response);
        }
        else
        {
            int ErrorCode = WSAGetLastError();
            switch(ErrorCode)
            {
                case WSAECONNRESET:
                {
                    printf("[thread] Lost connection to server\n");
                } break;
                
                default:
                {
                    PrintLastError();
                } break;
            }
        }
    }
    Sleep(1000);
 */
    
    SSL_shutdown(ssl);
    SSL_free(ssl);
    SSL_CTX_free(ssl_ctx);
    closesocket(Sock);
    WSACleanup();
#endif
}

inline r64
RadToDeg64(r64 Value)
{
    r64 Result = Value * 180.0f / Pi32;
    return(Result);
}

inline r64
DegToRad64(r64 Value)
{
    r64 Result = Value * Pi32 / 180.0f;
    
    return(Result);
}

inline r64
ForceAngle0To360(r64 Angle)
{
    r64 Result = fmod(Angle, 360.0);
    if(Angle < 0)
    {
        Result += 360.0;
    }
    return(Result);
}

inline s32
GetDegZone(r64 Lng)
{
    r64 LngDeg;
    s32 Result = 0;
    if(abs(Lng) < 10000)
    {
        LngDeg = ForceAngle0To360(Lng);
        Result = (s32)trunc((6.0f + LngDeg) / 6);
    }
    
    return(Result);
}

b32 KeepLastXYZone = false;

inline v2d
LatLngToXY(r64 CoorLat, r64 CoorLng)
{
    v2d Result = {};
    
    s32 LastXYZone = 0;
    
    s32 n, Buf_n;
    r64 l, l2;
    r64 sb2, sb4, sb6;
    r64 LngDeg;
    r64 SinLat, CosLat;
    
    LngDeg = RadToDeg64(CoorLng);
    LngDeg = ForceAngle0To360(LngDeg);
    
    Buf_n = GetDegZone(LngDeg);
    
    if(KeepLastXYZone && (((abs(Buf_n - LastXYZone) < 2) || 
                           ((Buf_n == 60) && (LastXYZone == 1)) || 
                           ((Buf_n == 1) && (LastXYZone == 60)))))
    {
        n = LastXYZone;
        if((Buf_n == 60) && (LastXYZone == 1)) {LngDeg = LngDeg - 360;}
        if((Buf_n == 1) && (LastXYZone == 60)) {LngDeg = 360 + LngDeg;}
    }
    else
    {
        n = Buf_n;
        LastXYZone = n;
    }
    
    SinLat = sin(CoorLat);
    CosLat = cos(CoorLat);
    sb2 = SinLat*SinLat;
    sb4 = sb2*sb2;
    sb6 = sb2*sb4;
    
    l = DegToRad64(LngDeg - (3 + 6*(n-1)));
    l2 = l*l;
    
    double Y = 6367558.4668*CoorLat - sin(2*CoorLat)*(16002.89 + 66.9607*sb2 + 0.3515*sb4 - l2*(1594561.25 + 5336.535*sb2 + 26.79*sb4 + 0.149*sb6 + l2*(672483.4 - 811219.9*sb2 + 5420.0*sb4 - 10.6*sb6 + l2*(278194.0 - 830174.0*sb2 + 572434.0*sb4 - 16010.0*sb6 + l2*(109500.0 - 574700.0*sb2 + 863700.0*sb4 - 398600.0*sb6)))));
    
    double X = (5.0 + 10*n)*1e5 + l*CosLat*(6378245.0 + 21346.1415*sb2 + 107.1590*sb4 + 0.5977*sb6 + l2*(1070204.16 - 2136826.66*sb2 + 17.988*sb4 - 11.99*sb6 + l2*(270806.0 - 1523417.0*sb2 + 1327645.0*sb4 - 21701.0*sb6 + l2*(79690.0 - 866190.0*sb2 + 1730360.0*sb4 - 945460.0*sb6))));
    KeepLastXYZone = true;
    Result = {X, Y};
    return(Result);
}

inline v2d
TestPerp(r64 x1, r64 y1, r64 x2, r64 y2, r64 x3, r64 y3)
{
    v2d Result = {};
    Result.x = (((x2-x1)*(y2-y1)*(y3-y1)+x1*pow(y2-y1, 2)+x3*pow(x2-x1, 2))/(pow(y2-y1, 2)+pow(x2-x1, 2)));
    Result.y = (y2-y1)*(Result.x-x1)/(x2-x1)+y1;
    return(Result);
}

inline b32
IsNumber(wchar_t C)
{
    b32 Result = ((C >= L'0') && (C <= L'9'));
    
    return(Result);
}

inline void
CheckHResult(HRESULT hr, char* message)
{
    if(FAILED(hr))
    {
        //wprintf(L"Error: %s (HRESULT: 0x%08X)\n", message, hr);
        Log->Add("Error: %s (HRESULT: 0x%08X)\n", message, hr);
    }
}

internal1 void
ParseKML(renderer_frame *Frame, renderer *Renderer, mode_task1 *ModeTask1, char *FileName)
{
    HRESULT hr = S_OK;
    IXmlReader* pReader = NULL;
    IStream* pFileStream = NULL;
    
    // Create a file stream for the XML file
    hr = SHCreateStreamOnFile(FileName, STGM_READ, &pFileStream);
    CheckHResult(hr, "Failed to create file stream.");
    
    // Create the XmlReader
    hr = CreateXmlReader(__uuidof(IXmlReader), (void**)&pReader, NULL);
    CheckHResult(hr, "Failed to create XmlReader.");
    
    // Set the input stream for the reader
    hr = pReader->SetInput(pFileStream);
    CheckHResult(hr, "Failed to set input stream.");
    
    XmlNodeType nodeType;
    const WCHAR* pwszLocalName = NULL;
    const WCHAR* pwszValue = NULL;
    
    b32 NameFound = false;
    // Read through the XML nodes
    while(S_OK == (hr = pReader->Read(&nodeType)))
    {
        switch(nodeType)
        {
            //~
            case XmlNodeType_XmlDeclaration:
            {
                Log->Add("XmlDeclaration\n");
            } break;
            
            //~
            case XmlNodeType_Element:
            {
                hr = pReader->GetLocalName(&pwszLocalName, NULL);
                CheckHResult(hr, "Failed to get local name for element.");
                //Log->Add("%sElement: %s\n", SpaceBuffer, wideCharToUtf8(pwszLocalName).c_str());
            } break;
            
            //~
            case XmlNodeType_Text:
            {
                UINT cwchValue = 0;
                hr = pReader->GetValue(&pwszValue, &cwchValue);
                CheckHResult(hr, "Failed to get value for text node.");
                //Log->Add("%sText: %s\n", SpaceBuffer, wideCharToUtf8(pwszValue).c_str());
                
                if((wcscmp(pwszLocalName, L"name") == 0) && (wcscmp(pwszValue, L"58:15:0491406:109") == 0))
                {
                    NameFound = true;
                    Log->Add("Found name\n");
                }
                else if(NameFound && (wcscmp(pwszLocalName, L"coordinates") == 0))
                {
                    Log->Add("Found coordinates by name\n");
                    u32 CoordinatesCount = 0;
                    wchar_t *EndCursor = (wchar_t *)pwszValue;
                    while(*EndCursor)
                    {
                        if(IsNumber(*EndCursor))
                        {
                            long double X = wcstold(EndCursor, &EndCursor);
                            ++EndCursor; // skip L','
                            long double Y = wcstold(EndCursor, &EndCursor);
                            ++EndCursor; // skip L','
                            long double Z = wcstold(EndCursor, &EndCursor);
                            ++EndCursor; // skip L' '
                            //Log->Add("%lf %lf\n", X, Y);
                            
                            // TODO(ezexff): mb start use double? r32 for testing (and fast calc)
                            //ModeTask1->CoordinateArray[CoordinatesCount] = LatLngToXY(DegToRad64((r64)Y), DegToRad64((r64)X));
                            ModeTask1->CoordinateArray[CoordinatesCount] = LatLonToMeters((r64)Y, (r64)X);
                            if(CoordinatesCount == 0)
                            {
                                r32 Zoom = 11.0f;
                                TestRequest3(Frame, X, Y, Zoom,
                                             ModeTask1->CoordinateArray[CoordinatesCount].x, ModeTask1->CoordinateArray[CoordinatesCount].y);
                                Renderer->Camera.P = V3((r32)ModeTask1->CoordinateArray[CoordinatesCount].x, 
                                                        (r32)ModeTask1->CoordinateArray[CoordinatesCount].y, 
                                                        Zoom);
                                GlobalFirstP = Renderer->Camera.P.xy;
                                v2 P = {};
                                //v2 Dim = {(r32)TestTileBitmap.Width, (r32)TestTileBitmap.Height};
                                //Dim *= 25;
                                v2 Dim = PixelsToMeters(TILE_SIZE, TILE_SIZE, Zoom);
                                //v2 HalfDim = V2(Dim.x / 2.0f, Dim.y / 2.0f);
                                //P.x = (r32)ModeTask1->CoordinateArray[CoordinatesCount].x;
                                //P.y = (r32)ModeTask1->CoordinateArray[CoordinatesCount].y;
                                v2d Meters1 = LatLonToMeters(GlobalTileOffset.y, GlobalTileOffset.x);
                                v2 TileOffsetInMeters = {(r32)Meters1.x / 2048, (r32)Meters1.y / 2048};
                                P.x = (r32)ModeTask1->CoordinateArray[CoordinatesCount].x - TileOffsetInMeters.x;
                                P.y = (r32)ModeTask1->CoordinateArray[CoordinatesCount].y - TileOffsetInMeters.y;
                                P.x = (r32)ModeTask1->CoordinateArray[CoordinatesCount].x - (r32)GlobalTileOffset.x * Dim.x;
                                P.y = (r32)ModeTask1->CoordinateArray[CoordinatesCount].y - (r32)GlobalTileOffset.y * Dim.y;
                                //GlobalTileRect = {P - HalfDim, P + HalfDim};
                                GlobalTileRect = {P, P + Dim};
                            }
                            Log->Add("%lf %lf\n",
                                     ModeTask1->CoordinateArray[CoordinatesCount].x, ModeTask1->CoordinateArray[CoordinatesCount].y);
                            
                            CoordinatesCount++;
                            Assert(CoordinatesCount < ArrayCount(ModeTask1->CoordinateArray));
                        }
                        else
                        {
                            ++EndCursor; // skip unknown symbol
                        }
                    };
                    ModeTask1->CoordinateArrayCount = CoordinatesCount;
                    Log->Add("Read %u cordinates\n", CoordinatesCount);
                    goto end_parse;
                }
            } break;
            
            //~
            case XmlNodeType_EndElement:
            {
                hr = pReader->GetLocalName(&pwszLocalName, NULL);
                CheckHResult(hr, "Failed to get local name for end element.");
                //Log->Add("%sEnd Element: %s\n", SpaceBuffer, wideCharToUtf8(pwszLocalName).c_str());
                
            } break;
            
            default:{} break;
        }
    }
    
    end_parse:
    
    if(hr != S_FALSE) // S_FALSE indicates end of document
    {
        CheckHResult(hr, "Error during XML reading.");
    }
    
    //wprintf(L"Finished reading XML.\n");
    Log->Add("Finished reading XML.\n");
    
    // Clean up
    if(pReader) pReader->Release();
    if(pFileStream) pFileStream->Release();
}

std::string wideCharToUtf8(const std::wstring& wstr)
{
    if(wstr.empty()) return std::string();
    
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if(size_needed == 0)
    {
        // Handle error, e.g., GetLastError()
        return std::string();
    }
    
    std::string utf8_str(size_needed - 1, 0); // -1 to exclude null terminator from size
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &utf8_str[0], size_needed, nullptr, nullptr);
    return utf8_str;
}

/* 
inline b32
IsAlpha(char C)
{
bool Result = (((C >= 'a') && (C <= 'z')) ||
   ((C >= 'A') && (C <= 'Z')));

return(Result);
}

inline char *
GetTag(char *Start)
{
char *At = Start;
while(At[0])
{
if((At == Start) && (At[1] == '/'))
{
// process close tag
++At;
}
else if(At == Start)
{
// process open tag
// identifier  && (IsAlpha(*At))
++At;

}
else if(*At == '>')
{
break;
}
else
{
InvalidCodePath;
}
++At;
}

return(At);
}

u32 ProcessOpen(char *At, char *Open, u32 OpenLen)
{
while(*At)
{
if(*At == '>')
{
// tag ended
OpenLen++;
Open[OpenLen] = 0;
return(OpenLen);
}
if(!IsAlpha(*At))
{
OpenLen = 0;
break;
}
Open[OpenLen] = *At;
OpenLen++;

++At;
}

return(0);
}
*/

inline r64
PerpendicularDistance(v2d p, v2d start, v2d end)
{
    r64 dx = end.x - start.x;
    r64 dy = end.y - start.y;
    r64 t = ((p.x - start.x) * dx + (p.y - start.y) * dy) / (dx * dx + dy * dy);
    
    if (t < 0) t = 0;
    else if (t > 1) t = 1;
    
    r64 closestX = start.x + t * dx;
    r64 closestY = start.y + t * dy;
    
    
    r64 Result = sqrt(Square(p.x - closestX) + Square(p.y - closestY));
    return(Result);
}

internal1 void
DouglasPeucker(v2d *PointArray, u32 PointArrayCount,
               r64 Epsilon, s32 Start, s32 End,
               v2d *ResultArray, u32 *ResultArrayCount)
{
    r64 MaxDistance = 0.0f;
    s32 FarthestIndex = -1;
    for(s32 Index = Start + 1;
        Index < End;
        ++Index)
    {
        r64 PerpDistance = PerpendicularDistance(PointArray[Index], PointArray[Start], PointArray[End]);
        if(PerpDistance > MaxDistance)
        {
            MaxDistance = PerpDistance;
            FarthestIndex = Index;
        }
    }
    
    if(MaxDistance > Epsilon)
    {
        DouglasPeucker(PointArray, PointArrayCount,
                       Epsilon, Start, FarthestIndex,
                       ResultArray, ResultArrayCount);
        DouglasPeucker(PointArray, PointArrayCount,
                       Epsilon, FarthestIndex, End,
                       ResultArray, ResultArrayCount);
    }
    else
    {
        if(GlobalMaxDistance < MaxDistance)
        {
            GlobalMaxDistance = MaxDistance;
            GlobalMaxDistanceArray[0] = TestPerp(PointArray[Start].x, PointArray[Start].y, 
                                                 PointArray[End].x, PointArray[End].y,
                                                 PointArray[FarthestIndex].x, PointArray[FarthestIndex].y);
            GlobalMaxDistanceArray[1] = PointArray[FarthestIndex];
        }
        if(*ResultArrayCount == 0)
        {
            ResultArray[*ResultArrayCount] = PointArray[Start];
            ++*ResultArrayCount;
        }
        ResultArray[*ResultArrayCount] = PointArray[End];
        ++*ResultArrayCount;
    }
    
    //GlobalMaxDistance = Maximum(GlobalMaxDistance, MaxDistance);
    //r32 MaxDistanceInMeters = MaxDistance * 111111.0f;
    //Log->Add("MaxDistance = %f, %fmeters\n", MaxDistance, MaxDistanceInMeters);
}

struct douglas_peucker_work
{
    task_with_memory *Task;
    
    mode_task1 *ModeTask1;
};
internal1 PLATFORM_WORK_QUEUE_CALLBACK(DouglasPeuckerWork)
{
    douglas_peucker_work *Work = (douglas_peucker_work *)Data;
    
#if ENGINE_INTERNAL
#if ENGINE_IMGUI
    //ParseKML(ModeTask1, FileName);
#endif
#endif
    
    mode_task1 *ModeTask1 = Work->ModeTask1;
    if(ModeTask1)
    {
        for(u32 Index = 0;
            Index < 5;
            ++Index)
        {
            Log->Add("[enginework] DouglasPeuckerWork tick %u of 5\n", Index + 1);
            Sleep(1000);
        }
        
        DouglasPeucker(ModeTask1->CoordinateArray, ModeTask1->CoordinateArrayCount,
                       ModeTask1->Epsilon, 0, ModeTask1->CoordinateArrayCount - 1,
                       ModeTask1->SimplifiedArray, &ModeTask1->SimplifiedArrayCount);
        Log->Add("[enginework] DouglasPeuckerWork completed\n");
        DisableButton = false;
    }
    else
    {
        InvalidCodePath;
    }
    
    EndTaskWithMemory(Work->Task);
}

internal1 void
UpdateAndRenderTask1(game_memory *Memory, game_input *Input)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    tran_state *TranState = (tran_state *)Memory->TransientStorage;
    
    memory_arena *ConstArena = &GameState->ConstArena;
    memory_arena *TranArena = &TranState->TranArena;
    
    renderer_frame *Frame = &Memory->Frame;
    renderer *Renderer = (renderer *)Frame->Renderer;
    
    mode_task1 *ModeTask1 = &GameState->ModeTask1;
    if(!ModeTask1->IsInitialized)
    {
        //TestRequest1();
        //TestRequest2();
        //TestRequest3();
        //ModeTask1->Scale = 1.0f;
        // TODO(ezexff): only for testing
        InitMercatorPyramid(&MercatorPyramid, TILE_SIZE);
        Renderer->Camera.P = V3(0.0f, 0.0f, 0.0f);
        ModeTask1->Epsilon = 0.00001f;
        /* 
    ModeTask1->XML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><kml xmlns=\"http://earth.google.com/kml/2.1\"><Placemark><name>Геленджик</name><description><![CDATA[<p>Геленджик, Краснодарский край, Россия.</p>Город располагается по&amp;nbsp;берегам Геленджикской бухты, но&amp;nbsp;не&amp;nbsp;равномерно (восточный берег исторически более населён).]]></description><LookAt id=\"khLookAt540_copy0\"><longitude>38.0576198113139</longitude><latitude>44.56963150481845</latitude><altitude>0</altitude><range>14693.40972993507</range><tilt>49.10268313434742</tilt><heading>37.85562764777833</heading></LookAt><Style><IconStyle><scale>0.9</scale><Icon><href>root://icons/palette-4.png</href><x>32</x><y>128</y><w>32</w><h>32</h></Icon></IconStyle><LabelStyle><scale>0.9</scale></LabelStyle></Style><Point id=\"khPoint541_copy0\"><coordinates>38.06284424434902,44.56842733252498,0</coordinates></Point></Placemark></kml>";
*/
        /* 
    ModeTask1->XML = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><kml xmlns=\"http://www.opengis.net/kml/2.2\"><Document><Placemark><name>New York City</name><description>New York City</description><Point><coordinates>-74.006393,40.714172,0</coordinates></Point></Placemark></Document></kml>";
*/
        
        /* 
        char *At = ModeTask1->XML;
        while(*At)
        {
            u32 OpenLen = 0;
            char Open[256] = {};
            u32 CloseLen = 0;
            char Close[256] = {};
            
            // Ищем начало тега
            if(*At == '<')
            {
                ++At;
                if(*At == '\\')
                {
                    // mb close tag
                    ProcessClose(At);
                    continue;
                }
                else if(IsAlpha(*At))
                {
                    // mb open tag
                    if(ProcessOpen(At, Open, OpenLen))
                    {
                        Log->Add("Open: %s\n", Open);
                    }
                    continue;
                }
                Log->Add("Skip: %c\n", *At);
            }
            else
            {
                Log->Add("Skip: %c\n", *At);
            }
            
            ++At;
        }
 */
        //TestParseXML(ModeTask1->XML);
        /* 
        ++At;
        
        GetIdentifier
        
        // Если идентификатор, то продолжаем разбор
        if(IsAlpha(At[1]))
           {
               
        }
        else
        {
            
        }
                At = GetTag(At);
*/
        
        // NOTE(ezexff): Test
        /* 
        ModeTask1->PointArrayCount = 5;
        ModeTask1->PointArray = PushArray(&GameState->ConstArena, ModeTask1->PointArrayCount, v2);
        ModeTask1->PointArrayTmp = PushArray(&GameState->ConstArena, ModeTask1->PointArrayCount, v2);
        ModeTask1->SimplifiedArray = PushArray(&GameState->ConstArena, 1000, v2);
        
        random_series Series = RandomSeed(300);
        for(u32 Index = 0;
            Index < ModeTask1->PointArrayCount;
            ++Index)
        {
            ModeTask1->PointArray[Index].x = 500 + (r32)Index * 100;
            ModeTask1->PointArray[Index].y = 500 + RandomUnilateral(&Series) * 100;
        }
 */
        
        ModeTask1->IsInitialized = true;
    }
    
    // NOTE(ezexff): inputs
    if(IsDown(Input->MouseButtons[PlatformMouseButton_Left]))
    {
        if(Input->dMouseP.x != 0)
        {
            //Renderer->Camera.P.x -= Input->dMouseP.x / Renderer->Camera.P.z / 750.0f / Frame->AspectRatio;
            Renderer->Camera.P.x -= Input->dMouseP.x * Frame->AspectRatio * 100.0f * Renderer->Camera.P.z / TILE_SIZE;
        }
        if(Input->dMouseP.y != 0)
        {
            Renderer->Camera.P.y -= Input->dMouseP.y * Frame->AspectRatio * 100.0f * Renderer->Camera.P.z / TILE_SIZE; 
        }
    }
    /*
    if(WasPressed(Input->MouseButtons[PlatformMouseButton_Left]))
    {
        ModeTask1->SimplifiedArrayCount = 0;
        DouglasPeucker(ModeTask1->PointArray, &ModeTask1->PointArrayCount,
                       ModeTask1->Epsilon, 0, ModeTask1->PointArrayCount - 1,
                       ModeTask1->SimplifiedArray, &ModeTask1->SimplifiedArrayCount);
    }
    
    if(WasPressed(Input->MouseButtons[PlatformMouseButton_Right]))
    {
        ModeTask1->SimplifiedArrayCount = 0;
    }
 */
    
    if(UI_State->Input->dMouseP.z != 0)
    {
        Renderer->Camera.P.z += UI_State->Input->dMouseP.z;
        Renderer->Camera.P.z = Clamp(1.0f, Renderer->Camera.P.z, 1000.0f);
        //ModeTask1->Scale += Input->dMouseP.z;
        //ModeTask1->Scale = Clamp(1, ModeTask1->Scale, 10);
        /* 
        ModeTask1->CameraP.x = Input->MouseP.x;
        ModeTask1->CameraP.y = Frame->Dim.y - Input->MouseP.y;
 */
        /* 
        v2 Center = V2(Frame->Dim.x / 2.0f, Frame->Dim.y / 2.0f);
        ModeTask1->CameraP = ModeTask1->CameraP + Center - V2(Input->MouseP.x, Input->MouseP.y);
 */
    }
    
    // NOTE(ezexff): draw
    /* 
    v4 OutlineColor = V4(1, 0, 0, 1);
    r32 CircleScale = 50;
    v2 Pos = V2(Input->MouseP.x, Input->MouseP.y);
    PushCircleOutlineOnScreen(&Renderer->PushBufferPhysics, Pos, CircleScale, 2, OutlineColor, 10001);
 */
    
    // NOTE(ezexff): points in camera space
    /* 
    for(u32 Index = 0;
        Index < ModeTask1->PointArrayCount;
        ++Index)
    {
        ModeTask1->PointArrayTmp[Index] = ModeTask1->PointArray[Index] * ModeTask1->Scale + ModeTask1->CameraP;
    }
 */
    
    /* 
    PushLinesOnScreen(&Renderer->PushBufferPhysics, ModeTask1->PointArrayCount, ModeTask1->PointArrayTmp, 3, V4(0, 0, 0, 1), 10000);
    
    PushLinesOnScreen(&Renderer->PushBufferPhysics, ModeTask1->SimplifiedArrayCount, ModeTask1->SimplifiedArray, 3, V4(0, 1, 0, 1), 10000);
 */
    
    if(ModeTask1->CoordinateArrayCount)
    {
        PushLinesOnScreen64(&Renderer->PushBufferPhysics, ModeTask1->CoordinateArrayCount, ModeTask1->CoordinateArray, 1, V4(0, 0, 0, 1), 10000);
    }
    if(ModeTask1->SimplifiedArrayCount)
    {
        PushLinesOnScreen64(&Renderer->PushBufferPhysics, ModeTask1->SimplifiedArrayCount, ModeTask1->SimplifiedArray, 1, V4(0, 1, 0, 1), 10000);
        
        PushLinesOnScreen64(&Renderer->PushBufferPhysics, 2, GlobalMaxDistanceArray, 1, V4(0, 1, 1, 1), 10000);
    }
    
    // NOTE(ezexff): Test tile bitmap
    /* 
    v2 Center = V2(Frame->Dim.x / 2.0f, Frame->Dim.y / 2.0f);
    v2 Dim = V2((r32)TestTileBitmap.Width, (r32)TestTileBitmap.Height);
    v2 P = Center - 0.5 * Dim;
    rectangle2 Rect = {P, P + Dim};
     */
    PushCircleOnScreen(&Renderer->PushBufferPhysics, GlobalFirstP, 10, V4(0, 1, 1, 1), 10001);
    DEBUGPushBitmapOnScreen(&Renderer->PushBufferPhysics, &GlobalTileBitmapArray[0], GlobalTileRect, 10000);
}