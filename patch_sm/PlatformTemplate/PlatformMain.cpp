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
char cvInputs[8];
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
        button.Debounce();
        encoderButton.Debounce();

        static unsigned int storedVals[8] = {0};
        bool update_needed[8] =  {false} ;
        for (int i = 0; i < 8; i++)
        {
            unsigned int readVal = patch.GetAdcValue(cvInputs[i]);
            if (storedVals[i] != readVal)
            {
                storedVals[i] = readVal;
                update_needed[i] = true;
            }
        }




    /** The easiest way to do pass thru is to simply copy the input to the output
   * In C++ the standard way of doing this is with std::copy. However, those
   * familliar with C can use memcpy. A simple loop is also a good way to do
   * this.
   *
   * Since you'll most likely want to be doing something between the input,
   *  and the output, and not just passing it through we'll demonstrate doing
   *  so with a for loop.
   */
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
    cvInputs[0] = CV_1;
    cvInputs[1] = CV_2;
    cvInputs[2] = CV_3;
    cvInputs[3] = CV_4;
    cvInputs[4] = CV_5;
    cvInputs[5] = CV_6;
    cvInputs[6] = CV_7;
    cvInputs[7] = CV_8;
    
    parameters[0].setup(cvInputs[0], paramID::PARAM_1);
    parameters[1].setup(cvInputs[1], paramID::PARAM_2);
    parameters[2].setup(cvInputs[2], paramID::PARAM_3);
    parameters[3].setup(cvInputs[3], paramID::PARAM_4);
    parameters[4].setup(cvInputs[4], paramID::PARAM_5);
    parameters[5].setup(cvInputs[5], paramID::PARAM_6);
    parameters[6].setup(cvInputs[6], paramID::PARAM_7);
    parameters[7].setup(cvInputs[7], paramID::PARAM_8);

    /** Start Processing the audio */
    patch.StartAudio(AudioCallback);
    while(1) {}
}
