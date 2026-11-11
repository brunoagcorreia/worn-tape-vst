#pragma once
#include <vector>
#include <cmath>
#include <cstdint>

struct TapeParams {
    float wow = 0.3f, flutter = 0.2f, bandwidth = 0.5f, drive = 0.3f,
          hiss = 0.1f, dropouts = 0.2f, pops = 0.1f;   // all 0..1
};

class TapeDSP {
public:
    TapeParams p;

    void prepare(double sampleRate, int numChannels) {
        sr = (float) sampleRate;
        const size_t len = (size_t) (sr * 0.05f) + 8;          // 50 ms delay buffer
        buf.assign(numChannels, std::vector<float>(len, 0.f));
        lp1.assign(numChannels, 0.f);
        lp2.assign(numChannels, 0.f);
        hp.assign(numChannels, 0.f);
        writePos = 0;
    }

    void process(float* const* io, int numCh, int n) {
        const float twoPi = 6.2831853f;
        const float bwHz  = 3000.f + p.bandwidth * 13000.f;
        const float aLp   = 1.f - std::exp(-twoPi * bwHz / sr);
        const float aHp   = 1.f - std::exp(-twoPi * 40.f / sr);
        const float g     = 1.f + p.drive * 8.f;
        const float gNorm = 1.f / std::tanh(g);
        const int   len   = (int) buf[0].size();

        for (int i = 0; i < n; ++i) {
            // --- modulation, shared by all channels (same tape, same wobble)
            wowPh  += twoPi * 0.7f / sr;  if (wowPh  > twoPi) wowPh  -= twoPi;
            flutPh += twoPi * 8.5f / sr;  if (flutPh > twoPi) flutPh -= twoPi;
            if (--driftCount <= 0) {
                driftTarget = white();
                driftCount  = (int) (sr * (0.2f + 0.3f * rand01()));
            }
            drift += (6.f / sr) * (driftTarget - drift);

            const float modMs = p.wow * 2.5f * std::sin(wowPh)
                              + p.flutter * 0.25f * std::sin(flutPh)
                              + p.wow * 1.0f * drift;
            const float delaySamples = (4.f + modMs) * 0.001f * sr;   // 4 ms base > max depth

            // --- dropouts
            if (dropLeft > 0) {
                if (--dropLeft == 0) dropTarget = 1.f;
            } else if (rand01() < p.dropouts * 1.5f / sr) {            // ~1.5 events/s at max
                dropLeft   = (int) (sr * (0.01f + 0.08f * rand01()));
                dropTarget = 0.15f + 0.4f * rand01();
            }
            dropGain += 0.005f * (dropTarget - dropGain);

            // --- pops
            if (rand01() < p.pops * 3.f / sr) popEnv = 0.3f + 0.7f * rand01();
            popEnv *= 0.985f;
            const float pop = popEnv * white() * 0.5f;

            for (int ch = 0; ch < numCh; ++ch) {
                buf[ch][writePos] = io[ch][i];

                float rp = (float) writePos - delaySamples;
                if (rp < 0.f) rp += (float) len;
                const int   i0 = (int) rp;
                const int   i1 = (i0 + 1) % len;
                const float fr = rp - (float) i0;
                float x = buf[ch][i0] * (1.f - fr) + buf[ch][i1] * fr;

                x = std::tanh(x * g) * gNorm;                          // saturation

                hp[ch]  += aHp * (x - hp[ch]);  x -= hp[ch];           // high-pass
                lp1[ch] += aLp * (x - lp1[ch]);                        // low-pass x2
                lp2[ch] += aLp * (lp1[ch] - lp2[ch]);
                x = lp2[ch] * dropGain;

                x += white() * p.hiss * 0.02f + pop;                   // hiss + pops
                io[ch][i] = x;
            }
            writePos = (writePos + 1) % len;
        }
    }

private:
    float sr = 44100.f, wowPh = 0, flutPh = 0, drift = 0, driftTarget = 0;
    float dropGain = 1.f, dropTarget = 1.f, popEnv = 0;
    int   driftCount = 0, dropLeft = 0, writePos = 0;
    uint32_t rng = 22222u;
    std::vector<std::vector<float>> buf;
    std::vector<float> lp1, lp2, hp;

    float rand01() {                       // xorshift: fast, no allocation, audio-thread safe
        rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5;
        return (float) (rng >> 8) * (1.f / 16777216.f);
    }
    float white() { return rand01() * 2.f - 1.f; }
};