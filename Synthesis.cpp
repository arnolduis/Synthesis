#include "Synthesis.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "IKeyboardControl.h"
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
    kHeight = GUI_HEIGHT,
    kKeybX = 1,
    kKeybY = 0
};

Synthesis::Synthesis(IPlugInstanceInfo instanceInfo)
    :   IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo),
    lastVirtualKeyboardNoteNumber(virtualKeyboardMinimumNoteNumber - 1) {
    TRACE;

    IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
    pGraphics->AttachBackground(BG_ID, BG_FN);

    IBitmap whiteKeyImage = pGraphics->LoadIBitmap(WHITE_KEY_ID, WHITE_KEY_FN, 6);
    IBitmap blackKeyImage = pGraphics->LoadIBitmap(BLACK_KEY_ID, BLACK_KEY_FN);

    //                            C#     D#          F#      G#      A#
    int keyCoordinates[12] = { 0, 7, 12, 20, 24, 36, 43, 48, 56, 60, 69, 72 };
    mVirtualKeyboard = new IKeyboardControl(this, kKeybX, kKeybY, virtualKeyboardMinimumNoteNumber, /* octaves: */ 5, &whiteKeyImage, &blackKeyImage, keyCoordinates);

    pGraphics->AttachControl(mVirtualKeyboard);

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

	processVirtualKeyboard();
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

void Synthesis::ProcessMidiMsg(IMidiMsg* pMsg) {
    mMIDIReceiver.onMessageReceived(pMsg);
    mVirtualKeyboard->SetDirty();
}

void Synthesis::processVirtualKeyboard() {
    IKeyboardControl* virtualKeyboard = (IKeyboardControl*) mVirtualKeyboard;
    int virtualKeyboardNoteNumber = virtualKeyboard->GetKey() + virtualKeyboardMinimumNoteNumber;

    if(lastVirtualKeyboardNoteNumber >= virtualKeyboardMinimumNoteNumber && virtualKeyboardNoteNumber != lastVirtualKeyboardNoteNumber) {
        // The note number has changed from a valid key to something else (valid key or nothing). Release the valid key:
        IMidiMsg midiMessage;
        midiMessage.MakeNoteOffMsg(lastVirtualKeyboardNoteNumber, 0);
        mMIDIReceiver.onMessageReceived(&midiMessage);
    }

    if (virtualKeyboardNoteNumber >= virtualKeyboardMinimumNoteNumber && virtualKeyboardNoteNumber != lastVirtualKeyboardNoteNumber) {
        // A valid key is pressed that wasn't pressed the previous call. Send a "note on" message to the MIDI receiver:
        IMidiMsg midiMessage;
        midiMessage.MakeNoteOnMsg(virtualKeyboardNoteNumber, virtualKeyboard->GetVelocity(), 0);
        mMIDIReceiver.onMessageReceived(&midiMessage);
    }

    lastVirtualKeyboardNoteNumber = virtualKeyboardNoteNumber;
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
}