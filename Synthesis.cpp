#include "Synthesis.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"
//#include <math.h>
#include <algorithm>

const int kNumPrograms = 5;

enum EParams
{
  kNumParams
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT
};

Synthesis::Synthesis(IPlugInstanceInfo instanceInfo)
  :	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo), mFrequency(1.)
{
	TRACE;

	IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
	AttachGraphics(pGraphics);
	CreatePresets();
}

Synthesis::~Synthesis() {}

void Synthesis::ProcessDoubleReplacing(
	double** inputs,
	double** outputs,
	int nFrames)
{
	// Mutex is already locked for us.

	double *leftOutput = outputs[0];
	double *rightOutput = outputs[1];

	for (int i = 0; i < nFrames; ++i) {
		mMIDIReceiver.advance();
		int velocity = mMIDIReceiver.getLastVelocity();
		if (velocity > 0) {
			mOscillator.setFrequency(mMIDIReceiver.getLastFrequency());
			mOscillator.setMuted(false);
		} else {
			mOscillator.setMuted(true);
		}
		leftOutput[i] = rightOutput[i] = mOscillator.nextSample() * velocity / 127.0;
	}

	mMIDIReceiver.Flush(nFrames);
}

void Synthesis::Reset()
{
	TRACE;
	IMutexLock lock(this);
	mOscillator.setSampleRate(GetSampleRate());
}

void Synthesis::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);
}

void Synthesis::CreatePresets() {
  MakeDefaultPreset((char *) "-", kNumPrograms);
}

void Synthesis::ProcessMidiMsg(IMidiMsg* pMsg) {
	mMIDIReceiver.onMessageReceived(pMsg);
}