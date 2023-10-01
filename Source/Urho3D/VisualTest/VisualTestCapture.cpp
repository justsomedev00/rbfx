#include <Urho3D/Core/Context.h>
#include <Urho3D/Resource/Image.h>
#include <Urho3D/Graphics/GraphicsEvents.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Math/StringHash.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/VisualTest/VisualTestCapture.h>
#include <Urho3D/VisualTest/VisualTestEvents.h>

namespace Urho3D
{

static const char* captureFormatNames[] =
{
    "BMP", "PNG", "TGA", "JPG", nullptr
};

static const char* captureFormatExts[] = {
    "bmp", "png", "tga", "jpg", nullptr
};

VisualTestCapture::VisualTestCapture(Context* context) :
    LogicComponent(context)
{}


void VisualTestCapture::RegisterObject(Context* context)
{
    context->AddFactoryReflection<VisualTestCapture>(Category_VisualTest);

    URHO3D_ACCESSOR_ATTRIBUTE("Is Enabled", IsEnabled, SetEnabled, bool, true, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("Name", GetName, SetName, ea::string, "", AM_DEFAULT);
    URHO3D_ENUM_ACCESSOR_ATTRIBUTE("Format", GetFormat, SetFormat, VisualTestCaptureFormat, captureFormatNames, VisualTestCaptureFormat::PNG, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("Delay", GetDelay, SetDelay, float, 0.0f, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("Duration", GetDuration, SetDuration, float, 0.0f, AM_DEFAULT);
    URHO3D_ACCESSOR_ATTRIBUTE("Frequency", GetFrequency, SetFrequency, float, 0.0f, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Image Width", unsigned int, imageSize_.x_, 1000, AM_DEFAULT);
    URHO3D_ATTRIBUTE("Image Height", unsigned int, imageSize_.y_, 1000, AM_DEFAULT);
}


void VisualTestCapture::SetDelay(float seconds)
{
    delay_ = Max(seconds, 0.0f);
    timeElapsed_ = 0.0f;
}


void VisualTestCapture::Update(float timeStep)
{
    if (IsDone()) {
        return;
    }

    timeElapsed_ += timeStep;

    if (timeElapsed_ < delay_) {
        return;
    }

    bool queueForRender = true;

    if (duration_ > 0.0f && frameNumber_ > 0) {
        float timeSinceLastCapture = timeElapsed_ - timeLastCapture_;
        if (timeSinceLastCapture < frequency_) {
            queueForRender = false;
        }
    }

    if (queueForRender) {
        timeLastCapture_ = timeElapsed_;
        SubscribeToEvent(E_RENDERSURFACEUPDATE,
            URHO3D_HANDLER(VisualTestCapture, HandleRenderSurfaceUpdate));
    }
}


bool VisualTestCapture::IsMultiFrame() const
{
    return duration_ > 0.0f;
}


Camera* VisualTestCapture::GetCamera() const
{
    if (!camera_.Expired()) {
        return camera_.Get();
    }
    return GetComponent<Camera>();
}


ea::string VisualTestCapture::GetImageFileName() const
{
    const char* ext = captureFormatExts[int(format_)];

    if (IsMultiFrame()) {
        return Format("{}_{}.{}", name_, frameNumber_, ext);
    } else {
        return Format("{}.{}", name_, ext);
    }
}


void VisualTestCapture::MarkFailed(const ea::string& reason)
{
    using namespace VisualTestCaptureFailed;
    state_ = VisualTestCaptureState::Failed;
    auto& eventData = GetEventDataMap();
    eventData[P_CAPTURE] = this;
    eventData[P_REASON] = reason;
    GetScene()->SendEvent(E_VISUALTESTCAPTUREFAILED, eventData);
}


void VisualTestCapture::MarkComplete()
{
    using namespace VisualTestCaptureComplete;
    state_ = VisualTestCaptureState::Complete;
    auto& eventData = GetEventDataMap();
    eventData[P_CAPTURE] = this;
    GetScene()->SendEvent(E_VISUALTESTCAPTURECOMPLETE, eventData);
}


void VisualTestCapture::NotifyImageReady()
{
    using namespace VisualTestCaptureImageReady;
    auto& eventData = GetEventDataMap();
    eventData[P_CAPTURE] = this;
    GetScene()->SendEvent(E_VISUALTESTCAPTUREIMAGEREADY, eventData);
}


void VisualTestCapture::QueueRender()
{
    auto* camera = GetCamera();

    if (!camera) {
        MarkFailed("No camera assigned for capture");
        return;
    }

    if (imageSize_.x_ <= 0 || imageSize_.y_ <= 0) {
        MarkFailed("Invalid image size");
        return;
    }

    if (GetName().empty()) {
        MarkFailed("No name provided for capture");
        return;
    }

    viewport_ = MakeShared<Viewport>(GetContext());
    viewport_->SetScene(GetScene());
    viewport_->SetCamera(camera);

    texture_ = MakeShared<Texture2D>(GetContext());
    texture_->SetSize(imageSize_.x_, imageSize_.y_, TextureFormat::TEX_FORMAT_RGBA8_UNORM, TextureFlag::BindRenderTarget);

    Renderer* renderer = GetSubsystem<Renderer>();
    renderer->QueueViewport(texture_->GetRenderSurface(), viewport_);

    state_ = VisualTestCaptureState::Queued;

    SubscribeToEvent(E_ENDRENDERING,
        URHO3D_HANDLER(VisualTestCapture, HandleEndRendering));
}


void VisualTestCapture::ReadRenderedImage()
{
    image_ = MakeShared<Image>(GetContext());
 
    // do image readback from texture
    if (!texture_ || !texture_->GetImage(*image_)) {
        image_.Reset();
        MarkFailed("Texture read failed");
    }

    // release viewport and texture
    viewport_.Reset();
    texture_.Reset();
}


void VisualTestCapture::HandleRenderSurfaceUpdate(StringHash eventType, VariantMap& eventData)
{
    QueueRender();
    UnsubscribeFromEvent(E_RENDERSURFACEUPDATE);
}


void VisualTestCapture::HandleEndRendering(StringHash eventType, VariantMap& eventData)
{
    ReadRenderedImage();

    if (IsMultiFrame()) {
        ++frameNumber_;
    }

    if (GetImage()) {
        NotifyImageReady();
    }

    if (!IsDone()) {
        if (timeElapsed_ >= (duration_ + delay_)) {
            MarkComplete();
        }
    }

    UnsubscribeFromEvent(E_ENDRENDERING);
}


}
