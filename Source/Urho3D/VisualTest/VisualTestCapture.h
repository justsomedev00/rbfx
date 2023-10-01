//
// Copyright (c) 2017-2023 the rbfx project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#pragma once

#include <Urho3D/VisualTest/VisualTestDefs.h>

#include <Urho3D/Math/Vector2.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/Viewport.h>
#include <Urho3D/Resource/Image.h>
#include <Urho3D/Scene/LogicComponent.h>

namespace Urho3D
{

/// Scene component for creating a visual capture of the scene state
/// using a given camera. If no specific camera is attached then the
/// camera is assumed to be a sibling of the component
/// 
class VISUALTEST_API VisualTestCapture : public LogicComponent
{
    URHO3D_OBJECT(VisualTestCapture, Component);

public:

    /// Creates a new empty capture 
    VisualTestCapture(Context* context);

    /// Register system metadata
    static void RegisterObject(Context* context);

    /// Capture status
    VisualTestCaptureState GetState() const { return state_; }
    bool IsState(VisualTestCaptureState state) const { return state_ == state; }
    bool IsFailed() const { return IsState(VisualTestCaptureState::Failed);  }
    bool IsComplete() const { return IsState(VisualTestCaptureState::Complete); }
    bool IsDone() const { return IsFailed() || IsComplete(); }

    /// Check if this is a multi-frame capture
    bool IsMultiFrame() const;

    /// Get/set the camera to use for the capture
    Camera* GetCamera() const;
    void SetCamera(Camera* camera) { camera_ = camera; }

    /// Get/set the name for the capture
    const ea::string& GetName() const { return name_; }
    void SetName(ea::string_view name) { name_ = name; }

    /// Get/set the format for the capture image
    VisualTestCaptureFormat GetFormat() const { return format_; }
    void SetFormat(VisualTestCaptureFormat format) { format_ = format; }

    /// Get/set the image size for the capture
    const IntVector2& GetImageSize() const { return imageSize_; }
    void SetImageSize(const IntVector2& size) { imageSize_ = size; }

    /// Get/set the delay (in seconds) for the capture
    float GetDelay() const { return delay_; }
    void SetDelay(float seconds);

    /// Get/set the duration (in seconds) for the capture, will perform
    /// multiple captures
    float GetDuration() const { return duration_; }
    void SetDuration(float seconds) { duration_ = seconds; }

    /// Get/set the frequency for the capture (in seconds), perform a capture
    /// every time period over the set duration
    float GetFrequency() const { return frequency_; }
    void SetFrequency(float seconds) { frequency_ = seconds; }

    /// Get the rendered image
    SharedPtr<Image> GetImage() const { return image_; }

    /// Get the filename for the current capture image
    ea::string GetImageFileName() const;

    /// Handle scene update
    void Update(float timeStep) override;

private:

    /// Mark this capture as failed for the given reason
    void MarkFailed(const ea::string& reason);

    /// Mark this capture as completed
    void MarkComplete();

    /// Queue this capture for render
    void QueueRender();

    /// Notify capture ready for processing
    void NotifyImageReady();

    /// Complete the capture, perform texture image readback 
    void ReadRenderedImage();

    /// Event handlers
    void HandleRenderSurfaceUpdate(StringHash eventType, VariantMap& eventData);
    void HandleEndRendering(StringHash eventType, VariantMap& eventData);

    /// Current capture state
    VisualTestCaptureState state_{ VisualTestCaptureState::Pending };
    /// The delay (in seconds) for the capture
    float delay_{ 0.0f };
    /// The duration (in seconds) for the capture
    float duration_{ 0.0f };
    /// The capture frequence (in seconds)
    float frequency_{ 0.0f };
    /// The time elapsed for the capture
    float timeElapsed_ { 0.0f };
    /// the time that the last capture was performed
    float timeLastCapture_{ 0.0f };
    /// The current frame of the capture
    int frameNumber_ { 0 };
    /// The name of the image capture
    ea::string name_{};
    /// The format of the saved image capture files
    VisualTestCaptureFormat format_{ VisualTestCaptureFormat::PNG};
    /// The size of the image capture to create (in pixels)
    IntVector2 imageSize_{ 1000,1000 };
    /// The pixel match ratio
    /// The camera to use for the image capture
    WeakPtr<Camera> camera_{ nullptr };
    /// Viewport for the active capture
    SharedPtr<Viewport> viewport_;
    /// The texture for the active capture
    SharedPtr<Texture2D> texture_;
    /// The in-memory image for the capture
    SharedPtr<Image> image_;
};


}
