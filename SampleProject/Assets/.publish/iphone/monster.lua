g_tblMonster = {}
g_monsterKey = 0
g_monsterSpeed = 0.4

monsterPosX = {90, 190, 320, 450, 550}

function createMonsters()
	UTIL_IntervalTimer	(
									0, 			--<timerID>
									"doCreateMonsters",--"<callback>" 
									1000,		--<interval>
									true		--[ , <repeat> ]
								)
end

function doCreateMonsters()
	local y = -200
	for i=1, 5 do
		local key = string.format("%d", g_monsterKey)
		local tbl = {}
		local pGenTask = TASK_Generic("execute_monster", "die_monster", key)

		tbl.image = GAME_ROLE(pGenTask, 9000, 0, 0, roleDB, "Dragon0")

		local prop = TASK_getProperty(tbl.image)
		prop.x = monsterPosX[i] - prop.width / 2;
		prop.y = y - prop.height / 2;
		tbl.boundingBox = {}
		tbl.boundingBox.x = monsterPosX[i] - prop.collisionWidth / 2
		tbl.boundingBox.y = y - prop.collisionHeight / 2
		tbl.boundingBox.width = prop.collisionWidth
		tbl.boundingBox.height = prop.collisionHeight
		TASK_setProperty(tbl.image, prop)
		tbl.HP = 50
		tbl.isDead = false
		
		tbl.animTime = 0
		
		g_tblMonster[key] = tbl	
		g_monsterKey = g_monsterKey + 1
	end		
end


function execute_monster(pTask, deltaT, key)
	local role = g_tblMonster[key]
	local roleImg = role.image
	role.animTime = role.animTime + deltaT
		
	if role.isDead then
		if role.animTime >= 500 then
			TASK_kill(pTask)
			TASK_kill(roleImg)
		end
		return
	end

	prop = TASK_getProperty(roleImg)
	prop.y = prop.y + deltaT * g_monsterSpeed
	TASK_setProperty(roleImg, prop)
	
	role.boundingBox.y = role.boundingBox.y + deltaT * g_monsterSpeed

	if role.animTime >= 200 then
		sysCommand(roleImg, SET_FLY_STATE)
	end
	
	if prop.y > 960+role.boundingBox.height then
		TASK_kill(pTask)
		TASK_kill(roleImg)
	end
end

function die_monster(pTask, key)
	g_tblMonster[key] = nil
end
