#ifndef __SYNTHESIS__
#define __SYNTHESIS__

#pragma warning( suppress : 4101 4129 )
#include "IPlug_include_in_plug_hdr.h"
#include "Oscillator.h"
#include "MIDIReceiver.h"

class Synthesis : public IPlug
{
public:
  Synthesis(IPlugInstanceInfo instanceInfo);
  ~Synthesis();

  void Reset();
  void OnParamChange(int paramIdx);
  void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);
  // to receive MIDI messages:
  void ProcessMidiMsg(IMidiMsg* pMsg);

private:
  double mFrequency;
  void CreatePresets();
  Oscillator mOscillator;
  MIDIReceiver mMIDIReceiver;
};

#endif
