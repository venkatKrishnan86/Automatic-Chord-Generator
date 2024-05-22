#include "HelperFunctions.h"

// Function to convert frequency to pitch class
int frequency_to_pitch_class(float frequency) {
    if (frequency <= 0) return -1; // Invalid frequency
    int pitch_class = static_cast<int>(std::round(12 * std::log2(frequency / 440.0) + 69)) % 12;
    return pitch_class;
}

// Function to predict the chord given the output from the model. Expected shape: (12, )
std::string predictChord(at::Tensor output, std::unordered_map<std::string, std::vector<int>> chordTemplate)
{
    std::unordered_map<std::string, std::vector<int>>::iterator it;
    std::string actualKey = "";
    at::Tensor minDistance = torch::tensor(10000.0); 
    for (it = chordTemplate.begin(); it != chordTemplate.end(); it++) {
        torch::Tensor chord = torch::tensor(it->second, {torch::kFloat64});
        at::Tensor distance = torch::norm(chord - output);
        if(minDistance.item<double>() > distance.item<double>()) {
            minDistance = distance;
            actualKey = it->first;
        }
    }
    return actualKey;
}

// Function to calculate the chroma spectrum
torch::Tensor calculate_chroma_spectrum(const torch::Tensor& fft_output, float sample_rate) {
    // Assume fft_output is a 1D tensor with the magnitudes of the FFT
    int n_bins = 12;
    auto chroma = torch::zeros({n_bins});

    // Calculate the frequencies corresponding to each bin in the FFT output
    long long fft_size = fft_output.size(0);
    float freq_bin = sample_rate / fft_size;

    for (int i = 0; i < fft_size / 2; ++i) { // Only need up to Nyquist frequency
        float frequency = i * freq_bin;
        int pitch_class = frequency_to_pitch_class(frequency);
        if (pitch_class >= 0) {
            chroma[pitch_class] += fft_output[i];
        }
    }

    return chroma;
}