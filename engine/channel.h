#pragma once
#include <config.h>
#include <player.h>
#include <string>
#include <utility>
#include <room_player.h>
namespace room_space
{
    struct channel
    {
		channel(std::string str, player *pl, uint16_t port);
        std::pair< std::string, player* > key;
        const std::string& name() const;
        player*       player() const;
        mutable uint16_t   port;
        mutable bool       is_enable;
		void open()const;
        void enable(bool is_enable);
		~channel();
    };
};