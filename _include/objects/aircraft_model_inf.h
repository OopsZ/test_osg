#pragma once

// �������� ����������� ��� aircraft.h
namespace aircraft
{
    //enum malfunction_kind_t
    //struct info
    //struct control


    struct int_control
    {
         virtual ~int_control(){}
         virtual void update(double dt)=0;
         virtual void set_trajectory(fms::trajectory_ptr  traj)=0;
         virtual fms::trajectory_ptr  get_trajectory()=0;
         virtual decart_position get_local_position()=0; 
    };

}