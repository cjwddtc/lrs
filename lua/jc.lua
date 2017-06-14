room.load_role("cm")
return function(player)
	print("jc")
	player.add_flag(1)
	room.add_condition(1,1);
	local is_main=false
	room.signals.get_signal("dark").connect(function()
		if(not player.is_dead)then
			if(room.add_group_button(player,"check",not_dead,not_dead))then
				is_main=true
				but=room.group_button("check")
				but.on_click=function(a,b)
					room.channels.sent("jc",string.format("%d is trying to check %d",a+1,b+1))
				end
			end
		end
	end)
	room.signals.get_signal("light").connect(function()
		if(is_main)then
			local but=room.group_button("check")
			but.on_max=function(a)
				if(a~=255)then
					room.channels.sent("jc",string.format("%d is %s",a+1,room.get_role(a)))
				end
			end
			but.generate(true)
			is_main=false
			room.remove_group_button("check")
		end
	end)
	local ss=room.channels.get_channel(player,"jc")
	ss.enable(true)
	player.set_camp(0)
end