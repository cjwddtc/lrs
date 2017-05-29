return function(player)
	room.load_lua("cm.lua")(player)
	if(room.killtables==nil) then
		room.killtables={}
		room.kill_flag=true
	end
	local channels=room.channels()
	local signals=room.signals()
	signals.get_signal("dark").connect(function()
		if(not player.is_dead())then
			player.add_button("kill",function (n)
				channels.sent("ss",string.format("%d is trying to kill %d",player.index()+1,n+1))
				room.killtables[player.index()]=n
			end)
		end
	end)
	signals.get_signal("light").connect(function()
		player.remove_button("kill")
		if(room.kill_flag)then
			room.kill_flag=false
			local kil_person=nil
			for key, value in pairs(room.killtables) do
				if(kil_person==nil) then
					kil_person=value
				else
					if(kil_person~=value) then
						kil_person=-1
					end
				end
			end
			if(kil_person==-1)then
				channels.sent("ss","you cann't agree kill same player,kill fail")
			else
				room.kil_person=kil_person
			end
		end
	end)
	local public=channels.get_channel(player,"public")
	public.enable(false)
	local ss=channels.get_channel(player,"ss")
	ss.enable(true)
	player.set_camp(0)
end