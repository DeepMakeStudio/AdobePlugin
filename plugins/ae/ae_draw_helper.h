#ifndef _AE_DRAW_HELPER_H
#define _AE_DRAW_HELPER_H


#include "draw_helper.h"
#include "AE_Effect.h"
#include "AE_EffectUI.h"
#include "AEFX_SuiteHelper.h"

class AEDrawHelper : public DrawHelper
{
public:
    AEDrawHelper(PF_InData *in_data, PF_OutData *out_data, PF_EventExtra* extra);
    virtual ~AEDrawHelper();
    virtual bool DrawCircle(Point2D inCenter, float inRadius, Color inColor, float inSize, bool inFill);    
    virtual bool DrawLine(Point2D inStart, Point2D inEnd, Color inColor, float inSize);
    
private:
    void AcquireSuites();
    void ReleaseSuites();

    PF_EventExtra                 *m_event_extra;
    PF_InData                     *m_in_data;
    PF_OutData                    *m_out_data;
    double                        m_scaleX;
    double                        m_scaleY;

    DRAWBOT_Suites                m_drawbotSuites;
    DRAWBOT_DrawRef               m_drawing_ref;
    DRAWBOT_SurfaceRef            m_surface_ref;
    DRAWBOT_SupplierRef           m_supplier_ref;
};


#endif // _AE_DRAW_HELPER_H
