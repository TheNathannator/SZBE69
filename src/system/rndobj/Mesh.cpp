#include "Mesh.h"
#include "decomp.h"
#include "math/strips/Striper.h"
#include "obj/ObjPtr_p.h"
#include "obj/Object.h"
#include "os/Debug.h"
#include "rndobj/MultiMesh.h"
#include "rndobj/Trans.h"
#include "rndobj/Utl.h"
#include "utl/BufStream.h"
#include "utl/Symbols.h"
#include "utl/Symbols4.h"
#include <vector>

INIT_REVS(RndMesh)

int RndMesh::MaxBones() { return MAX_BONES; }

// DECOMP_FORCEFUNC(Mesh, ObjVector<RndBone>, resize(3)) TODO figure out why this is a nothing burger

bool RndMesh::IsSkinned() const { return mBones.size(); }

void RndMesh::SetMat(RndMat* m) { mMat = m; }

void RndMesh::SetGeomOwner(RndMesh* m) {MILO_ASSERT(m, 487); mOwner = m;}

void RndMesh::ScaleBones(float f) {
    for (std::vector<RndBone>::iterator it = mBones.begin(); it != mBones.end(); it++) {
        it->mOffset.v *= f;
    }
}

RndMesh::RndMesh() : mMat(this, NULL), mOwner(this, this), mBones(this),
    unk_0xF0(0), unk_0xF4(1), unk_0xF8(0), unk_0xFC(0), unk_0x108(0), unk_0x10C(0), unk_0x110(0), unk_0x114(0),
    unk_0x118(0), unk_0x11C(0) {
    unk9p0 = false;
    mKeepMeshData = false;
    unk9p2 = true;
    unk9p3 = false;
}

RndMesh::~RndMesh() { 
    delete unk_0x11C; unk_0x11C = NULL; 
    delete unk_0xF8; unk_0xF8 = NULL; 
    delete unk_0xFC; unk_0xFC = NULL; ClearCompressedVerts();
    
}

RndMesh::Vert::Vert() : pos(0,0,0), norm(0,1.0f,0), boneWeights(),
    color(), uv(0,0) {
    for(int i = 0; i < 4; i++) boneIndices[i] = 0;
}

void RndMesh::PreLoadVertices(BinStream& bs) {
    if (gAltRev > 4) {
        unk_0x11C = new FileLoader("", "", kLoadFront, 0, true, true, &bs);
    }
}

void RndMesh::PostLoadVertices(BinStream& bs) {
    void* buf; int len;
    if (unk_0x11C) {
        buf = (void*)unk_0x11C->GetBuffer(NULL);
        len = unk_0x11C->GetSize();
        delete unk_0x11C;
        unk_0x11C = NULL;
    }
    BufStream bfs(buf, len, true);
    if (buf) bs = bfs;
}

RndMultiMesh* RndMesh::CreateMultiMesh() {
    RndMesh* m = &(*mOwner);
    if (!m->unk_0xFC) {
        m->unk_0xFC = Hmx::Object::New<RndMultiMesh>();
        m->unk_0xFC->SetMesh(m);
    }
    m->unk_0xFC->mInstances.resize(0);
    return m->unk_0xFC;
}

BinStream& operator>>(BinStream& bs, STRIPERRESULT& sr) {
    bs >> sr.NbStrips;
    int runs; bs >> runs;
    sr.AllocLengthsAndRuns(sr.NbStrips, runs);
    bs.Read(sr.StripLengths, sr.NbStrips * 4);
    bs.Read(sr.StripRuns, runs * 2);

    return bs;
}

bool RndMesh::CacheStrips(BinStream& bs) {
    bool ret = false;
    if (bs.Cached() && bs.GetPlatform() == kPlatformWii && mOwner.mPtr == this && mFaces.size() != 0
        && mVerts.size() != 0 && !(unk_0xF0 & 0x20)) ret = true;
    return ret;
}

void RndMesh::CreateStrip(int i, int j, Striper& striper, STRIPERRESULT& sr, bool onesided) {
    STRIPERCREATE sc;
    sc.WFaces = &mFaces[i].idx0;
    sc.NbFaces = j;
    sc.ConnectAllStrips = false;
    sc.OneSided = onesided;
    sc.SGIAlgorithm = false;
    MILO_ASSERT(striper.Init(sc), 1115);
    MILO_ASSERT(striper.Compute(sr), 1116);
    for (int i = 1; i < sr.NbStrips; i++) {
        sr.NbStrips += sr.StripLengths[i];
    }
}

SAVE_OBJ(RndMesh, 1135)

void RndMesh::Load(BinStream& bs) {
    PreLoad(bs);
    PostLoad(bs);
}

void RndMesh::PreLoad(BinStream& bs) {
    uint dump, asdf, fdsa, uuuuu;
    char blag[128];
    LOAD_REVS(bs)
    ASSERT_REVS(38, 6)
    if (gRev > 25) Hmx::Object::Load(bs);
    RndTransformable::Load(bs);
    RndDrawable::Load(bs);
    if (gRev < 15) {
        ObjPtrList<Hmx::Object, class ObjectDir> l(this, kObjListNoNull);
        bs >> dump >> l;
    }
    if (gRev < 20) {
        bs >> asdf >> fdsa;
    }
    if (gRev < 3) {
        bs >> uuuuu;
    }
    bs >> mMat;
    if (gRev == 27) {
        bs.ReadString(blag, 128); // allegedly this is for a second, funnier mat
        
        bs >> mOwner;
    }

    PreLoadVertices(bs);
}

#pragma push
#pragma dont_inline on
void RndMesh::PostLoad(BinStream& bs) {
    PostLoadVertices(bs);
    bs >> mFaces;
    if ((ushort)(gRev + 0xFFFB) <= 18) {
        int i; u16 a,b;
        for (bs >> i; i != 0; i--) {
            bs >> a >> b;
        }
    }
    if (gRev > 23) bs >> unk_0xD0;
    else {
        if (gRev > 21) {
            unk_0xD0.clear();
            int i; 
            for (bs >> i; i != 0; i--) {
                int x; std::vector<u16> v; std::vector<uint> v2;
                bs >> x >> v >> v2;
                u8 y = x; unk_0xD0.push_back(y);
            }
        } else {
            if (gRev > 16) bs >> unk_0xD0;
        }
    }
    if (gRev > 28) {
        bs >> mBones;
        if (mBones.size() > MaxBones()) mBones.resize(MaxBones());
    } else {
        if (gRev > 13) {
            ObjPtr<RndTransformable, class ObjectDir> t(this, NULL);
            bs >> t;
            if ((RndTransformable*)t) {
                mBones.resize(4);
                if (gRev > 22) {
                    (ObjPtr<RndTransformable, class ObjectDir>&)mBones[0] = t;
                    bs >> mBones[1] >> mBones[2] >> mBones[3];
                    bs >> mBones[0].mOffset >> mBones[1].mOffset >> mBones[2].mOffset >> mBones[3].mOffset;
                    if (gRev < 25) { // incoming headache
                        for (Vert* it = mVerts.begin(); it != mVerts.end(); it++) {
                            // it->why.Set(1 - it->why.GetX() - it->why.GetY() - it->why.GetZ(), 
                            //     it->why.GetX(), it->why.GetY(), it->why.GetZ());
                        }
                    }
                } else {
                    if (TransConstraint() == kParentWorld) 
                        (ObjPtr<RndTransformable, class ObjectDir>&)mBones[0] = TransParent(); 
                    else (ObjPtr<RndTransformable, class ObjectDir>&)mBones[0] = this;
                    mBones[0].mOffset.Reset();
                    (ObjPtr<RndTransformable, class ObjectDir>&)mBones[1] = t;
                    bs >> mBones[2];
                    bs >> mBones[1].mOffset >> mBones[2].mOffset;
                    (ObjPtr<RndTransformable, class ObjectDir>&)mBones[3] = NULL;
                }
            }
            mBones.clear();
        }
    }
    RemoveInvalidBones();


    for (Vert* it = mVerts.begin(); it != mVerts.end(); it++) {
                            
    }
    if (gRev > 37) { bool b; bs >> b; unk9p0 = b;}
    if (gAltRev > 1) { bool b; bs >> b; unk9p3 = b;}
    if (gAltRev > 3) { bool b; bs >> b;}
    Sync(191);
    if (gAltRev >= 3 || NumBones() > 1) MILO_WARN("%s", PathName(this));
}

DECOMP_FORCEBLOCK(Mesh, (Hmx::Color32* c), { c->Clear(); c->fr(); c->fg(); c->fb(); c->fa(); })
DECOMP_FORCEFUNC(Mesh, RndMesh, NumBones())
#pragma pop

BinStream& operator>>(BinStream& bs, RndMesh::Vert& v) {
    bs >> v.pos;
    if (RndMesh::gRev != 10 && RndMesh::gRev < 23) { int a,b; bs >> a >> b; }
    bs >> v.norm;


    if (RndMesh::gRev < 20) { int a,b; bs >> b >> a; }
    if (RndMesh::gRev > 28) bs >> v.boneIndices[0] >> v.boneIndices[1] >> v.boneIndices[2] >> v.boneIndices[3];
    if (RndMesh::gRev > 29) {
        int a,b,c,d;
        bs >> d >> c >> b >> a;
    }
    return bs;
}

TextStream& operator<<(TextStream& ts, RndMesh::Volume v) {
    if (v == RndMesh::kVolumeEmpty) ts << "kVolumeEmpty";
    else if (v == RndMesh::kVolumeTriangles) ts << "kVolumeTriangles";
    else if (v == RndMesh::kVolumeBSP) ts << "kVolumeBSP";
    else if (v == RndMesh::kVolumeBox) ts << "kVolumeBox";
    return ts;
}

int RndMesh::NumFaces() const { return mFaces.size(); }
int RndMesh::NumVerts() const { return mVerts.size(); }

BinStream& operator>>(BinStream& bs, RndMesh::Face& f) {
    bs >> f.idx0 >> f.idx1 >> f.idx2;
    if (RndMesh::gRev < 1) {
        int x, y, z;
        bs >> z >> y >> x;
    }
    return bs;
}

void RndMesh::Sync(int i) {
    OnSync(mKeepMeshData ? i | 0x200 : i);
}

void RndMesh::ClearCompressedVerts() {
    delete unk_0x114;
    unk_0x114 = NULL;
    unk_0x118 = 0;
}

#pragma dont_inline on
BEGIN_HANDLERS(RndMesh)
    HANDLE(compare_edge_verts, OnCompareEdgeVerts)
    HANDLE(attach_mesh, OnAttachMesh)
    HANDLE(get_face, OnGetFace)
    HANDLE(set_face, OnSetFace)
    HANDLE(get_vert_pos, OnGetVertXYZ)
    HANDLE(set_vert_pos, OnSetVertXYZ)
    HANDLE(get_vert_norm, OnGetVertNorm)
    HANDLE(set_vert_norm, OnSetVertNorm)
    HANDLE(get_vert_uv, OnGetVertUV)
    HANDLE(set_vert_uv, OnSetVertUV)
    HANDLE_EXPR(num_bones, NumBones())
    HANDLE(unitize_normals, OnUnitizeNormals)
    HANDLE(point_collide, OnPointCollide)
    HANDLE(configure_mesh, OnConfigureMesh)
#ifdef MILO_DEBUG
    HANDLE_EXPR(estimated_size_kb, EstimatedSizeKb())
#endif
    HANDLE_ACTION(clear_bones, CopyBones(NULL))
    HANDLE_ACTION(copy_geom_from_owner, CopyGeometryFromOwner())
    HANDLE_SUPERCLASS(RndDrawable)
    HANDLE_SUPERCLASS(RndTransformable)
    HANDLE_SUPERCLASS(Hmx::Object)
    HANDLE_CHECK(2306)
END_HANDLERS
#pragma dont_inline reset

DataNode RndMesh::OnAttachMesh(const DataArray* da) {
    RndMesh* m = da->Obj<RndMesh>(2);
    AttachMesh(this, m);
    delete m;
    return DataNode();
}

DataNode RndMesh::OnGetVertNorm(const DataArray* da) {
    Vert* v;
    s32 index = da->Int(2);
    MILO_ASSERT(index >= 0 && index < mVerts.size(), 2446);
    v = mVerts[index];
    *da->Var(3) = DataNode(v->norm.x);
    *da->Var(4) = DataNode(v->norm.y);
    *da->Var(5) = DataNode(v->norm.z);
    return DataNode();
}

DataNode RndMesh::OnSetVertNorm(const DataArray* da) {
    Vert* v;
    s32 index = da->Int(2);
    MILO_ASSERT(index >= 0 && index < mVerts.size(), 2457);
    v = mVerts[index];
    v->norm.x = da->Float(3);
    v->norm.y = da->Float(4);
    v->norm.z = da->Float(5);
    Sync(31);
    return DataNode();
}

DataNode RndMesh::OnGetVertXYZ(const DataArray* da) {
    Vert* v;
    s32 index = da->Int(2);
    MILO_ASSERT(index >= 0 && index < mVerts.size(), 2469);
    v = mVerts[index];
    *da->Var(3) = DataNode(v->pos.x);
    *da->Var(4) = DataNode(v->pos.y);
    *da->Var(5) = DataNode(v->pos.z);
    return DataNode();
}

DataNode RndMesh::OnSetVertXYZ(const DataArray* da) {
    Vert* v;
    s32 index = da->Int(2);
    MILO_ASSERT(index >= 0 && index < mVerts.size(), 2480);
    v = mVerts[index];
    v->pos.x = da->Float(3);
    v->pos.y = da->Float(4);
    v->pos.z = da->Float(5);
    Sync(31);
    return DataNode();
}

DataNode RndMesh::OnGetVertUV(const DataArray* da) {
    Vert* v;
    s32 index = da->Int(2);
    MILO_ASSERT(index >= 0 && index < mVerts.size(), 2492);
    v = mVerts[index];
    *da->Var(3) = DataNode(v->uv.x);
    *da->Var(4) = DataNode(v->uv.y);
    return DataNode();
}

DataNode RndMesh::OnSetVertUV(const DataArray* da) {
    Vert* v;
    s32 index = da->Int(2);
    MILO_ASSERT(index >= 0 && index < mVerts.size(), 2502);
    v = mVerts[index];
    v->uv.x = da->Float(3);
    v->uv.y = da->Float(4);
    Sync(31);
    return DataNode();
}

DataNode RndMesh::OnGetFace(const DataArray* da) {
    Face* f;
    int index = da->Int(2);
    MILO_ASSERT(index >= 0 && index < mFaces.size(), 2513);
    f = &mFaces[index];
    *da->Var(3) = DataNode(f->idx0);
    *da->Var(4) = DataNode(f->idx1);
    *da->Var(5) = DataNode(f->idx2);
    return DataNode();
}

DataNode RndMesh::OnSetFace(const DataArray* da) {
    Face* f;
    int index = da->Int(2);
    MILO_ASSERT(index >= 0 && index < mFaces.size(), 2524);
    f = &mFaces[index];
    f->idx0 = da->Int(3); f->idx1 = da->Int(4); f->idx2 = da->Int(5);
    Sync(32);
    return DataNode();
}

bool PropSync(RndBone& b, DataNode& _val, DataArray* _prop, int _i, PropOp _op) {
    if(_i == _prop->Size()) return true; \
    else { \
        Symbol sym = _prop->Sym(_i);
        SYNC_PROP(bone, (ObjPtr<RndTransformable,class ObjectDir>&)b);
        SYNC_PROP(offset, b.mOffset)
        return false;
    }
}

BEGIN_PROPSYNCS(RndMesh)
    SYNC_PROP(mat, mMat)
    SYNC_PROP(geom_owner, mOwner)


END_PROPSYNCS
