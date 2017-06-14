return function()
	not_dead=function(a)
		return not a.is_dead
	end
	on_sergeant=function(a)
		return elect_state_map[a.index]
	end
	election_map={}
	election_map[false]="electing"
	election_map[true]="diselecting"
	co=coroutine.create(function()
		coroutine.yield(2)
		local days=1
		room.sent_public("game start\n")
		while(true) do
			room.sent_public(string.format("the %d day\n",days))
			room.sent_public("It's getting dark, close your eyes\n")
			local n=1
			local p_c=0
			dark=room.signals.get_signal("dark")
			dark.trigger()
			coroutine.yield(1)
			kil_person={}
			room.sent_public("half night has passed\n")
			dark=room.signals.get_signal("midnight")
			dark.trigger()
			coroutine.yield(1)
			room.sent_public("It's light already\n")
			light=room.signals.get_signal("light")
			light.trigger()
			if(days==1)then
				room.sent_public("sergeant election start\n")
				elect_state_map={}
				sergeant=room.signals.get_signal("sergeant start")
				sergeant.trigger()
				coroutine.yield(15)
				sergeant=room.signals.get_signal("sergeant speak")
				sergeant.trigger()
				local a=""
				for i=0,room.size()-1,1 do
					if(elect_state_map[i]) then
						a=table.concat({a,tostring(i+1)," "})
					end				
				end
				room.sent_public(table.concat({a,"elect\n"}))
				local b=nil
				for i=0,room.size()-1,1 do
					if(elect_state_map[i]) then
						local a=room.get_player(i)
						if(b==nil or not a.is_dead)then
							room.sent_public(string.format("%d player please speak\n",a.index+1))
							room.channels.get_channel(a,"public").enable(true)
							if(b~=nil)then
								room.channels.get_channel(b,"public").enable(false)
							end
							b=a
							coroutine.yield(1)
						end
					end				
				end
				room.channels.get_channel(b,"public").enable(false)
				local a=""
				for i=0,room.size()-1,1 do
					if(elect_state_map[i]) then
						a=table.concat({a,tostring(i+1)," "})
					end				
				end
				room.sent_public(table.concat({a,"stil elect,please vote\n"}))
				sergeant=room.signals.get_signal("sergeant vote")
				sergeant.trigger()
				coroutine.yield(15)
				sergeant=room.signals.get_signal("sergeant vote end")
				sergeant.trigger()
				if(sergeant_chosen~=nil) then
					room.sent_public(string.format("%d is sergeant\n",sergeant_chosen+1))
				else
					room.sent_public(string.format("noboy is sergeant\n"))
				end
			end
			local flag=true
			for key, value in ipairs(kil_person) do 
				room.sent_public(string.format("%d is killed\n",key+1))
				room.set_dead(key,true)
				flag=false
			end	
			if(flag) then
				room.sent_public("no body is killed\n");
			end
			if(days==1)then
				local b=nil
				for key, value in ipairs(kil_person) do  
					local a=room.get_player(key)
					room.channels.get_channel(a,"public").enable(true)
					if(b~=nil)then
						room.channels.get_channel(b,"public").enable(false)
					end
					b=a
					coroutine.yield(1)
				end
				if(b~=nil)then
					room.channels.get_channel(b,"public").enable(false)
				end
			end
			if(room.check())then
				coroutine.yield(0)
			end
			speak_person=nil
			sergeant=room.signals.get_signal("prepare speak")
			sergeant.trigger()
			if(sergeant_chosen~=nil)then
				room.sent_public("wait for sergeant choose the speak order")
				coroutine.yield(5)
				if(speak_person==nil)then
					speak_person=(sergeant_chosen+1)%room.size()
				end
			end
			if(speak_person==nil)then
				speak_person=0
			end
			for i=speak_person,speak_person+room.size()-1,1 do
				local a=room.get_player(i)
				if(b==nil or not a.is_dead)then
					room.sent_public(string.format("%d player please speak\n",a.index+1))
					room.channels.get_channel(a,"public").enable(true)
					if(b~=nil)then
						room.channels.get_channel(b,"public").enable(false)
					end
					b=a
					coroutine.yield(1)
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
			days=days+1
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