#include "daisy_patch_sm.h"
#include "daisysp.h"
#include "Param.h"
#include "defines.h"
/** These are namespaces for the daisy libraries.
 *  These lines allow us to omit the "daisy::" and "daisysp::" before
 * referencing modules, and functions within the daisy libraries.
 */
using namespace daisy;
using namespace patch_sm;
using namespace daisysp;
using namespace defines; 

/** Our hardware board class handles the interface to the actual DaisyPatchSM
 * hardware. */
DaisyPatchSM patch;
Switch button;
Switch encoderButton;
Param parameters[8];



/** Callback for processing and synthesizing audio
 *
 *  The audio buffers are arranged as arrays of samples for each channel.
 *  For example, to access the left input you would use:
 *    in[0][n]
 *  where n is the specific sample.
 *  There are "size" samples in each array.
 *
 *  The default size is very small (just 4 samples per channel). This means the
 * callback is being called at 16kHz.
 *
 *  This size is acceptable for many applications, and provides an extremely low
 * latency from input to output. However, you can change this size by calling
 * patch.SetAudioBlockSize(desired_size). When running complex DSP it can be more
 * efficient to do the processing on larger chunks at a time.
 *
 */
void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    patch.ProcessAllControls();
    // button.Debounce();
    // encoderButton.Debounce();


    for (int i = 0; i < 8; i++)
    {
        float readVal = patch.GetAdcValue(i);

        if (parameters[i].getValue() != readVal)
        {
            parameters[i].setValue(readVal);
            if (i == 0)
                patch.WriteCvOut(2,5.0*readVal);
        }
    }


    for(size_t i = 0; i < size; i++)
    {
        out[0][i] = in[0][i]; /**< Copy the left input to the left output */
        out[1][i] = in[1][i]; /**< Copy the right input to the right output */
    }
}

int main(void)
{
    /** Initialize the hardware */
    patch.Init();
    patch.StartLog();

    parameters[0].setup(paramID::PARAM_1);
    parameters[1].setup(paramID::PARAM_2);
    parameters[2].setup(paramID::PARAM_3);
    parameters[3].setup(paramID::PARAM_4);
    parameters[4].setup(paramID::PARAM_5);
    parameters[5].setup(paramID::PARAM_6);
    parameters[6].setup(paramID::PARAM_7);
    parameters[7].setup(paramID::PARAM_8);

    /** Start Processing the audio */
    patch.StartAudio(AudioCallback);
        // patch.PrintLine("hellooooo");
        patch.WriteCvOut(2,5.0);
    while(1) {
        // System::Delay(1000); // Wait 1 second between printing
    }

}
