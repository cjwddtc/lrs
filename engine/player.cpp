#include "player.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <config.h>
#include <db_auto.h>
#include <iterator>
#include <match.h>
#include <room.h>
using db_gen::main;
extern thread_local boost::asio::io_service io_service;

lsy::port_all* lsy::player::operator->()
{
    return ptr;
}

lsy::player::player(port_all* soc, std::string id_)
    : as_contain< port_all >(soc)
    , id(id_)
{
    lsy::as_ptr< lsy::port > multiplay_port
        = ptr->resign_port(config::multiplay_port);
    multiplay_port->OnMessage.connect([this, multiplay_port](buffer buf) {
        main.get_base_room.bind(
            [& io = io_service, multiplay_port ](bool is_fin) {
                if (is_fin)
                {
                    multiplay_port->write(lsy::buffer(size_t(1)), []() {});
                }
                else
                {
                    std::string room_name = main.get_base_room[0];
                    multiplay_port->write(room_name, []() {});
                }
            });
    });
    multiplay_port->start();
    lsy::as_ptr< lsy::port > games_port = ptr->resign_port(config::games_port);
    games_port->OnMessage.connect([this, games_port](buffer buf) {
        main.get_extra_room.bind([games_port](bool is_fin) {
            if (is_fin)
            {
                games_port->write(lsy::buffer(size_t(1)), []() {});
            }
            else
            {
                std::string room_name = main.get_extra_room[0];
                games_port->write(room_name, []() {});
            }
        });
    });
    games_port->start();

    lsy::as_ptr< lsy::port > muti_info_port
        = ptr->resign_port(config::muti_info_port);
    muti_info_port->OnMessage.connect([this](lsy::buffer buf) {
        std::string name((char*)buf.get(0));
        main.get_room_info.bind_once(
            [name, this](bool have_data) {
                if (have_data)
                {
                    std::string info = main.get_room_info[0];
                    main.get_score.bind_once([ info = std::move(info),
                                               this ](bool have_data) {
                        if (have_data)
                        {
                            int score = main.get_score[0];
                            ptr->ports[config::muti_info_port]->write(info,
                                                                      []() {});
                            ptr->ports[config::muti_info_port]->write(
                                uint16_t(score), []() {});
                        }
                    },
                                             id, name);
                }
            },
            name);
    });
    muti_info_port->start();

    lsy::as_ptr< lsy::port > match_port = ptr->resign_port(config::match_port);
    match_port->OnMessage.connect(
        [ this, match_port, &io = io_service ](buffer buf) {
            std::string str((char*)buf.data());
            if (room_space::get_playing(id) == 0)
            {
                io.post([str, this]() { add_to_queue(str, this, true); });
            }
            else
            {
                match_port->write(uint16_t(0), []() {});
            }

        });
    match_port->start();


    lsy::as_ptr< lsy::port > game_info_port
        = ptr->resign_port(config::game_info_port);
    game_info_port->OnMessage.connect([this](lsy::buffer buf) {
        std::string name((char*)buf.get(0));
        main.get_room_info.bind_once(
            [name, this](bool have_data) {
                if (have_data)
                {
                    std::string info = main.get_room_info[0];
                    ptr->ports[config::game_info_port]->write(info, []() {});
                }
            },
            name);
    });
    game_info_port->start();

    lsy::as_ptr< lsy::port > match_noscore_port
        = ptr->resign_port(config::match_noscore_port);
    match_noscore_port->OnMessage.connect(
        [ this, match_port, &io = io_service ](buffer buf) {
            std::string str((char*)buf.data());
            if (room_space::get_playing(id) == 0)
            {
                io.post([str, this]() { add_to_queue(str, this, false); });
            }
            else
            {
                match_port->write(uint16_t(0), []() {});
            }

        });
    match_noscore_port->start();

    ptr->resign_port(config::match_noscore_status_port)->start();
    ptr->resign_port(config::match_status_port)->start();


    lsy::as_ptr< lsy::port > change_passwd_port
        = ptr->resign_port(config::passwd_change_port);
    change_passwd_port->OnMessage.connect([this](auto buf) {
        std::string oldps;
        std::string newps;
        buf.get(oldps);
        buf.get(newps);
        main.update_passwd.bind(
            [this](bool is) {
                assert(is);
                uint16_t flag = sqlite3_changes(main.db);
                ptr->ports[config::passwd_change_port]->write(flag, []() {});
            },
            newps, id, oldps);
    });
    change_passwd_port->start();

    lsy::as_ptr< lsy::port > player_role_create_port
        = ptr->resign_port(config::player_role_create_port);
    player_role_create_port->OnMessage.connect([this](auto buf) {
        std::string oldps;
        buf.get(oldps);
        std::string filename;
        filename.resize(8);
        std::generate(filename.begin(), filename.end(),
                      []() { return 'a' + rand() % 26; });
        FILE* fp = fopen(
            (boost::filesystem::path("lua") / filename).string().c_str(), "w");
        fwrite((void*)buf.get(0), 1, buf.remain(), fp);
        fclose(fp);
        main.add_role.bind([](bool flag) {}, oldps, filename, id);
    });
    player_role_create_port->start();

    lsy::as_ptr< lsy::port > player_rule_create_port
        = ptr->resign_port(config::player_rule_create_port);
    player_rule_create_port->OnMessage.connect([this](auto buf) {
        std::string oldps;
        buf.get(oldps);
        std::string filename;
        filename.resize(8);
        std::generate(filename.begin(), filename.end(),
                      []() { return 'a' + rand() % 26; });
        FILE* fp = fopen(
            (boost::filesystem::path("lua") / filename).string().c_str(), "w");
        fwrite((void*)buf.get(0), 1, buf.remain(), fp);
        fclose(fp);
        main.add_rule.bind([](bool flag) {}, oldps, filename, id);
    });
    player_rule_create_port->start();


    lsy::as_ptr< lsy::port > player_room_create_port
        = ptr->resign_port(config::player_room_create_port);
    player_room_create_port->OnMessage.connect([this](auto buf) {
        std::string room_name;
        std::string rule_name;
        uint16_t    size;
        buf.get(room_name);
        buf.get(rule_name);
        buf.get(size);
        while (size--)
        {
            uint16_t    count;
            std::string role_name;
            buf.get(role_name);
            buf.get(count);
            main.add_room_role.bind([](bool flag) {}, room_name, role_name,
                                    count);
        }
        main.add_room.bind([](bool flag) {}, room_name, rule_name, id);
    });
    player_room_create_port->start();

    lsy::as_ptr< lsy::port > player_role_delete_port
        = ptr->resign_port(config::player_role_delete_port);
    player_role_delete_port->OnMessage.connect([this](auto buf) {
        std::string role_name;
        buf.get(role_name);
        main.remove_role.bind([](bool flag) {}, role_name);
    });
    player_role_delete_port->start();

    lsy::as_ptr< lsy::port > player_rule_delete_port
        = ptr->resign_port(config::player_rule_delete_port);
    player_rule_delete_port->OnMessage.connect([this](auto buf) {
        std::string rule_name;
        buf.get(rule_name);
        main.remove_rule.bind([](bool flag) {}, rule_name);
    });
    player_rule_delete_port->start();

    lsy::as_ptr< lsy::port > player_room_delete_port
        = ptr->resign_port(config::player_room_delete_port);
    player_room_delete_port->OnMessage.connect([this](auto buf) {
        std::string rule_name;
        buf.get(rule_name);
        main.remove_room.bind([](bool flag) {}, rule_name);
        main.remove_room_role.bind([](bool flag) {}, rule_name);
    });
    player_room_delete_port->start();

    lsy::as_ptr< lsy::port > player_role_get_port
        = ptr->resign_port(config::player_role_get_port);
    player_role_get_port->OnMessage.connect([this](auto buf) {
        main.get_user_role.bind(
            [this](bool flag) {
                if (flag)
                {
                    ptr->ports[config::player_role_get_port]->write(
                        lsy::buffer(size_t(0)), []() {});
                }
                else
                {
                    std::string role_name = main.get_user_role[0];
                    ptr->ports[config::player_role_get_port]->write(role_name,
                                                                    []() {});
                }
            },
            id);
    });
    player_role_get_port->start();
    lsy::as_ptr< lsy::port > player_rule_get_port
        = ptr->resign_port(config::player_rule_get_port);
    player_rule_get_port->OnMessage.connect([this](auto buf) {
        main.get_user_rule.bind(
            [this](bool flag) {
                if (flag)
                {
                    ptr->ports[config::player_rule_get_port]->write(
                        lsy::buffer(size_t(0)), []() {});
                }
                else
                {
                    std::string rule_name = main.get_user_rule[0];
                    ptr->ports[config::player_rule_get_port]->write(rule_name,
                                                                    []() {});
                }
            },
            id);
    });
    player_rule_get_port->start();
    lsy::as_ptr< lsy::port > player_room_get_port
        = ptr->resign_port(config::player_room_get_port);
    player_room_get_port->OnMessage.connect([this](auto buf) {
        main.get_user_room.bind(
            [this](bool flag) {
                if (flag)
                {
                    ptr->ports[config::player_room_get_port]->write(
                        lsy::buffer(size_t(0)), []() {});
                }
                else
                {
                    std::string room_name = main.get_user_room[0];
                    ptr->ports[config::player_room_get_port]->write(room_name,
                                                                    []() {});
                }
            },
            id);
    });
    player_room_get_port->start();
    lsy::as_ptr< lsy::port > player_room_detail_port
        = ptr->resign_port(config::player_room_detail_port);
    player_room_detail_port->OnMessage.connect([this](auto buf) {
        std::string room_name;
        buf.get(room_name);
        auto roles = std::
            make_shared< std::vector< std::pair< std::string, uint16_t > > >();
        main.get_user_room_role.bind(
            [roles](bool flag) {
                if (flag)
                {
                }
                else
                {
                    roles->emplace_back(
                        (std::string)main.get_user_room_role[0],
                        (uint16_t)(int)main.get_user_room_role[1]);
                }
            },
            room_name, id);
        main.get_user_room_rule.bind_once(
            [roles, room_name, this](bool have_data) {
                if (have_data)
                {
                    std::string rule_name = main.get_user_room_rule[0];
                    size_t      size = room_name.size() + rule_name.size() + 4;
                    int         n    = roles->size();
                    for (int i = 0; i < n; i++)
                    {
                        size += (*roles)[i].first.size() + 1;
                        size += 2;
                    }
                    lsy::buffer buf(size);
                    buf.put(room_name);
                    buf.put(rule_name);
                    buf.put((uint16_t)n);
                    for (int i = 0; i < n; i++)
                    {
                        buf.put((*roles)[i].first);
                        buf.put((*roles)[i].second);
                    }
                    ptr->ports[config::player_room_detail_port]->write(buf,
                                                                       []() {});
                }
            },
            room_name);
    });
    player_room_detail_port->start();
}