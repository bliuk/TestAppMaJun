function setup()
	roleDB = ROLE_DB("asset://roleScheme.db_")
end

function execute(deltaT)
	sysLoad("asset://bg.lua")
end

function leave()

end
