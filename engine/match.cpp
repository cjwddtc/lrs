#include "match.h"
#include <config.h>
#include <db_auto.h>
#include <engine.h>
#include <room.h>

extern thread_local boost::asio::io_service io_service;
void lsy::queue::add_player(player* ptr, uint16_t score)
{
    if (size == 1)
    {
        group a;
        a.push_back(ptr);
        server_ptr->create_room(room_name, a);
    }
    else
    {
        uint16_t low = score - gap / 2;
        if (score < gap / 2)
        {
            low = 0;
        }
        uint16_t hight = score + gap / 2;
        auto     a     = groups.upper_bound(low);
        auto     b     = groups.upper_bound(hight);
        if (a != b)
        {
            a->second.push_back(ptr);
            if (a->second.size() == size)
            {
                server_ptr->create_room(room_name, a->second);
                groups.erase(a);
            }
            else
            {
                map[ptr].first  = &a->second;
                map[ptr].second = a->second.size() - 1;
            }
        }
        else
        {
            // create new group
            auto it = groups.emplace(std::make_pair(score, group()));
            it.first->second.push_back(ptr);
            map[ptr].first  = &it.first->second;
            map[ptr].second = 0;
        }
    }
}

void lsy::queue::remove_player(player* ptr)
{
    auto p = map[ptr].first;
    std::swap(p->at(map[ptr].second), p->back());
    p->pop_back();
    map.erase(ptr);
}

lsy::queue::queue(std::string room_name_, size_t size_, uint16_t gap_)
    : room_name(room_name_)
    , size(size_)
    , gap(gap_)
{
}

void lsy::group::push_back(player* ptr)
{
    std::vector< player* >::push_back(ptr);
    for (auto a : *this)
    {
        (*a)->ports[config::match_status_port]->write((uint16_t)size(),
                                                      []() {});
    }
}

void lsy::group::pop_back()
{
    (*back())->ports[config::match_status_port]->close();
    std::vector< player* >::pop_back();
    for (auto a : *this)
    {
        (*a)->ports[config::match_status_port]->write((uint16_t)size(),
                                                      []() {});
    }
}
std::map< std::string, lsy::queue >          queues;
std::map< std::string, lsy::queue_no_score > noscore_queues;

using db_gen::main;

void lsy::add_to_queue(std::string str, player* ptr, bool have_score)
{
    if (have_score)
    {
        auto it = queues.find(str);
        if (it != queues.end())
        {
            main.get_score.bind_once(
                [ ptr, it, &io = io_service ](bool is_data) {
                    if (is_data)
                    {
                        int score = main.get_score[0];
                        io.post([score, ptr, it]() {
                            it->second.add_player(ptr, score);
                            (*ptr)->ports[config::match_port]->write(
                                buffer(uint16_t(it->second.size)), []() {});
                        });
                    }
                    else
                    {
                        (*ptr)->ports[config::match_port]->write(
                            buffer(uint16_t(0)), []() {});
                    }
                },
                ptr->id, str);
        }
        else
        {
            main.get_room_size.bind_once(
                [ str, ptr, &io = io_service, have_score ](bool is_data) {
                    assert(is_data);
                    int n = main.get_room_size[0];
                    io.post([n, str, ptr, have_score]() {
                        auto it = queues.insert(
                            std::make_pair(str, lsy::queue(str, n, 500)));
                        main.get_score.bind_once([ ptr, it, &io = io_service ](
                                                     bool is_data) {
                            if (is_data)
                            {
                                int score = main.get_score[0];
                                io.post([score, ptr, it]() {
                                    it.first->second.add_player(ptr, score);
                                    (*ptr)->ports[config::match_port]->write(
                                        buffer(uint16_t(it.first->second.size)),
                                        []() {});
                                });
                            }
                            else
                            {
                                (*ptr)->ports[config::match_port]->write(
                                    buffer(uint16_t(0)), []() {});
                            }
                        },
                                                 ptr->id, str);
                    });
                },
                str);
        }
    }
    else
    {
        auto it = noscore_queues.find(str);
        if (it != noscore_queues.end())
        {
            it->second.add_player(ptr);
            (*ptr)->ports[config::match_noscore_port]->write(
                buffer(uint16_t(it->second.size)), []() {});
        }
        else
        {
            main.get_room_size.bind_once(
                [ str, ptr, &io = io_service, have_score ](bool is_data) {
                    assert(is_data);
                    int n = main.get_room_size[0];
                    io.post([n, str, ptr, have_score]() {
                        auto it = noscore_queues.emplace(
                            std::make_pair(str, lsy::queue_no_score(str, n)));
                        it.first->second.add_player(ptr);
                        (*ptr)->ports[config::match_noscore_port]->write(
                            buffer(uint16_t(it.first->second.size)), []() {});
                    });
                },
                str);
        }
    }
}

void lsy::remove_from_queue(std::string str, player* ptr)
{
    auto it = queues.find(str);
    assert(it != queues.end());
    it->second.remove_player(ptr);
}

lsy::queue_no_score::queue_no_score(std::string room_name_, size_t size_)
    : room_name(room_name_)
    , size(size_)
{
}

void lsy::queue_no_score::add_player(player* ptr)
{
    gr.push_back(ptr);
    map[ptr] = gr.size() - 1;
    if (gr.size() == size)
    {
        server_ptr->create_room(room_name, gr);
        map.clear();
        gr.clear();
    }
}

void lsy::queue_no_score::remove_player(player* ptr)
{
}
