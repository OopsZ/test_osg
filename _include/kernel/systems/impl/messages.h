#pragma once
#include "network/msg_base.h"

namespace kernel
{

namespace msg 
{
using namespace binary;

//! ���� ���������
enum id 
{
    sm_object_created,  // ������ ������
    sm_destroy_object,  // ����������� �������
    
    sm_object_msg,      // ��������� �������
    sm_container_msg,   // ��������� ���������� (?)
};

//! ��������� "������ ������"
struct object_created
    : network::msg_id<sm_object_created>
{
    bytes_t data;

    object_created()
    {
    }

    object_created(bytes_t&& data)
        : data  (move(data))
    {
    }
};

//! ��������� "����������� �������"
struct destroy_object
    : network::msg_id<sm_destroy_object>
{
    size_type obj_id;

    destroy_object(size_type obj_id = size_type(-1))
        : obj_id(obj_id)
    {
    }
};

namespace details
{
    typedef uint64_t msg_obj_id_t;
    
    //! �������� ������������� ������� �� 64-������� ���������� �������������
    inline uint32_t get_obj_id(msg_obj_id_t id) { return uint32_t((id >> 32) & 0xffffffff); }
    //! �������� ������������� ��������� �� 64-������� ���������� �������������
    inline uint32_t get_msg_id(msg_obj_id_t id) { return uint32_t(id & 0xffffffff); }
    //! ������ ��������� 64-������ ������������� �� ��������������� ������� � ���������
    inline msg_obj_id_t make_msg_obj_id(obj_id_t obj, uint32_t mesg) 
    {
        return (uint64_t(obj) << 32) + uint64_t(mesg);
    }
} // details

//! ��������� "��������� �������"
struct object_msg
    : network::msg_id<sm_object_msg>
{
    typedef set<details::msg_obj_id_t> msg_protocol_t;

    object_msg()
        : object_id(size_type(-1))
    {
    }

    object_msg(size_type object_id, bytes_cref data, bool just_cmd, msg_protocol_t&& prot)
        : object_id     (object_id)
        , data          (data)
        , just_cmd      (just_cmd)
        , msg_protocol_ (move(prot))
    {
    }

    size_type       object_id;
    bytes_t         data;
    bool            just_cmd;
    msg_protocol_t  msg_protocol_;
};

//! ��������� "��������� ����������"
struct container_msg
    : network::msg_id<sm_container_msg>
{
    //! ������ �������� ����
    typedef std::vector<bytes_t>  msgs_t;

    container_msg(){}
    
    container_msg(msgs_t&& msgs)
        : msgs(move(msgs))
    {
    }

    msgs_t msgs;
};

REFL_STRUCT(object_created)
    REFL_ENTRY(data)
REFL_END()

REFL_STRUCT(destroy_object)
    REFL_ENTRY(obj_id)
REFL_END   ()

REFL_STRUCT(object_msg)
    REFL_ENTRY(object_id)
    REFL_ENTRY(data)
    REFL_ENTRY(just_cmd)
    REFL_ENTRY(msg_protocol_)
REFL_END()

REFL_STRUCT(container_msg)
    REFL_ENTRY(msgs)
REFL_END()

} // msg
} //kernel