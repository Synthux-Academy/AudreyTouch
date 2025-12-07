/*
Copyright (c) 2020 Electrosmith, Corp

Use of this source code is governed by an MIT-style
license that can be found in the LICENSE file or at
https://opensource.org/licenses/MIT.
*/

#pragma once

namespace infrasonic
{
/** Removes DC component of a signal
    */
class DCBlock
{
  public:
    DCBlock(){};
    ~DCBlock(){};

    /** Initializes DCBlock module
    */
    void Init(float sample_rate);

    /** performs DCBlock Process 
    */
    float Process(float in);

  private:
    float input_, output_, gain_;
};
} // namespace infrasonic