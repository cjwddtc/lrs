room.load_role("lrs_cm")
return function(player)
	player.set_camp(1)
	player.add_flag(2)
	room.add_condition(0,2);
	for i=0,room.size()-1,1 do
		local role=room.get_role(i)
		if(role=="wolf")then
			player.show_role(i,true)
		end
	end
	local is_main=false
	room.signals.get_signal("dark").connect(function()
		if(not player.is_dead)then
			if(room.add_group_button(player,"kill",not_dead))then
				is_main=true
				but=room.group_button("kill")
				but.on_click=function(a,b)
					room.channels.sent("ss",string.format("%d is trying to kill %d",a+1,b+1))
				end
			end
		end
	end)
	room.signals.get_signal("midnight").connect(function()
		if(is_main)then
			local but=room.group_button("kill")
			but.on_max=function(a)
				if(a~=255)then
					kil_person=a
				end
			end
			but.generate(true)
			is_main=false
			room.remove_group_button("kill")
		end
	end)
	local public=room.channels.get_channel(player,"public")
	public.enable(false)
	local ss=room.channels.get_channel(player,"ss")
	ss.enable(true)
end