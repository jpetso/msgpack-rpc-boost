//
// msgpack::rpc::session_pool - MessagePack-RPC for C++
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
#include "loop.h"
#include "session_pool_impl.h"
#include "session_impl.h"
#include "future_impl.h"
#include "exception_impl.h"
#include "transport/tcp.h"

#include <functional>

namespace msgpack {
namespace rpc {


static const unsigned int SESSION_POOL_TIME_LIMIT = 60;


session_pool_impl::session_pool_impl(const builder& b, loop lo) :
    m_loop(lo),
    m_builder(b.copy()),
    m_step_timer(m_loop->io_service())
{
    arm_step_timer();
}

session_pool_impl::~session_pool_impl()
{
    disarm_step_timer();
}

session session_pool_impl::get_session(const address& addr)
{
    boost::mutex::scoped_lock lk(m_table_mutex);

    std::map<address, entry_t>::iterator found;
    found = m_table.find(addr);
    if (found != m_table.end()) {
        found->second.ttl = SESSION_POOL_TIME_LIMIT;
        return session(found->second.session);
    }

    shared_session s = session_impl::create(*m_builder, addr, m_loop);
    m_table.insert(std::pair<address, entry_t>
        (addr, entry_t(s, SESSION_POOL_TIME_LIMIT)));

    return session(s);
}

void session_pool_impl::set_timeout(unsigned int sec)
{
    m_builder->set_timeout(sec);
}

void session_pool_impl::arm_step_timer()
{
    m_step_timer.expires_from_now(boost::posix_time::seconds(1));
    m_step_timer.async_wait(std::bind(
            &session_pool_impl::step_timer_handler, this, std::placeholders::_1));
}

void session_pool_impl::disarm_step_timer()
{
    m_step_timer.cancel();
}

void session_pool_impl::step_timer_handler(const boost::system::error_code& err)
{
    if (err == boost::asio::error::operation_aborted) {
        return;
    }

    boost::mutex::scoped_lock lk(m_table_mutex);
    std::vector<shared_future> timedout;

    std::map<address, entry_t>::iterator it = m_table.begin();
    while (it != m_table.end()) {
        entry_t& e = it->second;
        if (e.session.unique()) {
            // There are no contexts that references the session.
            if (e.ttl <= 0) {
                // If e.session.unique() is true, m_pimpl->m_reqtable is empty
                // because it contains futures that references a session.
                m_table.erase(it++);
                continue;
            }
            --e.ttl;
        }
        ++it;
    }

    arm_step_timer();
}

// session pool

session_pool::session_pool(loop lo) :
    m_pimpl(new session_pool_impl(tcp_builder(), lo))
{
}

session_pool::session_pool(const builder& b, loop lo) :
    m_pimpl(new session_pool_impl(b, lo))
{
}

session_pool::session_pool(shared_session_pool pimpl) :
    m_pimpl(pimpl)
{
}

session_pool::~session_pool()
{
    end();
    join();
}

session session_pool::get_session(const address& addr)
{
    session s = m_pimpl->get_session(addr);
    return s;
}

session session_pool::get_session(const std::string& host, uint16_t port)
{
    return get_session(address(host, port));
}

session session_pool::get_session(const std::string& addr)
{
    return get_session(address(addr));
}

loop session_pool::get_loop()
{
    return m_pimpl->get_loop();
}

void session_pool::start(size_t num)
{
    get_loop()->start(num);
}

void session_pool::run(size_t num)
{
    get_loop()->run(num);
}

void session_pool::run_once()
{
    get_loop()->run_once();
}

void session_pool::end()
{
    get_loop()->flush();
    get_loop()->end();
}

void session_pool::join()
{
    get_loop()->join();
}

bool session_pool::is_running()
{
    return get_loop()->is_running();
}

void session_pool::set_timeout(unsigned int sec)
{
    return m_pimpl->set_timeout(sec);
}

}  // namespace rpc
}  // namespace msgpack
