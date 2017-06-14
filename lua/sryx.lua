return function()
	not_dead=function(a)
		return not a.is_dead
	end
	co=coroutine.create(function()
		coroutine.yield(2)
		room.checktable={}
		room.deadtable={}
		local days=1
		room.sent_public("game start\n")
		while(true) do
			room.sent_public(string.format("the %d day\n",days))
			days=days+1
			room.sent_public("It's getting dark, close your eyes\n")
			local n=1
			local p_c=0
			dark=room.signals.get_signal("dark")
			dark.trigger()
			room.sent_public("killer please choose a player to be killed\n")
			room.sent_public("police please choose a player to be check\n")
			coroutine.yield(3)
			kil_person=nil
			light=room.signals.get_signal("light")
			light.trigger()
			if(kil_person~=nil) then
				room.sent_public(string.format("%d is killed\n",kil_person+1))
				room.set_dead(kil_person,true)
			else
				room.sent_public("no body is killed\n");
				kil_person=0
			end
			if(room.check())then
				coroutine.yield(0)
			end
			local b=nil
			for i=kil_person,kil_person+room.size()-1,1 do
				local a=room.get_player(i)
				if(b==nil or not a.is_dead)then
					room.sent_public(string.format("%d player please speak\n",a.index+1))
					room.channels.get_channel(a,"public").enable(true)
					if(b~=nil)then
						room.channels.get_channel(b,"public").enable(false)
					end
					b=a
					coroutine.yield(3)
				end
			end
			room.channels.get_channel(b,"public").enable(false)
			room.sent_public("vote time\n")
			room.signals.get_signal("votestart").trigger()
			coroutine.yield(20)
			voteout=nil
			room.signals.get_signal("voteend").trigger()
			if(voteout~=nil) then
				room.set_dead(voteout,true)
				room.sent_public(string.format("%d player please speak\n",voteout+1))
				room.channels.get_channel(room.get_player(voteout),"public").enable(true)
				if(room.check())then
					coroutine.yield(0)
				end
				coroutine.yield(10)
				room.channels.get_channel(room.get_player(voteout),"public").enable(false)
			end
		end
	end)
	function cycle()
		local n,m=coroutine.resume(co)
		if(m~=0) then
			print(m,cycle)
			room.wait(m,cycle)
		end
	end
	cycle()
end