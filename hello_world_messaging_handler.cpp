#include <proton/connection.hpp>
#include <proton/default_container.hpp>
#include <proton/delivery.hpp>
#include <proton/messaging_handler.hpp>
#include <proton/url.hpp>
#include <proton/transport.hpp>
#include <proton/event_loop.hpp>
#include <proton/duration.hpp>

#include <iostream>

class hello_world_messaging_handler : public proton::messaging_handler {
private:
    std::string url;
    std::vector<std::string> msg_queue;

    proton::sender sender;
    //proton::receiver receiver;
    //proton::connection _conn;
    //proton::session _session;

public:
    hello_world_messaging_handler(const std::string &u, const std::vector<std::string> &r) : url(u), msg_queue(r) {
        std::cout << "new handler with url: " + u << std::endl;
    }

    void on_container_start(proton::container& c) override {
        std::cout << "on_container_start" << std::endl;
	
        sender = c.open_sender(url/*, proton::sender_options().auto_settle(true)*/);
    }

    void on_connection_open(proton::connection &c) override {
        std::cout << "on_connection_open" << std::endl;

        /**
        std::shared_ptr<proton::thread_safe<proton::connection> > ts_c = make_shared_thread_safe(c);
        proton::event_loop loop = ts_c->event_loop();
        loop.inject([](){
          std::cout << "tottoot" << std::endl;
        });
        */
    }

    int count = 5;
    void on_sendable(proton::sender &s) override {
        std::cout << "on_sendable" << std::endl;
        try {
            try_to_send(s);
        }
        catch(const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }

        //that code works (loop on on_sendable) re-opens connections
        //s.close();
        //s.container().open_sender(url);

        //that code does not work and end up with amqp:not found (on_sender_error)
        //s.close();
        //s.connection().open_sender(url);

        //that code does not loop on on_sendable
        //s.close();
        //s.open();
        //std::cout << "on_sendable_end for " << url << std::endl;
    }

    bool b_try = true;
    void try_to_send(proton::sender &s) {
        try {
            if (!msg_queue.empty()) {
                do {
                    proton::message m(msg_queue.front());
		    m.durable(false);
                    s.send(m);

                    //TODO not thread-safe
                    msg_queue.erase(msg_queue.begin());
                }
                while(!msg_queue.empty());

                //proton::message m("hello world");
                //s.send(m);

                b_try = true;
            } else {
                if(b_try) {
                    //std::cout<< "Nothing to send" << std::endl;
                    std::cout << "try_to_send - [" << s.source().address() << "]" << "[" << s.target().address() << "]" << "[" << s.credit() << "]" << std::endl;
                    b_try = false; //to avoid logging multiple times...
                }
            }

            if(s.credit() != 0) {
                //in all cases, reschedule an execution of the container
                proton::duration timeout(10);

                s.container().schedule(timeout, [this]() {
                    this->on_timer();
                });
            }
        }
        catch(const std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }

    void on_timer() {
        //std::cout << "on_timer" << std::endl;
        try_to_send(sender);
    }

    void on_message(proton::delivery &d, proton::message &m) override {
        std::cout << "on_message" << m.body() << std::endl;
    }

    void on_transport_error(proton::transport &t) override {
        std::cout << "on_transport_error" << t.error().description() << std::endl;
    }

    void on_connection_error(proton::connection &c) override {
        std::cout << "on_connection_error" << std::endl;
    }

    void on_session_error(proton::session &s) override {
        std::cout << "on_session_error" << std::endl;
    }

    void on_session_open(proton::session &s) override {
        std::cout << "on_session_open" << std::endl;
    }

    void on_session_close(proton::session &s) override {
        std::cout << "on_session_close" << std::endl;
    }

    void on_receiver_error(proton::receiver& l) override {
        std::cout << "on_receiver_error" << std::endl;
    }

    void on_sender_error(proton::sender& l) override {
        std::cout << "on_sender_error " << l.error().description() << l.error().name() << l.error().what() << std::endl;
    }

    void on_error(const proton::error_condition &c) override {
        std::cout << "on_error" << std::endl;
    }

    void on_tracker_accept(proton::tracker &d) override {
        std::cout << "on_tracker_accept" << std::endl;
    }

    void on_tracker_settle(proton::tracker &d) override {
        std::cout << "on_tracker_settle" << std::endl;
        //d.settle();
        //d.sender().return_credit();
    }

    void on_tracker_reject(proton::tracker &d) override {
        std::cout << "on_tracker_reject" << std::endl;
    }

    void on_tracker_release(proton::tracker &d) override {
        std::cout << "on_tracker_release" << std::endl;
    }

    void on_delivery_settle(proton::delivery &d) override {
        std::cout << "on_delivery_settle" << std::endl;
    }


    /**
     *
     */
    void add_message(std::string msg) {
      //TODO make it thread-safe
        msg_queue.push_back(msg);
    }

    /**
     * not good. the underlying objects like proton::sender are not thread-safe
     */
    //void send(std::string msg) {
    //    proton::message p_msg;
    //    p_msg.body(msg);
    //    sender.send(p_msg);
    //    //std::cout << "send msg: " << msg << std::endl;
    //}
};

/**
 *
 */
class message_mock {
private:

public:
    message_mock() {}
    /**
     *
     */
    std::vector<std::string> get_requests() {
        std::vector<std::string> requests;
        std::string str;
        for(int i = 0; i < 4; i++) {
            str = "msg_";
            requests.push_back(str.append(std::to_string(i)));
        }

        return requests;
    }
};


//auto do_something(){
//  std::cout << "I am doing something" << std::endl;
//}

/**
 *
 */
int main(int argc, char **argv) {
    try {
        std::string url = "amqp://admin:admin@localhost:5672/examples";
        //
        message_mock store;
        std::vector<std::string> requests = store.get_requests();
        hello_world_messaging_handler hw(url, requests);


        //Fill in the requests vector every xx milli_sec
        int interval = 1000;

        std::thread t([&hw, interval]() {
	    int seq = 0;
            try {
                while(true) {
                    //do_something();
                    //std::cout << "Adding new message \n";
                    hw.add_message("msg" + std::to_string(seq++));
                    std::this_thread::sleep_for<>(std::chrono::milliseconds(interval));
                }
            }
            catch(const std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
        });
        t.detach();

        //start the container in the main thread
        proton::default_container container = proton::default_container(hw);
        //proton::container_impl container = proton::container_impl(hw);
        //container.auto_stop(false);


        /**std::shared_ptr<proton::thread_safe<proton::connection> > ts_c = proton::make_shared_thread_safe(c);
        proton::event_loop loop = ts_c->event_loop();
        loop.inject([](){
          std::cout << "tottoot" << std::endl;
        });*/

        std::cout << "start container: " << container.id() << std::endl;
        container.run();

        return 0;
    }
    catch(const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    return 1;
}