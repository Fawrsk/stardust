/** 
* @file llpathfindingobject.cpp
* @brief Implementation of llpathfindingobject
* @author Stinson@lindenlab.com
*
* $LicenseInfo:firstyear=2012&license=viewerlgpl$
* Second Life Viewer Source Code
* Copyright (C) 2012, Linden Research, Inc.
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation;
* version 2.1 of the License only.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*
* Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
* $/LicenseInfo$
*/


#include "llviewerprecompiledheaders.h"

#include "llpathfindingobject.h"

#include <string>

#include "v3math.h"
#include "llavatarname.h"
#include "llavatarnamecache.h"
#include "llsd.h"
#include "lluuid.h"

#define PATHFINDING_OBJECT_NAME_FIELD        "name"
#define PATHFINDING_OBJECT_DESCRIPTION_FIELD "description"
#define PATHFINDING_OBJECT_OWNER_FIELD       "owner"
#define PATHFINDING_OBJECT_POSITION_FIELD    "position"

//---------------------------------------------------------------------------
// LLPathfindingObject
//---------------------------------------------------------------------------

LLPathfindingObject::LLPathfindingObject()
	: mUUID(),
	mName(),
	mDescription(),
	mOwnerUUID(),
	mHasOwnerName(false),
	mOwnerName(),
	mLocation()
{
}

LLPathfindingObject::LLPathfindingObject(const std::string &pUUID, const LLSD &pObjectData)
	: mUUID(pUUID),
	mName(),
	mDescription(),
	mOwnerUUID(),
	mHasOwnerName(false),
	mOwnerName(),
	mLocation()
{
	parseObjectData(pObjectData);
}

LLPathfindingObject::LLPathfindingObject(const LLPathfindingObject& pOther)
	: mUUID(pOther.mUUID),
	mName(pOther.mName),
	mDescription(pOther.mDescription),
	mOwnerUUID(pOther.mOwnerUUID),
	mHasOwnerName(false),
	mOwnerName(),
	mLocation(pOther.mLocation)
{
	fetchOwnerName();
}

LLPathfindingObject::~LLPathfindingObject()
{
}

LLPathfindingObject &LLPathfindingObject::operator =(const LLPathfindingObject& pOther)
{
	mUUID = pOther.mUUID;
	mName = pOther.mName;
	mDescription = pOther.mDescription;
	mOwnerUUID = pOther.mOwnerUUID;
	fetchOwnerName();
	mLocation = pOther.mLocation;

	return *this;
}

std::string LLPathfindingObject::getOwnerName() const
{
	std::string ownerName;

	if (hasOwner())
	{
		ownerName = mOwnerName.getCompleteName();
	}

	return ownerName;
}

void LLPathfindingObject::parseObjectData(const LLSD &pObjectData)
{
	llassert(pObjectData.has(PATHFINDING_OBJECT_NAME_FIELD));
	llassert(pObjectData.get(PATHFINDING_OBJECT_NAME_FIELD).isString());
	mName = pObjectData.get(PATHFINDING_OBJECT_NAME_FIELD).asString();
	
	llassert(pObjectData.has(PATHFINDING_OBJECT_DESCRIPTION_FIELD));
	llassert(pObjectData.get(PATHFINDING_OBJECT_DESCRIPTION_FIELD).isString());
	mDescription = pObjectData.get(PATHFINDING_OBJECT_DESCRIPTION_FIELD).asString();

#ifdef SERVER_SIDE_OWNER_ROLLOUT_COMPLETE
	llassert(pObjectData.has(PATHFINDING_OBJECT_OWNER_FIELD));
	llassert(pObjectData.get(PATHFINDING_OBJECT_OWNER_FIELD).isUUID());
	mOwnerUUID = pObjectData.get(PATHFINDING_OBJECT_OWNER_FIELD).asUUID();
	LLAvatarNameCache::get(mOwnerUUID, &mOwnerName);
#else // SERVER_SIDE_OWNER_ROLLOUT_COMPLETE
	if (pObjectData.has(PATHFINDING_OBJECT_OWNER_FIELD))
	{
		llassert(pObjectData.get(PATHFINDING_OBJECT_OWNER_FIELD).isUUID());
		mOwnerUUID = pObjectData.get(PATHFINDING_OBJECT_OWNER_FIELD).asUUID();
		fetchOwnerName();
	}
#endif // SERVER_SIDE_OWNER_ROLLOUT_COMPLETE

	llassert(pObjectData.has(PATHFINDING_OBJECT_POSITION_FIELD));
	llassert(pObjectData.get(PATHFINDING_OBJECT_POSITION_FIELD).isArray());
	mLocation.setValue(pObjectData.get(PATHFINDING_OBJECT_POSITION_FIELD));
}

void LLPathfindingObject::fetchOwnerName()
{
	mHasOwnerName = false;
	if (hasOwner())
	{
		LLAvatarNameCache::get(mOwnerUUID, boost::bind(&LLPathfindingObject::handleAvatarNameFetch, this, _1, _2));
	}
}

void LLPathfindingObject::handleAvatarNameFetch(const LLUUID &pOwnerUUID, const LLAvatarName &pAvatarName)
{
	llassert(mOwnerUUID == pOwnerUUID);
	mOwnerName = pAvatarName;
	mHasOwnerName = true;
}