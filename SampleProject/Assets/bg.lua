include ("asset://hero.lua")
include ("asset://monster.lua")
include ("asset://collision.lua")

g_bgSpeed = 0.15
g_heroSpeed = 0.15	
g_heroX = 320
g_heroY = 880
g_baseHeroX = g_heroX	

g_bgY = 0
g_pHero = nil
g_pBg = nil


function setup()
	local x = 0

	g_pBg = UI_VariableItem(	nil,							-- arg[1]:		親となるUIタスクポインタ
									7000,							-- arg[2]:		表示プライオリティ
									x, g_bgY,							-- arg[3,4]:	表示位置
									-1,-1,
									"asset://bg.png.imag"	-- arg[5]:		表示assetのパス
								)
	
	sysCommand(g_pBg, UI_VARITEM_CHANGE_ASSET, "asset://bg.png.imag")	-- 不得已，需要通过这个办法拿到图片的size
	local prop = TASK_getProperty(g_pBg)
	local w = prop.width
	local h = prop.height
	syslog(string.format("w: %i, h: %i", w, h))

								
	g_pHero = createHero(g_heroX, g_heroY)
	
	createMonsters()
	
	UI_Control(
							"onClick",
							"onDrag"
						)		
	
	startCollisionTest()
end


function execute(deltaT)
	g_bgY = g_bgY - deltaT * g_bgSpeed
	if g_bgY <= -1024 then
		g_bgY = g_bgY + 1024
	end
	sysCommand(g_pBg, UI_VARITEM_SET_UV, 0, g_bgY, 768, 1024) 

	heroShoot(g_heroX, 840)
end


function leave()
end


function onClick(x,y)
end

function onDrag(mode,x,y,mvX,mvY)
	local prop = TASK_getProperty(g_pHero)
	prop.x = g_baseHeroX + mvX;
	if prop.x > 640 then
		prop.x = 640
	end
	
	if prop.x < 0 then
		prop.x = 0
	end
	
	g_heroX = prop.x
	prop.x = prop.x - prop.width / 2;
		
	TASK_setProperty(g_pHero, prop)	
	
	if mode == 2 then
		g_baseHeroX = g_heroX
	end
end