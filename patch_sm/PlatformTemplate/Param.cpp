#include "Param.h"

Param::Param() : m_cvID(0), m_paramID(0), m_value(0.0f) {}

void Param::setup(int cvID, int paramID) {
    m_cvID = cvID;
    m_paramID = paramID;
}

void Param::setValue(float value) {
    m_value = value;
}

float Param::getValue() const {
    return m_value;
}