#include "room.h"
#include <algorithm>
#include <boost/asio/deadline_timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <config.h>
#include <db_auto.h>
#include <iostream>
#include <lua_bind.h>
#include <memory>
#include <string>
using db_gen::main;
extern thread_local boost::asio::io_service io_service;
using namespace std::string_literals;

using namespace room_space;
/*
*/


room_space::room::~room()
{
}

void room::is_dead(uint8_t index, bool is_dead)
{
    players[index].is_dead = is_dead;
    lsy::buffer buf(size_t(2));
    uint8_t     fl = is_dead;
    buf.put(&index, 1);
    buf.put(&fl, 1);
    for (auto& p : players)
    {
        (*p)->ports[config::player_dead]->write(buf, []() {});
    }
}

room::room(std::string room_name_, std::vector< lsy::player* > vec)
    : room_name(room_name_)
    , count(vec.size())
    , ls(luaL_newstate())
{
    luaL_openlibs(ls);
    lua_put(ls, this);
    setglobal(ls, "room"s);
    players.reserve(vec.size());
    // init room rule lua script
    main.get_room_rule.bind_once([ this, &io = io_service ](bool have_data) {
        assert(have_data);
        std::string str = main.get_room_rule[0];
        OnInit.connect(std::bind(load_file(str), nullptr));
    },
                                 room_name);
    // init room role
    main.get_room_role.bind([ this, &io = io_service,
                              vec = std::move(vec) ](bool is_fin) {
        if (!is_fin)
        {
            std::string role_name = main.get_room_role[0];
            int         count     = main.get_room_role[1];
            io.post([this, role_name, count]() {
                int c = count;
                while (c--)
                {
                    role_names.push_back(role_name);
                }
            });
        }
        else
        {
            io.post([ this, vec = std::move(vec) ]() {
                uint8_t index = 0;
                for (auto a : vec)
                {
                    players.emplace_back(this, a->operator->(), index++, a->id);
                }
                assert(role_names.size() == vec.size());
                std::random_shuffle(role_names.begin(), role_names.end());
                auto name_it = role_names.begin();
                for (auto& pl : players)
                {
                    std::string role_name(*name_it);
                    size_t      s = role_name.find_first_of('.');
                    if (s != std::string::npos)
                    {
                        std::string role_ver(role_name.begin() + s + 1,
                                             role_name.end());
                        role_name.resize(s);
                        main.get_role_ver.bind_once(
                            [ this, &io = io_service, ptr = &pl ](bool is_fin) {
                                if (is_fin)
                                {
                                    std::string filename = main.get_role_ver[0];
                                    OnInit.connect(
                                        std::bind(load_file(filename), ptr));
                                }
                            },
                            role_name, role_ver);
                    }
                    main.get_role.bind_once(
                        [ this, &io = io_service, ptr = &pl,
                          name = *name_it ](bool is_fin) {
                            if (is_fin)
                            {
                                std::string filename = main.get_role[0];
                                OnInit.connect(
                                    std::bind(load_file(filename), ptr));
                                count--;
                                if (count == 0)
                                {
                                    io.post([this]() { OnInit(); });
                                }
                            }
                        },
                        role_name);
                    ++name_it;
                }
            });
        }
    },
                            room_name);
}

signals* room::signals()
{
    return &sig;
}

channels* room::channels()
{
    return &chs;
}
void room_space::room::wait(int time, std::function< void() > func)
{
    auto t = std::make_shared< boost::asio::deadline_timer >(
        io_service, boost::posix_time::seconds(time));
    auto pt = t.get();
    pt->async_wait([t, func](auto a) { func(); });
}

player* room_space::room::get_player(uint8_t index)
{
    return &players[index % players.size()];
}

uint8_t room_space::room::size()
{
    return players.size();
}

void room_space::room::sent_public(std::string mes)
{
    replay << mes;
    log += mes;
    for (auto& a : players)
    {
        a.sent_public(mes);
    }
}

std::string& room_space::room::get_role(uint8_t index)
{
    return role_names[index];
}

void room_space::room::close(uint8_t camp)
{
    size_t size  = 0;
    int    index = 0;
    for (auto& p : players)
    {
        size += p.id.size() + 1;
        size += role_names[index++].size() + 1;
        size++;
    }
    lsy::buffer result(size);
    index = 0;
    for (auto& p : players)
    {
        result.put(p.id);
        result.put(role_names[index++]);
        uint8_t flag = p.camp == camp;
        result.put(&flag, 1);
    }
    index            = 0;
    std::string repl = replay.str();
    for (auto& p : players)
    {
        if (p.camp == camp)
        {
            main.add_score.bind([](bool is) {}, p.id, room_name);
        }
        else
        {
            main.dec_score.bind([](bool is) {}, p.id, room_name);
        }
        auto        po  = p.pl->resign_port(config::game_result);
        std::string str = p.id;
        str += "(";
        str += role_names[index++];
        str += ")";
        str += (p.camp == camp) ? "win" : "lose";
        po->OnMessage.connect(
            [ po, result, str = std::move(str), repl ](lsy::buffer buf) {
                po->write(str, []() {});
                po->write(result, []() {});
                po->write(repl, [po]() { po->close(); });
            });
        po->start();
    }
    delete this;
}

void room_space::room::for_player(int n, std::function< void(player*) > func,
                                  int m)
{
    if (m != size())
    {
        func(get_player(m));
        wait(n, [this, func, m, n]() { for_player(n, func, m + 1); });
    }
}

void room_space::room::for_each_player(int                            n,
                                       std::function< void(player*) > func)
{
    uint8_t size = players.size();
    for (int i = n; i != size; i++)
    {
        func(&players[i]);
    }
    for (int i = 0; i != n; i++)
    {
        func(&players[i]);
    }
}


uint8_t room_space::room::check()
{
    std::map< uint8_t, uint8_t > camp_map;
    for (auto& a : players)
    {
        if (!a.is_dead)
        {
            camp_map[a.camp]++;
        }
    }
    if (camp_map.size() == 1)
    {
        return camp_map.begin()->first;
    }
    else
    {
        return 0xff;
    }
}

bool room_space::room::add_group_button(room_space::player* pl,
                                        std::string         name)
{

    auto it   = group_data.find(name);
    bool flag = it == group_data.end();
    if (flag)
    {
        auto p = group_data.emplace(name, group_button());
        it     = p.first;
    }
    it->second.group_mem.insert(pl->index);
    pl->add_button(
        name, [ name, m = pl->index, &map_data = it->second, this ](uint8_t n) {
            map_data.click_map[n]++;
            if (map_data.on_click)
            {
                map_data.on_click(m, n);
            };
        });
    return flag;
}

group_button* room_space::room::get_group(std::string name)
{
    return &group_data[name];
}

void room_space::room::remove_group_button(std::string name)
{
    auto it = group_data.find(name);
    if (it != group_data.end())
    {
        for (auto a : it->second.group_mem)
            players[a].remove_button(name);
        group_data.erase(it);
    }
}

void room_space::group_button::generate(bool is_rand)
{
    uint8_t                max_num = 0;
    uint8_t                max_pos = 0;
    std::vector< uint8_t > same_list;
    for (auto a : click_map)
    {
        if (a.second > max_num)
        {
            same_list.clear();
            max_pos = a.first;
            max_num = a.second;
        }
        else if (a.second == max_num)
        {
            same_list.push_back(a.first);
            same_list.push_back(max_pos);
        }
    }
    if (on_max)
    {
        if (max_num == 0)
        {
            on_max(0xff);
        }
        else if (same_list.empty())
        {
            on_max(max_pos);
        }
        else if (is_rand)
        {
            on_max(same_list[rand() % same_list.size()]);
        }
        else
        {
            on_max(0xff);
        }
    }
}

void room_space::group_button::for_each_player(
    std::function< void(uint8_t) > func)
{
    for (auto a : group_mem)
    {
        func(a);
    }
}
