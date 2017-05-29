return function()
	local channels=room.channels()
	co=coroutine.create(function()
		coroutine.yield(2)
		room.checktable={}
		room.deadtable={}
		local days=1
		for i=0,room.size(),1 
		do 
			room.deadtable[i]={}
		end
		room.sent_public("game start\n")
		while(true) do
			room.sent_public(string.format("the %d day\n",days))
			days=days+1
			room.sent_public("It's getting dark, close your eyes\n")
			local chan=room.channels();
			local n=1
			local p_c=0
			local signals=room.signals()
			dark=signals.get_signal("dark")
			dark.trigger()
			room.sent_public("killer please choose a player to be killed\n")
			room.sent_public("police please choose a player to be check\n")
			coroutine.yield(10)
			room.kil_person=nil
			light=signals.get_signal("light")
			light.trigger()
			if(room.kil_person~=nil) then
				room.sent_public(string.format("%d is killed\n",room.kil_person+1))
				room.set_dead(room.kil_person,true)
			else
				room.kil_person=0
			end
			camp=room.check()
			if(camp~=255)then
				room.close(camp)
				coroutine.yield(10)
			end
			local b=nil
			for i=room.kil_person,room.kil_person+room.size()-1,1 do
				local a=room.get_player(i)
				if(b==nil or not a.is_dead())then
					room.sent_public(string.format("%d player please speak\n",a.index()+1))
					channels.get_channel(a,"public").enable(true)
					if(b~=nil)then
						channels.get_channel(b,"public").enable(false)
					end
					b=a
					coroutine.yield(10)
				end
			end
			channels.get_channel(b,"public").enable(false)
			room.sent_public("vote time\n")
			signals.get_signal("votestart").trigger()
			coroutine.yield(20)
			room.voteout=nil
			signals.get_signal("voteend").trigger()
			camp=room.check()
			if(camp~=255)then
				room.close(camp)
				coroutine.yield(10)
			end
			if(room.voteout~=nil) then
				room.sent_public(string.format("%d player please speak\n",room.voteout+1))
				channels.get_channel(room.get_player(room.voteout),"public").enable(true)
				coroutine.yield(10)
				channels.get_channel(room.get_player(room.voteout),"public").enable(false)
			end
		end
	end)
	function cycle()
		local n,m=coroutine.resume(co)
		if(m~=0) then
			room.wait(m,cycle)
		end
	end
	cycle()
end