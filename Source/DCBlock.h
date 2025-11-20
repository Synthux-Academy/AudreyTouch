#pragma once

namespace infrasonic
{
class DCBlock
{
  public:
    void Init(float sample_rate);
    float Process(float in);

  private:
    float input_, output_, gain_;
};
}