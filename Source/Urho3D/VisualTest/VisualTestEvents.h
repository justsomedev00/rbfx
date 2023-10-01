#pragma once

#include "../Core/Object.h"

namespace Urho3D
{

/// Event sent when a capture for a visual test fails for some reason
URHO3D_EVENT(E_VISUALTESTCAPTUREFAILED, VisualTestCaptureFailed)
{
    URHO3D_PARAM(P_CAPTURE, Capture);           // VisualTestCapture*
    URHO3D_PARAM(P_REASON, Reason);             // String
}

/// Event sent when a capture image for a visual test is complete, i.e.
/// the capture image is available. 
URHO3D_EVENT(E_VISUALTESTCAPTUREIMAGEREADY, VisualTestCaptureImageReady)
{
    URHO3D_PARAM(P_CAPTURE, Capture);           // VisualTestCapture*
}

/// Event sent when a capture is complete
URHO3D_EVENT(E_VISUALTESTCAPTURECOMPLETE, VisualTestCaptureComplete)
{
    URHO3D_PARAM(P_CAPTURE, Capture);           // VisualTestCapture*
}


}
