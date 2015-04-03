#pragma once

#include "airport_common.h"
#include "airport_view.h"
#include "common/phys_sys.h"
#include "objects/nodes_management.h"
#include "common/phys_object_model_base.h"

namespace airport
{
//! ���������� (?) ������ ���������
struct model
    : model_presentation        // ������� �����, �������������
    , view                      // ������� �������������
    , phys_object_model_base    // ������� ����� ��� �������� � ����������� ��������
{
    static object_info_ptr create(kernel::object_create_t const& oc, dict_copt dict);

private:
    model( kernel::object_create_t const& oc, dict_copt dict );

    // base_presentation
private:
    void update(double time) override;

    // base_view_presentation
private:
    void on_child_removing(object_info_ptr child) override;

private:
    void on_zone_created( size_t id );
    void on_zone_destroyed( size_t id );

    void create_phys();

private:
    //! �������� ��� ��������� (��������� ������)
    nodes_management::manager_ptr nodes_manager_;

    phys::static_mesh_ptr mesh_;
    //! �����
    std::vector<phys::static_convex_ptr> masts_;
};

} // end of namespace airport
