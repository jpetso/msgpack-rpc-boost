//
// msgpack::rpc::session_impl - MessagePack-RPC for C++
//
// Copyright (C) 2009-2010 FURUHASHI Sadayuki
//
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
//
//        http://www.apache.org/licenses/LICENSE-2.0
//
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//
#ifndef MSGPACK_RPC_SESSION_IMPL_H__
#define MSGPACK_RPC_SESSION_IMPL_H__

#include "session.h"
#include "reqtable.h"
#include "protocol.h"
#include "transport_impl.h"
#include "impl_fwd.h"

#include <boost/asio/deadline_timer.hpp>
#include <memory>

namespace msgpack {
namespace rpc {


class session_impl : public std::enable_shared_from_this<session_impl>
{
public:
    static shared_session create(const builder& b, const address addr, loop lo);

    ~session_impl();

private:
    session_impl(const address& addr, loop lo);
    void build(const builder& b);

public:
    const address& get_address() const {
        return m_addr;
    }

    loop get_loop() {
        return m_loop;
    }

    void set_timeout(unsigned int sec) {
        m_timeout = sec;
    }

    unsigned int get_timeout() const {
        return m_timeout;
    }

    msgid_t next_msgid();

public:
    future send_request_impl(msgid_t msgid, std::string m, sbuffer* sbuf);
    future send_request_impl(msgid_t msgid, std::string m, auto_vreflife vbuf);

    void send_notify_impl(sbuffer* sbuf);
    void send_notify_impl(auto_vreflife vbuf);

public:
    void on_notify(object method, object params, auto_zone z);
    void on_response(msgid_t msgid, object result, object error, auto_zone z);

    void on_connect_failed();
    void on_system_error(const boost::system::error_code& err);

private:
    void arm_step_timer();
    void disarm_step_timer();
    void step_timer_handler(const boost::system::error_code& err);
    void step_timeout(std::vector<shared_future>* timedout);

private:
    address m_addr;

    loop m_loop;

    std::unique_ptr<client_transport> m_tran;

    msgid_t m_msgid_rr;
    reqtable m_reqtable;

    unsigned int m_timeout;
    boost::asio::deadline_timer m_step_timer;

private:
    session_impl();
    session_impl(const session_impl&);
};


}  // namespace rpc
}  // namespace msgpack

#endif /* msgpack/rpc/session_impl.h */
