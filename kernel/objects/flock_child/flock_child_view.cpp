#include "stdafx.h"
#include "precompiled_objects.h"

#include "flock_child_view.h"

namespace flock
{

namespace child
{

object_info_ptr view::create(kernel::object_create_t const& oc, dict_copt dict)
{
    return object_info_ptr(new view(oc, dict));
}

AUTO_REG_NAME(flock_child_view, view::create);


view::view(kernel::object_create_t const& oc, dict_copt dict)
    : base_view_presentation(oc)
    , obj_data_base         (dict)
{

    if (nodes_manager_ = find_first_child<nodes_management::manager_ptr>(this))
    {
        conn_holder() << nodes_manager_->subscribe_model_changed(boost::bind(&view::on_model_changed_internal, this));
    }

    msg_disp()
        .add<msg::settings_msg>(boost::bind(&view::on_settings, this, _1));
}

geo_point_3 view::pos() const
{
    FIXME(fake position)  
    return ::get_base();
}

std::string const& view::name() const
{
    return settings_.icao_code;
}

settings_t const& view::settings() const
{
    return settings_;
}

void view::on_settings(msg::settings_msg const& msg)
{
    settings_ = msg.settings;
    on_new_settings();
}

void view::on_model_changed_internal()
{
    if (nodes_manager_)
    {   
        if (nodes_manager_->get_model() != get_model(name()))
            on_model_changed();
    }
}

}

} // flock 

