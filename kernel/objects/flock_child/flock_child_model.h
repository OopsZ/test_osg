#pragma once

#include "flock_child_common.h"
#include "flock_child_view.h"
#include "common/phys_sys.h"
#include "objects/nodes_management.h"
#include "common/phys_object_model_base.h"

namespace flock
{

namespace child
{

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
    
    void create_phys();
    void sync_phys          ( double dt );
    void sync_nodes_manager ( double dt );

private:
    void flap();
    void dive();
    void soar();
    void wander(float delay);

private:
    //nodes_management::manager_ptr nodes_manager_;

    phys::static_mesh_ptr                  mesh_;

    optional<size_t>                  phys_zone_;
    
    cg::geo_base_3                root_next_pos_;
    cg::quaternion              root_next_orien_;

	// Some data
private:

	bool									_soar; // state ?
	bool								   _dived;
	bool                                 _landing;

    double                                 _speed;
	double  						   _soarTimer;
	float								 _damping;
    float			    			 _lerpCounter;
	bool							 _flatFlyDown;
private:
    model_system *                           sys_;
	optional<double>                 last_update_;
	phys::flock::info_ptr             phys_flock_;
};

} // child

} // end of namespace flock
