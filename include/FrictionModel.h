#ifndef YOKE_FRICTION_MODEL_H
#define YOKE_FRICTION_MODEL_H
#include <math.h>
// Hayward & Armstrong (2000) haptic friction (Dahl-type presliding): an anchor w follows position x;
// z=x-w is the presliding (bristle) displacement, bounded by zmax. Static
// friction holds up to Fc without velocity estimation; sliding gives -Fc*sgn.
namespace YokeFriction {
class Model {
public:
    void reset() { ready=false; anchor=0; }
    // Returns normalized resistance with sign opposite to the displacement.
    float update(float x, float fc, float zmax) {
        if(!isfinite(x) || !isfinite(fc) || fc<=0 || !(zmax>0)) { reset(); return 0; }
        if(!ready) { anchor=x; ready=true; return 0; } // no startup kick
        float z=x-anchor;
        if(z>zmax) { anchor=x-zmax; z=zmax; }
        else if(z<-zmax) { anchor=x+zmax; z=-zmax; }
        return -fc*z/zmax;
    }
private:
    bool ready=false;
    float anchor=0;
};
}
#endif
