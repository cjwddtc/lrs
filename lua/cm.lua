return function(player)
	local signals=room.signals()
	if(room.vote_table==nil) then
		room.vote_table={}
		room.vote_flag=true
	end
	local channels=room.channels()
	local public=channels.get_channel(player,"public")
	public.enable(false)
	signals.get_signal("votestart").connect(function()
		if(not player.is_dead())then
			player.add_button("vote",function (n)
				room.vote_table[player.index()]=n
			end)
		end
	end)
	signals.get_signal("voteend").connect(function()
		player.remove_button("vote")
		if(room.vote_flag)then
			room.vote_flag=false
			local vote_table={}
			for key, value in pairs(room.vote_table) do
				if(vote_table[value]==nil) then
					vote_table[value]={key}
				else
					table.insert(vote_table[value],key)
				end
			end
			local maxkey=nil
			local maxvalue=0
			room.is_same=false
			for key, value in pairs(vote_table) do
				room.sent_public(string.format("%s vote %d\n",table.concat(value," "),key))
				if(table.maxn(value)>maxvalue) then
					maxvalue=table.maxn(value)
					maxkey=key
					room.is_same=false
				end
				if(table.maxn(value)==maxvalue) then
					room.is_same=true
				end
			end
			if(maxkey~=nil and not room.is_same)then
				room.sent_public(string.format("%d has been vote out\n",maxkey+1))
				room.set_dead(maxkey,true)
				room.voteout=maxkey
			end
		end
	end)
	player.set_camp(1)
end