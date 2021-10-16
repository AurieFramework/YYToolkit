#pragma once
struct RValue;
struct CWeakRef;
struct CCode;

template <typename Key, typename Value>
struct CHashMap;

struct CInstanceBase
{
	int (**_vptr$CInstanceBase)(void);
	RValue* yyvars;
};

struct YYObjectBase : CInstanceBase
{
	YYObjectBase* m_pNextObject;
	YYObjectBase* m_pPrevObject;
	YYObjectBase* m_prototype;
	void* m_pcre;
	void* m_pcreExtra;
	const char* m_class;
	void* m_getOwnProperty;
	void* m_deleteProperty;
	void* m_defineOwnProperty;
	CHashMap<int, RValue*>* m_yyvarsMap;
	CWeakRef** m_pWeakRefs;
	unsigned int m_numWeakRefs;
	unsigned int m_nvars;
	unsigned int m_flags;
	unsigned int m_capacity;
	unsigned int m_visited;
	unsigned int m_visitedGC;
	int m_GCgen;
	int m_GCcreationframe;
	int m_slot;
	int m_kind;
	int m_rvalueInitType;
	int m_curSlot;
};

struct CWeakRef : YYObjectBase
{
	YYObjectBase* pWeakRef;
};

struct CInstance : YYObjectBase
{
	__int64 m_CreateCounter;
	void* m_pObject;
	void* m_pPhysicsObject;
	void* m_pSkeletonAnimation;
	void* m_pControllingSeqInst;
	unsigned int m_Instflags;
	int i_id;
	int i_objectindex;
	int i_spriteindex;
	float i_sequencePos;
	float i_lastSequencePos;
	float i_sequenceDir;
	float i_imageindex;
	float i_imagespeed;
	float i_imagescalex;
	float i_imagescaley;
	float i_imageangle;
	float i_imagealpha;
	unsigned int i_imageblend;
	float i_x;
	float i_y;
	float i_xstart;
	float i_ystart;
	float i_xprevious;
	float i_yprevious;
	float i_direction;
	float i_speed;
	float i_friction;
	float i_gravitydir;
	float i_gravity;
	float i_hspeed;
	float i_vspeed;
	int i_bbox[4];
	int i_timer[12];
	void* m_pPathAndTimeline;
	CCode* i_initcode;
	CCode* i_precreatecode;
	void* m_pOldObject;
	int m_nLayerID;
	int i_maskindex;
	__int16 m_nMouseOver;
	CInstance* m_pNext;
	CInstance* m_pPrev;
	void* m_collisionLink[3]; // SLink
	void* m_dirtyLink[3]; // SLink
	void* m_withLink[3]; // SLink
	float i_depth;
	float i_currentdepth;
	float i_lastImageNumber;
	unsigned int m_collisionTestNumber;
};
