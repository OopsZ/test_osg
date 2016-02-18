#include "stdafx.h"

#include "av/Visual.h"

#include "async_services/async_services.h"
#include "network/msg_dispatcher.h"
#include "kernel/systems.h"
#include "kernel/systems/systems_base.h"
#include "kernel/systems/fake_system.h"
#include "kernel/object_class.h"
#include "kernel/msg_proxy.h"
#include "kernel/systems/vis_system.h"
#include "common/test_msgs.h"
#include "common/time_counter.h"
#include "utils/high_res_timer.h"
#include "tests/systems/factory_systems.h"
#include "objects/registrator.h"

#include "net_layer/net_worker.h"

using network::endpoint;
using network::async_acceptor;
using network::async_connector;
using network::tcp_socket;
using network::error_code;

typedef boost::system::error_code       error_code_t;

using network::msg_dispatcher;
using network::tcp_fragment_wrapper;

using namespace net_layer::msg;

namespace
{
    boost::asio::io_service* __main_srvc__ = 0;


    void tcp_error(error_code const& err)
    {
        LogError("Error happened: " << err);
        __main_srvc__->stop();
    }

    struct updater
    {
       updater(kernel::system_ptr  sys,double period = 0.001)
          : period_(period)
          , old_val(0)
          , sys_(sys)
       {

       }

       __forceinline void update(double time)
       {
           if(time - old_val >=period_)
           {
               sys_->update(time);
               old_val = time;
           }
       }

       __forceinline void reset()
       {
           sys_.reset();
       }

       __forceinline kernel::system_ptr get_sys()
       {
           return sys_;
       }

    private:
       double                                                   period_; 
       double                                                   old_val;
       kernel::system_ptr                                          sys_;
    };

}

namespace
{

struct net_worker
{
     typedef boost::function<void(const void* data, size_t size)>   on_receive_f;
     typedef boost::function<void(double time)>                     on_update_f;     
     

     net_worker(const  endpoint &  peer, const std::vector<endpoint>& vis_peers , on_receive_f on_recv , on_update_f on_update)
         : period_     (0.05)
         , ses_        (net_layer::create_session(binary::bytes_t(), true))
         , peer_       (peer)
         , vis_peers_  (vis_peers)
         , on_receive_ (on_recv)
         , on_update_  (on_update)
         , done_       (false)
     {
          worker_thread_ = boost::thread(&net_worker::run, this);
     }
     
     ~net_worker()
     {
         //  delete  ses_;
         //_workerService->post(boost::bind(&boost::asio::io_service::stop, _workerService));
         worker_thread_.join();
     }

     boost::asio::io_service* GetService()
     {
         return worker_service_;
     }

     void done()
     {
         done_ = true;
     }

     void send(void const* data, uint32_t size)
     {
         worker_service_->post(boost::bind(&net_worker::do_send, this, 0, binary::make_bytes_ptr(data, size)));
     }
     
     void send (binary::bytes_cref bytes)
     {
         worker_service_->post(boost::bind(&net_worker::do_send, this, 0, binary::make_bytes_ptr(&bytes[0], bytes.size())));
     }

     void reset_time(double new_time)
     {
         ses_->reset_time(new_time);
     }

     void set_factor(double factor)
     {
         ses_->set_factor(factor);
     }

private:
     void run()
     {
         async_services_initializer asi(false);

         acc_.reset(new  async_acceptor (peer_, boost::bind(&net_worker::on_accepted, this, _1, _2), tcp_error));
         
         worker_service_ = &(asi.get_service());
         
         boost::asio::io_service::work skwark(asi.get_service());


         calc_timer_   = ses_->create_timer ( period_, boost::bind(&net_worker::on_timer, this ,_1) , 1, false);

         
		 boost::system::error_code ec;
         size_t ret = worker_service_->run(ec);

         acc_.reset();
         sockets_.clear();
         srv_.reset();
     }

     uint32_t next_id()
     {
         static uint32_t id = 0;
         return id++;
     }

     // from struct tcp_connection
#if 0
     void do_send(int id, binary::bytes_ptr data)
     {   
         size_t size = binary::size(*data);
         error_code_t ec;

         sockets_[id]->send(binary::raw_ptr(*data), size);
         if (ec)
         {
             LogError("TCP send error: " << ec.message());
             return;
         }

     }
#endif

     void do_send(int id, binary::bytes_ptr data)
     {   
         size_t size = binary::size(*data);
         error_code_t ec;

         srv_->send(binary::raw_ptr(*data), size);
         if (ec)
         {
             LogError("TCP send error: " << ec.message());
             return;
         }

     }

private:

     void on_timer(double time)
     {
         if(done_)
         {
             ses_->stop();
             return;
         }

         on_update_(time);
     }
     

     void on_accepted(network::tcp::socket& sock, endpoint const& peer)
     {
         if (!srv_)
         {
             srv_ = std::shared_ptr<tcp_fragment_wrapper>(new tcp_fragment_wrapper(
                 sock, boost::bind(&net_worker::on_recieve, this, _1, _2, peer),
                 boost::bind(&net_worker::on_disconnected, this, _1, peer),
                 boost::bind(&net_worker::on_error, this, _1, peer)));  

            con_.reset(  new async_connector(vis_peers_[0], boost::bind(&net_worker::on_connected, this, _1, _2), tcp_error, tcp_error));

            LogInfo("Client " << peer << " accepted");
         }
         else
            LogError("Client " << peer << " rejected. Connection already esteblished");
     }

     void on_recieve(const void* data, size_t size, endpoint const& peer)
     {
          __main_srvc__->post(boost::bind(&net_worker::do_receive, this, binary::make_bytes_ptr(data, size)) );
     }
     
     void do_receive(binary::bytes_ptr data/*, endpoint const& peer*/) 
     {
         if (on_receive_)
             on_receive_(binary::raw_ptr(*data), binary::size(*data)/*, peer*/);
     }

     void on_disconnected(error_code const& ec)      
     {
         LogInfo("Client " << " disconnected with error: " << ec.message() );
         delete  ses_;
         worker_service_->post(boost::bind(&boost::asio::io_service::stop, worker_service_));
         //_workerThread.join();
     }

     void on_disconnected(boost::system::error_code const& ec, endpoint const& peer)
     {  
         LogInfo("Client " << peer.to_string() << " disconnected with error: " << ec.message() );
         sockets_.erase(peer);
         // peers_.erase(std::find_if(peers_.begin(), peers_.end(), [sock_id](std::pair<id_type, uint32_t> p) { return p.second == sock_id; }));
         // delete  ses_;
         worker_service_->post(boost::bind(&boost::asio::io_service::stop, worker_service_));
         delete  ses_;
         __main_srvc__->post(boost::bind(&boost::asio::io_service::stop, __main_srvc__));
         // _workerThread.join();
     }

     void on_error(boost::system::error_code const& ec, endpoint const& peer)
     {
         LogError("TCP error: " << ec.message());
         sockets_.erase(peer);
         // peers_.erase(std::find_if(peers_.begin(), peers_.end(), [sock_id](std::pair<id_type, uint32_t> p) { return p.second == sock_id; }));
     }


private:
    void on_connected(network::tcp::socket& sock, network::endpoint const& peer)
    {
        LogInfo("Connected to " << peer);

        sockets_[peer] = std::shared_ptr<tcp_fragment_wrapper>(new tcp_fragment_wrapper(
            sock, boost::bind(&net_worker::on_recieve, this, _1, _2, peer), &tcp_error, &tcp_error));  

    }

private:
    boost::scoped_ptr<async_connector>                                       con_;
    std::map<network::endpoint, std::shared_ptr<tcp_fragment_wrapper> >  sockets_;
    const std::vector<endpoint>&                                       vis_peers_;  

private:
    boost::thread                                                  worker_thread_;
    boost::asio::io_service*                                      worker_service_;
    std::shared_ptr<boost::asio::io_service::work>                          work_;
    const  endpoint                                                         peer_;

private:
    std::shared_ptr<tcp_fragment_wrapper>                                    srv_;
    boost::scoped_ptr<async_acceptor>                                        acc_;
                  
private:
    on_receive_f                                                      on_receive_;
    on_update_f                                                        on_update_;

private:
    bool                                                                    done_;

private:
    double                                                                period_;

private:
    net::timer_connection                                            calc_timer_ ;
    net::timer_connection                                             ctrl_timer_;
    net::ses_srv*                                                            ses_;
};


struct mod_app
{
    typedef boost::function<void(run const& msg)>                   on_run_f;
    typedef boost::function<void(container_msg const& msg)>   on_container_f;

    mod_app(endpoint peer, boost::function<void()> eol/*,  binary::bytes_cref bytes*/)
        : systems_  (get_systems())
        , ctrl_sys_ (systems_->get_control_sys())
        , mod_sys_  (systems_->get_model_sys  ())
        , end_of_load_(eol)
        , disp_     (boost::bind(&mod_app::inject_msg      , this, _1, _2)) 
    {   

        disp_
            .add<setup                 >(boost::bind(&mod_app::on_setup      , this, _1))
            .add<create                >(boost::bind(&mod_app::on_create     , this, _1))
            .add<state                 >(boost::bind(&mod_app::on_state      , this, _1))
            ;

        vis_peers_.push_back(endpoint(std::string("127.0.0.1:45002")));

        w_.reset (new net_worker( peer, vis_peers_
            , boost::bind(&msg_dispatcher<uint32_t>::dispatch, &disp_, _1, _2, 0/*, id*/)
            , boost::bind(&mod_app::update, this, _1)
            ));


    }

    ~mod_app()
    {
        w_.reset();
    }

private:

    void on_timer(double time)
    {   
    }

    void update(double time)
    {   
        systems_->update_messages();
        mod_sys_->update(time);
        ctrl_sys_->update(time);
    }


    void on_setup(setup const& msg)
    {
        w_->set_factor(0.0);

        create_objects(msg.icao_code);
        
        if(end_of_load_)
            end_of_load_();  //osg_vis_->EndSceneCreation();

        binary::bytes_t bts =  std::move(wrap_msg(ready_msg(0)));
        w_->send(&bts[0], bts.size());
    }

    void on_state(state const& msg)
    {
       w_->set_factor(msg.factor);
       w_->reset_time(msg.srv_time / 1000.0f);
    }

    void on_create(create const& msg)
    {
		reg_obj_->create_object(msg);

        LogInfo("Got create message: " << msg.model_name << "   " << (short)msg.object_kind << "   "<< msg.orien.get_course() << " : " << msg.pos.x << " : " << msg.pos.y  );

    }


    void inject_msg(const void* data, size_t size)
    {
        if (reg_obj_)
        {
            reg_obj_->inject_msg(data, size);
        }
    }

    void create_objects(const std::string& airport)
    {
        using namespace binary;
        using namespace kernel;

        high_res_timer hr_timer;

        systems_->create_auto_objects();

        force_log fl;       
        LOG_ODS_MSG( "create_objects(const std::string& airport): create_auto_objects() " << hr_timer.set_point() << "\n");

        auto fp = fn_reg::function<void(const std::string&)>("create_objects");
        if(fp)
            fp(airport);

        force_log fl2;  
        LOG_ODS_MSG( "create_objects(const std::string& airport): create_objects " << hr_timer.set_point() << "\n");


        reg_obj_ = objects_reg::control_ptr(find_object<object_info_ptr>(dynamic_cast<kernel::object_collection*>(ctrl_sys_.get()),"aircraft_reg")) ;   

        if (reg_obj_)
        {
            void (net_worker::*send_)       (binary::bytes_cref bytes)              = &net_worker::send;

            reg_obj_->set_sender(boost::bind(send_, w_.get(), _1 ));
        }

    }


private:
    systems_ptr                                                 systems_;
    kernel::system_ptr                                          mod_sys_;
    kernel::system_ptr                                         ctrl_sys_;

private:
    msg_dispatcher<uint32_t>                                       disp_;
    std::vector<endpoint>                                      vis_peers_;

private:
    on_container_f                                              on_cont_;
    boost::function<void()>                                 end_of_load_;
private:
    boost::scoped_ptr<net_worker>                                     w_;

private:
    objects_reg::control_ptr                                   reg_obj_;

};


}

int main_modapp( int argc, char** argv )
{
    
    logger::need_to_log(/*true*/);
    logging::add_console_writer();

    boost::asio::io_service  service_;
    typedef boost::shared_ptr<boost::asio::io_service::work> work_ptr;
    work_ptr dummy_work(new boost::asio::io_service::work(service_));
    

    __main_srvc__ = &service_;

    try
    {

        endpoint peer(cfg().network.local_address);

        kernel::vis_sys_props props;
        props.base_point = ::get_base();

        mod_app ma(peer,boost::function<void()>());

        boost::system::error_code ec;
        __main_srvc__->run(ec);

    }
    catch(verify_error const& err)
    {
        std::string serr(err.what());
        LogInfo( serr );
        return -1;
    }
    catch(std::exception const& err)
    {
        std::string errror (err.what()); 
        LogError("Exception caught: " << err.what());
        return -1;
    }

    return 0; 

}

AUTO_REG(main_modapp)
