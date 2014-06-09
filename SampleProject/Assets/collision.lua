
function startCollisionTest()								
	TASK_Generic("doCollisionTest", "doCollisionTestEnd", "collisionTestKey")
end


function checkCollision(boundingBox1, boundingBox2)
	local minX1 = boundingBox1.x
	local minY1 = boundingBox1.y
	local maxX1 = boundingBox1.x + boundingBox1.width
	local maxY1 = boundingBox1.y + boundingBox1.height
	
	local minX2 = boundingBox2.x
	local minY2 = boundingBox2.y
	local maxX2 = boundingBox2.x + boundingBox2.width
	local maxY2 = boundingBox2.y + boundingBox2.height
	
    return not(maxX1 < minX2 or
             maxX2 < minX1 or
             maxY1 < minY2 or
             maxY2 < minY1)
			 
end



function doCollisionTest()
	local bullet
	local bulletKey
	local monster
	local monsterKey
	for bulletKey, bullet in pairs(g_tblBullets) do
		if false == bullet.isDead then
			local bullet_prop = TASK_getProperty(bullet.image)
			for monsterKey, monster in pairs(g_tblMonster) do
				if false == monster.isDead then
					local monster_prop = TASK_getProperty(monster.image)
					if checkCollision(bullet.boundingBox, monster.boundingBox) then
						bullet.isDead = true
						monster.animTime = 0
						sysCommand(monster.image, SET_DAMAGE_STATE)
						monster.HP = monster.HP - bullet.damage
						if monster.HP <= 0 then
							-- UI_VariableItem无视资源中设置的AnchorPoint，不得已自己计算中心对齐
							local cX = monster.boundingBox.x + monster.boundingBox.width / 2
							local cY = monster.boundingBox.y + monster.boundingBox.height / 2
							sysCommand(monster.image, SET_DEAD_STATE)
							local prop = TASK_getProperty(monster.image)
							prop.x = cX - prop.width / 2;
							prop.y = cY - prop.height / 2;
							TASK_setProperty(monster.image, prop)
							monster.isDead = true
						end
					end
				end
			end
		end
	end
end



function doCollisionTestEnd()
end