#include "obj/Dir.h"
#include "utl/Messages.h"
#include <algorithm>

namespace {
    int gPreloadIdx;
    ObjDirPtr<class ObjectDir> gPreloaded[128];

    void DeleteShared() {
        for (; gPreloadIdx > 0; gPreloadIdx--) {
            gPreloaded[gPreloadIdx - 1] = NULL;
        }
    }
}

BinStream& operator>>(BinStream& stream, InlineDirType& type) {
    unsigned char i;
    stream >> i;
    type = (InlineDirType)i;
}

void ObjectDir::Reserve(int i, int j){
    if(mHashTable.mSize < i)
        mHashTable.Resize(i, 0);
    mStringTable.Reserve(j);
}

void ObjectDir::SyncObjects() {
    std::vector<ObjectDir*> dirs;
    ObjectDir* d;
    for (int i = 0; (d = NextSubDir(i)) != NULL; i++) {
        std::vector<ObjectDir*>::iterator it = std::find(dirs.begin(), dirs.end(), d);
        if (it != dirs.end()) {
            MILO_WARN("%s: subdir %s included more than once", PathName(this), PathName(d));
            continue;
        }

        dirs.push_back(d);
    }

    HandleType(sync_objects_msg);
}

void ObjectDir::SetSubDir(bool b){
    if(b){
        mIsSubDir = true;
        SetName(0, 0);
        SetTypeDef(0);
    }
}

bool ObjectDir::IsProxy() const {
    return this != mDir;
}

SAVE_OBJ(ObjectDir, 0x1A2)

ObjectDir::ObjectDir()
    : mHashTable(0), mStringTable(0), mProxyOverride(false), mInlineProxy(true),
      mLoader(0), mIsSubDir(false), mInlineSubDirType(kInlineNever), mPathName(gNullStr),
      mCurCam(0), mAlwaysInlined(false), mAlwaysInlineHash(gNullStr) {
}

Hmx::Object* Hmx::Object::NewObject(){
    return new Hmx::Object();
}

void ObjectDir::PostSave(BinStream& bs){
    SyncObjects();
}

bool ObjectDir::AllowsInlineProxy(){
    return mInlineProxy;
}
