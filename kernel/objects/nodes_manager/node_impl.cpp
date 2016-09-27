#include "stdafx.h"
#include "precompiled_objects.h"

#include "nm/node_impl.h"
#include "nodes_manager_common.h"
#include "nodes_manager_view.h"
//#include "reflection/proc/prop_attr.h"

#ifndef OSG_NODE_IMPL
namespace nodes_management
{

node_impl::node_impl( view * manager, node_impl const& parent, model_structure::node_data const& data, uint32_t id )
    : manager_(manager)
    , data_   (data)
    , node_id_(id)
{
    position_.set_local(local_position(manager->object_id(), parent.node_id(), data.pos, data.orien));
    extrapolated_position_ = position_;

    init_disp();
}

node_impl::node_impl( view * manager, geo_position const& pos, model_structure::node_data const& data, uint32_t id )
    : manager_(manager)
    , data_   (data)
    , node_id_(id)
{
    position_.set_global(pos);
    extrapolated_position_ = position_;

    init_disp();
}

node_impl::node_impl( view * manager, binary::input_stream & stream )
    : manager_(manager)
{
    using namespace binary;

    read(stream, node_id_); 
    read(stream, data_); 
    read(stream, position_); 
    read(stream, texture_);

    extrapolated_position_ = position_;
    
    init_disp();
}

void node_impl::save( binary::output_stream& stream ) const
{
    using namespace binary;

    write(stream, node_id_); 
    write(stream, data_); 
    write(stream, position_); 
    write(stream, texture_);
}

void node_impl::pre_update(double time)
{
	if (!time_)
        return;
    
	if(correction_update_time_)
		time -= *correction_update_time_;
	
	last_pre_update_time_ = time;

    double dt = time - *time_;
    if (dt <= 0)
    {
#ifdef TIME_DEBUG
        force_log fl;
#endif
        LOG_ODS_MSG( "pre_update(double time) dt <= 0 node name : " << name() << "  dt=" << dt 
            << " time "   << time
            << " time_ "   << *time_
			<< " node_type: " << (this->as_visual_node()?std::string("visual"):(this->as_model_node()?std::string("model"):std::string("control")))
            << "\n" );
         return;
    }

    last_dt_ = dt;

    if (!position_.is_static())
    {
        if (position_.is_local())
        {                                                                                                                                       
            extrapolated_position_.local().pos   = position_.local().pos + position_.local().ddpos_t(dt); 
            extrapolated_position_.local().orien = cg::quaternion(cg::rot_axis(position_.local().omega * dt)) * position_.local().orien;
            extrapolated_position_reseted();
        }
        else
        {
            extrapolated_position_.global().pos   = position_.global().pos(position_.global().ddpos_t(dt));
            extrapolated_position_.global().orien = cg::quaternion(cg::rot_axis(position_.global().omega * dt)) * position_.global().orien;

            {
	            point_3 e_pos = cg::geo_base_3(get_base())(extrapolated_position_.global().pos);
#ifdef TIME_DEBUG
                force_log fl;
#endif
	            LOG_ODS_MSG( "pre_update(double time) node name :" << name() << " time "   << time  << "  extr_pos_.global().pos: lat:"  <<  e_pos.x << "  lon:" << e_pos.y << "  z: " << e_pos.z << "  dt=" << dt 
	                << " dpos.x "   << position_.global().dpos.x
	                << " dpos.y "   << position_.global().dpos.y
	                << " dpos.z "   << position_.global().dpos.z
	                << " cg::norm " << cg::norm ( position_.global().dpos )
	                //<< " ddpos.x "  << position_.global().ddpos.x
	                //<< " ddpos.y "  << position_.global().ddpos.y
	                //<< " ddpos.z "  << position_.global().ddpos.z
	                << "\n" );
            }
            extrapolated_position_reseted();
        }
    }
}

void node_impl::post_update(double /*time*/)
{
}

node_position const& node_impl::position() const
{
    return extrapolated_position_;
}

transform_4 node_impl::transform  () const
{
    Assert(extrapolated_position_.is_local());
    return transform_4(cg::as_translation(extrapolated_position_.local().pos), cg::rotation_3(extrapolated_position_.local().orien.cpr()));
}

uint32_t node_impl::node_id() const
{
    return node_id_;
}

uint32_t node_impl::object_id() const
{
    return manager_->object_id();
}

string const& node_impl::name() const
{
    return data_.name;
}

node_info_ptr node_impl::rel_node() const
{
    Assert(position_.is_local());

    if (rel_node_)
        return *rel_node_;

    manager * rel_object ;
    if (position_.local().relative_obj == manager_->object_id())
        rel_object = manager_;
    else
        rel_object = manager_ptr(manager_->collection()->get_object(position_.local().relative_obj)).get();

    rel_node_ = rel_object ? rel_object->get_node(position_.local().relative_node) : node_info_ptr();

    return *rel_node_;
}

node_info_ptr node_impl::root_node() const
{
    Assert(position_.is_local());

    node_info_ptr n = rel_node();

    while(n->position().is_local())
        n = n->rel_node();

    return n;
}

transform_4 node_impl::get_root_transform() const
{
    //TODO: cache

    if (extrapolated_position_.is_local())
    {                         
        transform_4 rel_tr = rel_node()->get_root_transform();
        return rel_tr * transform();
    }
    else
        return transform_4();
}


geo_point_3 node_impl::get_global_pos() const
{
    if (extrapolated_position_.is_local())
    {
        transform_4 tr = get_root_transform();
        node_info_ptr root = root_node();
        return root->position().global().pos(root->position().global().orien.rotate_vector(tr.translation()));
    }
    else
        return extrapolated_position_.global().pos;
}

quaternion node_impl::get_global_orien() const
{
    if (extrapolated_position_.is_local())
    {
        transform_4 tr = get_root_transform();
        node_info_ptr root = root_node();
        return root->position().global().orien * quaternion(tr.rotation().cpr());
    }
    else
        return extrapolated_position_.global().orien;
}

model_structure::collision_volume const* node_impl::get_collision() const
{
    if (!collision_ && manager_->get_collision_structure())
    {
        auto const& volumes = manager_->get_collision_structure()->collision_volumes;
        auto it = volumes.find(data_.name);
        collision_.reset(it != volumes.end() ? &*(it->second) : NULL);
    }

    return collision_ ? *collision_ : NULL;
}

/*cg::sphere_3 */cg::rectangle_3   node_impl::get_bound() const
{
   return data_.bound;
}

void node_impl::on_msg(binary::bytes_cref data)
{               
    msg_disp().dispatch_bytes(data);
}

void node_impl::on_position(msg::node_pos_descr const& m)
{
    const node_position np = m.pos.get_pos(position_, m.components);

#if 0
    if(as_visual_node()  && last_pre_update_time_ && *m.time <= *last_pre_update_time_ && cg::norm(np.is_local()?np.local().dpos:np.global().dpos) !=0)
    {
        const double dt =  last_dt_?*last_dt_ * 0.05:0.0025; // *manager_->update_time() - *manager_->last_update_time()/**last_pre_update_time_*/; //last_dt_?*last_dt_ * 0.05:0.0025;
        time_                   = manager_->last_update_time() /*last_pre_update_time_*/;  *time_ += dt; 
        //position_               = m.pos.get_pos(extrapolated_position_, ct_dpos | ct_ddpos | ct_omega | ct_loc_glob);
        //extrapolated_position_  = position_;
        position_               = extrapolated_position_;
		
		force_log fl;
		LOG_ODS_MSG( "node_impl::on_position node name :" << name() << " manager_->update_time"   << time_
			<< "   manager_->last_update_time()" << manager_->last_update_time()
			<< "\n" );


        if (extrapolated_position_.is_local())
        {                                                                                                                                       
            position_.local().dpos = np.local().dpos;
            extrapolated_position_.local().pos   = extrapolated_position_.local().pos + position_.local().ddpos_t(dt); 
            extrapolated_position_.local().orien = cg::quaternion(cg::rot_axis(extrapolated_position_.local().omega * dt)) * extrapolated_position_.local().orien;
        }
        else
        {
            position_.global().dpos = np.global().dpos;
            extrapolated_position_.global().pos   = extrapolated_position_.global().pos(position_.global().ddpos_t(dt));
            extrapolated_position_.global().orien = cg::quaternion(cg::rot_axis(position_.global().omega * dt)) * extrapolated_position_.global().orien;
        }
    }
    else
#endif
#if 1
	if(as_visual_node()  && manager_->last_update_time() && *m.time < *manager_->last_update_time())
	{
			correction_update_time_ = double(*manager_->last_update_time() - *m.time);
#ifdef TIME_DEBUG
			if(!position_.is_local())
			{
				force_log fl;
				LOG_ODS_MSG( "node_impl::on_position  : correction_update_time_"  << correction_update_time_ << "\n" );
			}
#endif
	}
	else
		correction_update_time_.reset();
#endif
    {
        time_                   = m.time;
        position_               = m.pos.get_pos(position_, m.components);
        extrapolated_position_  = position_;
		
#ifdef TIME_DEBUG
		if(!position_.is_local())
		{
			force_log fl;
			LOG_ODS_MSG( "node_impl::on_position  : row set manager_->last_update_time() " << manager_->last_update_time() << "\n" );
		}
#endif
    }

    if (position_.is_local())
    {
        if (rel_node_ && *rel_node_)
        {
            if ((*rel_node_)->object_id() != position_.local().relative_obj || (*rel_node_)->node_id() != position_.local().relative_node)
                rel_node_.reset();
        }
    }
    else
    {
        rel_node_.reset();

        {
            point_3 e_pos = cg::geo_base_3(get_base())(extrapolated_position_.global().pos);
           
#ifdef TIME_DEBUG
            force_log fl;

            LOG_ODS_MSG( "on_position node name :" << name() << " time_ "   << time_  << "  extrapolated_position_.global().pos : lat: "  <<  e_pos.x << "  lon: " << e_pos.y << "  z: " << e_pos.z 
                << " dpos.x "   << position_.global().dpos.x
                << " dpos.y "   << position_.global().dpos.y
                << " dpos.z "   << position_.global().dpos.z       
                << " cg::norm " << cg::norm ( position_.global().dpos )
                //<< " msg.dpos.x "   << np.global().dpos.x
                //<< " msg.dpos.y "   << np.global().dpos.y
                //<< " msg.dpos.z "   << np.global().dpos.z 
                //<< " ddpos.x "  << position_.global().ddpos.x
                //<< " ddpos.y "  << position_.global().ddpos.y
                //<< " ddpos.z "  << position_.global().ddpos.z
                << "\n" );
#endif
        }
    }
    extrapolated_position_reseted();
}

void node_impl::on_texture(msg::node_texture_msg const& m)
{
    texture_ = m.tex;
}

void node_impl::set_position(node_position const& pos)
{   
    manager_->set_node_msg(node_id_, 
        network::wrap_msg(msg::node_pos_msg(
            manager_->update_time(), 
            ct_all, 
            node_pos(pos))));
}

void node_impl::play_animation(string const& seq, double len, double from, double size, double cross_fade)
{
    manager_->send_node_msg(
        node_id_,
        network::wrap_msg(msg::node_animation(
            *manager_->update_time(), 
            seq, 
            (float)len, 
            (float)from, 
            (float)size,
			(float)cross_fade
			)));
}

void node_impl::set_texture(string const& texture)
{
    if (!texture_ || *texture_ != texture)
        manager_->set_node_msg(
            node_id_, 
            network::wrap_msg(msg::node_texture_msg(texture)));
}

void node_impl::set_visibility  (bool visible)
{
    manager_->send_node_msg(
        node_id_, 
        network::wrap_msg(msg::visibility_msg(visible)));

    visibility_ = visible;
}

boost::optional<bool> node_impl::get_visibility()
{
    return visibility_;
}

void node_impl::on_object_created(object_info_ptr object)
{
    if ((!rel_node_ || !*rel_node_) && position_.is_local())
    {               
        if (position_.local().relative_obj == object->object_id())
        {
            manager_ptr rel_object = object;
            rel_node_ = rel_object->get_node(position_.local().relative_node);
        }
    }
}

void node_impl::on_object_destroying(object_info_ptr object)
{           
    if (rel_node_ && *rel_node_)
    {
        if ((*rel_node_)->object_id() == object->object_id())
            rel_node_.reset();
    }
}

model_structure::node_data const& node_impl::data() const 
{ 
    return data_; 
}

void node_impl::extrapolated_position_reseted() 
{
}

network::msg_dispatcher<>& node_impl::msg_disp()
{
    return msg_disp_;
}

void node_impl::init_disp()
{   
    msg_disp()
        .add<msg::freeze_state_msg  >(boost::bind(&node_impl::on_position , this, _1))
        .add<msg::node_pos_msg      >(boost::bind(&node_impl::on_position , this, _1))
        .add<msg::node_texture_msg  >(boost::bind(&node_impl::on_texture  , this, _1))
        .track<msg::node_animation  >()
        .track<msg::visibility_msg  >();
}

}

#endif
