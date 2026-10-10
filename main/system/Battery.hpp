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
    uint8_t calc_battery() const;
    
private: 
    AdcReader _adc_reader;
};