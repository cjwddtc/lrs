#pragma once
#include <config.h>
#include <player.h>
#include <room_player.h>
#include <string>
#include <utility>
namespace room_space
{
	struct channels;
    struct channel
    {
        channel(const channel&) = delete;
        channel(std::string str, player* pl, uint16_t port, channels *chs);
        std::pair< std::string, player* > key;
        const std::string& name() const;
        player*            player() const;
        mutable uint16_t   port;
        mutable bool       is_enable;
        void               open() const;
		channels *chs;
        void enable(bool is_enable);
        ~channel();
    };
};