#include "ae_draw_helper.h"
#include "AEGP_SuiteHandler.h"
#include "AE_Macros.h"

AEDrawHelper::AEDrawHelper(PF_InData *in_data, PF_OutData *out_data, PF_EventExtra* extra)
: m_in_data(in_data)
, m_out_data(out_data)
, m_event_extra(extra)
, m_scaleX(1.0f)
, m_scaleY(1.0f)
, m_drawing_ref(NULL)
, m_surface_ref(NULL)
, m_supplier_ref(NULL)
{
    AcquireSuites();
}

void AEDrawHelper::AcquireSuites()
{
    // Acquire all the Drawbot suites in one go; it should be matched with release routine.
    // You can also use C++ style AEFX_DrawbotSuitesScoper which doesn't need release routine.
    PF_Err err = PF_Err_NONE;
    
#if 0 //WIP for overlay controls
    ERR(AEFX_AcquireDrawbotSuites(m_in_data, m_out_data, &m_drawbotSuites));
    PF_EffectCustomUISuite1    *effectCustomUISuiteP;
    
    ERR(AEFX_AcquireSuite(m_in_data,
                          m_out_data,
                          kPFEffectCustomUISuite,
                          kPFEffectCustomUISuiteVersion1,
                          NULL,
                          (void**)&effectCustomUISuiteP));
    
    if (!err && m_event_extra && effectCustomUISuiteP) {
        // Get the drawing reference by passing context to this new api
        ERR((*effectCustomUISuiteP->PF_GetDrawingReference)(m_event_extra->contextH, &m_drawing_ref));
        AEFX_ReleaseSuite(m_in_data, m_out_data, kPFEffectCustomUISuite, kPFEffectCustomUISuiteVersion1, NULL);
        effectCustomUISuiteP = NULL;
    }
    // Get the Drawbot supplier from drawing reference; it shouldn't be released like pen or brush (see below)
    ERR(m_drawbotSuites.drawbot_suiteP->GetSupplier(m_drawing_ref, &m_supplier_ref));
    
    // Get the Drawbot surface from drawing reference; it shouldn't be released like pen or brush (see below)
    ERR(m_drawbotSuites.drawbot_suiteP->GetSurface(m_drawing_ref, &m_surface_ref));
#endif
    
}

void AEDrawHelper::ReleaseSuites()
{
    PF_Err err = PF_Err_NONE;
    // Release the earlier acquired Drawbot suites
    ERR(AEFX_ReleaseDrawbotSuites(m_in_data, m_out_data));
}

AEDrawHelper::~AEDrawHelper()
{
    ReleaseSuites();
}

bool AEDrawHelper::DrawCircle(Point2D inCenter, float inRadius, Color inColor, float inSize, bool inFill)
{
    PF_Err err = PF_Err_NONE;
    DRAWBOT_PointF32 dbCenter {inCenter.x, inCenter.y};
    DRAWBOT_ColorRGBA dbColor {inColor.r, inColor.g, inColor.b, inColor.a};
    
    DRAWBOT_PenRef pen_ref = NULL;
    DRAWBOT_PathRef path_ref = NULL;
    DRAWBOT_BrushRef brush_ref = NULL;

    ERR(m_drawbotSuites.supplier_suiteP->NewPath(m_supplier_ref, &path_ref));
    ERR(m_drawbotSuites.supplier_suiteP->NewBrush(m_supplier_ref, &dbColor, &brush_ref));
    ERR(m_drawbotSuites.supplier_suiteP->NewPen(m_supplier_ref, &dbColor, inSize, &pen_ref));
   
    ERR(m_drawbotSuites.path_suiteP->AddArc(path_ref, &dbCenter, inRadius, 0.0f, 360.0f));
    
    // Fill the path with the brush created
    if(inFill)
        ERR(m_drawbotSuites.surface_suiteP->FillPath(m_surface_ref, brush_ref, path_ref, kDRAWBOT_FillType_Default));
    
    ERR(m_drawbotSuites.surface_suiteP->StrokePath(m_surface_ref, pen_ref, path_ref));
    
    if (brush_ref)
        ERR(m_drawbotSuites.supplier_suiteP->ReleaseObject((DRAWBOT_ObjectRef)brush_ref));
    if (pen_ref)
        ERR(m_drawbotSuites.supplier_suiteP->ReleaseObject((DRAWBOT_ObjectRef)pen_ref));
    if (path_ref)
        ERR(m_drawbotSuites.supplier_suiteP->ReleaseObject(reinterpret_cast<DRAWBOT_ObjectRef>(path_ref)));

    return err;
}

bool AEDrawHelper::DrawLine(Point2D inStart, Point2D inEnd, Color inColor, float inSize)
{
    PF_Err err = PF_Err_NONE;
    DRAWBOT_ColorRGBA dbColor {inColor.r, inColor.g, inColor.b, inColor.a};

    DRAWBOT_PenRef pen_ref = NULL;
    DRAWBOT_PathRef path_ref = NULL;
    
    // Create a new path. It should be matched with release routine.
    // You can also use C++ style DRAWBOT_PathP that releases automatically at the end of scope.
    ERR(m_drawbotSuites.supplier_suiteP->NewPath(m_supplier_ref, &path_ref));
    ERR(m_drawbotSuites.supplier_suiteP->NewPen(m_supplier_ref, &dbColor, inSize, &pen_ref));
    
    ERR(m_drawbotSuites.path_suiteP->MoveTo(path_ref, inStart.x, inStart.y));
    ERR(m_drawbotSuites.path_suiteP->LineTo(path_ref, inEnd.x, inEnd.y));
    
    ERR(m_drawbotSuites.surface_suiteP->StrokePath(m_surface_ref, pen_ref, path_ref));
    
    if (path_ref) {
        ERR(m_drawbotSuites.supplier_suiteP->ReleaseObject(reinterpret_cast<DRAWBOT_ObjectRef>(path_ref)));
    }
    if (pen_ref) {
        ERR(m_drawbotSuites.supplier_suiteP->ReleaseObject((DRAWBOT_ObjectRef)pen_ref));
    }
    return err;
}
