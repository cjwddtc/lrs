return function(player)
	print("cm")
	print("player.id_dead:",player.is_dead)
	local public=room.channels.get_channel(player,"public")
	public.enable(false)
	player.set_camp(0)
	local is_main=false
	local vote_result
	print("testing",player.index,"\n")
	room.signals.get_signal("votestart").connect(function()
		if(not player.is_dead)then
			print(player.index," is not dead\n")
			if(room.add_group_button(player,"vote",not_dead))then
				vote_result=""
				local but=room.group_button("vote")
				but.on_click=function(a,b)
					vote_result=table.concat({vote_result,tostring(a),"->",tostring(b),"\n"})
				end
				is_main=true
			end
		else
			print(player.index," is dead\n")
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
			room.sent_public(vote_result)
			is_main=false
			room.remove_group_button("vote")
		end
	end)
	local public=room.channels.get_channel(player,"public")
	public.enable(false)
end