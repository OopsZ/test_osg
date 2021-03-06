#pragma once

#include "cpp_utils/polymorph_ptr.h"

#include "common/ext_msgs.h"

namespace objects_reg
{

struct info
{
    virtual ~info() {}
};

struct control
{
    typedef boost::function<void(binary::bytes_cref /*bytes*/)> remote_send_f;

    virtual void inject_msg     (const void* data, size_t size) = 0;
    virtual void set_sender     (remote_send_f s)=0;
    
    virtual ~control() {}
};

typedef polymorph_ptr<info> info_ptr;
typedef polymorph_ptr<control> control_ptr;

}