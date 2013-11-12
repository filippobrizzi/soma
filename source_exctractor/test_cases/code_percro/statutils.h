#ifndef STATUTILS_H
#define STATUTILS_H


// milliseconds
class MeanTimer
{
public:
    MeanTimer(): m(0), step(0),c(0) , accum(0),l(0) {}
    void pre() { t.restart(); }
    void post() { l = t.restart(); accum += l; step++; }
    double mean() const { return m ; }
    int count() const { return c;}
    double last() const {return l; }
    void reset() { m = step == 0 ? 0 : accum/step; c = step; step = 0; accum = 0; }
    void resetifdata() { if(step != 0) {m = accum/step; c = step; step = 0; accum = 0;} }

protected:
    double m;
    int step,c;
    double accum;
    double l;
    QTime t;
};

class MeanTimerScope
{
public:
    MeanTimerScope(MeanTimer & mt): mtt(mt)
    {
        mt.pre();
    }
    ~MeanTimerScope() { mtt.post(); }

    MeanTimer & mtt;
};

class MeanCounter
{
public:
    MeanCounter(): m(0), step(0),c(0) , accum(0) ,saccum(0) {}
    void push(float value) { accum += value; step++; }
    double mean() const { return m ; }
    double total() const { return saccum; }
    int count() const { return c;}
    void reset() { m = step == 0 ? 0 : accum/step; c = step; step = 0; saccum = accum; accum = 0; }


protected:
    double m;
    int step,c;
    double accum,saccum;
    QTime t;
};

#endif // STATUTILS_H
