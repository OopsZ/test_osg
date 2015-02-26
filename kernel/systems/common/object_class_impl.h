#pragma once

#include "kernel/object_class.h"

namespace kernel
{
//! ���������� "������ �������" (???)
//! ��������� ��� ���������� objects.xml � ������������ � ���� ������ xml
struct object_class_impl 
    : object_class
{
    //! ����������� ��� ����� ������
    object_class_impl( tinyxml2::XMLElement const * node );
    //! ����������� ��� �������� ���� ������
    object_class_impl( object_class const* parent, tinyxml2::XMLElement const * node );

// object_class
public:
    //! ������ - ��� ���� ����
    std::string         const& name             () const;
    //! ������ - ��� ��������
    named_values_t      const& attributes       () const;
    //! ������ - ��� �������� ������
    object_class_vector const& classes          () const;
    //! ������ - ������������ ����
    object_class const* parent() const;
    //! ����� ����� (����) �� �����
    object_class_ptr find_class     ( string const& name ) const ;
    //! ����� ������� �� ����� ����� ������ �������� (�������� ��������� �����)
    optional<string> find_attribute ( string const& name ) const ;
    //! ��������� ������� �������� � ������
    bool             check_attribute( string const& name, string const &value ) const ;
    //! ����� ����� �� �����, ���������� �� find_class
    object_class_ptr search_class (string const& name) const;


public:
    static object_class_ptr create_object_class( object_class const * parent, tinyxml2::XMLElement const * node ) ;

private:
    object_class const* parent_;
    object_class_vector ancestors_;
    object_class_vector classes_;

    string name_;

    named_values_t  attributes_;
};

} // kernel
