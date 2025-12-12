#include "include.h"
void test_gear(void)
{
    static u16 cnt = 0;
    static u8 dir = 0;

    cnt++;
    if (cnt >= 500)
    {
        cnt = 0;
        // fun_info.left_turn = dir;
        // fun_info.right_turn = dir;
        dir = !dir;
        // send_data_packet(SEND_LEFT_TURN);
        // send_data_packet(SEND_RIGHT_TURN);

        // if (0 == dir)
        // {
        //     fun_info.gear = GEAR_FIFTH;
        // }
        // else
        // {
        //     fun_info.gear = GEAR_SECOND;
        // }

        {
            static u8 i = 0;
            i++;
            if (i >= 7)
            {
                i = 0;
            }

            fun_info.gear = i;
            send_data_packet(SEND_GEAR);
        }
    }
}
