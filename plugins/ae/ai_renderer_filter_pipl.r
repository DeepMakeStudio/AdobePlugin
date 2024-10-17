#include "external/ae-sdk/Headers/AEConfig.h"
#include "external/ae-sdk/Headers/AE_EffectVers.h"
#include "core/filters/plugin_defs.h"
#include "video_plugin_version.h"

#ifndef AE_OS_WIN
	#include "external/ae-sdk/Resources/AE_General.r"
#endif
resource 'PiPL' (16000) {
	{	/* array properties: 12 elements */
		/* [1] */
		Kind {
			AEEffect
		},
		/* [2] */
		Name {
			kAIRendererPluginName
		},
		/* [3] */
		Category {
			kAIRendererCategoryName
		},
#ifdef AE_OS_WIN
		CodeWin64X86 {"EffectMain"},
#else
	#if defined(AE_OS_MAC)
  		CodeMacARM64 {"EffectMain"},
  		CodeMacIntel64 {"EffectMain"},
	#endif
#endif
		/* [6] */
		AE_PiPL_Version { /*unused according to the sdk doc*/
			2,
			0
		},
		/* [7] */
		AE_Effect_Spec_Version {
			PF_PLUG_IN_VERSION,
			PF_PLUG_IN_SUBVERS
		},
		/* [8] */
		AE_Effect_Version {
			kAEPiplVersion
		},
		/* [9] */
		AE_Effect_Info_Flags {
			0
		},
		/* [10] */
		AE_Effect_Global_OutFlags {
			0x8000  /* PF_OutFlag_CUSTOM_UI */
		},
		AE_Effect_Global_OutFlags_2 {
			0x0000008 /*  PF_OutFlag2_PARAM_GROUP_START_COLLAPSED_FLAG */
		},
		/* [11] */
		AE_Effect_Match_Name {
			kAIREndererMatchName
		},
		/* [12] */
		AE_Reserved_Info {
			0
		}
	}
};

