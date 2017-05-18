#include "match.h"
#include <config.h>
#include <db_auto.h>
#include <room.h>
#include <engine.h>

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
        auto a = groups.lower_bound(score);
        // create new group
        if (a == groups.end() || a->first + gap < score)
        {
            groups[score].push_back(ptr);
            map[ptr].first  = &groups[score];
            map[ptr].second = 0;
        }
        else
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
    (*ptr)->resign_port(config::match_status_port)->start();
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
std::map< std::string, lsy::queue > queues;

using db_gen::main;

void lsy::add_to_queue(std::string str, player* ptr)
{
    auto it = queues.find(str);
    if (it != queues.end())
    {
        main.get_score.bind_once([ ptr, it, &io = io_service ](bool is_data) {
            int score = main.get_score[0];
            io.post([score, ptr, it]() { it->second.add_player(ptr, score); });
        },
                                 ptr->id, str);
    }
    else
    {
        main.get_room_size.bind_once(
            [ str, ptr, &io = io_service ](bool is_data) {
                assert(is_data);
                int n = main.get_room_size[0];
                io.post([n, str, ptr]() {
                    auto it = queues.insert(
                        std::make_pair(str, lsy::queue(str, n, 100)));
                    main.get_score.bind_once(
                        [ ptr, it, &io = io_service ](bool is_data) {
                            int score = main.get_score[0];
                            io.post([score, ptr, it]() {
                                it.first->second.add_player(ptr, score);
                            });
                        },
                        ptr->id, str);
                });
            },
            str);
    }
}

void lsy::remove_from_queue(std::string str, player* ptr)
{
    auto it = queues.find(str);
    assert(it != queues.end());
    it->second.remove_player(ptr);
}
