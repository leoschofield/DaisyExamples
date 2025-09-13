#ifndef PARAM_H
#define PARAM_H


class Param {
private:
    int m_cvID;
    int m_paramID;
    float m_value;

public:
    Param();
    void setup(int cvID, int paramID);
    void setValue(float value);
    float getValue() const;
};



#endif