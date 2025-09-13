#include "daisy_patch_sm.h"
#include "daisysp.h"

/** These are namespaces for the daisy libraries.
 *  These lines allow us to omit the "daisy::" and "daisysp::" before
 * referencing modules, and functions within the daisy libraries.
 */
using namespace daisy;
using namespace patch_sm;
using namespace daisysp;

/** Our hardware board class handles the interface to the actual DaisyPatchSM
 * hardware. */
DaisyPatchSM patch;
Switch button;
Switch encoderButton;
char cvInputs[8];
    



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

        static unsigned int parameters[8] = {0};
        bool update_needed[8] =  {false} ;
        for (int i = 0; i < 8; i++)
        {
            unsigned int readVal = patch.GetAdcValue(cvInputs[i]);
            if (parameters[i] != readVal)
            {
                parameters[i] = readVal;
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
    cvInputs[1] = CV_3;
    cvInputs[2] = CV_3;
    cvInputs[3] = CV_4;
    cvInputs[4] = CV_5;
    cvInputs[5] = CV_6;
    cvInputs[6] = CV_7;
    cvInputs[7] = CV_8;

    /** Start Processing the audio */
    patch.StartAudio(AudioCallback);
    while(1) {}
}
