#define kVideoPluginMajorVersion 0
#define kVideoPluginMinorVersion 2
#define kVideoPluginPatchVersion 0
#define kVideoPluginBuildVersion 0
#define kVideoPluginVersionString "0.2.0"

//The pipl compiler only likes raw numbers, but this is how to caclulate the kAEPiplVersion
//const int kAEPiplVersion = (kVideoPluginMajorVersion * 524288) + (kVideoPluginMinorVersion * 32768) + (kVideoPluginPatchVersion * 2048) + (PF_Stage_DEVELOP  * 512) + kVideoPluginBuildVersion;
#define kAEPiplVersion 65536
