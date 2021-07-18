#pragma once
#ifdef __cplusplus
#pragma warning(disable : 26812)
#define DllExport extern "C" __declspec(dllexport)
#else //!__cplusplus
#define DllExport __declspec(dllexport)
#endif //__cplusplus

#ifdef _MSC_VER
#pragma warning(disable : 26812)
#define alignedTo(x) __declspec(align(x))
#else //!MSC_VER
#define alignedTo(x) __attribute__((aligned (x)))
#endif //MSC_VER

#define YYTK_MAGIC 'TFSI'

// Enums
enum YYTKStatus : int
{
	YYTK_OK = 0,				// The operation completed successfully.
	YYTK_FAIL = 1,				// Unspecified error occured, see source code.
	YYTK_UNAVAILABLE = 2,		// The called function is not available in the current context.
	YYTK_NO_MEMORY = 3,			// No more memory is available to the process.
	YYTK_NOT_FOUND = 4,			// The specified value could not be found.
	YYTK_NOT_IMPLEMENTED = 5,	// The specified function doesn't exist. (IPC error)
	YYTK_INVALID = 6			// One or more arguments were invalid.
};

enum b2BodyType : int
{
	b2_staticBody = 0x0,
	b2_kinematicBody = 0x1,
	b2_dynamicBody = 0x2,
};

enum EJSRetValBool : int
{
	EJSRVB_FALSE = 0x0,
	EJSRVB_TRUE = 0x1,
	EJSRVB_TYPE_ERROR = 0x2,
};

enum eGML_TYPE : unsigned int
{
	eGMLT_NONE = 0x0,
	eGMLT_ERROR = 0xFFFF0000,
	eGMLT_DOUBLE = 0x1,
	eGMLT_STRING = 0x2,
	eGMLT_INT32 = 0x4,
};

// Forward Declarations
struct YYRValue;
struct DValue;
struct CInstanceBase;
struct YYObjectBase;
struct CEvent;
struct CPhysicsDataGM;
struct CObjectGM;
struct b2Vec2;
struct b2Rot;
struct b2Transform;
struct b2Sweep;
struct b2Body;
struct CPhysicsObject;
struct CSkeletonInstance;
struct CSeqStackSnapshot;
struct CSeqTrackAudioInfo;
struct TrackAudio;
struct yyMatrix;
struct YYGMLFuncs;
struct YYRect;
struct cInstancePathAndTimeline;
struct SLink;
struct CInstance;
struct CSequence;
struct CWeakRef;
struct SLLVMVars;
struct CCode;
struct CStream;
struct VMBuffer;
struct CScript;
template <typename, typename>
struct CHashMap;
struct VMExec;

typedef void (*TRoutine)(YYRValue* Result, YYObjectBase* Self, YYObjectBase* Other, int argc, YYRValue* Args);
typedef void (*TGMLRoutine)(YYObjectBase* Self, YYObjectBase* Other);
typedef char (*PFUNC_CEXEC)(YYObjectBase* Self, YYObjectBase* Other, CCode* code, YYRValue* res, int flags);

typedef void GetOwnPropertyFunc(YYObjectBase*, YYRValue*, const char*);
typedef void DeletePropertyFunc(YYObjectBase*, YYRValue*, const char*, bool);
typedef EJSRetValBool DefineOwnPropertyFunc(YYObjectBase*, const char*, YYRValue*, bool);

typedef unsigned int uint32;
typedef int int32;
typedef float float32;
typedef unsigned __int16 uint16;
typedef const char* String;

#pragma pack(push, 4)
struct YYRValue
{
	union
	{
		void* Pointer;
		double Value;
	};

	int Flags;
	int Kind;
};
#pragma pack(pop)
using RValue = YYRValue;

// It's an RValue / YYRValue, except it's only a double
struct DValue
{
	double val;
	int dummy;
	int kind;
};

struct CInstanceBase
{
	void** vTable;
	RValue* yyvars;
};

struct YYObjectBase : CInstanceBase
{
	YYObjectBase* m_pNextObject;
	YYObjectBase* m_pPrevObject;
	YYObjectBase* m_prototype;
	void* m_pcre; // Linux Runners only
	void* m_pcreExtra; // Linux Runners only
	const char* m_class;
	GetOwnPropertyFunc* m_getOwnProperty;
	DeletePropertyFunc* m_deleteProperty;
	DefineOwnPropertyFunc* m_defineOwnProperty;
	CHashMap<int, RValue*>* m_yyvarsMap;
	CWeakRef** m_pWeakRefs;
	uint32 m_numWeakRefs;
	uint32 m_nvars;
	uint32 m_flags;
	uint32 m_capacity;
	uint32 m_visited;
	uint32 m_visitedGC;
	int32 m_GCgen;
	int32 m_GCcreationframe;
	int m_slot;
	int m_kind;
	int m_rvalueInitType;
	int m_curSlot;
};

struct alignedTo(8) CEvent
{
	CCode* e_code;
	int m_OwnerObjectID;
};

struct alignedTo(8) CPhysicsDataGM
{
	float* m_physicsVertices;
	bool m_physicsObject;
	bool m_physicsSensor;
	bool m_physicsAwake;
	bool m_physicsKinematic;
	int m_physicsShape;
	int m_physicsGroup;
	float m_physicsDensity;
	float m_physicsRestitution;
	float m_physicsLinearDamping;
	float m_physicsAngularDamping;
	float m_physicsFriction;
	int m_physicsVertexCount;
};

template <typename T>
struct SLinkedListNode
{
	SLinkedListNode<T>* m_pNext;
	SLinkedListNode<T>* m_pPrev;
	T m_pObj;
};

template <typename Element>
struct alignedTo(8) SLinkedList
{
	SLinkedListNode<Element>* m_pFirst;
	SLinkedListNode<Element>* m_pLast;
	int m_Count;
};

struct CObjectGM
{
	char* m_pName;
	CObjectGM* m_pParent;
	CHashMap<int, CObjectGM*>* m_childrenMap;
	CHashMap<unsigned long long, CEvent*>* m_eventsMap;
	CPhysicsDataGM m_physicsData;
	SLinkedList<CInstance> m_Instances;
	SLinkedList<CInstance> m_Instances_Recursive;
	uint32 m_Flags;
	int m_spriteindex;
	int m_depth;
	int m_parent;
	int m_mask;
	int m_ID;
};

struct b2Vec2
{
	float32 x;
	float32 y;
};

struct b2Rot
{
	float32 s;
	float32 c;
};

struct b2Transform
{
	b2Vec2 p;
	b2Rot q;
};

struct b2Sweep
{
	b2Vec2 localCenter;
	b2Vec2 c0;
	b2Vec2 c;
	float32 a0;
	float32 a;
	float32 alpha0;
};

struct b2Body
{
	b2BodyType m_type;
	uint16 m_flags;
	int32 m_islandIndex;
	b2Transform m_xf;
	b2Transform m_xf0;
	b2Sweep m_sweep;
	b2Vec2 m_linearVelocity;
	float32 m_angularVelocity;
	b2Vec2 m_force;
	float32 m_torque;
	void* m_world;
	b2Body* m_prev;
	b2Body* m_next;
	void* m_fixtureList;
	int32 m_fixtureCount;
	void* m_jointList;
	void* m_contactList;
	float32 m_mass;
	float32 m_invMass;
	float32 m_I;
	float32 m_invI;
	float32 m_linearDamping;
	float32 m_angularDamping;
	float32 m_gravityScale;
	float32 m_sleepTime;
	void* m_userData;
};

struct CPhysicsObject
{
	b2Body* m_pBody;
	b2Vec2 m_visualOffset;
	b2Vec2 m_previousPosition;
	int m_collisionCategory;
	unsigned int m_nextFixtureIndex;
	void* m_pFixtureMap;
};

template <typename Key, typename Value>
struct CHashMap
{
	int m_curSize;
	int m_numUsed;
	int m_curMask;
	int m_growThreshold;
	struct CElement
	{
		Value v;
		Key k;
		unsigned int Hash;
	} *m_pBuckets;

	bool CompareKeys(Key k1, Key k2)
	{
		return k1 == k2;
	}

	unsigned int CalculatePtrHash(unsigned char* k)
	{
		return k + 1;
	}

	unsigned int CalculateIntHash(int k)
	{
		return 0x9E3779B1 * k + 1;
	}
};

struct CSkeletonInstance
{
	float m_lastFrame;
	int m_lastFrameDir;
	bool m_drawCollisionData;
	bool m_forceUpdate;
	float m_skeletonScale[2];
	int m_attachmentCount;
	void** m_ppAttachments;
	void** m_ppAttachmentAtlases;
	void* m_skeleton;
	void* m_skeletonBounds;
	void* m_animation;
	void* m_animationState;
	void* m_animationStateData;
	void* m_skeletonData;
};

struct CSequenceBaseClass : YYObjectBase
{
	int changeIndex;
	int globalChangeIndex;
};

struct CSeqStackSnapshot
{
	int stacksize;
	YYObjectBase** pStack;
};

struct CSeqTrackAudioInfo
{
	int soundindex;
	int playdir;
	int emitterindex;
};

struct alignedTo(8) CTrackKeyBase : CSequenceBaseClass
{
	int channel;
};

struct alignedTo(8) CSeqTrackInstanceInfo
{
	CTrackKeyBase* pKeydata;
	int objectID;
	int instanceID;
	bool ownedBySequence;
};

struct TrackAudio
{
	int emitterIndex;
	int soundIndex;
};

struct alignedTo(8) TrackSequence
{
	CSequence* pSequence;
	int sequenceID;
};

struct yyMatrix
{
	union
	{
		float f[4][4];
		int n[4][4];
	};
};

struct TrackEval
{
	yyMatrix matrix;
	float matrixHeadPosition;
	char overridden;
	unsigned int hascreationvalue;
	unsigned int paramset;
	float X;
	float Y;
	float Rotation;
	float ScaleX;
	float ScaleY;
	float colorMultiply[4];
	float colorAdd[4];
	float XOrigin;
	float YOrigin;
	float Gain;
	float Pitch;
	float Falloff;
	float Width;
	float Height;
	float ImageIndex;
	float ImageSpeed;
	union
	{
		int spriteIndex;
		int instanceID;
	};
};

struct TrackEvalNode : CSequenceBaseClass
{
	void* track;
	TrackEval value;
	TrackEvalNode* next;
	TrackEvalNode* parent;
	TrackEvalNode* subtree;
};

struct CSequenceInstance : CSequenceBaseClass
{
	int id;
	TrackEvalNode* pEvalNodeHead;
	int sequenceID;
	float headPosition;
	float lastHeadPosition;
	float headDirection;
	float speedScale;
	float volume;
	bool paused;
	bool finished;
	bool hasPlayed;
	bool wrapped;
	int cachedElementID;
	CHashMap<CSeqStackSnapshot, CSeqTrackAudioInfo> trackAudio;
	CHashMap<CSeqStackSnapshot, CSeqTrackInstanceInfo> trackInstances;
};

struct YYRect
{
	int32 left;
	int32 top;
	int32 right;
	int32 bottom;
};

struct cInstancePathAndTimeline
{
	int i_pathindex;
	float i_pathposition;
	float i_pathpositionprevious;
	float i_pathspeed;
	float i_pathscale;
	float i_pathorientation;
	int i_pathend;
	float i_pathxstart;
	float i_pathystart;
	int i_timelineindex;
	float i_timelineprevposition;
	float i_timelineposition;
	float i_timelinespeed;
};

struct alignedTo(8) SLinkListEx
{
	SLink* head;
	SLink* tail;
	int offset;
};

struct SLink
{
	SLink* next;
	SLink* prev;
	SLinkListEx* list;
};

struct CInstance : YYObjectBase
{
	__int64 m_CreateCounter;
	CObjectGM* m_pObject;
	CPhysicsObject* m_pPhysicsObject;
	CSkeletonInstance* m_pSkeletonAnimation;
	CSequenceInstance* m_pControllingSeqInst;
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
	YYRect i_bbox;
	int i_timer[12];
	cInstancePathAndTimeline* m_pPathAndTimeline;
	CCode* i_initcode;
	CCode* i_precreatecode;
	CObjectGM* m_pOldObject;
	int m_nLayerID;
	int i_maskindex;
	__int16 m_nMouseOver;
	CInstance* m_pNext;
	CInstance* m_pPrev;
	SLink m_collisionLink;
	SLink m_dirtyLink;
	SLink m_withLink;
	float i_depth;
	float i_currentdepth;
	float i_lastImageNumber;
	unsigned int m_collisionTestNumber;
};

struct CSequence : CSequenceBaseClass
{
	int id;
	char* pDisplayName;
	int playback;
	float playbackSpeed;
	int playbackSpeedType;
	float length;
	float xorigin;
	float yorigin;
	float volume;
	bool fromWAD;
	void* pMessageEventKeyframes;
	void* pMomentEventKeyframes;
	void* pTracks;
	void* pTracksTail;
	int numEvents;
	CHashMap<int, int> pEventToFunction;
};

struct CWeakRef : YYObjectBase
{
	YYObjectBase* pWeakRef;
};

template <typename T>
struct CDynamicArray
{
	int Length;
	T** Array;
};

struct alignedTo(8) RToken
{
	int kind;
	eGML_TYPE type;
	int ind;
	int ind2;
	RValue value;
	int itemnumb;
	RToken* items;
	int position;
};

struct alignedTo(8) YYVAR
{
	const char* pName;
	int val;
};

struct alignedTo(8) SYYStackTrace
{
	SYYStackTrace* pNext;
	const char* pName;
	int line;
};

struct SLLVMVars
{
	char* pWad;
	int nWadFileLength;
	int nGlobalVariables;
	int nInstanceVariables;
	int nYYCode;
	YYVAR** ppVars;
	YYVAR** ppFuncs;
	YYGMLFuncs* pGMLFuncs;
	void* pYYStackTrace;
};

struct YYGMLFuncs
{
	const char* pName;
	TGMLRoutine pFunc;
	YYVAR* pFuncVar;
};

struct VMBuffer
{
	void** vTable;
	int m_size;
	int m_numLocalVarsUsed;
	int m_numArguments;
	char* m_pBuffer;
	void** m_pConvertedBuffer;
	char* m_pJumpBuffer;
};

struct CCode
{
	int (**_vptr$CCode)(void);
	CCode* m_pNext;
	int i_kind;
	bool i_compiled;
	String i_str;
	RToken i_token;
	RValue i_value;
	VMBuffer* i_pVM;
	VMBuffer* i_pVMDebugInfo;
	char* i_pCode;
	const char* i_pName;
	int i_CodeIndex;
	YYGMLFuncs* i_pFunc;
	bool i_watch;
	int i_offset;
	int i_locals;
	int i_args;
	int i_flags;
	YYObjectBase* i_pPrototype;
};

struct CStream
{
	bool m_ReadOnly;
	__int64 internal_buffer_size;
	__int64 internal_current_position;
	void* internal_buffer;
};

struct alignedTo(8) CScript
{
	int (**_vptr$CScript)(void);
	CStream* s_text;
	CCode* s_code;
	YYGMLFuncs* s_pFunc;
	CInstance* s_pStaticObject;
	union
	{
		String s_script;
		int s_compiledIndex;
	};
	const char* s_name;
	int s_offset;
};

struct VMExec
{
	VMExec* pPrev;
	VMExec* pNext;
	char* pStack;
	int localCount;
	YYObjectBase* pLocals;
	YYObjectBase* pSelf;
	YYObjectBase* pOther;
	CCode* pCCode;
	RValue* pArgs;
	int argumentCount;
	const char* pCode;
	char* pBP;
	VMBuffer* pBuffer;
	int line;
	const char* pName;
	VMBuffer* pDebugInfo;
	const char* pScript;
	int stackSize;
	int offs;
	int boffs;
	int retCount;
	int bufferSize;
	int prevoffs;
	void** buff;
	int* jt;
};

struct AUMIFunctionInfo
{
	int Index;
	char Name[72];
	TRoutine Function;
	int Arguments;
};