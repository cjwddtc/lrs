return function(player)
	local is_self=function(a)
		return a.index==player.index
	end
	local public=room.channels.get_channel(player,"public")
	public.enable(false)
	player.set_camp(0)
	local is_main=false
	room.signals.get_signal("sergeant start").connect(function()
		elect_state_map[player.index]=false;
		local func=nil
		func=function(a)
			player.remove_button(election_map[elect_state_map[player.index]])
			elect_state_map[player.index]=not elect_state_map[player.index]
			player.add_button(election_map[elect_state_map[player.index]],func,is_self)
		end
		player.add_button(election_map[elect_state_map[player.index]],func,is_self)
	end)
	room.signals.get_signal("sergeant speak").connect(function()
		player.remove_button(election_map[false])
		player.remove_button(election_map[true])
		if(elect_state_map[player.index])then
			player.add_button(election_map[true],function(a)
				elect_state_map[player.index]=false
				player.remove_button(election_map[true])
				room.sent_public(table.concat({tostring(player.index+1)," is diselect"}))
			end,is_self)
		end
	end)
	sergeant=room.signals.get_signal("sergeant vote").connect(function()
		player.remove_button(election_map[true])
		if(not player.is_dead and not elect_state_map[player.index])then
			if(room.add_group_button(player,"sergeant",on_sergeant))then
				local but=room.group_button("sergeant")
				vote_result=""
				but.on_click=function(a,b)
					vote_result=table.concat({vote_result,tostring(a+1),"->",tostring(b+1),"\n"})
				end
				is_main=true
			end
		end
	end)
	sergeant=room.signals.get_signal("sergeant vote end").connect(function()
		if(is_main)then
			local but=room.group_button("sergeant")
			but.on_max=function(a)
				if(a~=255)then
					sergeant_chosen=a
				end
			end
			but.generate(false)
			room.sent_public(vote_result)
			is_main=false
			room.remove_group_button("sergeant")
		end
	end)
	room.signals.get_signal("votestart").connect(function()
		if(not player.is_dead)then
			if(room.add_group_button(player,"vote",not_dead))then
				sergeant_post=nil
				vote_result=""
				local but=room.group_button("vote")
				but.on_click=function(a,b)
					if(a==sergeant_chosen)then
						print("seg_post")
						sergeant_post=b
					end
					print("post:",a,":",sergeant_chosen,"\n")
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
				else
					voteout=sergeant_post
				end
			end
			but.generate(false)
			room.sent_public(vote_result)
			is_main=false
			room.remove_group_button("vote")
		end
	end)
end