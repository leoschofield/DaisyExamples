#include "Param.h"

Param::Param() : m_paramID(0), m_value(0.0f) {}

void Param::setup(int paramID) {
    m_paramID = paramID;
}

void Param::setValue(float value) {
    m_value = value;
}

float Param::getValue() {
    return m_value;
}