#ifndef PARAM_H
#define PARAM_H

class Param {
private:
    float m_value;

public:
    Param();
    void setup(int paramID);
    void setValue(float value);
    float getValue();
    int m_paramID;
};

#endif