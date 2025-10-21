#ifndef LINE_FOLLOWER_HPP
#define LINE_FOLLOWER_HPP

#include "line_sensor.hpp"
class LineFollower {
public:
    enum class Mode { WHITHE_LINE, BLACK_LINE, UNKNOWN };

    enum class Status { STRAIGHT_LINE, TURN_LEFT, TURN_RIGHT, LOSS_LINE, SEARCH_LINE, UNKNOWN };

    void setMode(Mode mode);

    // 正常直线巡线
    void straightLine();

    // 左直角
    void turnLeft();

    // 右直角 应该可以直接复用上面的
    void turnRight();

    // 丢失线
    void lossLine();

    // 寻找线
    void searchLine();



private:
    Mode mode_;
    LineSensor line_sensor_;
};



#endif  // LINE_FOLLOWER_HPP