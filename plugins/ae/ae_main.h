#ifndef AE_MAIN_H
#define AE_MAIN_H

#include <AEConfig.h>
#ifdef AE_OS_WIN
	#include <Windows.h>
#endif

#include <entry.h>

#include <AE_Effect.h>
#include <AE_GeneralPlug.h>
#include <A.h>
#include <AE_EffectCB.h>	
#include <AE_Macros.h>

#include <Param_Utils.h>

#define DESCRIPTION	"DeepMake AI Renderer"

#define kPromptSize 1024
#define kImageIDSize 64
#define kCachedParamSize 1024
#define kSequenceDataVersion 2

typedef struct ARK_SeqData
{
    char prompt[kPromptSize];
    char rendered_image_id[kImageIDSize]; //The most recent rendered image id
    char cached_params[kCachedParamSize]; //the params that were used to render the most recent image
	int version = -1;
	bool license_status = false;
	
} ARK_SeqData;


typedef struct ARK_GlobalData
{
    PF_Boolean initializedB;
    AEGP_PluginID plugin_id;
} ARK_GlobalData, *ARK_GlobalDataP, **ARK_GlobalDataH;

extern "C" {

	DllExport
	PF_Err
	EffectMain (	
		PF_Cmd cmd,
		PF_InData *in_dataP,
		PF_OutData *out_data,
		PF_ParamDef *params[],
		PF_LayerDef *output,
		void *extraP);
}

#endif // AE_MAIN_H
