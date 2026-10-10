#pragma once
#include "AdcReader.hpp"

class Battery final {
public:
    static Battery& get();

private:
    explicit Battery();
    ~Battery() = default;

public:
    uint8_t percentage() const;

private: 
    AdcReader _adc_reader;
};