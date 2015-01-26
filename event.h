#pragma once

#include <boost/signals2/signal.hpp>

//! @@@ �����
//! ��������� ����������, ��������� � �������� �������� � ������, �� ������� �������� ������� ���������
//! ��������, ������������ ��� ��������������� ������� ���������� �������� � ������ ��� ����������� �������,
//! �.�. ���� ������� ���������� �� ����� ��� ���� �������; � ��� ��� ������ �������� � ������ �������-�������
struct connection_holder 
{
    typedef boost::signals2::connection         conn_t;
    typedef boost::signals2::scoped_connection  scoped_conn_t;

    typedef std::unique_ptr<scoped_conn_t> scoped_conn_ptr;
    typedef std::vector<scoped_conn_ptr>   scoped_conn_ptrs;

    //! �������� ���������� ���������� �� ���������� ������
    connection_holder& operator<<(conn_t const& conn)
    {
        ptrs_.emplace_back(scoped_conn_ptr(new scoped_conn_t(conn)));
        return *this;
    }

    void release()
    {
        ptrs_ = scoped_conn_ptrs();
    }

private:
    //! ���������� ������������ ������ ����������
    scoped_conn_ptrs ptrs_;
};

//! @@@ �����
//! ���������� ������� (�������, �.�. ���������); 
//! ������: ��������� 
//!  ��� �����      '���_slot_type', 
//!  ������ ������� '���_signal_', �
//!  �������        'subscribe_���(slot)' 
//! ������� ��������� ������������ �� ������ ����� � ����� �������� �������
//! �� ��������� ��������� �������, �� ���� ��� �� ��� ����� ������������� ������, ������ ������� name_signal_() � �����������
//! ��� ��� ����� ��� ��� ��������� (����� � ������), ��� ������ ����������� �� ��� �������
//! ������ � Qt - emit, �� ���� ����� ���� �� ������� ����������� ������ EMIT_EVENT(name)
//! ���, ��� ����� ����������� �� ������� ������� �������, ������� �������� subscribe_name, 
//! ������ � Qt - connect, ���������� ����� ���� �� ������� ������ SUBSCRIBE_EVENT(name, slot)
//! ������� �� ������� ����������� ������ �������� - ����� ����� .release() ������� ����������, ������� ���������� ������� subscribe_*

#define DECLARE_EVENT(name,arglist)                                                     \
    typedef boost::signals2::signal<void arglist>::slot_type name##_slot_type;          \
    virtual boost::signals2::connection subscribe_##name( name##_slot_type const& slot )\
    {                                                                                   \
        return name##_signal_.connect(slot);                                            \
    }                                                                                   \
                                                                                        \
    boost::signals2::signal<void arglist> name##_signal_

//! ������� ��� ������������ �� �������
#define SUBSCRIBE_EVENT(obj,name,handler) \
    obj->subscribe_##name(handler)

//! ������� ��� ��������� �������
#define EMIT_EVENT_ARGS(name,arglist) \
    name_##signal_(arglist)

#define EMIT_EVENT(name) \
    name_##signal_()

/*  DEPRECATED

    / * be careful, verify the module of to_track allocation still alive while signal is called. * /\
    / * Even if you unsubscribe!! (because of lazy unsubscription in boost::signals2) * / \
                                                                                        \
    template<class T>                                                                   \
    void subscribe_##name( name##_slot_type slot, boost::shared_ptr<T> to_track)        \
    {                                                                                   \
        name##_signal_.connect(slot.track(to_track));                                   \
    }                                                                                   \
*/
    




