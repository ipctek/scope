#ifndef IPCRANGE_H
#define IPCRANGE_H


class IPCRange
{
public:
    IPCRange(double min, double max);

    // Setters
    void setMin(double val){mMin = val;}
    void setMax(double val){mMax = val;}

    // Getters
    double min(){return mMin;}
    double max(){return mMax;}
private:
    double mMin;
    double mMax;
};

#endif // IPCRANGE_H
