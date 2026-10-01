#include <Strategic/HitActor.hpp>
#include <math.h>

static inline f32 approxSqrt(f32 x)
{
	char trash[0x20];
	(void)trash;
	volatile f32 f = x * __frsqrte(x);
	char trash2[8];
	(void)trash2;
	return f;
}

f32 THitActor::calcEntryRadius()
{
	f32 rad;
	if (mAttackRadius > mDamageRadius)
		rad = mAttackRadius;
	else
		rad = mDamageRadius;

	f32 height;
	if (mAttackHeight > mDamageHeight)
		height = mAttackHeight;
	else
		height = mDamageHeight;

	f32 height2 = height * height;
	rad = rad * rad + height2;

	if (rad > 0.0f) {
		f32 f = approxSqrt(rad);
		mEntryRadius = 1.4142135f * f;
		return f;
	}

	mEntryRadius = 0.0f;
	return height2;
}

void THitActor::perform(u32 cue, JDrama::TGraphics* graphics)
{
	JDrama::TActor::perform(cue, graphics);
}

f32 THitActor::initHitActor(u32 actor_type, u16 max_collisions, int hit_flags,
                            f32 attack_radius, f32 attack_height,
                            f32 damage_radius, f32 damage_height)
{
	mActorType   = actor_type;
	mColCapacity = max_collisions;
	mCollisions  = new THitActor*[mColCapacity];

	for (int i = 0; i < mColCapacity; ++i)
		mCollisions[i] = nullptr;

	onHitFlag(hit_flags);

	mAttackRadius = attack_radius;
	mAttackHeight = attack_height;
	mDamageRadius = damage_radius;
	mDamageHeight = damage_height;

	return calcEntryRadius();
}

THitActor::THitActor(const char* name)
    : JDrama::TActor(name)
    , mCollisions(nullptr)
    , mColCount(0)
    , mColCapacity(0)
    , mActorType(0)
    , mAttackRadius(0.0f)
    , mAttackHeight(0.0f)
    , mDamageRadius(0.0f)
    , mDamageHeight(0.0f)
    , mEntryRadius(0.0f)
    , mHitFlags(0)
{
}
