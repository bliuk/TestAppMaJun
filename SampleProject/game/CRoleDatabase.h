#ifndef __CRoleDatabase_h__
#define __CRoleDatabase_h__

#include "Dictionnary.h"
#include "sqlite3.h"
#include "CKLBLuaPropTask.h"

class CRoleInfo
{
public:
    CRoleInfo()
    : id(-1)
    , key(NULL)
    , width(0.f)
    , height(0.f)
    , flyFrameName(NULL)
    , flyFrameCount(0)
    , flyFrameInterval(0.f)
    , damageFrameName(NULL)
    , damageFrameCount(0)
    , damageFrameInterval(0.f)
    , dyingFrameName(NULL)
    , dyingFrameCount(0)
    , dyingFrameInterval(0.f)
    {}
    
    ~CRoleInfo()
    {
        KLBDELETEA(key);
        KLBDELETEA(flyFrameName);
        KLBDELETEA(damageFrameName);
        KLBDELETEA(dyingFrameName);
    }
    
    int id;
    const char* key;
    float width;
    float height;
    const char* flyFrameName;
    int flyFrameCount;
    float flyFrameInterval;
    const char* damageFrameName;
	int damageFrameCount;
	float damageFrameInterval;
	const char* dyingFrameName;
	int dyingFrameCount;
	float dyingFrameInterval;
};


class CRoleDatabase : public CKLBLuaPropTask
{
	friend class CKLBTaskFactory<CRoleDatabase>;
private:
	bool init(CKLBTask* pTask, CKLBTask::TASK_PHASE phase, const char* dbName);
public:
    static CRoleDatabase* create(CKLBTask* pParentTask, CKLBTask::TASK_PHASE phase, const char* dbName);
    
    CRoleDatabase();
	virtual ~CRoleDatabase();
    
    bool	initScript		(CLuaState& lua);
    int		commandScript	(CLuaState& lua);
    
    void	execute			(u32 deltaT);
    void	die				();
    
	u32		getClassID		();
    
    CRoleInfo* getRoleInfo(const char* key);
private:
    static
	int			callbackFct			(void* ctx,int colNum,char** columnText,char** columnName);
	int			callBack			(int /*colNum*/,char** columnText,char** /*columnName*/);
    static void callbackDictionnary(const void* this_, const void* ptrToDelete);
    
    bool        loadAll     ();
    
    sqlite3*		m_db;
    Dictionnary*	m_dictionnary;
};


#endif // 
