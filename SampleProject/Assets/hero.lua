g_tblBullets = {}
g_bulletKey = 0
g_bulletSpeed = 1

function createHero(x, y)

	local role = GAME_ROLE(nil, 8000, 0, 0, roleDB, "Hero0")
	local prop = TASK_getProperty(role)
	prop.x = x - prop.width / 2;
	prop.y = y - prop.height / 2;
	TASK_setProperty(role, prop)
	
	return role
end


function heroShoot(x, y)
	local tbl = {}
	local key = string.format("%d", g_bulletKey)
	local pGenTask = TASK_Generic("execute_bullet", "die_bullet", key)

	tbl.image = GAME_ROLE(pGenTask, 9000, 0, 0, roleDB, "Bullet0")

	local prop = TASK_getProperty(tbl.image)
	prop.x = x - prop.width / 2;
	prop.y = y - prop.height / 2;
	tbl.boundingBox = {}
	tbl.boundingBox.x = x - prop.collisionWidth / 2
	tbl.boundingBox.y = y - prop.collisionHeight / 2
	tbl.boundingBox.width = prop.collisionWidth
	tbl.boundingBox.height = prop.collisionHeight
	TASK_setProperty(tbl.image, prop)
	tbl.isDead = false
	tbl.damage = 10
	g_tblBullets[key] = tbl	
	g_bulletKey = g_bulletKey + 1					
end


function execute_bullet(pTask, deltaT, key)
	local role = g_tblBullets[key]
	local roleImg = role.image
	if role.isDead then
		TASK_kill(pTask)
		TASK_kill(roleImg)
		return
	end
	
	local prop = TASK_getProperty(roleImg)
	
	prop.y = prop.y - deltaT * g_bulletSpeed
	role.boundingBox.y = role.boundingBox.y - deltaT * g_bulletSpeed
	TASK_setProperty(roleImg, prop)
	if prop.y < -role.boundingBox.height then
		TASK_kill(pTask)
		TASK_kill(roleImg)
	end
end

function die_bullet(pTask, key)
	g_tblBullets[key] = nil
end
