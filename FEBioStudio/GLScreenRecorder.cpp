/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
#include "GLScreenRecorder.h"
#include "GLView.h"
#include "Animation.h"

GLScreenRecorder::GLScreenRecorder() : m_glview(nullptr), m_video(nullptr)
{
	m_state = RECORDING_STATE::STOPPED;
}

void GLScreenRecorder::AttachToView(CGLView* glview)
{
	m_glview = glview;
}

RECORDING_STATE GLScreenRecorder::GetRecordingState() const
{
	return m_state;
}

bool GLScreenRecorder::HasRecording() const
{
	return (m_video != nullptr);
}

bool GLScreenRecorder::SetVideoStream(CAnimation* video)
{
	if (m_glview == nullptr) return false;
	if (video == nullptr) return false;

	if (m_video) return false;

	m_video = video;
	m_state = RECORDING_STATE::STOPPED;

	return true;
}

void GLScreenRecorder::Start()
{
	if (m_video && m_glview)
	{
		m_state = RECORDING_STATE::RECORDING;
		m_glview->repaint();
	}
}

void GLScreenRecorder::Stop()
{
	if (m_video)
	{
		// stop the animation
		m_state = RECORDING_STATE::STOPPED;

		// get the nr of frames before we close
		int nframes = m_video->Frames();

		// close the stream
		m_video->Close();

		// delete the object
		delete m_video;
		m_video = nullptr;
	}
}

void GLScreenRecorder::Pause()
{
	if (m_video)
	{
		m_state = RECORDING_STATE::PAUSED;
	}
}

bool GLScreenRecorder::IsRecording() const
{
	return (m_video != nullptr) && (m_state == RECORDING_STATE::RECORDING);
}

bool GLScreenRecorder::IsPaused() const
{
	return (m_video != nullptr) && (m_state == RECORDING_STATE::PAUSED);
}

bool GLScreenRecorder::IsStopped() const
{
	return (m_video == nullptr) || (m_state == RECORDING_STATE::STOPPED);
}

bool GLScreenRecorder::AddFrame(QImage& im)
{
	if (m_video == nullptr) return false;
	return m_video->Write(im);
}
