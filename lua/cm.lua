return function(player)
	print("------------------\n-----------------------------\n")
	local public=room.channels.get_channel(player,"public")
	public.enable(false)
	player.set_camp(0)
	local is_main=false
	room.signals.get_signal("votestart").connect(function()
		if(not player.is_dead())then
			if(room.add_group_button(player,"vote"))then
				is_main=true
			end
		end
	end)
	room.signals.get_signal("voteend").connect(function()
		if(is_main)then
			local but=room.group_button("vote")
			but.on_max=function(a)
				if(a~=255)then
					voteout=a
				end
			end
			but.generate(false)
			is_main=false
			room.remove_group_button("vote")
		end
	end)
	local public=room.channels.get_channel(player,"public")
	public.enable(false)
end