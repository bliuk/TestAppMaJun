#include "CRoleDatabase.h"
#include "CKLBLanguageDatabase.h"
#include "CKLBUtility.h"
#include "Common.h"

// Command Values
//enum {
//	CM_ADD_SIMPLE = 0,
//};

static IFactory::DEFCMD cmd[] = {
//	{ "CM_ADD_SIMPLE", CM_ADD_SIMPLE },
	{0, 0}
};

static CKLBTaskFactory<CRoleDatabase> factory("ROLE_DB", CLS_ROLE_DB, cmd);


enum {
    ARGS_DB_NAME = 1,
    ARGS_PHASE,
	ARGS_PARENT,
    
    ARGS_REQUIRE    = ARGS_DB_NAME,
    ARGS_MAX        = ARGS_PARENT
};

CRoleDatabase::CRoleDatabase()
{
	m_newScriptModel = true;
    m_db = NULL;
    m_dictionnary = NULL;
}

CRoleDatabase::~CRoleDatabase()
{
    if (m_db) {
		sqlite3_close(m_db);
		m_db = NULL;
	}
}

u32
CRoleDatabase::getClassID()
{
    return CLS_ROLE_DB;
}

CRoleDatabase*
CRoleDatabase::create(CKLBTask* pParentTask, CKLBTask::TASK_PHASE phase, const char* dbName)
{
	CRoleDatabase* pTask = KLBNEW(CRoleDatabase);
    if(!pTask) { return NULL; }
    
	if(!pTask->init(pParentTask, phase, dbName)) {
		KLBDELETE(pTask);
		return NULL;
	}
	return pTask;
}

bool
CRoleDatabase::init(CKLBTask* pTask, CKLBTask::TASK_PHASE phase, const char* dbName)
{
    do
    {
        BREAK_IF(NULL == dbName);
        BREAK_IF(!regist(pTask, phase));
        
        IPlatformRequest& platform = CPFInterface::getInstance().platform();
        const char* fullPath = platform.getFullPath(dbName);
        
        m_dictionnary = KLBNEW(Dictionnary);
        BREAK_IF(NULL == m_dictionnary);
        
        m_dictionnary->setOwnerCallback(this, callbackDictionnary);
        BREAK_IF(!m_dictionnary->init(3000));
        
        int rc = sqlite3_open_v2(fullPath, &m_db, SQLITE_OPEN_READONLY, NULL);
        delete[] fullPath;
        
        BREAK_IF(rc != 0);
        BREAK_IF(!loadAll());

        return true;
    } while (false);
    
    return false;
}


bool CRoleDatabase::loadAll()
{
    if (!m_db)
    {
        return false;
    }
    
    const char* sql = "SELECT * FROM roleTable;";
    char* errMsg;
    int rc = sqlite3_exec(m_db, sql, CRoleDatabase::callbackFct, this, &errMsg);
    if (rc!=SQLITE_OK) {
        klb_assertAlways("DB Error : %s", errMsg);
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}



void
CRoleDatabase::execute(u32 deltaT)
{
}



bool
CRoleDatabase::initScript(CLuaState &lua)
{
	int argc = lua.numArgs();
    if(argc < ARGS_REQUIRE || argc > ARGS_MAX) {
        return false;
    }
    
    const char* dbName = lua.isNil(ARGS_DB_NAME) ? NULL : lua.getString(ARGS_DB_NAME);
	CKLBTask::TASK_PHASE phase = (CKLBTask::TASK_PHASE)((argc >= ARGS_PHASE) ? lua.getInt(ARGS_PHASE) : P_NORMAL);
	CKLBTask * pParent = (argc >= ARGS_PARENT) ? (CKLBTask *)lua.getPointer(ARGS_PARENT) : NULL;
    
	return init(pParent, phase, dbName);
}



void
CRoleDatabase::die()
{
    //	CKLBScriptEnv::getInstance().call_genTaskDie(m_luaFuncDie, this, m_ArrayIndex);
    
    //	KLBDELETEA(m_luaFuncExec); m_luaFuncExec = NULL;
    //    KLBDELETEA(m_luaFuncDie);  m_luaFuncDie  = NULL;
    //    KLBDELETEA(m_ArrayIndex);  m_ArrayIndex  = NULL;
}

int
CRoleDatabase::commandScript(CLuaState& lua)
{
	int argc = lua.numArgs();
	if(argc < 2) {
		lua.retBoolean(false);
		return 1;
	}
	int cmd = lua.getInt(2);
	int ret = 1;
    
	switch(cmd)
	{
        default:
		{
			lua.retBoolean(false);
			ret = 1;
		}
            break;
	}
	return ret;
}



int CRoleDatabase::callbackFct (void* ctx,int colNum,char** columnText,char** columnName) {
	return ((CRoleDatabase*)ctx)->callBack(colNum,columnText,columnName);
}

int CRoleDatabase::callBack(int colNum,char** columnText,char** columnName) {
    
    CRoleInfo* info = KLBNEW(CRoleInfo);
    info->id = atoi(columnText[0]);
    info->key = CKLBUtility::copyString(columnText[1]);
    info->width = (float)atof(columnText[2]);
    info->height = (float)atof(columnText[3]);
    info->flyFrameName = CKLBUtility::copyString(columnText[4]);
    info->flyFrameCount = atoi(columnText[5]);
    info->flyFrameInterval = (float)atof(columnText[6]);
    
    info->damageFrameName = columnText[7] ? CKLBUtility::copyString(columnText[7]) : NULL;
    info->damageFrameCount = atoi(columnText[8]);
    info->damageFrameInterval = (float)atof(columnText[9]);
    
    info->dyingFrameName = columnText[10] ? CKLBUtility::copyString(columnText[10]) : NULL;
    info->dyingFrameCount = atoi(columnText[11]);
    info->dyingFrameInterval = (float)atof(columnText[12]);
    
    m_dictionnary->add(info->key, info);
    
	return 0;
}



void CRoleDatabase::callbackDictionnary(const void* /*this_*/, const void* ptrToDelete) {
	// Dico Entry Name String.
	KLBDELETE((CRoleInfo*)ptrToDelete);
}



CRoleInfo* CRoleDatabase::getRoleInfo(const char* key)
{
    return (CRoleInfo*)m_dictionnary->find(key);
}
