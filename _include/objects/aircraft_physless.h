#pragma once

#include "aircraft_physless_fwd.h"
#include "common/event.h"

namespace aircraft_physless
{
    namespace ums
    {

        struct state_t
        {
            state_t()
                : course(0)
                , fuel_mass  (1)
                , TAS   (0)
                //, cfg   (CFG_CR)
            {}

            state_t(cg::geo_point_3 const& pos, double course, double fuel_mass, double TAS/*, air_config_t cfg*/)
                : pos   (pos)
                , course(course)
                , fuel_mass (fuel_mass)
                , TAS   (TAS)
                //, cfg   (cfg)
            {}

            cg::geo_point_3 pos;
            double course;
            double fuel_mass;
            double TAS;
            //air_config_t cfg;
        };

        struct pilot_state_t
        {
            pilot_state_t(state_t const& dyn_state)
                : dyn_state(dyn_state)
            {}

            pilot_state_t()
            {}

            state_t                 dyn_state;
        };
    }

    struct state_t : ums::pilot_state_t
    {
        state_t();
        state_t(ums::pilot_state_t const& ps, double pitch, double roll, uint32_t version);
        cpr orien() const;

        double pitch;
        double roll;
    };

    inline state_t::state_t()
        : pitch(0)
        , roll(0)
    {}

    inline state_t::state_t(ums::pilot_state_t const& ps, double pitch, double roll, uint32_t version)
        : pilot_state_t(ps)
        , pitch(pitch)
        , roll(roll)
    {}

    inline cpr state_t::orien() const
    {
        return cpr(dyn_state.course, pitch, roll);
    }

    struct info
    {
        virtual ~info(){}

        virtual geo_point_3 const&              pos              () const = 0;
        virtual point_3                         dpos             () const = 0;
        virtual cpr                             orien            () const = 0;
        virtual aircraft::settings_t const &    settings         () const = 0;
        //virtual fpl::info_ptr                   get_fpl          () const = 0;
        //virtual bool                            has_assigned_fpl () const = 0;

        virtual transform_4 const&              tow_point_transform() const = 0;
        virtual nodes_management::node_info_ptr tow_point        () const = 0;

        virtual nodes_management::node_info_ptr root             () const = 0;
        virtual bool                            malfunction      (aircraft::malfunction_kind_t kind) const = 0;

        //virtual aircraft_fms::info_ptr          get_fms          () const = 0;

        virtual optional<double> get_prediction_length() const = 0;
        virtual optional<double> get_proc_length      () const = 0;

        DECLARE_EVENT(assigned_fpl_changed, (fpl::info_ptr));
        DECLARE_EVENT(responder_changed, (fpl::info*));
    };

    struct control
    {
        virtual ~control(){}
        virtual void unassign_fpl() = 0;
        //
        virtual void set_kind(std::string const& kind) = 0;
        virtual void set_turbulence(unsigned turb) = 0;
    };



}

