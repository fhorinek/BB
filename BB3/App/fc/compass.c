#include "compass.h"
#include "fc.h"

#define SENSOR_TO_COMPASS_OFFSET 90.0
#define LPF_Beta 0.75

float compass_atan2(float x, float y)
{   // calculates atan2, returns: angle in deg, range < 0 ; 360 ), max error 0.162 deg
    if(x < 0 && y == 0)    return 270.0;
    if(x > 0 && y == 0)    return 90.0;
    if(x == 0 && y == 0)   return (-1.0);//error condition
    if(x == 0 && y > 0)    return 0.0;
    if(x == 0 && y < 0)    return 180.0;

    ///arctan aproximation
    float fi = fabs( x / y );
    float fi2 = fi * fi;
    fi = ((0.596227 * fi + fi2) / (1 + 2 * 0.596227 * fi + fi2 )) * 90;

    /// cover all quadrants
    if(x >= 0 && y > 0)    return (fi);
    if(x < 0 && y > 0)     return (360.0 - fi);
    if(x >= 0 && y < 0)    return (180.0 - fi);
    if(x < 0 && y < 0)     return (fi + 180.0);
//  if(x < 0 && y == 0)    return (fi - 180.0);
    return (-1.0); //error condition
}

void compass_calc()
{
    if (fc.imu.status != fc_dev_ready)
        return;

    float azimut = 0;

//    if (fc.compass.on_riser > 0)
//    {
//        //roll
//        float a31 =   2.0f * (fc.imu.quat[0] * fc.imu.quat[1] + fc.imu.quat[2] * fc.imu.quat[3]);
//        float a33 =   fc.imu.quat[0] * fc.imu.quat[0] - fc.imu.quat[1] * fc.imu.quat[1] - fc.imu.quat[2] * fc.imu.quat[2] + fc.imu.quat[3] * fc.imu.quat[3];
//
//        //mix vertical calculation according to
//        azimut = -compass_atan2(a31, a33);
//    }

    //yaw
    float a12 =   2.0f * (fc.imu.quat.q1 * fc.imu.quat.q2 + fc.imu.quat.q0 * fc.imu.quat.q3);
    float a22 =   fc.imu.quat.q0 * fc.imu.quat.q0 + fc.imu.quat.q1 *fc.imu.quat.q1 - fc.imu.quat.q2 * fc.imu.quat.q2 - fc.imu.quat.q3 * fc.imu.quat.q3;

    azimut += compass_atan2(a12, a22);

//    azimut += fc.compass.declination;

    while(azimut > 360.0)
        azimut -= 360.0;
    while(azimut < 0.0)
        azimut += 360.0;

    fc.fused.azimuth_filtered = fc.fused.azimuth_filtered - (LPF_Beta * (fc.fused.azimuth_filtered - azimut));

    fc.fused.azimuth = azimut;
}
