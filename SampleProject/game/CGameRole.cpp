/* 
   Copyright 2013 KLab Inc.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#include "CGameRole.h"

#include "CKLBDataHandler.h"
#include "CKLBUtility.h"
#include "CKLBLuaEnv.h"


// ƒRƒ}ƒ“ƒh’l
enum {
	UI_VARITEM_CHANGE_ASSET = 0,	// 頂点位置を指定
	UI_VARITEM_SET_UV,				// UV値を画像内ピクセル位置とピクセルサイズで指定する
	UI_VARITEM_RESET_UV,			// UV値をデフォルトに戻す
	UI_VARITEM_SET_MASK,			// Assign a Mask to multivar item.
    SET_FLY_STATE,
    SET_DAMAGE_STATE,
    SET_DEAD_STATE,
};

static IFactory::DEFCMD cmd[] = {
	{ "UI_VARITEM_CHANGE_ASSET",	UI_VARITEM_CHANGE_ASSET },
	{ "UI_VARITEM_SET_UV",			UI_VARITEM_SET_UV       },
	{ "UI_VARITEM_RESET_UV",		UI_VARITEM_RESET_UV     },
	{ "UI_VARITEM_SET_MASK",		UI_VARITEM_SET_MASK     },
    { "SET_FLY_STATE",              SET_FLY_STATE           },
    { "SET_DAMAGE_STATE",           SET_DAMAGE_STATE        },
    { "SET_DEAD_STATE",             SET_DEAD_STATE          },
	{ 0, 0}
};
static CKLBTaskFactory<CGameRole> factory("GAME_ROLE", CLS_GAME_ROLE, cmd);

CGameRole::CGameRole()
: CKLBUITask	()
, m_handle		(0)
, m_pTex		(NULL)
, m_asset		(NULL)
, m_pMaskTex	(NULL)
, m_pDynSprite	(NULL)
, m_Maskhandle	(0)
, m_org_height	(0.0f)
, m_org_width	(0.0f)
, m_flipTime    (0)
, m_curFrame    (0)
, m_roleState   (ROLE_STATE_FLY)
{
//	setNotAlwaysActive();
	m_newScriptModel = true;
    m_roleInfo = NULL;
}

CGameRole::~CGameRole() 
{
}

u32 CGameRole::getClassID() 
{
	return CLS_GAME_ROLE;
}

// 持たせるプロパティキー
CKLBLuaPropTask::PROP_V2 CGameRole::ms_propItems[] = {
	UI_BASE_PROP,
	{	"order",	UINTEGER,	(setBoolT)&CGameRole::setOrder,	(getBoolT)&CGameRole::getOrder,	0 },
	{	"asset",	STRING,		(setBoolT)&CGameRole::changeAsset,	(getBoolT)&CGameRole::getAsset,	0 },
	{	"width",	NUMERIC,	(setBoolT)&CGameRole::setWidth,	(getBoolT)&CGameRole::getWidth,	0 },
	{	"height",	NUMERIC,	(setBoolT)&CGameRole::setHeight,	(getBoolT)&CGameRole::getHeight,	0 },
    {   "collisionWidth", NUMERIC,   (setBoolT)&CGameRole::setCollisionWidth, (getBoolT)&CGameRole::getCollisionWidth, 0 },
    {   "collisionHeight", NUMERIC,   (setBoolT)&CGameRole::setCollisionHeight, (getBoolT)&CGameRole::getCollisionHeight, 0 },
};

enum PROP_ID{
	PR_ORDER,
	PR_ASSET,
	PR_WIDTH,
	PR_HEIGHT,
};

// 引数のインデックス定義
enum {
	ARG_PARENT = 1,
	ARG_ORDER,
	ARG_X,
	ARG_Y,
	ARG_DB,
	ARG_NAME,

	ARG_REQUIRE = ARG_NAME,
};


CGameRole *
CGameRole::create(CKLBUITask * pParent, CKLBNode * pNode,
                           u32 order, float x, float y, CRoleDatabase* db,
                           const char * name)
{
	CGameRole * pTask = KLBNEW(CGameRole);
    if(!pTask) { return NULL; }
	if(!pTask->init(pParent, pNode,
					order, x, y, db, name)) {
		KLBDELETE(pTask);
		return NULL;
	}

	return pTask;
}

bool
CGameRole::init(CKLBUITask * pParent, CKLBNode * pNode,
                u32 order, float x, float y, CRoleDatabase* db,
                const char * name)
{
    if(!setupNode()) { return false; }

	// ユーザ定義初期化を呼び、初期化に失敗したら終了。
	bool bResult = initCore(order, x, y, db, name);

	// 初期化処理終了後の登録。失敗時の処理も適切に行う。
	bResult = registUI(pParent, bResult);
	if(pNode) {
		pParent->getNode()->removeNode(getNode());
		pNode->addNode(getNode());
	}

	return bResult;
}

bool
CGameRole::initUI(CLuaState& lua)
{
	int argc = lua.numArgs();
	if(argc < ARG_REQUIRE) return false;

	// 基本プロパティの取り込み
	u32 order       = lua.getInt(ARG_ORDER);
	float x         = lua.getFloat(ARG_X);
	float y         = lua.getFloat(ARG_Y);
    CRoleDatabase* db = (CRoleDatabase*)lua.getPointer(ARG_DB);
	const char * name = (lua.isNil(ARG_NAME)) ? NULL : lua.getString(ARG_NAME);

	return initCore(order, x, y, db, name);
}

bool
CGameRole::initCore(u32 order, float x, float y, CRoleDatabase* db, const char * name)
{
	// 対応するプロパティリストを規定する。
	if(!setupPropertyList((const char**)ms_propItems,SizeOfArray(ms_propItems))) {
		return false;
	}

    m_roleInfo = db->getRoleInfo(name);
	// 読み込んだ頂点データと、座標値、orderで表示物を用意する。
	CKLBRenderingManager& pRdrMgr = CKLBRenderingManager::getInstance();

	// 基本プロパティの取り込み
	setInitPos(x, y);

	klb_assert((((s32)order) >= 0), "Order Problem");

	m_order     = order;

	m_pDynSprite = pRdrMgr.allocateCommandDynSprite(8, 12);

	if(!m_pDynSprite) {
		return false;
	}
	
	m_pNode = KLBNEW(CKLBNode);

	// 初期assetを設定する
    char s[256];
    sprintf(s, "asset://%s%d.png.imag", m_roleInfo->flyFrameName, 0);
	setAsset(s);

	m_pNode->setRender(m_pDynSprite);

	m_pNode->setRenderOnDestroy(true);
	getNode()->addNode(m_pNode);

	m_pNode->setTranslate(0, 0);
	m_pNode->setPriority(order);

	return true;
}

bool
CGameRole::setAsset(const char * asset)
{
	u32 handle = 0;
	CKLBImageAsset * pTex = NULL;
	if(asset) {
		pTex = (CKLBImageAsset *)CKLBUtility::loadAssetScript(asset, &handle);
	}
	bool ret;

	if(!pTex) {
		m_pDynSprite->setVICount(4,6); // Default no display.
		u16* pIdxBuff = m_pDynSprite->getSrcIndexBuffer();

		pIdxBuff[0] = 0;
		pIdxBuff[1] = 1;
		pIdxBuff[2] = 3;
		pIdxBuff[3] = 1;
		pIdxBuff[4] = 2;
		pIdxBuff[5] = 3;

		ret = false;
	} else {
		m_pDynSprite->setVICount(pTex->getVertexCount(),pTex->getIndexCount());
		u16* pIdxBuff = m_pDynSprite->getSrcIndexBuffer();
		u16* pSrcIdx  = pTex->getIndexBuffer();
		for(u32 n = 0; n < pTex->getIndexCount(); n++) {
			pIdxBuff[n] = pSrcIdx[n];
		}

		// 今回ロードしたassetを設定する
		getUV(pTex);
		getSize(pTex);

		m_pDynSprite->setTexture(pTex);

		m_pNode->markUpMatrixAndColor();
		// 以前にロードしたassetは破棄する
		if(m_pTex && m_handle) {
			CKLBDataHandler::releaseHandle(m_handle);
			KLBDELETEA(m_asset);
		}
		// 今回ロードしたassetを保持する
		m_pTex   = pTex;
		m_handle = handle;
		m_asset  = CKLBUtility::copyString(asset);
		setStrC(m_asset, (asset) ? asset : "");

		SKLBRect * pRect = pTex->getSize();

		m_org_width  = pRect->getWidth();
		m_org_height = pRect->getHeight();
		ret = true;
	}

	// 指定サイズから頂点データを生成する
	createArrVert(-1, -1);
	m_pDynSprite->mark(CKLBDynSprite::MARK_CHANGE_UV | CKLBDynSprite::MARK_CHANGE_XY | FLAG_BUFFERSHIFT);

	return ret;
}

bool
CGameRole::setMaskAsset(const char* asset) {
	// 以前にロードしたassetは破棄する
	if(m_pMaskTex && m_Maskhandle) {
		CKLBDataHandler::releaseHandle(m_Maskhandle);
	}

	u32 handle = 0;
	CKLBImageAsset * pTex = NULL;
	if(asset) {
		pTex = (CKLBImageAsset *)CKLBUtility::loadAssetScript(asset, &handle);
		if(!pTex) {
			return false;
		}
	} else {
		pTex = NULL;
	}

	if(m_pDynSprite) {
		m_pDynSprite->setMask(pTex);
	}

	// 今回ロードしたassetを保持する
	m_pMaskTex   = pTex;
	m_Maskhandle = handle;

	return true;
}

void
CGameRole::getUV(CKLBImageAsset * pTex)
{
	// klb_assert(pTex->hasStandardAttribute(CKLBImageAsset::IS_STANDARD_RECT), "Must use a standard rectangular image in VariableItem task.");
	float* pBuffUV = pTex->getUVBuffer();
	for(u32 i = 0; i < pTex->getVertexCount(); i++) {
		m_pDynSprite->setVertexUV(i, pBuffUV[i*2], pBuffUV[i*2+1]); 
	}
}

void
CGameRole::getSize(CKLBImageAsset * pTex)
{
	// klb_assert(pTex->hasStandardAttribute(CKLBImageAsset::IS_STANDARD_RECT), "Must use a standard rectangular image in VariableItem task.");
	SKLBRect * pRect = pTex->getSize();
	m_org_width = pRect->getWidth();
	m_org_height = pRect->getHeight();
}

#include "CKLBDrawTask.h"

void
CGameRole::createArrVert(float width, float height)
{
	bool originalSize = (width == -1 && height == -1);
	
	float ratioX;
	float ratioY;
	if(originalSize) {
		ratioX = 1.0f;
		ratioY = 1.0f;
	} else {
		ratioX = width / m_org_width;
		ratioY = height/ m_org_height;
	}

	if(m_pTex) {
		float* vertex = m_pTex->getXYBuffer();
		float  cx	  = m_pTex->m_iCenterX;
		float  cy	  = m_pTex->m_iCenterY;

		// Keep original vertex information alive.
		for(u32 n = 0; n < m_pTex->getVertexCount(); n++) {
			float x = (*vertex++ + cx) * ratioX;
			float y = (*vertex++ + cy) * ratioY;
			m_pDynSprite->setVertexXY(n, x , y);
			m_pDynSprite->setVertexColor(m_pNode,n,0xffffffff);
		}
	} else {
		m_pDynSprite->setVertexXY(0, 0.0f,  0.0f);
		m_pDynSprite->setVertexXY(1, width, 0.0f);
		m_pDynSprite->setVertexXY(2, width, height);
		m_pDynSprite->setVertexXY(3, 0.0f,  height);
		m_pDynSprite->setVertexColor(m_pNode,0,0xffffffff);
		m_pDynSprite->setVertexColor(m_pNode,1,0xffffffff);
		m_pDynSprite->setVertexColor(m_pNode,2,0xffffffff);
		m_pDynSprite->setVertexColor(m_pNode,3,0xffffffff);
	}
}

// 画像内におけるピクセル単位の位置とサイズの指定でUV値を設定する。
void
CGameRole::changeUV(float x, float y, float width, float height)
{

	// それぞれを「ピクセル単位による画像内の位置およびサイズ」として解釈する。
	if(m_pTex) {
		klb_assert(m_pTex->hasStandardAttribute(CKLBImageAsset::IS_STANDARD_RECT), "Must use a standard rectangular image in VariableItem task.");

		float* arrOriginalUV = m_pTex->getUVBuffer();
		float uv_x = arrOriginalUV[0];
		float uv_y = arrOriginalUV[1];
		float uv_width  = arrOriginalUV[2] - arrOriginalUV[0];
		float uv_height = arrOriginalUV[7] - arrOriginalUV[1];

		// 位置の設定
		x = uv_x + x * uv_width / m_org_width;
		y = uv_y + y * uv_height / m_org_height;

		// 幅と高さの設定
		width  = width  * uv_width  / m_org_width;
		height = height * uv_height / m_org_height;

		m_pDynSprite->setVertexUV(0, x,y);
		m_pDynSprite->setVertexUV(1, x + width, y);
		m_pDynSprite->setVertexUV(2, x + width, y + height);
		m_pDynSprite->setVertexUV(3, x , y + height);
	} else {
		// fill white texture in openGL
		m_pDynSprite->setVertexUV(0, 0.0f,0.0f);
		m_pDynSprite->setVertexUV(1, 0.0f,0.0f);
		m_pDynSprite->setVertexUV(2, 0.0f,0.0f);
		m_pDynSprite->setVertexUV(3, 0.0f,0.0f);
	}

	m_pNode->markUpMatrix();
	m_pDynSprite->mark(CKLBDynSprite::MARK_CHANGE_UV | CKLBDynSprite::MARK_CHANGE_XY | FLAG_BUFFERSHIFT);
}

void
CGameRole::dieUI()
{
	KLBDELETEA(m_asset);
	CKLBUtility::deleteNode(m_pNode, m_handle);

	if(m_pMaskTex && m_Maskhandle) {
		CKLBDataHandler::releaseHandle(m_Maskhandle);
	}
}

void
CGameRole::execute(u32 deltaT)
{
//	createArrVert(-1, -1);
//	m_pNode->markUpMatrix();
//	m_pDynSprite->mark(CKLBDynSprite::MARK_CHANGE_UV | CKLBDynSprite::MARK_CHANGE_XY | FLAG_BUFFERSHIFT);
//	RESET_A;
    m_flipTime += deltaT;
    
    
    char s[256];
    switch (m_roleState)
    {
        case ROLE_STATE_DAMAGE:
            if (m_flipTime > m_roleInfo->damageFrameInterval * 1000)
            {
                m_flipTime -= m_roleInfo->damageFrameInterval * 1000;
                if (m_roleInfo->damageFrameCount > 0)
                {
                    m_curFrame = (m_curFrame + 1) % m_roleInfo->damageFrameCount;
                    sprintf(s, "asset://%s%d.png.imag", m_roleInfo->damageFrameName, m_curFrame);
                    setAsset(s);
                }
            }
            break;
        case ROLE_STATE_FLY:
            if (m_flipTime > m_roleInfo->flyFrameInterval * 1000)
            {
                m_flipTime -= m_roleInfo->flyFrameInterval * 1000;
                if (m_roleInfo->flyFrameCount > 0)
                {
                    m_curFrame = (m_curFrame + 1) % m_roleInfo->flyFrameCount;
                    sprintf(s, "asset://%s%d.png.imag", m_roleInfo->flyFrameName, m_curFrame);
                    setAsset(s);
                }
            }
            break;
        case ROLE_STATE_DEAD:
            if (m_flipTime > m_roleInfo->dyingFrameInterval * 1000)
            {
                m_flipTime -= m_roleInfo->dyingFrameInterval * 1000;
                if (m_roleInfo->dyingFrameCount > 0)
                {
                    m_curFrame = (m_curFrame + 1) % m_roleInfo->dyingFrameCount;
                    sprintf(s, "asset://%s%d.png.imag", m_roleInfo->dyingFrameName, m_curFrame);
                    setAsset(s);
                }
            }
            break;
        default:
            break;
    }
}

int
CGameRole::commandUI(CLuaState& lua, int argc, int cmd)
{
	int ret = 1;
    char s[256];
	switch(cmd)
	{
	default:
		{
			lua.retBoolean(false);
			ret = 1;
		}
		break;
	case UI_VARITEM_CHANGE_ASSET:
		{
			bool bResult = false;
			if(argc == 3) {
				const char * asset = (lua.isNil(3)) ? NULL : lua.getString(3);
				bResult = changeAsset(asset);
			}
			lua.retBoolean(bResult);
			ret = 1;
		}
		break;
	case UI_VARITEM_SET_UV:
		{
			bool bResult = false;
			if(argc == 6) {
				float x      = lua.getFloat(3);
				float y      = lua.getFloat(4);
				float width  = lua.getFloat(5);
				float height = lua.getFloat(6);

				changeUV(x, y, width, height);

				bResult = true;
			}
			lua.retBool(bResult);
			ret = 1;
		}
		break;
	case UI_VARITEM_RESET_UV:
		{
			bool bResult = false;
			if(argc == 2) {
				getUV(this->m_pTex);
				m_pNode->markUpMatrix();
				m_pDynSprite->mark(CKLBDynSprite::MARK_CHANGE_UV | CKLBDynSprite::MARK_CHANGE_XY | FLAG_BUFFERSHIFT);
				bResult = true;
			}
			lua.retBool(bResult);
			ret = 1;
		}
		break;
	case UI_VARITEM_SET_MASK:
		{
			bool bResult = false;
			if(argc == 3) {
				const char * asset = (lua.isNil(3)) ? 0 : lua.getString(3);
				bResult = setMaskAsset(asset);
			}
			lua.retBool(bResult);
			ret = 1;
		}
            break;
        case SET_FLY_STATE:
        {
            if (m_roleState != ROLE_STATE_FLY)
            {
                m_flipTime = 0;
                m_roleState = ROLE_STATE_FLY;
                m_curFrame = 0;
                sprintf(s, "asset://%s%d.png.imag", m_roleInfo->flyFrameName, m_curFrame);
                setAsset(s);
            }
            break;
        }
        case SET_DAMAGE_STATE:
        {
            if (m_roleState != ROLE_STATE_DAMAGE)
            {
                m_flipTime = 0;
                m_roleState = ROLE_STATE_DAMAGE;
                m_curFrame = 0;
                sprintf(s, "asset://%s%d.png.imag", m_roleInfo->damageFrameName, m_curFrame);
                setAsset(s);
            }
            break;
        }
        case SET_DEAD_STATE:
        {
            if (m_roleState != ROLE_STATE_DEAD)
            {
                m_flipTime = 0;
                m_roleState = ROLE_STATE_DEAD;
                m_curFrame = 0;
                sprintf(s, "asset://%s%d.png.imag", m_roleInfo->dyingFrameName, m_curFrame);
                setAsset(s);
            }
            break;
        }
	}
	return ret;
}
