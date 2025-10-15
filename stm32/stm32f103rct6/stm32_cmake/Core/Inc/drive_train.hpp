#include "motor.hpp"

class DriveTrain {
public:
    DriveTrain() = delete;

    explicit DriveTrain(Motor leftFront, Motor leftBack, Motor rightFront, Motor rightBack);

    void init(Motor leftFront, Motor leftBack, Motor rightFront, Motor rightBack);

    void drive(int straightSpeed, int turnSpeed=0);
    
    void turn(int turnSpeed);

    void stop();

    // 新增方法
    int getStraightSpeed() const;
    int getTurnSpeed() const;
    int getLeftSpeed() const;
    int getRightSpeed() const;
    
    void setMaxSpeed(int maxSpeed);
    void smoothDrive(int targetStraightSpeed, int targetTurnSpeed, float smoothingFactor = 0.2f);
    void arcDrive(int speed, int turnRadius);

private:
    Motor leftFrontMotor_;
    Motor leftBackMotor_;
    Motor rightFrontMotor_;
    Motor rightBackMotor_;

    bool initialized_ = false;

    // -100 to 100 前正后负
    int straightSpeed_ = 0;
    // -100 to 100 正左负右
    int turnSpeed_ = 0;
};
