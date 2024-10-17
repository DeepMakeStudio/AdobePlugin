
#include "ae_main.h"
#include "video_plugin_version.h"
#include "utils.h"
#include "logger.h"
#include "core/filters/plugin_defs.h"
#include "ae_host_delegate.h"
#include "video_host.h"
#include "video_filter_manager.h"
#include "AEFX_SuiteHelper.h"
#include "AEGP_SuiteHandler.h"
#include <iostream>


static PF_Err 
About (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output ) 
{
	UNUSED(params);
	UNUSED(output);
	PF_SPRINTF(	out_data->return_msg, 
				"%s, v%d.%d\n%s",
				kAIRendererPluginName, 
				kVideoPluginMajorVersion, 
				kVideoPluginMinorVersion, 
				DESCRIPTION);
		// No need to return the thrown error in this case, since this isn't a critical error.
		// It just means the Sweetie SDK sample wasn't loaded.
	return PF_Err_NONE;
	}

static PF_Err 
GlobalSetup(
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output)
{
	UNUSED(params);
	UNUSED(output);

    PF_Err    err = PF_Err_NONE;

    out_data->out_flags = PF_OutFlag_CUSTOM_UI;
    
    out_data->out_flags2    =   PF_OutFlag2_PARAM_GROUP_START_COLLAPSED_FLAG;
    
	out_data->my_version = PF_VERSION(	kVideoPluginMajorVersion,
										kVideoPluginMinorVersion,
										kVideoPluginPatchVersion, 
										PF_Stage_DEVELOP,
                                        kVideoPluginBuildVersion);


    //    This looks more complex than it is. Basically, this
    //    code allocates a handle, and (if successful) sets a
    //    sentinel value for later use, and gets an ID for using
    //    AEGP calls. It then stores those value in  in_data->global_data.
    AEGP_SuiteHandler suites(in_data->pica_basicP);

    PF_Handle globH = NULL;
    ARK_GlobalDataP globP = NULL;

    globH = suites.HandleSuite1()->host_new_handle(sizeof(ARK_GlobalData));
    
    if (globH) 
	{
        globP = reinterpret_cast<ARK_GlobalDataP>(suites.HandleSuite1()->host_lock_handle(globH));
        if (globP) 
		{
            globP->initializedB     = TRUE;
            ERR(suites.UtilitySuite3()->AEGP_RegisterWithAEGP(NULL, kAIREndererMatchName, &globP->plugin_id));

			if (!err)
			{
				out_data->global_data = globH;
			}
        }
        suites.HandleSuite1()->host_unlock_handle(globH);
	}
	else    
	{
        err = PF_Err_INTERNAL_STRUCT_DAMAGED;
    }
	
	LOG_ASSERT(err == PF_Err_NONE,"GlobalSetup failure in DeepMake Effect");
	return err;
}

static PF_Err
GlobalSetdown(
    PF_InData        *in_data,
    PF_OutData        *out_data,
    PF_ParamDef        *params[],
    PF_LayerDef        *output)
{
    UNUSED(params);
    UNUSED(output);
    UNUSED(in_data);

    PF_Err    err = PF_Err_NONE;
    
    VideoHostDelegatePtr delegate = std::make_shared<AEVideoHostDelegate>(PF_Cmd_GLOBAL_SETDOWN, in_data, out_data, params, output);
    auto filter = VideoFilterManager::instance().getFilter(0);
    VideoHost video_host(delegate, filter);
    video_host.shutdown();

    LOG_ASSERT(err == PF_Err_NONE,"GlobalSetdown failure in DeepMake Effect");
    return err;
}

static PF_Err Render (
    PF_InData *in_dataP,
    PF_OutData *out_data,
    PF_ParamDef *params[],
    PF_LayerDef *output )
{
    PF_Err err = PF_Err_NONE;
    VideoHostDelegatePtr delegate = std::make_shared<AEVideoHostDelegate>(PF_Cmd_RENDER, in_dataP, out_data, params, output);
    auto filter = VideoFilterManager::instance().getFilter(0);
    VideoHost video_host(delegate, filter);
    video_host.registerParams(false);
    video_host.render();
    return err;
}

extern "C" DllExport
PF_Err PluginDataEntryFunction(
	PF_PluginDataPtr inPtr,
	PF_PluginDataCB inPluginDataCallBackPtr,
	SPBasicSuite* inSPBasicSuitePtr,
	const char* inHostName,
	const char* inHostVersion)
{
  UNUSED(inSPBasicSuitePtr);
  UNUSED(inHostName);
  UNUSED(inHostVersion);

	PF_Err result = PF_Err_INVALID_CALLBACK;

	result = PF_REGISTER_EFFECT(
		inPtr,
		inPluginDataCallBackPtr,
		kAIRendererPluginName, // Name
		kAIREndererMatchName, // Match Name
		kAIRendererCategoryName, // Category
		AE_RESERVED_INFO); // Reserved Info

	return result;
}

PF_ParamIndex changedParamHostIndex(void *extraP,
                             PF_InData *in_dataP,
                             PF_ParamDef *params[])
{
    PF_UserChangedParamExtra* extraPtr = (PF_UserChangedParamExtra*)extraP;
    if (extraPtr)
    {
        return extraPtr->param_index;
    }
    return -1;
}

static PF_Err
SequenceSetup(
              PF_InData        *in_data,
              PF_OutData        *out_data)
{
    PF_Err err = PF_Err_NONE;
    ARK_SeqData **seqH = nullptr;
    ARK_SeqData *seqP = nullptr;
    
    if (in_data->sequence_data)
    {
        PF_DISPOSE_HANDLE(in_data->sequence_data);
        out_data->sequence_data = NULL;
    }
    
    seqH = (ARK_SeqData**)PF_NEW_HANDLE(sizeof(ARK_SeqData));
    
    if (seqH)
    {
        memset( *seqH, 0, sizeof(ARK_SeqData));
        seqP = (ARK_SeqData*)PF_LOCK_HANDLE(seqH);
        seqP->prompt[0] = '\0';
        seqP->rendered_image_id[0] = '\0';
        seqP->cached_params[0] = '\0';
        seqP->version = kSequenceDataVersion;
        seqP->license_status = false;

        if (!err)
        {
            in_data->sequence_data = out_data->sequence_data = (PF_Handle)seqH;
        }
    
        PF_UNLOCK_HANDLE(seqH);
        
    }
    else
    {
        err = PF_Err_OUT_OF_MEMORY;
    }
    
    if (err)
    {
        PF_SPRINTF(out_data->return_msg, "SequenceSetup failure in DeepMake Effect");
        out_data->out_flags |= PF_OutFlag_DISPLAY_ERROR_MESSAGE;
    }
    
    return err;
}

static PF_Err
SequenceResetup(
                PF_InData        *in_data,
                PF_OutData        *out_data,
                PF_ParamDef        *params[])
{
    return SequenceSetup(in_data, out_data);
}

static PF_Err
SequenceFlatten(
                PF_InData        *in_data,
                PF_OutData        *out_data,
                PF_ParamDef        *params[],
                PF_LayerDef        *output)
{
    PF_Err        err = A_Err_NONE;
    
    if (in_data->sequence_data)
    {
        out_data->sequence_data = in_data->sequence_data;
    }
    
    return err;
}

static PF_Err
SequenceSetdown(
                PF_InData *in_data,
                PF_OutData *out_data,
                PF_ParamDef *params[],
                PF_LayerDef *output)
{
    PF_Err err = PF_Err_NONE;
    
    if (in_data->sequence_data)
    {
        PF_DISPOSE_HANDLE(in_data->sequence_data);
        in_data->sequence_data = out_data->sequence_data = NULL;
    }
    return err;
    
}

MouseEventType AEEventTypeToArk(PF_EventType ae_type)
{
    if (ae_type == PF_Event_DO_CLICK)
        return eMouseDown;
    // else if (ae_type == PF_Event_MOUSE_UP)
    //     return eMouseUp;
    // else if (ae_type == PF_Event_MOUSE_MOVE)
    //     return eMouseMove;
    else if (ae_type == PF_Event_ADJUST_CURSOR)
        return eMouseOver;
    else if (ae_type == PF_Event_DRAG)
        return eDrag;

    return eMouseOver;
}

PF_Err 
HandleEvent(	
				PF_InData		*in_data,
				PF_OutData		*out_data,
				PF_ParamDef		*params[],
				PF_LayerDef		*output,
				PF_EventExtra	*extra)
{
	PF_Err		err		= PF_Err_NONE;

    //Currently we only support Comp window events
    if( (*extra->contextH)->w_type == PF_Window_COMP || (*extra->contextH)->w_type ==  PF_Window_LAYER)
    {
        VideoHostDelegatePtr delegate = std::make_shared<AEVideoHostDelegate>(PF_Cmd_EVENT, in_data, out_data, params, output, extra);
        auto filter = VideoFilterManager::instance().getFilter(0);
        VideoHost video_host(delegate, filter);
        video_host.registerParams(false);
        if (extra->e_type == PF_Event_DRAW)
        {
            video_host.handleOverlayDraw();
        }
        else
        {
            MouseEventType mouse_event_type = AEEventTypeToArk(extra->e_type);
            //TODO: convert from screen to image coordinates (extra->u.do_click.screen_point)
            video_host.handleOverlayEvent(mouse_event_type, extra->u.do_click.screen_point.h, extra->u.do_click.screen_point.v);
        }
    }
    
	return err;
}

PF_Err
EffectMain (	
	PF_Cmd cmd,
	PF_InData *in_data,
	PF_OutData *out_data,
	PF_ParamDef *params[],
	PF_LayerDef *output,
    void *extra)
{
	PF_Err		err = PF_Err_NONE;

	try {
		switch (cmd)
        {
			case PF_Cmd_ABOUT:
				err = About(in_data, out_data, params, output);
				break;		
			case PF_Cmd_GLOBAL_SETUP:
				err = GlobalSetup(in_data, out_data, params, output);
				break;
            case PF_Cmd_GLOBAL_SETDOWN:
                err = GlobalSetdown(in_data, out_data, params, output);
                break;
            case PF_Cmd_SEQUENCE_SETUP:
                err = SequenceSetup(in_data, out_data);
                break;
            case PF_Cmd_SEQUENCE_SETDOWN:
                err = SequenceSetdown(in_data, out_data, params, output);
                break;
            case PF_Cmd_SEQUENCE_RESETUP:
                //err = SequenceResetup(in_dataP, out_data, params);
                break;
            case PF_Cmd_SEQUENCE_FLATTEN:
                err = SequenceFlatten(in_data, out_data, params, output);
                break;
			case PF_Cmd_PARAMS_SETUP:
			{
				VideoHostDelegatePtr delegate = std::make_shared<AEVideoHostDelegate>(cmd, in_data, out_data, params, output);
    			auto filter = VideoFilterManager::instance().getFilter(0);
   				VideoHost video_host(delegate, filter);	
				video_host.registerParams(true);
				out_data->num_params = video_host.numOfParams() + 1;
				break;
			}
			case PF_Cmd_UPDATE_PARAMS_UI:
			{
				VideoHostDelegatePtr delegate = std::make_shared<AEVideoHostDelegate>(cmd, in_data, out_data, params, output);
    			auto filter = VideoFilterManager::instance().getFilter(0);
   				VideoHost video_host(delegate, filter);
				video_host.updateParams(-1);
				break;
			}
			case PF_Cmd_USER_CHANGED_PARAM:
			{
				VideoHostDelegatePtr delegate = std::make_shared<AEVideoHostDelegate>(cmd, in_data, out_data, params, output);
    			auto filter = VideoFilterManager::instance().getFilter(0);
   				VideoHost video_host(delegate, filter);
                PF_ParamIndex hostIndex = changedParamHostIndex(extra, in_data, params);
				if (video_host.updateParams(static_cast<int>(hostIndex)))
                    out_data->out_flags |= PF_OutFlag_FORCE_RERENDER;
				break;
			}
    		case PF_Cmd_RENDER:
				err = Render(in_data, out_data, params, output);
				break;
            case PF_Cmd_EVENT:
				err = HandleEvent(in_data, out_data, params, output, reinterpret_cast<PF_EventExtra*>(extra));
				break;
			default:
				break;
		}
	} catch(PF_Err &thrown_err) {
		// Never EVER throw exceptions into AE.
		err = thrown_err;
	} catch(...) {
		// Never EVER throw exceptions into AE.
		err = PF_Err_INTERNAL_STRUCT_DAMAGED;
	}
	return err;
}
